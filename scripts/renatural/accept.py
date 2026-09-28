#!/usr/bin/env python3
"""accept —— 单篇验收门（READY 前最后一道机器检查）。

三道检查，全过才 exit 0：
1. humanizer：``.claude/content_forge/tools/humanizer_lint.py --format json``
   的 **error 级命中必须为 0，一条都不豁免**（2026-09-28 作者澄清：全部
   error 级规则对 AI 新稿归零；本脚本不实现任何预算 / 豁免 / 白名单放行
   逻辑，见 renatural/state/lint-baseline.md §4.2）。review 级不阻断，
   但必须显式报数，不得静默吞掉；
2. ``scripts/renatural/invariants.py check``：不变量对账。工具未落地时
   按不通过处理——缺一道门不是放行的理由；
3. 战役红线（本脚本自查）：正文「」=0；``^## \\d+.`` 报数标题=0；
   H1 / frontmatter title 带「现代Qt开发教程」前缀=0；``━`` 连续 ≥3 =0。

``--dry-run`` 只打印报告，不写 ledger / history（只读，不要求篇目已入账）。
非 dry-run 要求篇目已入账（plan.py 建队）；通过时按 ledger.py 状态迁移表放行
（review → ready；merged 保持；其余状态不越级改写，只在 history 留痕）；不通过
不改状态、不动 attempts（attempts 由 block 类事件累加，见 ledger.py
ATTEMPT_EVENTS）。账本 IO 一律复用 ledger 模块（原子写 + 校验 + RENATURAL_STATE）。
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ledger  # noqa: E402  同目录共享账本模块

REPO = ledger.REPO_ROOT
HUMANIZER = REPO / ".claude" / "content_forge" / "tools" / "humanizer_lint.py"
INVARIANTS = REPO / "scripts" / "renatural" / "invariants.py"

H1_PREFIX = "现代Qt开发教程"
LIST_CAP = 50  # 报告里逐条列出的上限，超出部分显式报数，不静默


def resolve_article(raw: str) -> Path:
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


def strip_for_prose(text: str) -> str:
    """去 frontmatter、围栏代码块、行内代码，只留正文可读文本。"""
    lines = text.splitlines()
    i = 0
    if lines and lines[0].strip() == "---":
        i = 1
        while i < len(lines) and lines[i].strip() != "---":
            i += 1
        i += 1
    kept: list[str] = []
    in_code = False
    while i < len(lines):
        s = lines[i].lstrip()
        if s.startswith("```") or s.startswith("~~~"):
            in_code = not in_code
            i += 1
            continue
        if in_code:
            i += 1
            continue
        kept.append(re.sub(r"`[^`]*`", "", lines[i]))
        i += 1
    return "\n".join(kept)


def run_humanizer(md: Path) -> tuple[list[dict], list[dict], str]:
    """返回 (error 命中, review 命中, 工具错误说明)。工具错误时前两项为空。"""
    cmd = [sys.executable, str(HUMANIZER), "--format", "json", str(md)]
    try:
        proc = subprocess.run(cmd, cwd=str(REPO), capture_output=True, text=True,
                              encoding="utf-8", errors="replace", timeout=120)
    except subprocess.TimeoutExpired:
        return [], [], "humanizer 超时（>120s）"
    except FileNotFoundError:
        return [], [], f"找不到 humanizer_lint.py：{HUMANIZER}"
    try:
        findings = json.loads(proc.stdout)
    except json.JSONDecodeError:
        tail = re.sub(r"\s+", " ", (proc.stderr or "").strip())[-200:]
        return [], [], f"humanizer 输出不可解析（exit {proc.returncode}）：{tail}"
    if not isinstance(findings, list):
        return [], [], "humanizer 输出不是 JSON 数组"
    errors = [f for f in findings if f.get("level") == "error"]
    reviews = [f for f in findings if f.get("level") == "review"]
    return errors, reviews, ""


def run_invariants(md: Path) -> tuple[bool, str]:
    """跑 scripts/renatural/invariants.py check。未落地 = 不通过（不静默放行）。"""
    if not INVARIANTS.is_file():
        return False, f"invariants.py 未落地（{INVARIANTS.relative_to(REPO)}），验收门缺一不放行"
    cmd = [sys.executable, str(INVARIANTS), "check", str(md)]
    try:
        proc = subprocess.run(cmd, cwd=str(REPO), capture_output=True, text=True,
                              encoding="utf-8", errors="replace", timeout=300)
    except subprocess.TimeoutExpired:
        return False, "invariants check 超时（>300s）"
    except FileNotFoundError:
        return False, "invariants.py 无法启动"
    if proc.returncode == 0:
        return True, ""
    tail = re.sub(r"\s+", " ", ((proc.stderr or "") + (proc.stdout or "")).strip())[-200:]
    return False, f"invariants check 失败（exit {proc.returncode}）：{tail}"


def check_redlines(text: str) -> list[dict]:
    """战役红线四条，逐条返回 {id, ok, detail, lines}。"""
    results: list[dict] = []

    prose = strip_for_prose(text)
    corner_hits = [i + 1 for i, line in enumerate(prose.splitlines())
                   if ("「" in line or "」" in line)]
    results.append({
        "id": "corner_bracket",
        "ok": not corner_hits,
        "detail": f"正文直角引号「」命中 {len(corner_hits)} 行（代码块与行内代码除外）",
        "lines": corner_hits[:5],
    })

    numbered = [i + 1 for i, line in enumerate(text.splitlines())
                if re.match(r"^##\s+\d+\.", line)]
    results.append({
        "id": "numbered_h2",
        "ok": not numbered,
        "detail": f"报数标题（^## 数字.）命中 {len(numbered)} 行",
        "lines": numbered[:5],
    })

    prefix_hits: list[str] = []
    for line in text.splitlines():
        m = re.match(r"^#\s+(.+?)\s*$", line)
        if m and m.group(1).startswith(H1_PREFIX):
            prefix_hits.append(f"H1:{m.group(1)[:30]}")
    fm_title = re.search(r"(?m)^title:\s*[\"'“]?(.+?)[\"'”]?\s*$", text)
    if fm_title and fm_title.group(1).startswith(H1_PREFIX):
        prefix_hits.append(f"frontmatter title:{fm_title.group(1)[:30]}")
    results.append({
        "id": "h1_prefix",
        "ok": not prefix_hits,
        "detail": f"H1 / floating title 带「{H1_PREFIX}」前缀命中 {len(prefix_hits)} 处",
        "lines": prefix_hits[:5],
    })

    heavy = re.findall(r"━{3,}", text)
    results.append({
        "id": "heavy_rule",
        "ok": not heavy,
        "detail": f"连续重横线（━≥3）命中 {len(heavy)} 处",
        "lines": [],
    })
    return results


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="accept.py",
        description="单篇验收门：humanizer error 级归零（零豁免）+ invariants + 战役红线，"
                    "全过 exit 0。--dry-run 只报告不改账本。",
    )
    parser.add_argument("path", help="教程 Markdown 路径（仓库相对或绝对）")
    parser.add_argument("--dry-run", action="store_true",
                        help="只打印检查报告，不写 ledger.json / history.jsonl")
    args = parser.parse_args(argv)

    md = resolve_article(args.path)
    if not md.is_file():
        print(f"找不到教程文件：{args.path}", file=sys.stderr)
        return 1
    rel = rel_key(md)

    errors, reviews, tool_error = run_humanizer(md)
    inv_ok, inv_detail = run_invariants(md)
    redlines = check_redlines(md.read_text(encoding="utf-8"))

    humanizer_ok = not tool_error and not errors
    all_ok = humanizer_ok and inv_ok and all(r["ok"] for r in redlines)

    # ---- 报告（error 逐条列出、超上限显式报数，不得静默吞掉） ----
    if tool_error:
        print(f"[FAIL] humanizer 工具异常：{tool_error}")
    else:
        state = "PASS" if not errors else "FAIL"
        print(f"[{state}] humanizer：error {len(errors)} / review {len(reviews)}"
              + ("（review 级须声音走查引用原文，不阻断 READY）" if reviews else ""))
        for idx, f in enumerate(errors):
            if idx >= LIST_CAP:
                print(f"        …… 其余 {len(errors) - LIST_CAP} 条 error 未逐行列出，计数已计入")
                break
            print(f"        L{f.get('line')} {f.get('rule')} 「{f.get('hit')}」")
    state = "PASS" if inv_ok else "FAIL"
    print(f"[{state}] invariants" + (f"：{inv_detail}" if inv_detail else "：不变量对账通过"))
    for r in redlines:
        state = "PASS" if r["ok"] else "FAIL"
        extra = f"；样例 {r['lines']}" if (not r["ok"] and r["lines"]) else ""
        print(f"[{state}] 红线 {r['id']}：{r['detail']}{extra}")

    # ---- 账本与留痕 ----
    if args.dry_run:
        print("（--dry-run：仅报告，未写 ledger / history）")
        return 0 if all_ok else 1

    try:
        ledger.require_article(rel)  # 未入账即报错，不静默补账
    except ledger.LedgerError as e:
        print(f"accept.py 失败：{e}", file=sys.stderr)
        return 1

    status_now = ledger.load_ledger()["articles"][rel]["status"]
    if all_ok:
        summary = f"humanizer error=0 review={len(reviews)}；invariants 通过；红线全零"
        if status_now == "review":
            # ledger.py 迁移表：ready 只允许从 review 发起
            ledger.transition(rel, "ready", summary)
            print(f"[ready] {rel} 通过验收门，review → ready")
        elif status_now == "merged":
            ledger.append_history(rel, "accept_ready", summary + "（该篇已是 merged，状态不变）")
            print(f"[ready] {rel} 通过验收门（该篇已是 merged，状态不变）")
        else:
            ledger.append_history(
                rel, "accept_ready",
                summary + f"（当前 status={status_now}，按 ledger.py 迁移表"
                          "只有 review 才升 ready，未越级改写状态）")
            print(f"[ready] {rel} 检查全过；当前 status={status_now}，"
                  "按迁移表须先经 review 才放行 ready（未改状态）")
    else:
        # 不通过：不改状态、不动 attempts（attempts 由 block 类事件累加），只留痕
        parts = []
        if tool_error:
            parts.append(f"humanizer 工具异常：{tool_error}")
        elif errors:
            parts.append(f"humanizer error={len(errors)}")
        if not inv_ok:
            parts.append(inv_detail)
        parts += [f"红线 {r['id']}：{r['detail']}" for r in redlines if not r["ok"]]
        ledger.append_history(rel, "accept_failed", "；".join(parts))
        print(f"[fail] {rel} 未过验收门（详见上方报告）")
    return 0 if all_ok else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except SystemExit:
        raise
    except Exception as error:  # 兜底：验收门不许 traceback 崩
        print(f"accept 内部错误（{type(error).__name__}）：{error}", file=sys.stderr)
        raise SystemExit(1)
