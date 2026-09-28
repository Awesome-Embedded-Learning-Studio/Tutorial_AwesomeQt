#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""invariants.py —— base 卷改版「不变量对账」工具（纯标准库，零第三方依赖）。

子命令：
  snapshot <path>                 采集度量，落快照 renatural/state/invariants/<slug>.json
  check <path> [--against <snap>] 与快照逐项对账（默认对比已存快照）

计数口径：正文 = 剥离 frontmatter 与围栏代码块后的文本（行内代码保留）。
  qt_src_line_refs    qt_src/qt6.9.1/...:行号 引用数（支持中文冒号与行号区间）
  doc_qt_io_urls      doc.qt.io 的 URL 数
  fenced_code_blocks  围栏代码块数（``` 与 ~~~）
  code_block_sha1     逐块内容 sha1（不含围栏行与语言标注）
  code_block_langs    逐块语言标注
  fence_unclosed      未闭合围栏数（完整性哨兵，应为 0）
  images              图片数（Markdown ![](...) 与 <img>）
  relative_links      文内相对链接数（排除 http/https/mailto 等带 scheme 与纯锚点）

另验（校验项，失败即 exit 1）：frontmatter title/description 齐全，
且 description ≠ 正文第一句——判据为两者规范化后既不相等、也互不为对方子串
（防「整句照抄首句」与「首句开头扩写」两种抄法）。

退出码：0 全等且校验全过；1 存在漂移或校验失败；2 用法/文件错误。
事件留痕：snapshot 与 check 漂移时向 renatural/state/history.jsonl 追加一条。
"""
import argparse
import datetime
import hashlib
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ledger  # noqa: E402  同目录共享账本模块（路径 / history / 原子写单一实现）

SCHEMA = "renatural/invariants/1"
REPO_ROOT = ledger.REPO_ROOT
STATE_DIR = ledger.STATE_DIR
INV_DIR = ledger.INVARIANTS_DIR

FENCE_RE = re.compile(r"^ {0,3}(`{3,}|~{3,})(.*)$")
KEY_RE = re.compile(r"^([A-Za-z][A-Za-z0-9_-]*)\s*:(?!//)\s*(.*)$")
QT_SRC_LINE_RE = re.compile(r"qt_src/qt6\.9\.1[A-Za-z0-9_./\\-]*[:：]\d+")
URL_RE = re.compile(r"""https?://[^\s<>"'`”「」『』）)\]]+""")
URL_TAIL = "。，、；：！？）)>,;:."
IMAGE_MD_RE = re.compile(r"!\[[^\]]*\]\([^)]+\)")
IMAGE_HTML_RE = re.compile(r"<img\b[^>]*>", re.IGNORECASE)
LINK_RE = re.compile(r"""(?<!!)\[[^\]]*\]\(\s*([^)\s]+)(?:\s+"[^"]*")?\s*\)""")
SCHEME_RE = re.compile(r"^[a-zA-Z][a-zA-Z0-9+.\-]*:")
TAG_LINE_RE = re.compile(r"^<[A-Za-z/!]")
SENT_RE = re.compile(r"^(.*?[。！？?!])")
NORM_STRIP_RE = re.compile(r"[\s“”\"'「」『』*_~`·]")

METRIC_KEYS = [
    "qt_src_line_refs",
    "doc_qt_io_urls",
    "fenced_code_blocks",
    "code_block_sha1",
    "code_block_langs",
    "fence_unclosed",
    "images",
    "relative_links",
]
CHECK_KEYS = [
    "frontmatter_present",
    "title_present",
    "description_present",
    "description_differs_from_first_sentence",
]


def now_iso():
    return datetime.datetime.now().astimezone().isoformat(timespec="seconds")


def die(msg, code=2):
    print(f"[invariants] 错误：{msg}", file=sys.stderr)
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
    # slug 与 ledger.slug 同口径：index.md 以 <章目录>-index 限定，八部不同名
    stem = f"{cand.parent.name or 'beginner'}-index" if cand.stem == "index" else cand.stem
    return cand, rel, stem


def split_frontmatter(text):
    """返回 (fm_dict|None, body, fm_len)。支持键值折行（多行 description）。"""
    lines = text.split("\n")
    if not lines or lines[0].strip() != "---":
        return None, text, 0
    raw = {}
    cur = None
    end = None
    for i in range(1, len(lines)):
        line = lines[i]
        if line.strip() == "---":
            end = i
            break
        m = KEY_RE.match(line)
        if m:
            cur = m.group(1)
            raw[cur] = [m.group(2).strip()]
        elif cur is not None:
            raw[cur].append(line.strip())
    if end is None:
        return None, text, 0
    fm = {}
    for k, v in raw.items():
        s = "\n".join(v).strip()
        if len(s) >= 2 and s[0] == s[-1] and s[0] in "\"'":
            s = s[1:-1].strip()
        fm[k] = s
    body = "\n".join(lines[end + 1:])
    fm_len = sum(len(l) + 1 for l in lines[: end + 1])
    return fm, body, fm_len


def sha1_lines(lines):
    return hashlib.sha1("\n".join(lines).encode("utf-8")).hexdigest()


def split_fences(body):
    """返回 (剥离后的文本, [{lang, sha1}], 未闭合数)。剥离行以空行占位。"""
    out = []
    blocks = []
    unclosed = 0
    fence = None  # (围栏字符, 长度)
    lang = ""
    cur = []
    for line in body.split("\n"):
        m = FENCE_RE.match(line)
        if fence is not None:
            if m and m.group(1)[0] == fence[0] and len(m.group(1)) >= fence[1]:
                blocks.append({"lang": lang, "sha1": sha1_lines(cur)})
                fence = None
                cur = []
            else:
                cur.append(line)
            out.append("")
        elif m:
            fence = (m.group(1)[0], len(m.group(1)))
            lang = m.group(2).strip()
            cur = []
            out.append("")
        else:
            out.append(line)
    if fence is not None:
        blocks.append({"lang": lang, "sha1": sha1_lines(cur)})
        unclosed = 1
    return "\n".join(out), blocks, unclosed


def extract_first_sentence(stripped_body):
    """取正文第一句：跳过标题/图片/表格/HTML，剥行内 markdown 后按 。！？?! 截断。"""
    in_comment = False
    for raw_line in stripped_body.split("\n"):
        line = raw_line.strip()
        while True:
            if in_comment:
                if "-->" in line:
                    line = line.split("-->", 1)[1]
                    in_comment = False
                    continue
                break
            if "<!--" in line:
                head, _, tail = line.partition("<!--")
                if "-->" in tail:
                    line = head + tail.split("-->", 1)[1]
                    continue
                in_comment = True
                line = head
                continue
            break
        s = line.strip()
        if not s:
            continue
        if s.startswith(("#", "!", "|", "---")) or TAG_LINE_RE.match(s):
            continue
        s = s.lstrip("> ").strip()
        if not s or s.startswith(("#", "!", "|")):
            continue
        t = re.sub(r"!\[[^\]]*\]\([^)]*\)", "", s)
        t = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", t)
        t = re.sub(r"[`*_~]+", "", t)
        t = re.sub(r"</?[A-Za-z][^>]*>", "", t)
        t = t.strip()
        if not t:
            continue
        m = SENT_RE.match(t)
        return m.group(1) if m else t
    return ""


def norm_cmp(s):
    return NORM_STRIP_RE.sub("", s or "")


def description_copies_first(desc, first):
    """description 是否抄了正文第一句：相等，或（长度足够时）互为对方子串。"""
    nd, nf = norm_cmp(desc), norm_cmp(first)
    if not nd or not nf:
        return False  # 空值由「齐全」校验兜底
    if nd == nf:
        return True
    if len(nd) >= 6 and nd in nf:
        return True
    if len(nf) >= 6 and nf in nd:
        return True
    return False


def compute_invariants(abs_path):
    text = read_text(abs_path)
    fm, body, _ = split_frontmatter(text)
    stripped, blocks, unclosed = split_fences(body)
    title = (fm or {}).get("title", "")
    desc = (fm or {}).get("description", "")
    first_sent = extract_first_sentence(stripped)
    urls = [u.rstrip(URL_TAIL) for u in URL_RE.findall(stripped)]
    rel_links = []
    for tgt in LINK_RE.findall(stripped):
        tgt = tgt.strip()
        if not tgt or tgt.startswith("#") or SCHEME_RE.match(tgt):
            continue
        rel_links.append(tgt)
    metrics = {
        "qt_src_line_refs": len(QT_SRC_LINE_RE.findall(stripped)),
        "doc_qt_io_urls": sum(1 for u in urls if "doc.qt.io" in u),
        "fenced_code_blocks": len(blocks),
        "code_block_sha1": [b["sha1"] for b in blocks],
        "code_block_langs": [b["lang"] for b in blocks],
        "fence_unclosed": unclosed,
        "images": len(IMAGE_MD_RE.findall(stripped)) + len(IMAGE_HTML_RE.findall(stripped)),
        "relative_links": len(rel_links),
    }
    checks = {
        "frontmatter_present": fm is not None,
        "title_present": bool(title.strip()),
        "description_present": bool(desc.strip()),
        "description_differs_from_first_sentence": not description_copies_first(desc, first_sent),
    }
    context = {"title": title, "description": desc, "first_sentence": first_sent}
    return metrics, checks, context


def metric_label(key):
    return {
        "qt_src_line_refs": "qt_src/qt6.9.1 行号引用",
        "doc_qt_io_urls": "doc.qt.io URL",
        "fenced_code_blocks": "围栏代码块",
        "code_block_sha1": "逐块 sha1",
        "code_block_langs": "代码块语言标注",
        "fence_unclosed": "未闭合围栏",
        "images": "图片",
        "relative_links": "文内相对链接",
    }.get(key, key)


def cmd_snapshot(args):
    abs_path, rel_path, slug = resolve_path(args.path)
    metrics, checks, context = compute_invariants(abs_path)
    INV_DIR.mkdir(parents=True, exist_ok=True)
    snap = {
        "schema": SCHEMA,
        "path": rel_path,
        "slug": slug,
        "captured_at": now_iso(),
        "metrics": metrics,
        "checks": checks,
        "context": context,
    }
    out = INV_DIR / f"{slug}.json"
    ledger.atomic_write_json(out, snap)
    print(f"[snapshot] {rel_path}")
    print(f"  快照已写入 {ledger.display(out)}")
    print(
        f"  qt_src行号引用={metrics['qt_src_line_refs']} doc.qt.io={metrics['doc_qt_io_urls']}"
        f" 代码块={metrics['fenced_code_blocks']} 图片={metrics['images']}"
        f" 相对链接={metrics['relative_links']} 未闭合围栏={metrics['fence_unclosed']}"
    )
    fm_ok = checks["frontmatter_present"] and checks["title_present"] and checks["description_present"]
    print(
        f"  校验：frontmatter 齐全={'通过' if fm_ok else '失败'}；"
        f"description≠正文首句={'通过' if checks['description_differs_from_first_sentence'] else '失败'}"
        f"（首句「{trunc(context['first_sentence'], 40)}」）"
    )
    if not all(checks.values()):
        print("  注意：校验存在失败项，快照仍已落盘（供对账与追踪）")
    append_history(
        rel_path,
        "invariants_snapshot",
        f"qtsrc={metrics['qt_src_line_refs']} urls={metrics['doc_qt_io_urls']}"
        f" code={metrics['fenced_code_blocks']} imgs={metrics['images']}"
        f" links={metrics['relative_links']}",
    )
    return 0


def cmd_check(args):
    abs_path, rel_path, slug = resolve_path(args.path)
    if args.against:
        snap_path = Path(args.against)
        if not snap_path.is_absolute():
            c_cwd = Path.cwd() / snap_path
            snap_path = c_cwd if c_cwd.exists() else (REPO_ROOT / snap_path)
        if not snap_path.exists():
            die(f"--against 指定的快照不存在：{args.against}")
    else:
        snap_path = INV_DIR / f"{slug}.json"
        if not snap_path.exists():
            die(f"未找到快照 renatural/state/invariants/{slug}.json，请先对该篇运行 snapshot")
    old = json.loads(snap_path.read_text(encoding="utf-8"))
    if old.get("schema") != SCHEMA:
        die(f"快照 schema 不兼容：{old.get('schema')!r} ≠ {SCHEMA!r}")
    metrics, checks, context = compute_invariants(abs_path)

    lines = [f"[check] {rel_path}", f"  对账基准：{snap_path}（采集于 {old.get('captured_at', '?')}）"]
    if old.get("path") not in (None, rel_path):
        lines.append(f"  提示：快照来自 {old.get('path')}，与当前路径不同，仅提示不算漂移")

    drifts = []
    for key in METRIC_KEYS:
        ov = old["metrics"].get(key)
        nv = metrics.get(key)
        if ov == nv:
            extra = f"（{len(nv)} 块）" if isinstance(nv, list) else f"= {nv}"
            lines.append(f"  相等  {metric_label(key)} {extra}")
            continue
        drifts.append(key)
        if isinstance(ov, list) or isinstance(nv, list):
            ol = ov if isinstance(ov, list) else []
            nl = nv if isinstance(nv, list) else []
            bad = [i + 1 for i, (a, b) in enumerate(zip(ol, nl)) if a != b]
            msg = f"{len(ol)} 块 → {len(nl)} 块"
            if bad:
                msg += f"；首处不一致：第 {bad[0]} 块"
            lines.append(f"  漂移  {metric_label(key)}：{msg}")
        else:
            lines.append(f"  漂移  {metric_label(key)}：{ov} → {nv}")

    for key in CHECK_KEYS:
        ov = old["checks"].get(key)
        nv = checks.get(key)
        if ov == nv:
            lines.append(f"  相等  校验 {key} = {nv}")
        else:
            drifts.append(key)
            lines.append(f"  漂移  校验 {key}：{ov} → {nv}")

    validity_fail = [k for k in CHECK_KEYS if not checks[k]]
    lines.append(
        f"  校验  description≠正文首句：{'通过' if checks['description_differs_from_first_sentence'] else '失败'}"
        f"（首句「{trunc(context['first_sentence'], 40)}」）"
    )
    if validity_fail:
        lines.append(f"  校验  失败项：{', '.join(validity_fail)}")

    n_items = len(METRIC_KEYS) + len(CHECK_KEYS)
    print("\n".join(lines))
    if drifts or validity_fail:
        print(
            f"  结论：{n_items - len(drifts)}/{n_items} 相等；"
            f"漂移 {len(drifts)} 项（{'、'.join(metric_label(k) for k in drifts) if drifts else '无'}），"
            f"校验失败 {len(validity_fail)} 项 —— exit 1"
        )
        append_history(
            rel_path,
            "invariants_drift",
            "drift=" + ",".join(drifts) + (";invalid=" + ",".join(validity_fail) if validity_fail else ""),
        )
        return 1
    print(f"  结论：{n_items}/{n_items} 相等，校验全过")
    return 0


def build_parser():
    p = argparse.ArgumentParser(
        prog="invariants.py",
        description="base 卷改版不变量对账工具：snapshot 落快照，check 逐项对账（漂移或校验失败 exit 1）。",
        epilog=(
            "示例：\n"
            "  invariants.py snapshot tutorial/beginner/01-qtbase/02-signal-slot-beginner.md\n"
            "  invariants.py check  tutorial/beginner/01-qtbase/02-signal-slot-beginner.md\n"
            "  invariants.py check  tutorial/beginner/01-qtbase/02-signal-slot-beginner.md \\\n"
            "      --against renatural/state/invariants/02-signal-slot-beginner.json\n"
            "快照落在 renatural/state/invariants/<slug>.json，slug = 文件名去扩展名。"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    sub = p.add_subparsers(dest="cmd", required=True)
    ps = sub.add_parser("snapshot", help="采集度量并落快照到 renatural/state/invariants/<slug>.json")
    ps.add_argument("path", help="文章路径（仓库相对或绝对）")
    ps.set_defaults(func=cmd_snapshot)
    pc = sub.add_parser("check", help="与快照逐项对账，输出相等/漂移清单")
    pc.add_argument("path", help="文章路径（仓库相对或绝对）")
    pc.add_argument("--against", metavar="SNAP", help="指定基准快照文件（默认对比已存快照 invariants/<slug>.json）")
    pc.set_defaults(func=cmd_check)
    return p


def main(argv=None):
    args = build_parser().parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
