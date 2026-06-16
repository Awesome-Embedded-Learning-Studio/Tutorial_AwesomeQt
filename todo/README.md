# AwesomeQt · 待办

> 项目待办入口。按工作区分文件，翻到对应区开干。完成一项把 `[ ]` 改 `[x]` + 日期。

## 速览

```
入门 137✅ · 进阶 134✅ · 专家 2/102（01·02 COW 已审结，见 expert.md）
实例库 widget 2/13（status-led✅ + toggle-switch✅），app/model/industrial 空
examples 275✅ · 基建 P0✅ 基本清完
```

## 接力点（下次会话从这里接 · 2026-06-16 更）

- **已合入 main（f8349b0）**：status-led 中等档标杆(PR#10) · D3 sweep「需要注意的是」×30 + 死链(PR#11) · Task A 风格违例(值得注意的是/一般来说/大概) · toggle-switch 控件 + 双文档
- 工作区干净、本地 `main == origin/main`
- **下一步二选一**：
  - ① 专家篇 `01-qobject-meta-system`（慢·审核门限·一次一篇·A/B 双 Agent 取证，见 [.claude/production-paradigm.md](../.claude/production-paradigm.md) §2）
  - ② 实例库续 `circle-progress`（widget 撑场批）或破 `app/image-viewer`（快·D2 放量·照 status-led/toggle-switch 模板，构建门 + 双文档）
- **挂账**：05-other-modules ~25 篇缺踩坑段——按「不编坑」原则，等真写到该模块再补真坑（memory: no-fabricated-pitfalls）
- ⚠ **作者会在终端并行 commit/push/merge**（本会话发生过）：AI 改文件前先 `git status`；提交 / push 全归作者；commit / PR **不带任何 AI 署名**（memory: no-ai-attribution / user-handles-all-pushes）

## 工作区 → 文件

| 工作区 | 文件 | 内容 | 当下 |
|---|---|---|---|
| 专家层 | [expert.md](expert.md) | 102 篇源码拆解（每篇带 qt_src 行号证据） | ✅ 01·02 COW 已审结 → 下一篇 qobject |
| 实例库 | [instance-library.md](instance-library.md) | widget/app/model/industrial 成品 + 两套文档 | widget 2/13（status-led + toggle-switch✅）· app/model/industrial 空 |
| 基建 | [infra.md](infra.md) | P0/P0.5/P1/P4 + widget 化简 + 地基债 | ✅ P0 基本清完 · 剩 sidebar/结构漂移 |
| embedded | [embedded.md](embedded.md) | Layer1 公共基础 + Layer2 板级 | ⚠ 生产方式待定 |
| 延后 | [parked.md](parked.md) | community/translation/interactive/CI门… | 不投入 |

## 当下优先

- 专家层：01·02 已审结 → 下一篇 01-qobject-meta-system（D1·审核门限·一次一篇）
- 实例库：widget 2/13 → 续 circle-progress；app/model/industrial 三栏零起步待破
- 基建：P0✅ 基本清完（死链 + 需要注意的是×30 + 风格违例已入 main）→ 剩 专家 sidebar 收敛 + 入门结构漂移

## gate

- ✅ 01·02 篇已审结（2026-06-13，gate 实质过）→ expert.md
- ✅ status-led 中等档标杆 + toggle-switch 已合入 main，构建门通
- embedded 生产方式待定 → embedded.md

## 全量实例清单（参考，非待办）

选下一波时查 [registries/](registries/)（widget 500 / app 200 / model 317 / qml 52 / industrial 6）。
