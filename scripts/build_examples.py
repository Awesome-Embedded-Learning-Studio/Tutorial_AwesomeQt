#!/usr/bin/env python3
"""批量编译 examples/ + 实例库根 的 Qt6 CMake 工程，保证「克隆即跑」不退化。

发现策略：
- examples/ 下只编译**叶子工程**（CMakeLists 不含 add_subdirectory）。每个叶子按
  example_style 规范自洽（自带 AUTOMOC + find_package），可独立 `cmake -B build`。
  这天然跳过所有聚合器（beginner 顶层根、各模块聚合器、05-other-modules 聚合器）。
- widget/app/model/industrial 是 root-owns-config，编译各自的**根 CMakeLists**。
- S3 砍掉的 8 个小众模块示例（NFC/SCXML/Quick3D-Physics/WebChannel/WebEngine/
  RemoteObjects/SpatialAudio/TextToSpeech）从发现中排除——CI 不装这些 Qt6 模块。

用法：
  python3 scripts/build_examples.py                 # 全量
  python3 scripts/build_examples.py --root examples/beginner   # 分层放量
  python3 scripts/build_examples.py --dry-run       # 只列出编译单元，不编译
"""
import argparse
import shutil
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
EXAMPLES_ROOT = REPO_ROOT / "examples"
LIB_ROOTS = ["widget", "app", "model", "industrial"]

# S3 砍掉的 8 个小众模块（目录名片段匹配，大小写不敏感）——CI 不装，从发现中排除
S3_EXCLUDE_FRAGMENTS = (
    "qtnfc", "qtscxml", "qtquick3d-physics", "qtwebchannel",
    "qtwebengine", "qtremoteobjects", "qtspatial-audio", "qttexttospeech",
)

RED, GREEN, YELLOW, CYAN, RESET = "\033[31m", "\033[32m", "\033[33m", "\033[36m", "\033[0m"


def is_leaf_cmake(cmake_path: Path) -> bool:
    text = cmake_path.read_text(encoding="utf-8", errors="replace")
    return "add_subdirectory" not in text


def is_s3_excluded(unit_dir: Path) -> bool:
    name = unit_dir.name.lower()
    return any(frag in name for frag in S3_EXCLUDE_FRAGMENTS)


def discover_build_units() -> list[Path]:
    units: list[Path] = []
    if EXAMPLES_ROOT.is_dir():
        for cmake in sorted(EXAMPLES_ROOT.rglob("CMakeLists.txt")):
            lower_parts = {p.lower() for p in cmake.parts}
            if lower_parts & {"build", "_build_ci", ".cache", "node_modules"}:
                continue
            if not is_leaf_cmake(cmake):
                continue
            if is_s3_excluded(cmake.parent):
                continue
            units.append(cmake.parent)
    for lib in LIB_ROOTS:
        root_cmake = REPO_ROOT / lib / "CMakeLists.txt"
        if root_cmake.exists() and not is_s3_excluded(root_cmake.parent):
            units.append(root_cmake.parent)
    return units


def build_one(unit: Path, use_ccache: bool) -> tuple[Path, bool, str, float]:
    build_dir = unit / "_build_ci"
    if build_dir.exists():
        shutil.rmtree(build_dir)
    configure = ["cmake", "-S", str(unit), "-B", str(build_dir), "-G", "Ninja"]
    if use_ccache:
        configure += ["-DCMAKE_CXX_COMPILER_LAUNCHER=ccache"]
    t0 = time.time()
    cfg = subprocess.run(configure, capture_output=True, text=True)
    if cfg.returncode != 0:
        return unit, False, f"configure failed:\n{cfg.stderr[-1500:]}", time.time() - t0
    bld = subprocess.run(["cmake", "--build", str(build_dir)], capture_output=True, text=True)
    if bld.returncode != 0:
        return unit, False, f"build failed:\n{bld.stderr[-1500:]}", time.time() - t0
    # 清理产物（_build_ci 已在 .gitignore）
    shutil.rmtree(build_dir, ignore_errors=True)
    return unit, True, "", time.time() - t0


def main():
    ap = argparse.ArgumentParser(description="批量编译 examples/ + 实例库 Qt6 CMake 工程")
    ap.add_argument("--workers", type=int, default=8, help="并行编译数（默认 8）")
    ap.add_argument("--root", help="只编译匹配此前缀的工程（相对仓库根，分层放量用）")
    ap.add_argument("--no-ccache", action="store_true", help="禁用 ccache")
    ap.add_argument("--dry-run", action="store_true", help="只列出编译单元，不编译")
    args = ap.parse_args()

    units = discover_build_units()
    if args.root:
        units = [u for u in units if str(u.relative_to(REPO_ROOT)).startswith(args.root)]

    print(f"{CYAN}build_examples{RESET}: 发现 {GREEN}{len(units)}{RESET} 个编译单元")
    if args.dry_run:
        for u in units:
            print(f"  · {u.relative_to(REPO_ROOT)}")
        # 顺带统计被 S3 排除的
        return

    if not units:
        print(f"{YELLOW}  无可编译工程{RESET}")
        sys.exit(0)

    use_ccache = not args.no_ccache and shutil.which("ccache") is not None
    if use_ccache:
        print(f"  ccache: {GREEN}on{RESET} ({shutil.which('ccache')})")
    else:
        print(f"  {YELLOW}ccache: off（未安装或 --no-ccache）{RESET}")

    ok, failed = 0, []
    t0 = time.time()
    with ThreadPoolExecutor(max_workers=args.workers) as pool:
        futures = {pool.submit(build_one, u, use_ccache): u for u in units}
        for i, fut in enumerate(as_completed(futures), 1):
            unit, success, err, elapsed = fut.result()
            rel = unit.relative_to(REPO_ROOT)
            if success:
                ok += 1
                print(f"  {GREEN}✓{RESET} [{i}/{len(units)}] {rel} ({elapsed:.1f}s)")
            else:
                failed.append((unit, err))
                print(f"  {RED}✗{RESET} [{i}/{len(units)}] {rel}")

    total_t = time.time() - t0
    print(f"\n{CYAN}{'=' * 50}{RESET}")
    print(f"  总数   : {len(units)}")
    print(f"  {GREEN}通过   : {ok}{RESET}")
    print(f"  {RED}失败   : {len(failed)}{RESET}")
    print(f"  耗时   : {total_t:.1f}s")

    if failed:
        print(f"\n{RED}失败详情：{RESET}")
        for unit, err in failed:
            print(f"  {RED}✗{RESET} {unit.relative_to(REPO_ROOT)}")
            for line in err.strip().splitlines()[:12]:
                print(f"      {line}")
        sys.exit(1)
    print(f"\n{GREEN}全部编译通过！{RESET}")
    sys.exit(0)


if __name__ == "__main__":
    main()
