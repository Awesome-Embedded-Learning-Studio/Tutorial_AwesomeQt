---
title: "03 · QtWidgets 传统界面（专家）"
description: "深入 QtWidgets 源码：布局尺寸分配算法、QApplication::notify 事件分发全流程、Model/View 解耦机制、QSS 解析与应用、控件渲染脏矩形与 Backing Store、对话框模态事件循环、主窗口 Dock 布局引擎、BSP 树碰撞检测、动画时间轴、MDI 子窗口管理，以及 64 个控件源码拆解。共 74 篇。"
---

# 03 · QtWidgets 传统界面（专家）

> 规划中（74 篇），敬请期待...

## 章节规划

### 主题能力篇（10 篇）

- **01 布局系统源码** — `expandingDirections()` 空间分配、`QBoxLayout::setGeometry()` 分配循环、Stretch Factor 权重
- **02 事件分发源码** — `QApplication::notify()` 过滤器链、鼠标坐标转换、焦点系统、`QShortcut` 拦截优先级
- **03 Model/View 源码** — `notifyIndexObservers()` 变更通知、`QItemSelectionModel` 区间合并、`QPersistentModelIndex` 重映射
- **04 QSS 源码** — `QCss::Parser` 词法语法分析、`QStyleSheetStyle` 叠加逻辑、选择器特异性计算
- **05 控件渲染源码** — `drawWidget()` 调用链、`QBackingStore` 后端、脏矩形合并、`WA_OpaquePaintEvent` 影响
- **06 对话框源码** — `QDialog::exec()` 嵌套 `QEventLoop`、模态 Widget 屏蔽机制
- **07 主窗口源码** — `QMainWindowLayout` 自定义引擎、Dock 区域 BSP 树、工具栏溢出菜单
- **08 图形视图源码** — `QGraphicsSceneBspTree` 二叉空间分割、`collidingItems()` BVH 查询
- **09 动画框架源码** — `QAnimationTimer` 帧调度、`QVariantAnimation` 插值路径、暂停时间补偿
- **10 MDI 源码** — `QMdiAreaPrivate` Z-order 栈、子窗口拖动调整、`TabbedView` 标签页集成

### 控件速查篇（64 篇）

64 个控件的源码拆解，每个控件深入 `QWidgetPrivate` 内部状态、`paintEvent` 绘制实现、`QStyle::drawControl` 平台风格适配、性能关键路径。
