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
路径 `widget/<家族>/<控件名>/`。①status-led✅中等档标杆(颜色过渡+呼吸+Q_PROPERTY全+双文档pilot待验格式) ②toggle-switch ③circle-progress ④speed-meter ⑤range-slider ⑥line-chart(纯QPainter) ⑦editable-table ⑧checkbox-tree ⑨checkbox-list ⑩log-viewer ⑪password-edit ⑫ip-edit(P1) ⑬fade-animation(P1)。未覆盖9族首波不碰：calendar/datetime/opengl/specialized/network-widget/data-display/multimedia/map/print。

### app 栏（7旗舰+1拉伸·撑「真库不是片段」）
路径 `app/<类目>/<应用名>/`。image-viewer★5 / sqlite-browser★5 / json-editor★4 / network-tool★4-5 / cpu-memory-monitor★4 / serial-tool★4-5(补入·嵌入式最高频) / tetris★4-5 / ~~audio-visualizer~~(拉伸·须先做模拟数据发生器)。

### model 栏（18·先打穿 undo-redo-framework）
⚠ model 目录零参考——先打穿 undo-redo-framework 定范式（目录布局/是否进 AwesomeQt 命名空间/demo 形态待定），其余放量。
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
②撑场批：status-led→toggle→circle→speed-meter→line-chart + image-viewer/sqlite-browser/serial-tool
③覆盖批：password/ip/range-slider/editable-table/checkbox-tree/list/log-viewer/fade + json-editor/network-tool/cpu-mem
④model reference：undo-redo-framework 1个→定范式→其余17放量
⑤收尾：tetris；audio-visualizer(须先模拟数据发生器)
⑥industrial pilot：widget 链落地后→hmi-dashboard 一个

## 规模（参考）
~1075 = widget 500 + app 200 + model 317 + qml 52 + industrial 6。全量清单见 [registries/](registries/)（选下一波时查）。qml(52) 已延后。
