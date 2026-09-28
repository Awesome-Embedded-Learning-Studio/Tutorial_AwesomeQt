#!/usr/bin/env python3
"""preflight —— base 卷自然化改版的单篇飞行检查。

对一篇教程 Markdown 做三件事：
1. 盘对应示例工程（examples/beginner/<所在章目录>/<slug>/）：
   - 构建必需件：CMakeLists.txt + main.cpp——缺即 blocked（cmake 直跑无从谈起）；
   - 五件套欠账件（不阻断，记进报告供 W-EX 补齐）：.gitignore、widget 实现
     .h/.cpp 对（任意命名；全仓 134 个示例无一用字面 widget.h/widget.cpp，
     AGENTS.md 五件套命名是新示例模板，存量按类目盘点、债归 W-EX）；
2. 必需件齐则真跑 ``cmake -B build && cmake --build build``（子进程，每段超时
   300s）。Qt 未装 / cmake 失败 / 超时 → exit 2，账本经 ledger.transition 记
   blocked + block_reason（attempts+1），绝不 traceback 崩；
3. 构建成功才摘邻篇 digest（同目录按文件名序取前一篇与后一篇的
   H2 列表 + 首段前 120 字），落 renatural/state/preflight/<slug>.json。

状态文件契约见 renatural/state/（queue.json / ledger.json / history.jsonl），
IO 一律复用同目录 ledger 模块（原子写 + history 留痕 + RENATURAL_STATE 沙箱），
本脚本零第三方依赖，纯 python3 标准库。篇目必须已入账（plan.py 建队），未入账
直接报错——不许静默补建账目打脱 queue/ledger 一致性。
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ledger  # noqa: E402  同目录共享账本模块

REPO = ledger.REPO_ROOT
STATE = ledger.STATE_DIR
PREFLIGHT_DIR = ledger.PREFLIGHT_DIR

#: 构建必需件：缺了 cmake 直跑无从谈起，直接 blocked
BUILD_REQUIRED = ("CMakeLists.txt", "main.cpp")
#: 五件套欠账件：不阻断飞行检查，记进报告与 stdout 欠账行，补齐归 W-EX 工作包
DEBT_ITEMS = (".gitignore", "widget_impl_pair")
BUILD_TIMEOUT = 300  # 秒，configure 与 build 各自上限


def now_iso() -> str:
    return ledger.now_iso()


def resolve_article(raw: str) -> Path:
    """接受绝对路径 / 仓库相对路径 / 相对当前目录路径，返回绝对路径。"""
    p = Path(raw).expanduser()
    if p.is_absolute():
        return p
    if p.exists():
        return p.resolve()
    alt = REPO / p
    if alt.exists():
        return alt.resolve()
    return p


def rel_key(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def article_slug(md: Path) -> str:
    """slug = 文件名去扩展名；index.md 以 <章目录>-index 限定（与 ledger.slug 同口径）。"""
    if md.stem == "index":
        return f"{md.parent.name or 'beginner'}-index"
    return md.stem


def example_state(exdir: Path) -> dict:
    """示例工程盘点：构建必需件 + 欠账件，一次算齐进报告。"""
    has = lambda name: (exdir / name).is_file()  # noqa: E731
    heads = sorted(p.stem for p in exdir.glob("*.h"))
    widget_pair = (
        any((exdir / f"{h}.cpp").is_file() for h in heads)
        or (has("widget.h") and has("widget.cpp"))
    )
    return {
        "CMakeLists.txt": has("CMakeLists.txt"),
        "main.cpp": has("main.cpp"),
        ".gitignore": has(".gitignore"),
        "widget_impl_pair": widget_pair,
    }


def extract_digest(text: str, rel: str) -> dict:
    """H2 列表 + 首个正文段的前 120 字。跳过 frontmatter / 标题 / 代码块。"""
    h2 = [m.group(1).strip() for m in re.finditer(r"(?m)^##\s+(.+?)\s*$", text)]
    lines = text.splitlines()
    i = 0
    if lines and lines[0].strip() == "---":
        i = 1
        while i < len(lines) and lines[i].strip() != "---":
            i += 1
        i += 1
    in_code = False
    para: list[str] = []
    while i < len(lines):
        s = lines[i].strip()
        if s.startswith("```") or s.startswith("~~~"):
            in_code = not in_code
        elif not in_code:
            if not s:
                if para:
                    break
            elif s.startswith(("#", "<", "|", ":::", "<!--", "---")):
                if para:
                    break
            else:
                para.append(s)
        i += 1
    return {"path": rel, "h2": h2, "first_paragraph": "".join(para)[:120]}


def run_stage(cmd: list[str], cwd: Path) -> dict:
    """跑一段子进程，异常（超时 / 命令不存在）折叠成结构化结果，不崩。"""
    try:
        proc = subprocess.run(
            cmd, cwd=str(cwd), capture_output=True, text=True,
            encoding="utf-8", errors="replace", timeout=BUILD_TIMEOUT,
        )
    except subprocess.TimeoutExpired:
        return {"cmd": " ".join(cmd), "ok": False, "timeout": True,
                "output_tail": ""}
    except FileNotFoundError:
        return {"cmd": " ".join(cmd), "ok": False, "tool_missing": True,
                "output_tail": ""}
    out = (proc.stderr or "") + (proc.stdout or "")
    tail = re.sub(r"\s+", " ", out.strip())[-300:]
    return {"cmd": " ".join(cmd), "ok": proc.returncode == 0,
            "returncode": proc.returncode, "output_tail": tail}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="preflight.py",
        description="单篇飞行检查：示例必需件（CMakeLists.txt + main.cpp）+ cmake 实跑"
                    "（300s 超时）+ 成功后摘邻篇摘要。五件套欠账件（.gitignore / "
                    "widget .h+.cpp 对）不阻断、记欠账归 W-EX。Qt 未装 / 构建失败时"
                    " exit 2 并把 blocked + block_reason 记进账本。篇目必须已入账。"
    )
    parser.add_argument("path", help="教程 Markdown 路径（仓库相对或绝对），"
                                     "如 tutorial/beginner/03-qtwidgets/17-qpushbutton-beginner.md")
    args = parser.parse_args(argv)

    md = resolve_article(args.path)
    if not md.is_file():
        print(f"找不到教程文件：{args.path}", file=sys.stderr)
        return 1
    rel = rel_key(md)
    try:
        ledger.require_article(rel)  # 未入账即报错，不静默补账
    except ledger.LedgerError as e:
        print(f"preflight.py 失败：{e}", file=sys.stderr)
        return 1
    slug = article_slug(md)
    chapter = md.parent.name
    exdir = REPO / "examples" / "beginner" / chapter / slug
    ex_rel = rel_key(exdir)

    report: dict = {
        "slug": slug,
        "path": rel,
        "sha256": hashlib.sha256(md.read_bytes()).hexdigest(),
        "ts": now_iso(),
        "example_dir": ex_rel,
    }
    debts: list[str] = []
    ex_files = example_state(exdir) if exdir.is_dir() else {}
    report["example_files"] = ex_files

    def finish(status: str, reason: str, exit_code: int) -> int:
        report["status"] = status
        report["block_reason"] = reason
        report["debts"] = debts
        target = PREFLIGHT_DIR / f"{slug}.json"
        ledger.ensure_dirs()
        ledger.atomic_write_json(target, report)
        where = ledger.display(target)
        if status == "blocked":
            # 与 ledger.py 迁移表同语义：block 任意来源可发，attempts +1
            ledger.transition(rel, "block", f"preflight：{reason}")
            print(f"[blocked] {slug}：{reason}")
        else:
            # 与 ledger.py 迁移表同语义：仅解除本工具自己拉的黑（block_reason 带
            # preflight 前缀）；调度器 batch.py 拉的黑原因各异，preflight 无权代解。
            rec = ledger.load_ledger()["articles"][rel]
            if rec["status"] == "blocked" and str(rec.get("block_reason") or "").startswith("preflight："):
                ledger.transition(rel, "requeue", "preflight 复检通过，解除拉黑（attempts 保留）")
            elif rec["status"] == "blocked":
                print(f"[提示] {slug} 当前 blocked 由调度器拉黑（{rec.get('block_reason')}），"
                      "preflight 复检通过但无权代解，处置走 batch.py requeue")
            ledger.append_history(rel, "preflight_ok", f"示例 {ex_rel} 必需件齐 + 构建通过")
            print(f"[ok] {slug}：必需件齐，构建通过，摘要已落 {where}")
        for debt in debts:
            print(f"[债] {debt}")
        return exit_code

    # ---- 门 1：示例目录与构建必需件（不齐即 blocked，不启动构建） ----
    if not exdir.is_dir():
        return finish("blocked", f"示例目录不存在：{ex_rel}", 2)
    missing = [name for name in BUILD_REQUIRED if not ex_files.get(name)]
    if missing:
        return finish("blocked", "示例缺构建必需件：" + "、".join(missing), 2)
    debts = [
        f".gitignore 缺失（五件套欠账，归 W-EX 补齐）" if not ex_files[".gitignore"] else "",
        f"widget 实现 .h/.cpp 对缺失（现有源文件：{sorted(p.name for p in exdir.glob('*'))[:8]}；五件套欠账，归 W-EX 补齐）"
        if not ex_files["widget_impl_pair"] else "",
    ]
    debts = [d for d in debts if d]

    # ---- 门 2：cmake -B build && cmake --build build ----
    configure = run_stage(["cmake", "-B", "build"], exdir)
    report["build"] = {"configure": configure}
    if configure.get("tool_missing"):
        return finish("blocked", "cmake 不可用：PATH 里找不到 cmake，请先安装", 2)
    if configure.get("timeout"):
        return finish("blocked", f"cmake configure 超时（>{BUILD_TIMEOUT}s）", 2)
    if not configure["ok"]:
        tail = configure["output_tail"]
        if "Qt6" in tail and "package configuration file" in tail:
            reason = f"Qt6 未装或 CMake 找不到 Qt6 包（configure 失败）：{tail}"
        else:
            reason = f"cmake configure 失败（exit {configure.get('returncode')}）：{tail}"
        return finish("blocked", reason, 2)

    build = run_stage(["cmake", "--build", "build"], exdir)
    report["build"]["build"] = build
    if build.get("timeout"):
        return finish("blocked", f"cmake --build 超时（>{BUILD_TIMEOUT}s）", 2)
    if not build["ok"]:
        tail = build["output_tail"]
        reason = f"cmake --build 失败（exit {build.get('returncode')}）：{tail}"
        return finish("blocked", reason, 2)

    # ---- 门 3：邻篇 digest（同目录按文件名序取前后各一篇） ----
    siblings = sorted(p for p in md.parent.glob("*.md") if p != md)
    names = [p.name for p in siblings]
    anchor = md.name
    ordered = sorted(names + [anchor])
    idx = ordered.index(anchor)
    digest: dict = {"prev": None, "next": None}
    for slot, pos in (("prev", idx - 1), ("next", idx + 1)):
        if 0 <= pos < len(ordered):
            neighbor = md.parent / ordered[pos]
            try:
                digest[slot] = extract_digest(
                    neighbor.read_text(encoding="utf-8"), rel_key(neighbor))
            except OSError as error:
                digest[slot] = {"path": rel_key(neighbor), "error": f"读取失败：{error}"}
    report["digest"] = digest
    return finish("ok", "", 0)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except SystemExit:
        raise
    except Exception as error:  # 兜底：任何内部错误都不许 traceback 崩
        print(f"preflight 内部错误（{type(error).__name__}）：{error}", file=sys.stderr)
        raise SystemExit(1)
