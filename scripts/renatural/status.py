#!/usr/bin/env python3
"""status —— 渲染 renatural/state/STATUS.md 三张表。

数据源：ledger.json（状态真相）+ queue.json（波次 / 批次补齐）+
history.jsonl（速度样本）。三张表：
1. 按章进度：各章的状态分布与完成率；
2. 按批进度：各（wave, batch）的状态分布与完成率；
3. 速度 ETA：取 history.jsonl 最近 20 条 merged 事件，按相邻事件时间差
   的均值估节奏，推剩余篇数的预计完成时间。

账实不符以内容重算为准；本脚本只读不写状态，仅产出 STATUS.md。
"""

from __future__ import annotations

import argparse
import json
import sys
from datetime import datetime
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ledger  # noqa: E402  同目录共享账本模块（路径 / 沙箱重定向 / 原子写单一实现）

REPO = ledger.REPO_ROOT
STATE = ledger.STATE_DIR
LEDGER = ledger.LEDGER_PATH
HISTORY = ledger.HISTORY_PATH
QUEUE = ledger.QUEUE_PATH
STATUS_MD = STATE / "STATUS.md"

STATUS_ORDER = ["queued", "blocked", "writing", "review", "ready", "merged", "reverted"]
ETA_SAMPLE = 20  # 最近 merged 事件样本数


def now_iso() -> str:
    return datetime.now().astimezone().isoformat(timespec="seconds")


def load_json(path: Path, default):
    if not path.exists():
        return default
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        print(f"警告：{path.name} 解析失败（{error}），按空数据处理", file=sys.stderr)
        return default


def load_history() -> list[dict]:
    events: list[dict] = []
    if not HISTORY.exists():
        return events
    try:
        lines = HISTORY.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        print(f"警告：history.jsonl 读取失败（{error}）", file=sys.stderr)
        return events
    for line in lines:
        line = line.strip()
        if not line:
            continue
        try:
            rec = json.loads(line)
            if isinstance(rec, dict):
                events.append(rec)
        except json.JSONDecodeError:
            print("警告：history.jsonl 有损坏行，已跳过", file=sys.stderr)
    return events


def parse_ts(raw) -> datetime | None:
    if not isinstance(raw, str):
        return None
    try:
        return datetime.fromisoformat(raw.replace("Z", "+00:00"))
    except ValueError:
        return None


def collect_articles() -> dict[str, dict]:
    """ledger 为真相源；queue 里有而 ledger 还没有的按 queued 计。"""
    ledger = load_json(LEDGER, {})
    articles = ledger.get("articles") if isinstance(ledger, dict) else None
    articles = articles if isinstance(articles, dict) else {}
    queue = load_json(QUEUE, [])
    queue_rows = {r.get("path"): r for r in queue if isinstance(r, dict) and r.get("path")}

    merged: dict[str, dict] = {}
    for path, art in articles.items():
        if not isinstance(art, dict):
            art = {}
        row = queue_rows.get(path, {})
        merged[path] = {
            "status": art.get("status") or "queued",
            "wave": art.get("wave") if art.get("wave") not in (None, "") else row.get("wave", ""),
            "batch": art.get("batch") if art.get("batch") not in (None, "") else row.get("batch", ""),
        }
    for path, row in queue_rows.items():
        if path not in merged:
            merged[path] = {"status": "queued",
                            "wave": row.get("wave", ""), "batch": row.get("batch", "")}
    return merged


def chapter_of(path: str) -> str:
    parts = Path(path).parts
    if len(parts) >= 2:
        return parts[-2]
    return "(根目录)"


def batch_sort_key(item) -> tuple:
    wave, batch = item
    batch_val = batch
    if isinstance(batch_val, str) and batch_val.isdigit():
        batch_val = int(batch_val)
    elif isinstance(batch_val, str):
        batch_val = 10**6  # 非数字批名排后
    try:
        batch_val = (0, batch_val) if isinstance(batch_val, int) else (1, str(batch_val))
    except TypeError:
        batch_val = (1, str(batch_val))
    wave_s = str(wave)
    return (wave_s, batch_val, str(batch))


def status_cell(counts: dict, key: str) -> str:
    return str(counts.get(key, 0)) if counts.get(key) else "·"


def render_group_table(title: str, groups: list[tuple[str, dict]]) -> list[str]:
    lines = [f"## {title}", "",
             "| 分组 | 总数 | queued | blocked | writing | review | ready | merged | reverted | 完成率 |",
             "|---|---|---|---|---|---|---|---|---|---|"]
    for name, counts in groups:
        total = sum(counts.values())
        merged = counts.get("merged", 0)
        rate = f"{merged / total:.1%}" if total else "—"
        cells = [status_cell(counts, k) for k in STATUS_ORDER]
        lines.append("| " + " | ".join([name, str(total)] + cells + [rate]) + " |")
    lines.append("")
    return lines


def render_eta(articles: dict[str, dict], events: list[dict]) -> list[str]:
    lines = ["## 速度与 ETA", ""]
    # ledger.py 的规范迁移事件叫 "merge"（目标态 merged）；本表两者都认
    merged_events = [e for e in events if e.get("event") in ("merge", "merged")]
    sample = merged_events[-ETA_SAMPLE:]
    total = len(articles)
    merged_now = sum(1 for a in articles.values() if a["status"] == "merged")
    remaining = max(total - merged_now, 0)

    lines.append("| 指标 | 数值 |")
    lines.append("|---|---|")
    lines.append(f"| 账本总篇数 | {total} |")
    lines.append(f"| 当前 merged | {merged_now} |")
    lines.append(f"| 剩余 | {remaining} |")
    lines.append(f"| 速度样本 | 最近 {len(sample)} 条 merged 事件（共 {len(merged_events)} 条） |")

    stamps = [parse_ts(e.get("ts")) for e in sample]
    stamps = [t for t in stamps if t is not None]
    if len(stamps) < 2:
        lines.append("| 平均节奏 | 数据不足（需 ≥2 条 merged 事件） |")
        lines.append("| 预计完成 | 数据不足 |")
        lines.append("")
        return lines

    diffs = [(stamps[i + 1] - stamps[i]).total_seconds() for i in range(len(stamps) - 1)]
    diffs = [d for d in diffs if d > 0] or [diffs[0] if diffs else 0]
    pace_sec = sum(diffs) / len(diffs)
    per_day = 86400.0 / pace_sec if pace_sec > 0 else 0.0
    eta = datetime.fromtimestamp(datetime.now().timestamp() + pace_sec * remaining).astimezone()
    lines.append(f"| 平均节奏 | {pace_sec / 3600:.2f} 小时/篇（≈{per_day:.2f} 篇/天） |")
    if remaining == 0:
        lines.append("| 预计完成 | 已全部 merged |")
    else:
        lines.append(f"| 预计完成 | {eta.strftime('%Y-%m-%d %H:%M')}（按当前节奏外推，仅供参考） |")
    lines.append("")
    return lines


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="status.py",
        description="读 ledger + queue + history，渲染 STATUS.md 三张表：按章进度 / 按批进度 / 速度 ETA。",
    )
    parser.add_argument("--output", default=str(STATUS_MD),
                        help=f"STATUS.md 输出路径（默认 {STATUS_MD}）")
    args = parser.parse_args(argv)

    articles = collect_articles()
    events = load_history()

    by_chapter: dict[str, dict] = {}
    by_batch: dict[tuple, dict] = {}
    for path, art in sorted(articles.items()):
        ch = chapter_of(path)
        by_chapter.setdefault(ch, {}).setdefault(art["status"], 0)
        by_chapter[ch][art["status"]] += 1
        key = (art["wave"] or "—", art["batch"] if art["batch"] not in (None, "") else "—")
        by_batch.setdefault(key, {}).setdefault(art["status"], 0)
        by_batch[key][art["status"]] += 1

    chapter_groups = sorted(by_chapter.items())
    batch_groups = sorted(by_batch.items(), key=batch_sort_key)

    total = len(articles)
    merged_now = sum(1 for a in articles.values() if a["status"] == "merged")
    blocked_now = sum(1 for a in articles.values() if a["status"] == "blocked")

    out = ["# base 卷自然化改版进度（STATUS）", "",
           f"- 生成时间：{now_iso()}",
           "- 数据源：ledger.json（状态真相）+ queue.json（波次补齐）+ history.jsonl（速度样本）",
           f"- 总览：{total} 篇在账 / merged {merged_now} / blocked {blocked_now}",
           "- 口径：账实不符以内容重算为准；ETA 按最近 "
           f"{ETA_SAMPLE} 条 merged 事件时间差均值外推，仅供参考", ""]

    if not articles:
        out += ["（账本与队列均为空：先跑 plan.py 建队列，或 preflight.py / accept.py 会按需建账）", ""]

    out += render_group_table("按章进度", chapter_groups)
    batch_named = [(f"{w} / 批 {b}", c) for (w, b), c in batch_groups]
    out += render_group_table("按批进度", batch_named)
    out += render_eta(articles, events)

    target = Path(args.output)
    target.parent.mkdir(parents=True, exist_ok=True)
    ledger.atomic_write_text(target, "\n".join(out))
    print(f"STATUS.md 已生成：{target}")
    print(f"  按章 {len(chapter_groups)} 组 / 按批 {len(batch_groups)} 组 / "
          f"merged {merged_now} / 剩余 {total - merged_now}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except SystemExit:
        raise
    except Exception as error:  # 兜底：渲染不许 traceback 崩
        print(f"status 内部错误（{type(error).__name__}）：{error}", file=sys.stderr)
        raise SystemExit(1)
