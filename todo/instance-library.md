# 实例库（widget/app/model/industrial）

> 拿来学结构 / 练手 / 做发现的教学成品（非可发布产品）。文档出两套：成品导览（架构/决策/踩坑，不内联完整 code）+ 手搓手册（分步任务/提示/检查点，repo 成品当答案钥匙）。构建须 `cmake -B build && cmake --build build` 直接成功。

## 编码规范

- **C++17**，仅 Qt 6 API；头文件 `#pragma once`；`override` 必加；信号槽用函数指针语法（非 SIGNAL/SLOT 宏）；range-based for（非 Q_FOREACH）；不用废弃 API（QString::null、QRegExp）。
- 命名：目录/项目 kebab-case（`circle-progress`）、类名 PascalCase、头源与类名一致。
- CMake：`find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)`；第三方用 FetchContent。
- widget/ 库式构建：`AwesomeQt::` 命名空间 + Q_PROPERTY/Q_ENUM + STATIC 库 + demo 子目录 + root-owns-config（详见 [infra.md](infra.md) widget 化简）。
- 参考来源：TTK(TTKWidgetTools LGPL) / QWD(QWidgetDemo) / MTC(MyTestCode) / GH 社区 / NEW 原创。代码为原创重写，与参考项目实现独立。

## 首波 39 件（已定 2026-06-12）

### 6 项已拍板
1. ✅ statusled 降回 C++17（widget 化简时已落，C++17 全局设在 widget/CMakeLists.txt）
2. 控件命名统一 kebab：`widget/statusled/` → `widget/status-led/`
3. model 先打穿 undo-redo-framework 作 reference，定范式再放量其余 17
4. automotive-cluster 走纯 QtWidgets 自绘（不碰 QML，否则延后 QML Part6）
5. industrial 整机允许 QtCharts，widget 单控件纯自绘
6. industrial 链接 widget 库 .so 复用（speed-meter/circle-progress/line-chart 先落地）

### widget 栏（13·每族代表·自绘递进链）
路径 `widget/<家族>/<控件名>/`。①status-led✅中等档标杆(颜色过渡+呼吸+Q_PROPERTY全+双文档pilot待验格式) ②toggle-switch✅(2026-06-16,滑动动画+拖动/点击+Q_PROPERTY配色+双文档) ③circle-progress✅(2026-06-25·value/progress解耦+drawArc) ④speed-meter✅(2026-06-25·动画指针+刻度) ⑤range-slider✅(2026-06-25·双柄拖拽) ⑥line-chart✅(纯QPainter·2026-06-25) ⑦editable-table✅(2026-06-25·委托校验+数据往返) ⑧checkbox-tree✅(2026-06-25·三态+父子联动) ⑨checkbox-list✅(2026-06-25·QListWidget勾选+批量守卫) ⑩log-viewer✅(2026-06-25·级别染色+裁旧) ⑪password-edit✅(2026-06-25·显隐+强度) ⑫ip-edit✅(2026-06-25·4段跳焦+0-255校验) ⑬fade-animation✅(2026-06-25·OpacityEffect淡入淡出)。**widget 栏 13/13 收齐**。未覆盖9族首波不碰：calendar/datetime/opengl/specialized/network-widget/data-display/multimedia/map/print。

### app 栏（7旗舰+1拉伸·撑「真库不是片段」）
路径 `app/<类目>/<应用名>/`。①image-viewer✅(整机破零标杆·自定义画布+QTransform+翻页跳坏图+幻灯片) ②json-editor✅(01-dev-tools·编辑/格式化/校验/树递归) ③sqlite-browser✅(10-database-tools·打开 db/表列表/可编辑表格/任意 SQL) ④serial-tool✅(02-network-tools·串口配置/收发/Hex-ASCII/错误收敛·嵌入式高频) ⑤network-tool✅(02-network-tools·TCP Server/Client + UDP 调试·loopback 可 offscreen 验) ⑥tetris✅(08-games·俄罗斯方块自绘·7 形态/旋转/消行/计分) ⑦cpu-memory-monitor✅(04-system-tools·CPU/内存进度条+历史曲线·跨平台·**Windows 路径 #ifdef 隔离标「尚未验证」**) / ~~audio-visualizer~~(拉伸·须先做模拟数据发生器)。**app 栏 7 件整机成品（2026-07-07 已合 main · PR#16）**，整机应用范式(整机 demo 非库式) + 双文档范式已立。

### model 栏（18·范式已定 2026-06-16）
✅ 三范式已定（undo-redo-framework 骨架打穿）：目录布局 `model/<NN-类目>/<名>/include|src|demo`（与 widget 同构）/ 库类进 `AwesomeQt::` / demo 形态 = 库式 STATIC + 独立 demo。undo-redo 骨架已落（`move_command` STATIC + QUndoStack/QUndoView demo，构建门+冒烟通），深度成品待 D2 放量，其余 17 照范式复刻。
- MV链(3)：proxy-model · custom-model · tree-drag-move
- 设计模式链(15)：undo-redo-framework⭐ref · pimpl-pattern · shared-data-pattern · observer-pattern · sidebar-navigation · theme-system · animation-stack · ide-layout · window-single-instance · config-manager · pdf-export · variant-property-editor(降级·先手写轻量) · state-machine · frameless-window · toast-notification
- 边界：pimpl/shared-data/observer/state-machine 对齐专家层源码——文档须明确「练手实例 vs 专家源码行号」边界，别和 [expert.md](expert.md) 03/04/19 篇重复。

### industrial 栏（首波 pilot 只1）
- hmi-dashboard ✅唯一pilot：温度/压力/流量自绘仪表+趋势曲线+报警，复用 widget 递进链；industrial/ 目录首建脚手架；整机允许 QtCharts
- scada-panel ⏸缓：SerialBus+Modbus 重 + embedded 未就绪——先做「模拟从站」版
- automotive-cluster ⏸缓/降级：纯 Widgets 自绘（套 speed-meter），排 hmi 之后

### 产出优先序（分批）

> **[格式 gate]** ✅ status-led 已产 Full 导览 + Handbook（入口+01/02/03+troubleshooting）两套样板（2026-06-14，落 `tutorial/engineering/instances/widget/status-led/`）→ 待作者验格式放量后续

①✅已完成：status-led 基建化简(降C++17+重命名+STATIC库) + 深度改造(中等档：颜色过渡 QPropertyAnimation + 呼吸 QVariantAnimation + Q_PROPERTY 全 + minimumSizeHint) + 双文档 pilot(成品导览 + 手搓手册 5 文件) → 作者验过格式再放量
②✅toggle-switch 已落(2026-06-16，代码+构建门+冒烟+Full+Handbook双文档)
②b✅自绘链撑场批(2026-06-25·分支 instance/widget-painter-batch)：circle-progress+speed-meter+range-slider+line-chart 四件，全 STATIC 库+demo+构建门绿(零warning)+offscreen冒烟+Full导览+Handbook 5文件。Workflow 并行产(speed/range/line)，circle-progress 因 429 限流挂→手补。待作者抽审放量
②c✅model/view 批(2026-06-25·同分支)：editable-table(委托校验+数据往返)+checkbox-tree(三态+父子联动) 两件，全 STATIC 库+demo+构建门绿(零warning)+offscreen冒烟+Full+Handbook。Workflow 并行产(本轮无 429)。待作者抽审
②d✅input/display 批(2026-06-25·同分支)：checkbox-list(QListWidget勾选+批量守卫)+log-viewer(QPlainTextEdit级别染色+裁旧)+password-edit(显隐+强度) 三件，全 STATIC 库+demo+构建门绿+offscreen冒烟+Full+Handbook。本批起 code Agent 预 clang-format，行号不漂+提交一把过。待作者抽审
②e✅widget 收尾批(2026-06-25·同分支)：ip-edit(4段跳焦+0-255校验)+fade-animation(QGraphicsOpacityEffect淡入淡出) 两件（P1 也做了），全 STATIC 库+demo+构建门绿+offscreen冒烟+Full+Handbook。**widget 栏 13/13 全部完成**。待作者抽审
②f✅app 栏破零(2026-06-27·分支 instance/app-image-viewer)：image-viewer 整机成品（自定义 ImageView 画布 paintEvent+QTransform 一次合成缩放/旋转/居中 + QScrollArea widgetResizable=false 滚动 + 同目录翻页循环跳坏图 + 幻灯片全屏恢复 maximized）。构建门零 warning + offscreen 像素验证(旋转 90° 左蓝右红方向正确) + 对抗 review(3 维度)修正 6 处 + Full 导览 + Handbook 5 文件。**app 栏整机范式 + 双文档范式首立**。✅ 2026-07-07 已合 main（PR#16 764f035）
③app/model 放量：json-editor/network-tool/cpu-mem/serial-tool/sqlite-browser（image-viewer 已破零见②f）
④✅model reference 骨架已落(2026-06-16)：undo-redo-framework 库式骨架(`move_command` STATIC + demo) + 三范式已定 → 其余 17 照范式放量
⑤收尾：tetris；audio-visualizer(须先模拟数据发生器)
⑥industrial pilot：widget 链落地后→hmi-dashboard 成品化（骨架已立，复用契约见下）
⑦✅三栏架构骨架已落(2026-06-16)：app/model/industrial 顶层 CMake + README + 各 1 种子(image-viewer / undo-redo-framework / hmi-dashboard)，构建门 + offscreen 冒烟全通；industrial 跨栏复用契约写明（骨架零依赖，成品期待 widget 链 speed-meter/circle-progress/line-chart 就位再接 link）

## 规模（参考）
~1075 = widget 500 + app 200 + model 317 + qml 52 + industrial 6。全量清单见 [registries/](registries/)（选下一波时查）。qml(52) 已延后。
