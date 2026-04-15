---
id: "068"
title: "RK3568: QML 仪表盘与 GPU 加速"
category: embedded
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["066"]
blocks: []
estimated_effort: medium
---

## 目标

编写 Qt Quick 在 RK3568 上的性能优化教程，以 QML 仪表盘为实际案例。

## 验收标准

- [ ] QML 仪表盘示例实现（速度表、转速表、温度表）
- [ ] GPU 加速渲染分析
- [ ] QML 性能优化技巧（Loader、缓存、异步加载）
- [ ] Scene Graph 渲染管线调优
- [ ] 帧率稳定性测试

## 实施说明

1. **仪表盘实现**：
   - 使用 Qt Quick Canvas 2D 或 `QQuickPaintedItem` 绘制仪表盘
   - 动画效果使用 `NumberAnimation`/`SpringAnimation`
   - 数据绑定与实时更新
2. **GPU 加速**：
   - 确认 Mali-G52 GPU 正常工作
   - 使用 `QSG_RHI_BACKEND=vulkan` 或 `opengl` 测试
   - 分析 Scene Graph 渲染批次
3. **优化技巧**：
   - 使用 `layer.enabled: true` 缓存静态内容
   - `Loader` 延迟加载非可见组件
   - 避免在动画回调中创建/销毁对象
   - 使用 `QSG_RENDER_TIMING=1` 分析渲染耗时
4. 目标帧率：60fps 稳定输出

## 涉及文件

- `document/tutorials/embedded/rk3568-qml-dashboard-gpu.md`（新建）
- `examples/embedded/rk3568/qml-dashboard/`（示例代码）

## 参考资料

- Qt Quick 性能: https://doc.qt.io/qt-6/qtquick-performance.html
- Qt Scene Graph: https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph.html
