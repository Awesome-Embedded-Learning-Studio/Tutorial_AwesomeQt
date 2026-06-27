# AwesomeQt · 待办

> 项目待办入口。按工作区分文件，翻到对应区开干。完成一项把 `[ ]` 改 `[x]` + 日期。

## 速览

```
入门 137✅ · 进阶 134✅ · 专家 2/102（01·02 COW 已审结，见 expert.md）
实例库 widget 13/13 ✅收齐（status-led✅ + toggle-switch✅ + circle-progress✅ + speed-meter✅ + range-slider✅ + line-chart✅ + editable-table✅ + checkbox-tree✅ + checkbox-list✅ + log-viewer✅ + password-edit✅ + ip-edit✅ + fade-animation✅），app image-viewer✅ + json-editor✅ + sqlite-browser✅(3 件整机成品) · model/industrial 骨架已立
examples 275✅ · 基建 P0✅ 基本清完
```

## 接力点（下次会话从这里接 · 2026-06-27 更）

- **已合入 main（fcde408 / PR#14）**：widget 栏 13/13 全收齐（status-led/toggle-switch/circle-progress/speed-meter/range-slider/line-chart/editable-table/checkbox-tree/checkbox-list/log-viewer/password-edit/ip-edit/fade-animation）
- **分支 `instance/app-image-viewer`（本机已 commit 未 push）**：app 栏 3 件整机成品——①image-viewer(自定义画布+QTransform+翻页跳坏图+幻灯片) ②json-editor(01-dev-tools·编辑/格式化/校验/树递归) ③sqlite-browser(10-database-tools·打开 db/表列表/可编辑表格/任意 SQL)。每件构建门零 warning + offscreen 验证 + 对抗 review 修正(image-viewer 6 处/json-editor 3 bug/sqlite 2 bug) + Full 导览 + Handbook。**app 栏 3 件整机范式 + 双文档范式已立**。待作者抽审放量
- 工作区：分支上 3 件 app 整机 + 双文档 + app/CMakeLists 纳入 + todo 更新（含累计清单 instance-batch-log.md）；`main` 含 widget 13/13
- **下一步（审完合入后）**：app 栏续 serial-tool(SerialPort·嵌入式高频)/network-tool/cpu-memory-monitor；或转 model 栏放量（17 件照 undo-redo 范式）；industrial hmi-dashboard 复用 widget 链；专家层另线推进
- **挂账**：05-other-modules ~25 篇缺踩坑段——按「不编坑」原则，等真写到该模块再补真坑（memory: no-fabricated-pitfalls）
- ⚠ **作者会在终端并行 commit/push/merge**：AI 改文件前先 `git status`；提交 / push 全归作者；commit / PR **不带任何 AI 署名**（memory: no-ai-attribution / user-handles-all-pushes）

## 工作区 → 文件

| 工作区 | 文件 | 内容 | 当下 |
|---|---|---|---|
| 专家层 | [expert.md](expert.md) | 102 篇源码拆解（每篇带 qt_src 行号证据） | ✅ 01·02 COW 已审结 → 下一篇 qobject |
| 实例库 | [instance-library.md](instance-library.md) | widget/app/model/industrial 成品 + 两套文档 | widget 6/13（+circle/speed/range/line✅·分支待审）· app/model/industrial 骨架已立 |
| 基建 | [infra.md](infra.md) | P0/P0.5/P1/P4 + widget 化简 + 地基债 | ✅ P0 基本清完 · 剩 sidebar/结构漂移 |
| embedded | [embedded.md](embedded.md) | Layer1 公共基础 + Layer2 板级 | ⚠ 生产方式待定 |
| 延后 | [parked.md](parked.md) | community/translation/interactive/CI门… | 不投入 |

## 当下优先

- 专家层：01·02 已审结 → 下一篇 01-qobject-meta-system（D1·审核门限·一次一篇）
- 实例库：widget 13/13✅已合 main(PR#14) · app image-viewer✅破零(分支 instance/app-image-viewer 待审) → 续 app sqlite-browser/serial-tool 或 model 栏放量
- 基建：P0✅ 基本清完（死链 + 需要注意的是×30 + 风格违例已入 main）→ 剩 专家 sidebar 收敛 + 入门结构漂移

## gate

- ✅ 01·02 篇已审结（2026-06-13，gate 实质过）→ expert.md
- ✅ status-led 中等档标杆 + toggle-switch 已合入 main，构建门通
- embedded 生产方式待定 → embedded.md

## 全量实例清单（参考，非待办）

选下一波时查 [registries/](registries/)（widget 500 / app 200 / model 317 / qml 52 / industrial 6）。
