#!/usr/bin/env python3
"""renatural 调度共享账本模块：queue.json / ledger.json / history.jsonl 的读写与状态迁移。

文件契约（scripts/renatural/ 三件共用，勿各自发明）：
  renatural/state/queue.json     = [{"path","wave","batch","seq","priority","reason"}]（path 为仓库相对路径）
  renatural/state/ledger.json    = {"version":1,"articles":{path:{status,wave,batch,family,attempts,block_reason,updated}}}
                                   status ∈ queued|blocked|writing|review|ready|merged|reverted
  renatural/state/history.jsonl  = 追加只增，每行 {"ts"(ISO),"path","event","detail"}；一切状态迁移都追加一条
  renatural/state/preflight/<slug>.json、invariants/<slug>.json、echo-blacklist.json 由后续工具落位，
  本模块只负责确保目录存在。slug = md 文件名去扩展名；index.md 以“<章目录>-index”限定避免八部同名冲突。

工程约束：
- 纯 python3 标准库，零第三方依赖；
- 一切 JSON 落盘走原子写（同目录临时文件 + os.replace + fsync），断电不留半文件；
- history.jsonl 只增不删不改，每行独立 fsync；
- 读取时做迁移校验：版本不符、状态枚举外取值、缺关键字段均报错带路径，绝不静默吞。

红线约束（作者 2026-09-28 澄清，见 renatural/state/lint-baseline.md §4.2）：本模块与调用它的
任何脚本都不得实现 lint 预算、豁免、白名单放行逻辑——lint 计数只可作为排期参考数据被记录，
放行判断一律不在调度器里做。
"""

from __future__ import annotations

import json
import os
import sys
import tempfile
from datetime import datetime
from pathlib import Path
from typing import Any

# ---------------------------------------------------------------------------
# 路径与常量
# ---------------------------------------------------------------------------

REPO_ROOT = Path(__file__).resolve().parents[2]

#: 状态目录。默认落仓库 renatural/state/；测试 / 沙箱可用环境变量 RENATURAL_STATE 重定向，
#: 不改变生产行为（重定向是调度沙箱，不是 lint 豁免通道）。
STATE_DIR = Path(os.environ.get("RENATURAL_STATE") or (REPO_ROOT / "renatural" / "state"))

QUEUE_PATH = STATE_DIR / "queue.json"
LEDGER_PATH = STATE_DIR / "ledger.json"
HISTORY_PATH = STATE_DIR / "history.jsonl"
PREFLIGHT_DIR = STATE_DIR / "preflight"
INVARIANTS_DIR = STATE_DIR / "invariants"

LEDGER_VERSION = 1

#: 账本合法状态全集
STATUSES = ("queued", "blocked", "writing", "review", "ready", "merged", "reverted")

#: queue.json 条目必备字段
QUEUE_FIELDS = ("path", "wave", "batch", "seq", "priority", "reason")

#: ledger.json 单篇记录字段（缺省可选字段读取时补默认）
LEDGER_FIELDS = ("status", "wave", "batch", "family", "attempts", "block_reason", "updated")

#: 状态迁移表：event -> (允许的来源状态集合或 None=任意, 目标状态)。
#: “来源为 None”表示从任意状态发起均可（账本只增不改，宽进严记）。
TRANSITIONS: dict[str, tuple[set[str] | None, str]] = {
    "queue":   (None, "queued"),          # plan.py 初始化入队
    "lock":    ({"queued"}, "queued"),    # batch.py next 锁批：不改状态，只留痕与批号
    "start":   ({"queued"}, "writing"),   # batch.py start：开写
    "review":  ({"writing"}, "review"),   # batch.py done：写手完成，进四走查
    "ready":   ({"review"}, "ready"),     # accept.py：不变量 + humanizer 全零后放行
    "merge":   ({"ready"}, "merged"),     # PR squash 合并后
    "revert":  ({"merged", "ready"}, "reverted"),
    "block":   (None, "blocked"),         # 显式拉黑 / next 时 attempts≥2 自动拉黑
    "requeue": ({"blocked", "reverted"}, "queued"),  # 否决后重新入队（不动 attempts）
}

#: 会累加 attempts 的事件（一轮失败记一次）
ATTEMPT_EVENTS = frozenset({"block"})


class LedgerError(RuntimeError):
    """账本读写 / 迁移校验失败。中文报错，带出错位置，绝不静默吞。"""


# ---------------------------------------------------------------------------
# 基础工具
# ---------------------------------------------------------------------------

def now_iso() -> str:
    """本地时区 ISO 8601 时间戳（含偏移，秒级）。"""
    return datetime.now().astimezone().isoformat(timespec="seconds")


def slug(path: str) -> str:
    """md 文件名去扩展名；index.md 以“<章目录>-index”限定，八部不同名。"""
    p = Path(path)
    if p.stem == "index":
        chapter = p.parent.name or "beginner"
        return f"{chapter}-index"
    return p.stem


def repo_relative(path: str | Path) -> str:
    """把任意输入路径规范成仓库相对路径（批量命令的 path 参数统一入口）。"""
    p = Path(path)
    if p.is_absolute():
        try:
            p = p.relative_to(REPO_ROOT)
        except ValueError:
            raise LedgerError(f"路径不在仓库内：{p}")
    s = p.as_posix().lstrip("./")
    return s


def display(path: str | Path) -> str:
    """给人看的路径：仓库内显示仓库相对路径；RENATURAL_STATE 沙箱重定向到仓库外时显示绝对路径。

    供 invariants / echo_lint / preflight 打印与报错复用（同一实现，不各写一份
    relative_to——沙箱路径不在仓库内会 ValueError 崩，见 2026-09-28 验收）。
    """
    p = Path(path)
    try:
        return p.relative_to(REPO_ROOT).as_posix()
    except ValueError:
        return str(p)


def ensure_dirs() -> None:
    """确保状态目录及 preflight / invariants 子目录存在。"""
    for d in (STATE_DIR, PREFLIGHT_DIR, INVARIANTS_DIR):
        d.mkdir(parents=True, exist_ok=True)


def _atomic_write_text(path: Path, text: str) -> None:
    """同目录临时文件 + os.replace + fsync 的原子写。"""
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(dir=str(path.parent), prefix=f".{path.name}.", suffix=".tmp")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as f:
            f.write(text)
            f.flush()
            os.fsync(f.fileno())
        os.replace(tmp_name, path)
    except BaseException:
        try:
            os.unlink(tmp_name)
        except OSError:
            pass
        raise
    # 目录项也 fsync，防掉电后 rename 丢失（个别文件系统不支持则忽略）
    try:
        dir_fd = os.open(str(path.parent), os.O_RDONLY)
        try:
            os.fsync(dir_fd)
        finally:
            os.close(dir_fd)
    except OSError:
        pass


def _atomic_write_json(path: Path, data: Any) -> None:
    _atomic_write_text(path, json.dumps(data, ensure_ascii=False, indent=2) + "\n")


def atomic_write_json(path: Path, data: Any) -> None:
    """公共原子写：供 preflight / invariants / status 等落各自报告文件复用（同实现，不另发明）。"""
    _atomic_write_json(Path(path), data)


def atomic_write_text(path: Path, text: str) -> None:
    """公共原子写（纯文本，如 STATUS.md）。"""
    _atomic_write_text(Path(path), text)


def _load_json(path: Path, what: str) -> Any:
    if not path.exists():
        return None
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except json.JSONDecodeError as e:
        raise LedgerError(f"{what} 不是合法 JSON：{path}（第 {e.lineno} 行：{e.msg}）")


# ---------------------------------------------------------------------------
# queue.json
# ---------------------------------------------------------------------------

def load_queue() -> list[dict]:
    """读 queue.json 并做结构校验；文件不存在返回空表。"""
    raw = _load_json(QUEUE_PATH, "queue.json")
    if raw is None:
        return []
    if not isinstance(raw, list):
        raise LedgerError(f"queue.json 顶层必须是列表，实际是 {type(raw).__name__}")
    seqs: set[int] = set()
    for i, item in enumerate(raw):
        if not isinstance(item, dict):
            raise LedgerError(f"queue.json 第 {i + 1} 项不是对象")
        for field in QUEUE_FIELDS:
            if field not in item:
                raise LedgerError(f"queue.json 第 {i + 1} 项缺字段 {field}：{item!r}")
        seq = item["seq"]
        if not isinstance(seq, int):
            raise LedgerError(f"queue.json 第 {i + 1} 项 seq 不是整数：{item['path']!r}")
        if seq in seqs:
            raise LedgerError(f"queue.json seq 重复：{seq}（{item['path']!r}）")
        seqs.add(seq)
    return raw


def save_queue(items: list[dict]) -> None:
    """原子写 queue.json（按 seq 升序落盘）。"""
    ordered = sorted(items, key=lambda x: x["seq"])
    _atomic_write_json(QUEUE_PATH, ordered)


# ---------------------------------------------------------------------------
# ledger.json
# ---------------------------------------------------------------------------

def _default_article() -> dict:
    return {
        "status": "queued",
        "wave": "",
        "batch": "",
        "family": "",
        "attempts": 0,
        "block_reason": None,
        "updated": "",
    }


def load_ledger() -> dict:
    """读 ledger.json 并做迁移校验；文件不存在返回空骨架。

    迁移校验口径：
    - version 大于本模块已知的 LEDGER_VERSION：拒绝读取（未来版本可能改结构，降级读会静默丢字段）；
    - version 缺失但形似 v1（有 articles 字典）：按 v1 收编；
    - 单篇记录缺可选字段：补默认值（block_reason=None、attempts=0 等）；
    - status 不在枚举内：报错带路径，绝不静默吞。
    """
    raw = _load_json(LEDGER_PATH, "ledger.json")
    if raw is None:
        return {"version": LEDGER_VERSION, "articles": {}}
    if not isinstance(raw, dict):
        raise LedgerError(f"ledger.json 顶层必须是对象，实际是 {type(raw).__name__}")
    version = raw.get("version")
    if version is None:
        if "articles" in raw:
            version = 1
        else:
            raise LedgerError("ledger.json 缺 version 字段且无法按 v1 识别（无 articles 键）")
    if not isinstance(version, int):
        raise LedgerError(f"ledger.json version 必须是整数，实际是 {version!r}")
    if version > LEDGER_VERSION:
        raise LedgerError(
            f"ledger.json 版本 {version} 高于本工具支持的 {LEDGER_VERSION}，"
            "拒绝降级读取——请升级 scripts/renatural/ 后再操作"
        )
    articles = raw.get("articles")
    if not isinstance(articles, dict):
        raise LedgerError("ledger.json articles 必须是对象（path -> 记录）")
    normalized: dict[str, dict] = {}
    for path, rec in articles.items():
        if not isinstance(rec, dict):
            raise LedgerError(f"ledger.json articles[{path!r}] 不是对象")
        merged = _default_article()
        merged.update(rec)
        if merged["status"] not in STATUSES:
            raise LedgerError(
                f"ledger.json articles[{path!r}] status={merged['status']!r} 不在枚举 "
                f"{STATUSES} 内——账本被外部改动过，请人工核对 renatural/state/"
            )
        if not isinstance(merged["attempts"], int) or merged["attempts"] < 0:
            raise LedgerError(f"ledger.json articles[{path!r}] attempts 必须是非负整数")
        normalized[path] = {k: merged[k] for k in LEDGER_FIELDS}
    return {"version": LEDGER_VERSION, "articles": normalized}


def save_ledger(data: dict) -> None:
    """原子写 ledger.json（补齐 version，保持字段序稳定）。"""
    out = {
        "version": data.get("version", LEDGER_VERSION),
        "articles": {
            path: {k: rec.get(k, _default_article()[k]) for k in LEDGER_FIELDS}
            for path, rec in data.get("articles", {}).items()
        },
    }
    _atomic_write_json(LEDGER_PATH, out)


# ---------------------------------------------------------------------------
# history.jsonl
# ---------------------------------------------------------------------------

def load_history() -> list[dict]:
    """读 history.jsonl（追加只增档案）；逐行校验，坏行报错带行号。"""
    if not HISTORY_PATH.exists():
        return []
    records: list[dict] = []
    with open(HISTORY_PATH, "r", encoding="utf-8") as f:
        for lineno, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            try:
                rec = json.loads(line)
            except json.JSONDecodeError as e:
                raise LedgerError(f"history.jsonl 第 {lineno} 行不是合法 JSON：{e.msg}")
            if not isinstance(rec, dict) or not {"ts", "path", "event"} <= set(rec):
                raise LedgerError(
                    f"history.jsonl 第 {lineno} 行缺 ts/path/event 字段：{line[:120]}"
                )
            records.append(rec)
    return records


def append_history(path: str, event: str, detail: str = "") -> None:
    """向 history.jsonl 追加一条迁移记录（逐行 fsync）。"""
    ensure_dirs()
    rec = {"ts": now_iso(), "path": path, "event": event, "detail": detail or ""}
    with open(HISTORY_PATH, "a", encoding="utf-8") as f:
        f.write(json.dumps(rec, ensure_ascii=False) + "\n")
        f.flush()
        os.fsync(f.fileno())


# ---------------------------------------------------------------------------
# 状态迁移原语
# ---------------------------------------------------------------------------

def transition(path: str, event: str, detail: str = "") -> dict:
    """对单篇执行一次状态迁移：校验 -> 改 ledger -> 原子落盘 -> history 追加。

    参数：
      path   仓库相对路径（可用 repo_relative() 规范化）
      event  TRANSITIONS 里的迁移事件（queue/lock/start/review/ready/merge/revert/block/requeue）
      detail 迁移说明，原样进 history（如拉黑原因、批次号）

    返回该篇更新后的账本记录。任何不合法迁移（未知事件、来源状态不符、篇目不在账）抛
    LedgerError，不留半写状态。
    """
    if event not in TRANSITIONS:
        raise LedgerError(f"未知迁移事件 {event!r}，可选：{sorted(TRANSITIONS)}")
    path = repo_relative(path)
    ledger = load_ledger()
    articles = ledger["articles"]
    if path not in articles:
        raise LedgerError(f"篇目不在账本里：{path}（先跑 plan.py 初始化）")
    rec = articles[path]
    sources, target = TRANSITIONS[event]
    if sources is not None and rec["status"] not in sources:
        raise LedgerError(
            f"{path}：事件 {event!r} 只允许从 {sorted(sources)} 发起，当前 status="
            f"{rec['status']!r}——账本与现实脱节，请先对账 renatural/state/ledger.json"
        )
    rec["status"] = target
    rec["updated"] = now_iso()
    if event in ATTEMPT_EVENTS:
        rec["attempts"] = int(rec.get("attempts", 0)) + 1
    if event == "block":
        rec["block_reason"] = detail or "（未填原因）"
    elif event == "requeue":
        rec["block_reason"] = None
    save_ledger(ledger)
    append_history(path, event, detail)
    return rec


def summary() -> dict:
    """账本概览（各状态篇数），供命令行与后续 status.py 复用。"""
    ledger = load_ledger()
    counts: dict[str, int] = {s: 0 for s in STATUSES}
    for rec in ledger["articles"].values():
        counts[rec["status"]] = counts.get(rec["status"], 0) + 1
    return {"total": len(ledger["articles"]), "by_status": counts}


def require_article(path: str) -> str:
    """校验篇目已入账并返回仓库相对键；不在账即抛错。

    未入账的篇目严禁被工具静默补建账目——preflight / accept 若往 ledger.json 塞
    queue.json 里没有的路径，batch.py next 的账实一致门立刻报脱节，等于毒化调度。
    新篇目一律先 plan.py --merge 入队，再进流水线。
    """
    rel = repo_relative(path)
    if rel not in load_ledger()["articles"]:
        raise LedgerError(
            f"篇目不在账本里：{rel}（先跑 plan.py 初始化；新篇目用 plan.py --merge 入队）"
        )
    return rel


if __name__ == "__main__":
    # 无参数运行时打印账本概览，方便人工速查
    try:
        ensure_dirs()
        info = summary()
    except LedgerError as e:
        print(f"账本读取失败：{e}", file=sys.stderr)
        sys.exit(1)
    print(f"账本篇目：{info['total']}")
    for status in STATUSES:
        n = info["by_status"].get(status, 0)
        if n:
            print(f"  {status:<8} {n}")
