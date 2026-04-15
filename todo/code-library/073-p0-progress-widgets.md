---
id: "073"
title: "实现 P0 进度类控件（circle-progress/step-progress等）"
category: code-library
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: []
estimated_effort: medium
---

## 目标

根据 `catalogs/01-widget.md` 中的 P0 进度类控件清单批量实现，包含 circle-progress、step-progress、wave-progress 等。

## 验收标准

- [ ] 每个 Widget 独立目录，含 `CMakeLists.txt`、`.h`、`.cpp`、`main.cpp`
- [ ] 每个 Widget 独立可编译运行
- [ ] 包含 `demo.gif` 演示动图
- [ ] 支持动画过渡效果

## 实施说明

1. 读取 `catalogs/01-widget.md`，提取 P0 优先级的进度类控件清单
2. 为每个控件创建标准目录结构：
   ```
   examples/widgets/progress/<widget-name>/
   ├── CMakeLists.txt
   ├── <WidgetName>.h
   ├── <WidgetName>.cpp
   └── main.cpp
   ```
3. 实现要点：
   - circle-progress：环形进度条（可自定义颜色、宽度、动画）
   - step-progress：步骤进度条（水平/垂直，支持节点自定义）
   - wave-progress：水波纹进度效果
4. 使用 `QPropertyAnimation` 实现平滑过渡动画
5. 支持通过样式表自定义外观

## 涉及文件

- `examples/widgets/progress/circle-progress/`（新建）
- `examples/widgets/progress/step-progress/`（新建）
- `examples/widgets/progress/wave-progress/`（新建）
- `catalogs/01-widget.md`（读取）

## 参考资料

- QProgressBar: https://doc.qt.io/qt-6/qprogressbar.html
- QPropertyAnimation: https://doc.qt.io/qt-6/qpropertyanimation.html
- Qt 自定义绘制: https://doc.qt.io/qt-6/qpainter.html
