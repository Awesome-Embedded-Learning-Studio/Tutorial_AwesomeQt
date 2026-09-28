#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""echo_lint.py —— 跨篇回声（模板腔 / 互相抄句）探测（纯标准库，零第三方依赖）。

用法：
  echo_lint.py --build-blacklist [--n N] [--min-articles K]
      扫描 tutorial/beginner/**/*.md：剥离 frontmatter、围栏代码块、行内代码、图片、
      HTML 注释与白名单片段（Q 类名 / Qt 标识符 / cmake 命令等技术符号）后，取 ≥5 字
      n-gram；出现在 ≥K（默认 3）篇即入黑名单 → renatural/state/echo-blacklist.json
  echo_lint.py --check <file>
      统计该文相对「其余各篇 ≥K 篇共有」黑名单的命中数；新成稿要求零新增，
      有命中即 exit 1（无预算、无豁免）。

口径说明：
  - n-gram 只在空白 / 被剥离片段构成的边界内取，不跨边界拼接；
  - 黑名单条目须含至少一个汉字（纯英文 / 纯标点串不收）；
  - 白名单是技术标识符过滤（防 Qt 术语与代码符号污染黑名单），不是对任何
    lint 规则的放行；黑名单条目供人工初审后增补白名单迭代校准；
  - --check 对语料内文章按「其余各篇」口径计数（该文自身不计入 K）。
"""
import argparse
import datetime
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ledger  # noqa: E402  同目录共享账本模块（路径 / history / 原子写单一实现）

SCHEMA = "renatural/echo-blacklist/1"
REPO_ROOT = ledger.REPO_ROOT
STATE_DIR = ledger.STATE_DIR
BLACKLIST_PATH = STATE_DIR / "echo-blacklist.json"
DEFAULT_N = 5
DEFAULT_MIN_ARTICLES = 3

FENCE_RE = re.compile(r"^ {0,3}(`{3,}|~{3,})(.*)$")
KEY_RE = re.compile(r"^([A-Za-z][A-Za-z0-9_-]*)\s*:(?!//)\s*(.*)$")
CJK_RE = re.compile(r"[一-鿿]")

# 白名单初版：全部为「技术符号」类正则，作用是把代码 / 标识符 / 版本号从
# n-gram 流里隔离成边界，防止 Qt 术语污染黑名单。人工初审后按需增补。
_TECH_WORDS = (
    "signal signals slot slots emit connect disconnect widget widgets window windows "
    "button buttons dialog dialogs menu menus toolbar statusbar layout layouts box grid form "
    "page tab tabs stack split dock view views model models delegate item items tree table list "
    "lists edit line label event events timer timers thread threads mutex paint mouse keyboard "
    "focus parent child action actions main app lambda function pointer reference memory string "
    "char int float double bool void const static class struct enum public private protected "
    "virtual override namespace template header source project example examples code debug "
    "release build rebuild run install setup update version default custom style theme icon "
    "image audio video network socket server client local remote async compile linker cmake "
    "make ninja qmake moc pnpm npm git bash shell script test tests doc docs tutorial error "
    "warning crash leak bug assert flow margin spacing hidpi"
).split()

WHITELIST = [
    # URL（结尾不吞中文与全角标点）
    re.compile(r"""https?://[^\s<>"'`”「」『』）)\]]+"""),
    # 限定名 a::b::c（QSlider::valueChanged / std::string / Qt::AlignCenter）
    re.compile(r"(?<![A-Za-z0-9_:])[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)+"),
    # 常见带扩展名的文件名（main.cpp / CMakeLists.txt / xx.png）
    re.compile(
        r"(?<![A-Za-z0-9_])[A-Za-z0-9_.-]+\.(?:cpp|h|hpp|cc|cxx|ui|pro|pri|qrc|cmake|txt|md|"
        r"json|png|jpe?g|gif|svg|webp|py|ts|yml|yaml)(?![A-Za-z0-9_])"
    ),
    # Q 系标识符：QPushButton / Q_OBJECT / Qt6（贴着汉字也能命中）
    re.compile(r"(?<![A-Za-z0-9_])Q[A-Za-z_][A-Za-z0-9_]*"),
    # qDebug / qWarning 与 qt_metacall 一类
    re.compile(r"(?<![A-Za-z0-9_])q[A-Z][A-Za-z0-9_]*"),
    re.compile(r"(?<![A-Za-z0-9_])qt_[A-Za-z0-9_]+"),
    # 全大写宏与缩写（SIGNAL / CMAKE_XXX / GUI / API）
    re.compile(r"(?<![A-Za-z0-9_])[A-Z][A-Z0-9_]{2,}(?![A-Za-z0-9_])"),
    # snake_case / camelCase 标识符
    re.compile(r"(?<![A-Za-z0-9_])[a-z][a-z0-9]*(?:_[A-Za-z0-9]+)+(?![A-Za-z0-9_])"),
    re.compile(r"(?<![A-Za-z0-9_])[a-z]{2,}[A-Z][A-Za-z0-9_]*"),
    # 版本号与裸数字（6.9.1 / 145）
    re.compile(r"(?<![A-Za-z0-9_.])\d+(?:\.\d+)+(?![A-Za-z0-9_.])"),
    re.compile(r"(?<![A-Za-z0-9_.])\d+(?![A-Za-z0-9_.])"),
    # C++ / C++17
    re.compile(r"(?<![A-Za-z0-9_])C\+\+\d*(?![A-Za-z0-9_])"),
    # 高频技术英文词（含贴中文胶水场景，如「main 函数」「emit 信号」）
    re.compile(r"(?<![A-Za-z0-9_])(?:" + "|".join(_TECH_WORDS) + r")(?![A-Za-z0-9_])", re.IGNORECASE),
]


def now_iso():
    return datetime.datetime.now().astimezone().isoformat(timespec="seconds")


def die(msg, code=2):
    print(f"[echo_lint] 错误：{msg}", file=sys.stderr)
    sys.exit(code)


def trunc(s, n):
    s = str(s)
    return s if len(s) <= n else s[: n - 1] + "…"


def read_text(path):
    with open(path, "r", encoding="utf-8-sig") as f:
        return f.read().replace("\r\n", "\n").replace("\r", "\n")


def append_history(path, event, detail):
    ledger.append_history(path, event, detail)


def resolve_path(p):
    cand = Path(p)
    if not cand.is_absolute():
        c_cwd = Path.cwd() / cand
        cand = c_cwd if c_cwd.exists() else (REPO_ROOT / cand)
    if not cand.exists():
        die(f"文件不存在：{p}")
    cand = cand.resolve()
    try:
        rel = str(cand.relative_to(REPO_ROOT))
    except ValueError:
        rel = str(cand)
    return cand, rel, cand.stem


def split_frontmatter(text):
    lines = text.split("\n")
    if not lines or lines[0].strip() != "---":
        return None, text, 0
    end = None
    for i in range(1, len(lines)):
        if lines[i].strip() == "---":
            end = i
            break
    if end is None:
        return None, text, 0
    body = "\n".join(lines[end + 1:])
    fm_len = sum(len(l) + 1 for l in lines[: end + 1])
    return {}, body, fm_len


def _space_out(m):
    return " " * (m.end() - m.start())


def mask_body(text):
    """等长空格化被剥离内容，保留原文索引。返回 (masked_body, fm_len)。"""
    _, body, fm_len = split_frontmatter(text)
    out = []
    fence = None
    for line in body.split("\n"):
        m = FENCE_RE.match(line)
        if fence is not None:
            out.append(" " * len(line))
            if m and m.group(1)[0] == fence[0] and len(m.group(1)) >= fence[1]:
                fence = None
        elif m:
            fence = (m.group(1)[0], len(m.group(1)))
            out.append(" " * len(line))
        else:
            out.append(line)
    masked = "\n".join(out)
    masked = re.sub(r"<!--.*?-->", _space_out, masked, flags=re.S)
    masked = re.sub(r"`+[^`]*`+", _space_out, masked)          # 行内代码
    masked = re.sub(r"!\[[^\]]*\]\([^)]*\)", _space_out, masked)  # 图片整体
    for pat in WHITELIST:
        masked = pat.sub(_space_out, masked)
    return masked, fm_len


def gram_spans(text, n):
    """返回 (grams, fm_len)。grams = [(gram, body_start, body_end)]，不跨边界、须含汉字。"""
    masked, fm_len = mask_body(text)
    grams = []
    run = []
    run_start = 0

    def flush():
        if len(run) >= n:
            base = run_start
            for i in range(len(run) - n + 1):
                grams.append(("".join(run[i:i + n]), base + i, base + i + n))

    for idx, ch in enumerate(masked):
        if ch.isspace():
            flush()
            run = []
        else:
            if not run:
                run_start = idx
            run.append(ch)
    flush()
    return [g for g in grams if CJK_RE.search(g[0])], fm_len


def cmd_build(args):
    n = args.n
    if n < 5:
        die("--n 最小为 5（口径：≥5 字 n-gram）")
    if args.min_articles < 2:
        die("--min-articles 最小为 2")
    files = sorted(REPO_ROOT.glob("tutorial/beginner/**/*.md"))
    if not files:
        die("语料为空：tutorial/beginner/**/*.md")
    count = {}
    corpus = []
    for f in files:
        rel = str(f.relative_to(REPO_ROOT))
        corpus.append(rel)
        grams, _ = gram_spans(read_text(f), n)
        for g in {t for t, _s, _e in grams}:
            count[g] = count.get(g, 0) + 1
    entries = sorted(
        ((g, c) for g, c in count.items() if c >= args.min_articles),
        key=lambda kv: (-kv[1], kv[0]),
    )
    data = {
        "schema": SCHEMA,
        "built_at": now_iso(),
        "n": n,
        "min_articles": args.min_articles,
        "corpus_size": len(corpus),
        "corpus": corpus,
        "entries": [[g, c] for g, c in entries],
    }
    STATE_DIR.mkdir(parents=True, exist_ok=True)
    ledger.atomic_write_text(BLACKLIST_PATH, json.dumps(data, ensure_ascii=False, indent=1) + "\n")
    n_idx = sum(1 for c in corpus if c.endswith("index.md"))
    print(f"[build-blacklist] 语料 {len(corpus)} 篇（正文 {len(corpus) - n_idx} + index.md {n_idx}），n={n}，门槛 ≥{args.min_articles} 篇")
    print(f"  黑名单条目：{len(entries)}")
    print(f"  已写入 {ledger.display(BLACKLIST_PATH)}（{BLACKLIST_PATH.stat().st_size // 1024} KB）")
    if entries:
        print("  top5 样例（篇数 / n-gram）：")
        for g, c in entries[:5]:
            print(f"    {c:>3} 篇  {g}")
    append_history(
        "tutorial/beginner",
        "echo_blacklist_build",
        f"corpus={len(corpus)} entries={len(entries)} n={n} min={args.min_articles}",
    )
    return 0


def cmd_check(args):
    if not BLACKLIST_PATH.exists():
        die(f"黑名单不存在，请先运行 --build-blacklist（预期路径 {ledger.display(BLACKLIST_PATH)}）")
    data = json.loads(BLACKLIST_PATH.read_text(encoding="utf-8"))
    n = data.get("n", DEFAULT_N)
    min_a = data.get("min_articles", DEFAULT_MIN_ARTICLES)
    entries = {g: c for g, c in data.get("entries", [])}
    corpus = data.get("corpus", [])
    abs_path, rel_path, _slug = resolve_path(args.file)
    text = read_text(abs_path)
    grams, fm_len = gram_spans(text, n)
    in_corpus = rel_path in corpus
    hits = []
    for g, s, e in grams:
        c = entries.get(g)
        if c is None:
            continue
        eff = c - 1 if in_corpus else c  # 语料内文章：按「其余各篇」口径
        if eff >= min_a:
            hits.append((g, s, e, eff))
    print(f"[check] {rel_path}")
    basis = "其余各篇" if in_corpus else "全语料（该文不在黑名单语料内）"
    print(f"  基准：{ledger.display(BLACKLIST_PATH)}（n={n}，{basis} ≥{min_a} 篇共有）")
    if not hits:
        print("  命中：0 —— 无跨篇回声")
        return 0
    spans = []
    for g, s, e, c in sorted(hits, key=lambda h: h[1]):
        if spans and s <= spans[-1][1]:
            spans[-1][1] = max(spans[-1][1], e)
            spans[-1][2] += 1
        else:
            spans.append([s, e, 1])
    distinct = len({h[0] for h in hits})
    print(f"  命中：{len(hits)} 个 n-gram（去重 {distinct} 个），合并为 {len(spans)} 段连续回声 —— exit 1")
    spans.sort(key=lambda sp: -(sp[1] - sp[0]))
    for s, e, cnt in spans[:10]:
        excerpt = re.sub(r"\s+", " ", text[fm_len + s: fm_len + e]).strip()
        print(f"    [{cnt:>3} gram] {trunc(excerpt, 60)}")
    if len(spans) > 10:
        print(f"    …（其余 {len(spans) - 10} 段略，全文共 {len(spans)} 段）")
    append_history(rel_path, "echo_check_hit", f"grams={len(hits)} distinct={distinct} spans={len(spans)}")
    return 1


def build_parser():
    p = argparse.ArgumentParser(
        prog="echo_lint.py",
        description="跨篇回声探测：--build-blacklist 构建黑名单，--check 查单篇命中（有命中即 exit 1，无预算、无豁免）。",
        epilog=(
            "示例：\n"
            "  echo_lint.py --build-blacklist\n"
            "  echo_lint.py --check tutorial/beginner/01-qtbase/02-signal-slot-beginner.md\n"
            "黑名单落在 renatural/state/echo-blacklist.json。"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument("--build-blacklist", action="store_true", help="扫描 tutorial/beginner/**/*.md 构建 n-gram 黑名单")
    g.add_argument("--check", dest="file", metavar="FILE", help="检查指定文章的黑名单命中（新成稿要求零命中）")
    p.add_argument("--n", type=int, default=DEFAULT_N, help="n-gram 长度（默认 5；口径 ≥5，小于 5 拒绝）")
    p.add_argument("--min-articles", type=int, default=DEFAULT_MIN_ARTICLES, help="入黑名单的最少共有篇数（默认 3，仅 --build-blacklist 生效）")
    return p


def main(argv=None):
    args = build_parser().parse_args(argv)
    if args.build_blacklist:
        return cmd_build(args)
    return cmd_check(args)


if __name__ == "__main__":
    sys.exit(main())
