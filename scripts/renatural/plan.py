#!/usr/bin/env python3
"""renatural 计划器：扫 tutorial/beginner 全部 md，按内置波次表生成 queue.json 并初始化 ledger。

波次表（内置，来源 ROADMAP_BASE_RESTYLE.md 路线阶段 + lint-baseline.md §4.2 试点回炉）：
  M1A  02-qtgui 全 6 篇（双样板批 A）
  M1B  03-qtwidgets 的 12、17-21（按钮族，双样板批 B）
  W3   01-qtbase 16 篇（余 15 + 试点 01/02 按全零口径回炉）
  W4   04-qtnetwork 6 篇
  W5   03-qtwidgets 其余 68 篇，按族推进：主题 01-11 → 显示 34-37 → 容器 13/14+38-45
       → 列表树表格 15+46-54 → 输入 16+22-33 → 主窗口对话框 55-74（族长先于族成员）
  W6   05-other-modules 25 篇
  W7   06-qml 7 篇
  W8   00-environment-setup 3 篇 + 全部 8 个 index.md（单批）

批号规则：波内切批、批不跨波；默认每批不超过 7 篇（对齐路线图的 5-8 篇/批），W8 因含
8 个轻量 index 特批为单批。批号形如 M1A-1 / W3-2 / W8-1。

--with-lint：对每篇跑 .claude/content_forge/tools/humanizer_lint.py --format json，把
error 级命中数记进 queue 的 reason 字段。口径红线（作者 2026-09-28 澄清，lint-baseline.md
§4.2）：计数只是排期参考（债重的先写），不是放行判断——本工具不设预算、不设豁免、不设
白名单，任何“低于 N 处即通过”的逻辑都不允许出现在这里；放行判断归 accept.py 的归零检查。
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ledger  # noqa: E402  同目录共享模块

REPO_ROOT = ledger.REPO_ROOT
BEGINNER_DIR = REPO_ROOT / "tutorial" / "beginner"
LINT_TOOL = REPO_ROOT / ".claude" / "content_forge" / "tools" / "humanizer_lint.py"

#: 波次推进顺序（也是 queue.json 的 seq 顺序）
WAVE_ORDER = ("M1A", "M1B", "W3", "W4", "W5", "W6", "W7", "W8")

#: 章目录名 -> 默认波次（正文篇超出波次表编号范围时按章收编，reason 里注明）
CHAPTER_DEFAULT_WAVE = {
    "00-environment-setup": "W8",
    "01-qtbase": "W3",
    "02-qtgui": "M1A",
    "03-qtwidgets": "W5",
    "04-qtnetwork": "W4",
    "05-other-modules": "W6",
    "06-qml": "W7",
}

#: 章目录名 -> 家族名（非 03 章以章为族；03 章正文另有族表覆盖）
CHAPTER_FAMILY = {
    "00-environment-setup": "env",
    "01-qtbase": "qtbase",
    "02-qtgui": "qtgui",
    "04-qtnetwork": "network",
    "05-other-modules": "other-modules",
    "06-qml": "qml",
}

#: M1B 按钮族：03 章的 12（族长）+ 17-21
M1B_NUMBERS = frozenset({12, 17, 18, 19, 20, 21})

#: W5 族表：(族名, 覆盖篇号列表)；族长篇排在本族最前（路线图：族长先行走查）
W5_FAMILIES: list[tuple[str, list[int]]] = [
    ("主题", list(range(1, 12))),                                  # 01-11（11 为 QWidget 大族长）
    ("显示", list(range(34, 38))),                                  # 34-37
    ("容器", [13, 14] + list(range(38, 46))),                       # 族长 13/14 + 38-45
    ("列表树表格", [15] + list(range(46, 55))),                     # 族长 15 + 46-54
    ("输入", [16] + list(range(22, 34))),                           # 族长 16 + 22-33
    ("主窗口对话框", list(range(55, 75))),                           # 55-74
]

#: 每批篇数上限（路线图口径 5-8 篇/批，取 7）；W8 特批为单批
BATCH_MAX = 7

WAVE_DESC = {
    "M1A": "批 A：qtgui 旧模板最重、体量最小，验证主题篇全重写打法",
    "M1B": "批 B：按钮族（12 族长 + 17-21），验证族长导览与条目五拍",
    "W3": "W3 波：01-qtbase 以试点 1.2 为基准，09/12/05 最僵篇优先",
    "W4": "W4 波：qtnetwork 六篇，拆全码节归位 examples",
    "W5": "W5 波：03 章其余按族换声",
    "W6": "W6 波：05-other-modules 两阶段（mechanical 剥离 + editorial 换声）",
    "W7": "W7 波：06-qml 补结构欠账，首篇接住 Widgets 七十篇的来路",
    "W8": "W8 波：env 隐性测验显性化 + 八部 index 部首四要素落位",
}

_NUM_RE = re.compile(r"^(\d+)-")


def _article_num(filename: str) -> int | None:
    """文件名数字前缀（如 20-qcheckbox-beginner.md -> 20）；index.md 返回 None。"""
    if filename == "index.md":
        return None
    m = _NUM_RE.match(filename)
    return int(m.group(1)) if m else None


def scan() -> list[dict]:
    """扫 tutorial/beginner/**/*.md，返回原始条目（path/chapter/num/is_index）。"""
    if not BEGINNER_DIR.is_dir():
        raise ledger.LedgerError(f"找不到教程目录：{BEGINNER_DIR}")
    items: list[dict] = []
    for path in sorted(BEGINNER_DIR.rglob("*.md")):
        rel = path.relative_to(REPO_ROOT).as_posix()
        chapter = path.parent.name if path.parent != BEGINNER_DIR else ""
        items.append(
            {
                "path": rel,
                "chapter": chapter,
                "num": _article_num(path.name),
                "is_index": path.name == "index.md",
            }
        )
    return items


def _w5_family(num: int) -> tuple[str, int] | None:
    """03 章正文篇 -> (族名, 族内序)；返回 None 表示不在族表内。"""
    for family_rank, (family, nums) in enumerate(W5_FAMILIES):
        if num in nums:
            return family, nums.index(num) + 100 * family_rank
    return None


def classify(item: dict) -> dict:
    """把扫描条目归入波次 / 家族，并给出派序键与 reason。返回补全后的条目。

    未知情况一律显式处理：不认识的章目录直接报错；正文篇编号超出波次表按章默认波次
    收编并在 reason 里注明——queue 不允许静默缺篇。
    """
    chapter, num, is_index = item["chapter"], item["num"], item["is_index"]

    if is_index:
        family = "index"
        if chapter and chapter not in CHAPTER_DEFAULT_WAVE:
            raise ledger.LedgerError(f"不认识的章节目录 {chapter!r}（{item['path']}），请先更新波次表")
        note = "index 页：部首四要素（您现在在哪 / 本部解决什么 / 用到前面哪几篇 / 读完能做到什么）"
        return {**item, "wave": "W8", "family": family, "order": 10**9, "reason": note}

    if chapter not in CHAPTER_DEFAULT_WAVE:
        raise ledger.LedgerError(f"不认识的章节目录 {chapter!r}（{item['path']}），请先更新波次表")

    # M1B 覆盖：03 章按钮族
    if chapter == "03-qtwidgets" and num in M1B_NUMBERS:
        return {**item, "wave": "M1B", "family": "按钮", "order": num, "reason": WAVE_DESC["M1B"]}

    wave = CHAPTER_DEFAULT_WAVE[chapter]
    fallback = False
    if chapter == "03-qtwidgets":
        hit = _w5_family(num) if num is not None else None
        if hit is None:
            # 编号超出族表（未来新增篇目）：按章默认波次收编，归主题族，reason 注明
            fallback = True
            family, order = "主题", (num or 0)
        else:
            family, order = hit
    else:
        family = CHAPTER_FAMILY[chapter]
        order = num or 0

    reason = WAVE_DESC[wave]
    if wave == "W5":
        reason = f"{WAVE_DESC['W5']}（{family}族）"
    if fallback:
        reason += "；篇号超出波次表，按章默认波次收编，请人工核对"
    if wave == "W3" and chapter == "01-qtbase" and num in (1, 2):
        reason = "试点篇按全零口径回炉（lint-baseline.md §4.2），随 W3 波重写"
    if wave == "W8" and chapter == "00-environment-setup":
        reason = "W8 波：00-env 三篇按全零口径回炉（lint-baseline.md §4.2）+ 隐性测验显性化"

    return {**item, "wave": wave, "family": family, "order": order, "reason": reason}


def assign_batches(by_wave: dict[str, list[dict]]) -> None:
    """波内切批：批不跨波，每批不超过 BATCH_MAX 篇（W8 含 8 个轻量 index，特批为单批）。

    就地给每个条目写入 batch 字段。切法：k = ceil(n/上限)，前 n%k 批多一篇，
    保证批量为 5-8 篇且顺序与 queue 序完全一致（batch.py next 按序取件即按批取件）。
    """
    for wave in WAVE_ORDER:
        items = sorted(by_wave.get(wave, []), key=lambda x: x["order"])
        by_wave[wave] = items
        n = len(items)
        if n == 0:
            continue
        if wave == "W8":
            for item in items:
                item["batch"] = "W8-1"
            continue
        k = -(-n // BATCH_MAX)  # ceil
        base, rem = divmod(n, k)
        idx = 0
        for i in range(k):
            size = base + (1 if i < rem else 0)
            for item in items[idx:idx + size]:
                item["batch"] = f"{wave}-{i + 1}"
            idx += size


def lint_error_count(rel_path: str) -> tuple[int | None, str]:
    """跑 humanizer_lint --format json 数 error 级命中。

    返回 (error 数, 附注)。工具退出码 1 表示“有 error 命中”，是正常信号；
    只有崩溃 / 超时 / 输出不可解析才算失败，返回 (None, 失败说明)。
    计数只进 reason 作排期参考，不构成任何放行判断。
    """
    cmd = [sys.executable, str(LINT_TOOL), "--format", "json", rel_path]
    try:
        proc = subprocess.run(
            cmd, cwd=str(REPO_ROOT), capture_output=True, text=True, timeout=300
        )
    except subprocess.TimeoutExpired:
        return None, "lint 执行超时（300s）"
    except OSError as e:
        return None, f"lint 无法执行：{e}"
    if proc.returncode not in (0, 1):
        return None, f"lint 退出码 {proc.returncode}：{proc.stderr.strip()[:160]}"
    try:
        findings = json.loads(proc.stdout)
    except json.JSONDecodeError as e:
        return None, f"lint 输出不是合法 JSON：{e.msg}"
    if not isinstance(findings, list):
        return None, "lint 输出顶层不是列表"
    errors = sum(1 for f in findings if isinstance(f, dict) and f.get("level") == "error")
    return errors, ""


def build_plan(with_lint: bool) -> tuple[list[dict], dict[str, str], dict]:
    """生成完整 queue（seq/batch/priority/reason 齐全）、path->family 映射与统计信息。"""
    items = [classify(raw) for raw in scan()]
    by_wave: dict[str, list[dict]] = {w: [] for w in WAVE_ORDER}
    for it in items:
        by_wave[it["wave"]].append(it)
    assign_batches(by_wave)
    families = {it["path"]: it["family"] for it in items}

    queue: list[dict] = []
    seq = 0
    lint_failures: list[str] = []
    lint_total = 0
    lint_counts: dict[str, int] = {}
    for wave in WAVE_ORDER:
        for it in sorted(by_wave[wave], key=lambda x: x["order"]):
            seq += 1
            reason = it["reason"]
            priority = 0
            if with_lint:
                count, note = lint_error_count(it["path"])
                if count is None:
                    lint_failures.append(it["path"])
                    reason += f"；lint 执行失败（{note}）"
                else:
                    lint_counts[it["path"]] = count
                    lint_total += count
                    # 排期参考：债越重越靠前。仅排序用，不是放行判断，更不是预算。
                    priority = min(3, count // 100)
                    reason += f"；lint error 命中 {count} 处（排期参考，非放行判断）"
            queue.append(
                {
                    "path": it["path"],
                    "wave": wave,
                    "batch": it["batch"],
                    "seq": seq,
                    "priority": priority,
                    "reason": reason,
                }
            )

    family_stat: dict[str, int] = {}
    for it in items:
        if it["wave"] == "W5" and not it["is_index"]:
            family_stat[it["family"]] = family_stat.get(it["family"], 0) + 1

    stats = {
        "total": len(items),
        "articles": sum(1 for it in items if not it["is_index"]),
        "indexes": sum(1 for it in items if it["is_index"]),
        "by_wave": {
            w: {
                "articles": sum(1 for it in by_wave[w] if not it["is_index"]),
                "indexes": sum(1 for it in by_wave[w] if it["is_index"]),
                "batches": len({it["batch"] for it in by_wave[w]}),
            }
            for w in WAVE_ORDER
        },
        "w5_families": family_stat,
        "with_lint": with_lint,
        "lint_failures": lint_failures,
        "lint_total": lint_total,
        "lint_top": sorted(lint_counts.items(), key=lambda kv: -kv[1])[:5],
    }
    return queue, families, stats


def print_stats(stats: dict) -> None:
    out: list[str] = []
    out.append(f"扫描 tutorial/beginner：md 共 {stats['total']} = 正文 {stats['articles']} 篇 + index {stats['indexes']} 个")
    out.append("波次分布（波 / 正文篇数 / index / 批数）：")
    for wave in WAVE_ORDER:
        s = stats["by_wave"][wave]
        idx = f"/ index {s['indexes']}" if s["indexes"] else ""
        out.append(f"  {wave:<4} 正文 {s['articles']:>3} 篇{idx} / {s['batches']} 批")
    if stats["w5_families"]:
        rank = {family: i for i, (family, _) in enumerate(W5_FAMILIES)}
        fams = sorted(stats["w5_families"].items(), key=lambda kv: rank.get(kv[0], 99))
        fam = "、".join(f"{k} {v}" for k, v in fams)
        out.append(f"W5 族分布：{fam}")
    if stats["with_lint"]:
        out.append(f"lint 全量汇总：error 共 {stats['lint_total']} 处，失败 {len(stats['lint_failures'])} 篇")
        for path, count in stats["lint_top"]:
            out.append(f"  top  {count:>4}  {path}")
        out.append("（计数进 queue.reason 作排期参考；放行判断不在本工具，AI 新稿对全部 error 级规则归零）")
    print("\n".join(out))


def has_progress(ledger_data: dict) -> bool:
    """账本是否已有推进（非 queued 状态或尝试过拉黑）——重排会抹掉进度，必须显式 --force。"""
    for rec in ledger_data["articles"].values():
        if rec["status"] != "queued" or rec.get("attempts", 0) > 0:
            return True
    return False


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="plan.py",
        description="扫 tutorial/beginner 全部 md，按内置波次表生成 queue.json 并初始化 ledger（原子写 + history 留痕）",
        epilog="口径提醒：--with-lint 的计数只进 reason 作排期参考；本工具不含任何 lint 预算 / 豁免 / 白名单放行逻辑（lint-baseline.md §4.2）。",
    )
    parser.add_argument("--dry-run", action="store_true", help="只打印波次分布统计，不落盘任何文件")
    parser.add_argument(
        "--with-lint",
        action="store_true",
        help="对每篇跑 humanizer_lint --format json，把 error 级命中数记进 reason（排期参考，非放行判断）",
    )
    parser.add_argument(
        "--merge",
        action="store_true",
        help="非破坏重排：已有篇目保留 status/attempts/block_reason（只刷新 wave/batch/family），新篇目入队——账本已有推进时的正确重排方式",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="账本已有推进（存在非 queued 状态或 attempts>0）时仍强制重置全部状态——会抹掉进度，慎用",
    )
    args = parser.parse_args(argv)

    # 先过保护门再干活：账本已有推进时拒绝静默重排（--merge 保留进度 / --force 显式放弃）。
    # dry-run 不落盘，永远放行——只读预览不允许被保护门挡住。
    existing = ledger.load_ledger()
    progressing = (
        not args.dry_run
        and bool(existing["articles"])
        and has_progress(existing)
    )
    if progressing and not (args.force or args.merge):
        print(
            "账本已有推进（非 queued 状态或 attempts>0）：保留进度请加 --merge，确认全部重置才用 --force",
            file=sys.stderr,
        )
        return 1

    queue, families, stats = build_plan(with_lint=args.with_lint)
    if stats["total"] == 0:
        print("没有扫到任何 md 文件，拒绝生成空队列", file=sys.stderr)
        return 1
    print_stats(stats)

    if args.dry_run:
        print("dry-run：未落盘（queue.json / ledger.json / history.jsonl 均未改动）")
        return 0

    ledger.ensure_dirs()
    ts = ledger.now_iso()
    # build_plan 期间（--with-lint 全量约 8s）其他工具可能改账，落盘前重读最新账本再合并
    existing = ledger.load_ledger()
    old_articles = existing["articles"]  # --force 走重置，--merge / 空账走保留
    articles: dict[str, dict] = {}
    fresh, kept = 0, 0
    for entry in queue:
        old = None if args.force else old_articles.get(entry["path"])
        if old is None:
            articles[entry["path"]] = {
                "status": "queued",
                "wave": entry["wave"],
                "batch": entry["batch"],
                "family": families.get(entry["path"], ""),
                "attempts": 0,
                "block_reason": None,
                "updated": ts,
            }
            fresh += 1
        else:
            # 非破坏重排：调度元数据以新计划为准，执行状态原样保留
            articles[entry["path"]] = {
                "status": old["status"],
                "wave": entry["wave"],
                "batch": entry["batch"],
                "family": families.get(entry["path"], ""),
                "attempts": old["attempts"],
                "block_reason": old.get("block_reason"),
                "updated": old.get("updated") or ts,
            }
            kept += 1
    leftovers = sorted(set(old_articles) - {e["path"] for e in queue}) if not args.force else []
    for path in leftovers:
        articles[path] = old_articles[path]  # 不在新计划里的篇目原样保留，不静默丢
    ledger.save_ledger({"version": ledger.LEDGER_VERSION, "articles": articles})
    ledger.save_queue(queue)
    for entry in queue:
        if entry["path"] not in old_articles or args.force:
            ledger.append_history(
                entry["path"], "queue", f"入队 {entry['wave']}/{entry['batch']}：{entry['reason']}"
            )
    if leftovers:
        print(f"注意：{len(leftovers)} 篇在账但不在新计划里，已原样保留：{leftovers[:5]}")
    print(
        f"已落盘：queue.json（{len(queue)} 条）+ ledger.json（新入队 {fresh} / 沿用 {kept}）"
        f"+ history.jsonl 追加 {fresh if not args.force else len(queue)} 条 queue 事件"
    )
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except ledger.LedgerError as e:
        print(f"plan.py 失败：{e}", file=sys.stderr)
        sys.exit(1)
