---
title: "06 · QML 现代界面（专家）"
description: "深入 QML 引擎源码：绑定引擎表达式求值与依赖追踪、Controls 样式插件加载、C++/QML 类型注册与元对象桥接、动画引擎时间轴与渲染线程、组件编译器与 AOT、模型视图虚拟化与回收池、WorkerScript 独立 JS 引擎，以及 V4 JIT/GC 和 QQmlEngine 类型桥接 2 个专家专属章节。共 9 篇。"
---

# 06 · QML 现代界面（专家）

> 规划中（9 篇），敬请期待...

## 章节规划

- **01 QML 绑定引擎源码** — `QQmlBinding` 字节码编译、`QQmlNotifier` 依赖追踪、延迟批处理、`QQmlPropertyData` 属性元数据
- **02 Qt Quick Controls 源码** — `QQuickStylePlugin` 样式插件加载、delegate 创建路径、Theme/Palette 继承覆盖
- **03 C++/QML 类型系统源码** — `QQmlTypeRegistry` 注册表、`QML_ELEMENT` 宏展开、`QQmlContext` 查找链、`createQmlObject()` 动态实例化
- **04 QML 动画引擎源码** — `QQuickAnimator` 渲染线程执行、`QUnifiedTimer` 全局时钟、`SmoothedAnimation` 弹簧-阻尼积分
- **05 QML 组件编译器** — `.qml` → `QQmlComponent` 编译单元、作用域隔离、`qmldir` 解析、Quick Compiler AOT 原理
- **06 QML 模型视图源码** — `QQuickListView` delegate 创建/回收池、`QQmlDelegateModel` 分组过滤、可见区域增量计算
- **07 WorkerScript 源码** — 独立 `QQmlEngine` 实例、`sendMessage` 序列化深拷贝、内存隔离边界
- **08【专属】V4 JavaScript 引擎** — 解释器/Baseline JIT/Optimizing JIT 三层、`Heap::Base` 标记清除 GC、NaN boxing 值表示、QML-V4 双向引用
- **09【专属】QQmlEngine 类型桥接** — `QQmlData` 附着数据、C++ 属性 V4 accessor 生成、`QMetaObject`/`QQmlTypeData` 双向查找、信号跨边界类型转换
