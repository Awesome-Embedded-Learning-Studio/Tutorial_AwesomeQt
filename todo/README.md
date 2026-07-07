# AwesomeQt · 待办

> 项目待办入口。按工作区分文件，翻到对应区开干。完成一项把 `[ ]` 改 `[x]` + 日期。

## 速览

```
入门 137✅ · 进阶 134✅ · 专家 2/102（01·02 COW 已审结，见 expert.md）
实例库 widget 13/13 ✅收齐（status-led✅ + toggle-switch✅ + circle-progress✅ + speed-meter✅ + range-slider✅ + line-chart✅ + editable-table✅ + checkbox-tree✅ + checkbox-list✅ + log-viewer✅ + password-edit✅ + ip-edit✅ + fade-animation✅），app image-viewer✅ + json-editor✅ + sqlite-browser✅ + serial-tool✅ + network-tool✅ + tetris✅ + cpu-memory-monitor✅(7 件整机成品·已合 main PR#16 · Windows 路径待作者实机复验) · model/industrial 骨架已立
examples 275✅ · 基建 P0✅ 基本清完
```

## 接力点（下次会话从这里接 · 2026-07-07 更）

- **已合入 main**：
  - **PR#14（fcde408）** — widget 栏 13/13 全收齐（status-led/toggle-switch/circle-progress/speed-meter/range-slider/line-chart/editable-table/checkbox-tree/checkbox-list/log-viewer/password-edit/ip-edit/fade-animation）
  - **PR#16（764f035）** — app 栏 7 件整机成品（image-viewer/json-editor/sqlite-browser/serial-tool/network-tool/tetris/cpu-memory-monitor）+ 双文档 + app/CMakeLists 纳入 + 累计清单 instance-batch-log.md
  - **PR#17（de7eeb4）** — 站点样式大更新（侧栏拖拽 + Qt 绿身份 + 字号切换 + 代码折叠 + 阅读进度条）

  每件实例构建门零 warning + offscreen 验证 + 对抗 review 修正 + Full 导览 + Handbook。**widget/app 双范式 + 双文档范式已立**。
- **下一步**：app 栏暂收（7 件完整）；转 model 栏放量（17 件照 undo-redo 范式）/ industrial hmi-dashboard 复用 widget 链 / 专家层另线推进；cpu-memory-monitor 的 Windows 路径需作者 Windows 实机复验
- **挂账**：05-other-modules ~25 篇缺踩坑段——按「不编坑」原则，等真写到该模块再补真坑（memory: no-fabricated-pitfalls）
- ⚠ **作者会在终端并行 commit/push/merge**：AI 改文件前先 `git status`；提交 / push 全归作者；commit / PR **不带任何 AI 署名**（memory: no-ai-attribution / user-handles-all-pushes）

## 工作区 → 文件

| 工作区 | 文件 | 内容 | 当下 |
|---|---|---|---|
| 专家层 | [expert.md](expert.md) | 102 篇源码拆解（每篇带 qt_src 行号证据） | ✅ 01·02 COW 已审结 → 下一篇 qobject |
| 实例库 | [instance-library.md](instance-library.md) | widget/app/model/industrial 成品 + 两套文档 | widget 13/13✅ · app 7/7✅（均合 main）· model 1/18（骨架）· industrial 1/1（骨架） |
| 基建 | [infra.md](infra.md) | P0/P0.5/P1/P4 + widget 化简 + 地基债 | ✅ P0 基本清完 · 剩 sidebar/结构漂移 |
| embedded | [embedded.md](embedded.md) | Layer1 公共基础 + Layer2 板级 | ⚠ 生产方式待定 |
| 延后 | [parked.md](parked.md) | community/translation/interactive/CI门… | 不投入 |

## 当下优先

- 专家层：01·02 已审结 → 下一篇 01-qobject-meta-system（D1·审核门限·一次一篇）
- 实例库：widget 13/13 + app 7/7 全合 main（PR#14/#16）→ 转 model 栏放量（17 件照 undo-redo 范式）
- 基建：P0✅ 基本清完（死链 + 需要注意的是×30 + 风格违例已入 main）→ 剩 专家 sidebar 收敛 + 入门结构漂移

## gate

- ✅ 01·02 篇已审结（2026-06-13，gate 实质过）→ expert.md
- ✅ status-led 中等档标杆 + toggle-switch 已合入 main，构建门通
- embedded 生产方式待定 → embedded.md

## 全量实例清单（参考，非待办）

选下一波时查 [registries/](registries/)（widget 500 / app 200 / model 317 / qml 52 / industrial 6）。
