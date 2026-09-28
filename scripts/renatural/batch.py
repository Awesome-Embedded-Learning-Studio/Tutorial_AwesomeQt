#!/usr/bin/env python3
"""renatural 批调度器：next 锁批 -> start 开写 -> done 单篇交走查 / block 拉黑。

用法：
  batch.py next [--n N]              打印并锁定下一批（默认 6 篇；批不跨波、不跨预排批号）
  batch.py start <batch-id>          把该批全部 queued 篇目置为 writing（如 start M1A-1）
  batch.py done <path>               单篇写手完成：writing -> review（四走查入口）
  batch.py block <path> --reason R   拉黑单篇（attempts +1；拉黑原因必填）
  batch.py requeue <path> [--note N] blocked / reverted 重新入队（不动 attempts，
                                     处置后的人工确认动作——block 的逆操作）

调度纪律（ROADMAP_BASE_RESTYLE.md 单批 runbook）：
- 单篇两轮不过自动降 blocked：block 每次累加 attempts；queued 且 attempts≥2 的篇，
  next 派发前会被自动拉黑，不再进入任何新批；批照常合其余篇；
- 批不跨波、不跨 plan.py 预排批号：next 永远从“仍有 queued 篇目的最小批号”补齐，
  半途启动的批下次 next 优先补完；
- 一切迁移走 ledger.transition（原子写 + history.jsonl 留痕），账本只增不改。

口径红线（作者 2026-09-28 澄清，见 renatural/state/lint-baseline.md §4.2）：本工具不含
lint 预算 / 豁免 / 白名单放行逻辑——done 不看 lint 计数（归零检查归 accept.py 的
humanizer error 级全零门），block 只管排期现实（示例缺失 / 断言对不上等）。
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ledger  # noqa: E402  同目录共享模块


def _resolve(path: str) -> str:
    """命令行 path 参数 -> 账本键（仓库相对路径），未入账报错。"""
    return ledger.require_article(path)


def cmd_next(n: int) -> int:
    queue = ledger.load_queue()
    data = ledger.load_ledger()
    articles = data["articles"]

    # 账实一致：queue 与 ledger 的篇目集必须吻合，脱节就大声报错，不静默跳过
    q_paths = {e["path"] for e in queue}
    l_paths = set(articles)
    if q_paths != l_paths:
        only_q = sorted(q_paths - l_paths)[:3]
        only_l = sorted(l_paths - q_paths)[:3]
        raise ledger.LedgerError(
            "queue.json 与 ledger.json 篇目脱节（queue 有 "
            f"{len(q_paths - l_paths)} 篇不在账本、账本有 {len(l_paths - q_paths)} 篇不在队列，"
            f"如 {only_q or only_l}）——请重跑 plan.py 对账"
        )

    # 0) 自动拉黑：queued 且 attempts>=2（单篇两轮不过降 blocked，不阻塞其余篇）
    auto_blocked = [
        e["path"]
        for e in queue
        if articles.get(e["path"], {}).get("status") == "queued"
        and articles[e["path"]].get("attempts", 0) >= 2
    ]
    for path in auto_blocked:
        ledger.transition(
            path, "block", "attempts>=2 自动 blocked：单篇两轮不过，降级待人工处置"
        )
        print(f"自动拉黑：{path}（attempts>=2，不再派发）")
    if auto_blocked:
        # 自动拉黑改变了账本，重读一份新视图
        data = ledger.load_ledger()
        articles = data["articles"]

    # 1) 候选 = queued，按 seq 升序（queue 落盘即按 seq 排好）
    queued_entries = [e for e in queue if articles.get(e["path"], {}).get("status") == "queued"]
    if not queued_entries:
        counts = ledger.summary()["by_status"]
        done_like = {k: v for k, v in counts.items() if v}
        print("没有可派发的 queued 篇目。当前账本：" + ", ".join(f"{k}={v}" for k, v in done_like.items()))
        return 1

    # 2) 批不跨波、不跨预排批号：锁定“仍有 queued 的最小批”，条数不超过 --n
    first = queued_entries[0]
    batch_id, wave = first["batch"], first["wave"]
    same_batch = [e for e in queued_entries if e["batch"] == batch_id]
    selected = same_batch[:n]
    left_in_batch = len(same_batch) - len(selected)

    # 3) 锁批：history 留痕（状态保持 queued，start 才切 writing）
    for e in selected:
        ledger.transition(e["path"], "lock", f"锁入批次 {batch_id}（batch.py next）")

    print(f"下一批：{batch_id}（波次 {wave}，本批锁 {len(selected)} 篇）")
    for e in selected:
        rec = articles.get(e["path"], {})
        print(f"  seq={e['seq']:<3} {e['path']}")
        print(f"        family={rec.get('family') or '-'}  priority={e['priority']}  {e['reason']}")
    if left_in_batch:
        print(f"提示：{batch_id} 还有 {left_in_batch} 篇 queued 未锁（--n 上限截断，下次 next 优先补齐）")
    print(f"开写：python3 scripts/renatural/batch.py start {batch_id}")
    return 0


def cmd_start(batch_id: str) -> int:
    data = ledger.load_ledger()
    members = [(p, r) for p, r in data["articles"].items() if r.get("batch") == batch_id]
    if not members:
        def _batch_key(b: str) -> tuple:
            head, _, tail = b.rpartition("-")
            return (head, int(tail)) if tail.isdigit() else (b, 0)

        available = sorted(
            {r.get("batch") for r in data["articles"].values() if r.get("batch")}, key=_batch_key
        )
        raise ledger.LedgerError(
            f"找不到批次 {batch_id!r}。账本里的批号：{', '.join(available) or '（空）'}"
        )
    started, skipped = [], []
    for path, rec in sorted(members, key=lambda kv: kv[1].get("wave", "")):
        if rec["status"] == "queued":
            ledger.transition(path, "start", f"批次 {batch_id} 开写（batch.py start）")
            started.append(path)
        else:
            skipped.append((path, rec["status"]))
    print(f"批次 {batch_id}：start {len(started)} 篇 -> writing")
    for p in started:
        print(f"  writing  {p}")
    for p, s in skipped:
        print(f"  跳过     {p}（当前 {s}，非 queued）")
    return 0 if started or skipped else 1


def cmd_done(path: str) -> int:
    rel = _resolve(path)
    rec = ledger.transition(rel, "review", "batch.py done：写手完成，进入四走查（声音/保真/连续性/指代）")
    print(f"{rel}：writing -> review（attempts={rec['attempts']}）")
    print("下一步：四走查问题单 -> 修订 -> recheck -> accept.py（humanizer error 级全零才可 ready）")
    return 0


def cmd_block(path: str, reason: str) -> int:
    rel = _resolve(path)
    rec = ledger.transition(rel, "block", reason)
    print(f"{rel}：-> blocked（attempts={rec['attempts']}，block_reason 已记）")
    if rec["attempts"] >= 2:
        print("已达两轮上限：保持 blocked，next 不再自动派发；处置后重排走 batch.py requeue 人工确认")
    return 0


def cmd_requeue(path: str, note: str) -> int:
    rel = _resolve(path)
    rec = ledger.transition(
        rel, "requeue", note or "batch.py requeue：拉黑已处置，重新入队（attempts 保留）"
    )
    print(f"{rel}：-> queued（attempts={rec['attempts']} 保留，block_reason 已清）")
    print("下一步：batch.py start <batch-id> 恢复开写，或等下次 next 派发")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="batch.py",
        description="renatural 批调度器：next 锁批 / start 开写 / done 交走查 / block 拉黑（一切迁移走 ledger 原子写 + history 留痕）",
        epilog="口径提醒：done 不看 lint 计数，放行判断归 accept.py 的 error 级全零门；本工具无任何预算 / 豁免逻辑（lint-baseline.md §4.2）。",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p_next = sub.add_parser("next", help="打印并锁定下一批（默认 6 篇；批不跨波、不跨预排批号）")
    p_next.add_argument("--n", type=int, default=6, metavar="N", help="本批最多锁定的篇数（默认 6）")

    p_start = sub.add_parser("start", help="把该批全部 queued 篇目置为 writing")
    p_start.add_argument("batch_id", metavar="batch-id", help="批号，如 M1A-1 / W3-2（next 会打印）")

    p_done = sub.add_parser("done", help="单篇写手完成：writing -> review")
    p_done.add_argument("path", help="仓库相对路径，如 tutorial/beginner/03-qtwidgets/20-qcheckbox-beginner.md")

    p_block = sub.add_parser("block", help="拉黑单篇（attempts +1；attempts>=2 的 queued 篇 next 时自动 blocked）")
    p_block.add_argument("path", help="仓库相对路径")
    p_block.add_argument("--reason", required=True, help="拉黑原因（必填，进 block_reason 与 history）")

    p_requeue = sub.add_parser("requeue", help="blocked / reverted 重新入队（不动 attempts，处置后的人工确认动作）")
    p_requeue.add_argument("path", help="仓库相对路径")
    p_requeue.add_argument("--note", default="", help="处置说明（进 history；拉黑原因的闭环交代）")

    args = parser.parse_args(argv)
    if args.command == "next":
        if args.n < 1:
            raise ledger.LedgerError("--n 至少为 1")
        return cmd_next(args.n)
    if args.command == "start":
        return cmd_start(args.batch_id)
    if args.command == "done":
        return cmd_done(args.path)
    if args.command == "block":
        return cmd_block(args.path, args.reason)
    return cmd_requeue(args.path, args.note)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except ledger.LedgerError as e:
        print(f"batch.py 失败：{e}", file=sys.stderr)
        sys.exit(1)
