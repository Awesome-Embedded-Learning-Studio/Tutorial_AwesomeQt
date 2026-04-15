---
id: "074"
title: "实现 P0 图表类控件（bar-chart/pie-chart等）"
category: code-library
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016"]
blocks: []
estimated_effort: large
---

## 目标

根据 `catalogs/01-widget.md` 中的 P0 图表类控件清单批量实现，包含 bar-chart、pie-chart、line-chart 等。

## 验收标准

- [ ] 每个 Widget 独立目录，含 `CMakeLists.txt`、`.h`、`.cpp`、`main.cpp`
- [ ] 每个 Widget 独立可编译运行
- [ ] 包含 `demo.gif` 演示动图
- [ ] 支持数据动态更新和动画

## 实施说明

1. 读取 `catalogs/01-widget.md`，提取 P0 优先级的图表类控件清单
2. 为每个控件创建标准目录结构：
   ```
   examples/widgets/charts/<widget-name>/
   ├── CMakeLists.txt
   ├── <WidgetName>.h
   ├── <WidgetName>.cpp
   └── main.cpp
   ```
3. 实现要点：
   - bar-chart：柱状图（支持分组、堆叠、动画）
   - pie-chart：饼图（支持扇区点击、悬浮提示、动画展开）
   - line-chart：折线图（支持多系列、网格线、坐标轴、区域填充）
4. 不依赖 Qt Charts 模块，纯 QPainter 实现
5. 支持数据模型（QAbstractItemModel）或简单的 QList 数据源
6. 支持鼠标交互（悬浮提示、点击选区）

## 涉及文件

- `examples/widgets/charts/bar-chart/`（新建）
- `examples/widgets/charts/pie-chart/`（新建）
- `examples/widgets/charts/line-chart/`（新建）
- `catalogs/01-widget.md`（读取）

## 参考资料

- QPainter 绘制: https://doc.qt.io/qt-6/qpainter.html
- QPainterPath: https://doc.qt.io/qt-6/qpainterpath.html
- Qt Charts 模块: https://doc.qt.io/qt-6/qtcharts-index.html
