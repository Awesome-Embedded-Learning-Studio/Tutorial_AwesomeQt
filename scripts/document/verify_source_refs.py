#!/usr/bin/env python3
"""核对专家层文章里的 Qt 源码引用是否真实存在。

专家层文章用 `basename.ext:行号`（如 qstring.h:1340、qstring.h:1340-1341）引用 Qt 源码。
本脚本：
  1. 解析每篇专家文章开头的「源码文件表」，建立 basename → qt_src 完整路径 的映射；
  2. 扫描正文里的 `basename.ext:行号` / `basename.ext:起-止` 引用；
  3. 去 qt_src/qt6.9.1 核对那一行真实存在、不是空行。

qt_src 不在仓库里（5.5G，gitignore），所以：
  - 本地有 qt_src 时：核对，发现「引用的行不存在 / 越界 / 空行 / basename 未登记」报错 exit 1；
  - 没有 qt_src 时（CI 或贡献者未下载）：打印跳过提示 exit 0（不阻塞）。
"""
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
QT_SRC = REPO_ROOT / "qt_src" / "qt6.9.1"
EXPERT_DIR = REPO_ROOT / "tutorial" / "expert"

# 源码文件表里的完整路径：qt_src/qt6.9.1/<...>/basename.ext
SRC_PATH_RE = re.compile(r"qt_src/qt6\.9\.1(/[^\s|`]+?\.(?:cpp|h|hpp|cc|cxx))")
# 正文引用：basename.ext:行号 或 basename.ext:起-止。前面不能是 / 或单词字符（避开路径/URL）。
CITE_RE = re.compile(r"(?<![/\w])([a-z_][\w]*\.(?:cpp|h|hpp|cc|cxx)):(\d+)(?:-(\d+))?(?!\w)")

RED, GREEN, YELLOW, RESET = "\033[31m", "\033[32m", "\033[33m", "\033[0m"


def strip_fenced(text: str) -> str:
    """剔除 ``` 围栏代码块，只留散文（表格与引用保留）。"""
    return re.sub(r"```.*?```", "", text, flags=re.DOTALL)


def parse_source_map(article_text: str) -> dict[str, list[str]]:
    """从源码文件表提取 basename → [qt_src 下的完整相对路径列表]。"""
    mapping: dict[str, list[str]] = {}
    for m in SRC_PATH_RE.finditer(article_text):
        rel = m.group(1).lstrip("/")
        mapping.setdefault(Path(rel).name, []).append(rel)
    return mapping


def verify_article(article: Path, qt_src: Path) -> list[tuple[str, str]]:
    problems = []
    text = article.read_text(encoding="utf-8", errors="replace")
    src_map = parse_source_map(text)
    if not src_map:
        return []  # 没有源码表，不是源码拆解篇，跳过
    prose = strip_fenced(text)
    for m in CITE_RE.finditer(prose):
        basename, start, end = m.group(1), int(m.group(2)), m.group(3)
        cite = f"{basename}:{start}" + (f"-{end}" if end else "")
        candidates = src_map.get(basename)
        if not candidates:
            problems.append((cite, f"basename {basename} 未在本篇源码表里登记"))
            continue
        end_line = int(end) if end else start
        ok, last_err = False, ""
        for rel in candidates:
            f = qt_src / rel
            if not f.exists():
                last_err = f"文件不存在: qt_src/qt6.9.1/{rel}"
                continue
            lines = f.read_text(encoding="utf-8", errors="replace").splitlines()
            if start > len(lines) or end_line > len(lines):
                last_err = f"行号越界（文件共 {len(lines)} 行）: {rel}:{start}"
                continue
            if any(lines[i - 1].strip() for i in range(start, end_line + 1)):
                ok = True
                break
            last_err = f"第 {start}{'-'+str(end_line) if end else ''} 行是空行: {rel}"
        if not ok:
            problems.append((cite, last_err))
    return problems


def main():
    if not QT_SRC.is_dir():
        print(f"{YELLOW}verify_source_refs: 未找到 {QT_SRC}（qt_src 未下载或在 CI 中），跳过源码引用核对。{RESET}")
        sys.exit(0)
    if not EXPERT_DIR.is_dir():
        print("verify_source_refs: 无 tutorial/expert 目录，跳过。")
        sys.exit(0)

    articles = [
        a for a in sorted(EXPERT_DIR.rglob("*.md"))
        if a.name != "index.md" and "code-index" not in a.parts
    ]
    all_problems, checked = [], 0
    for a in articles:
        if "qt_src" not in a.read_text(encoding="utf-8", errors="replace"):
            continue
        checked += 1
        all_problems.extend((str(a.relative_to(REPO_ROOT)), c, e) for c, e in verify_article(a, QT_SRC))

    if all_problems:
        print(f"\n{RED}✗ 发现 {len(all_problems)} 处源码引用问题：{RESET}")
        for art, cite, err in all_problems:
            print(f"  {RED}✗{RESET} {art}: {cite} — {err}")
        sys.exit(1)
    print(f"{GREEN}verify_source_refs: ✓ {checked} 篇专家层文章源码引用全部核对通过。{RESET}")
    sys.exit(0)


if __name__ == "__main__":
    main()
