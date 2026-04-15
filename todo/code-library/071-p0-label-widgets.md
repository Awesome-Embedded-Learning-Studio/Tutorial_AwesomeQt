---
id: "071"
title: "实现 P0 标签类控件（status-led/info-badge等）"
category: code-library
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016", "029"]
blocks: []
estimated_effort: medium
---

## 目标

根据 `catalogs/01-widget.md` 中的 P0 标签类控件清单批量实现，包含 status-led、info-badge、typing-label、elided-label 等。

## 验收标准

- [ ] 每个 Widget 独立目录，含 `CMakeLists.txt`、`.h`、`.cpp`、`main.cpp`
- [ ] 每个 Widget 独立可编译运行
- [ ] 包含 `demo.gif` 演示动图
- [ ] 遵循项目代码规范

## 实施说明

1. 读取 `catalogs/01-widget.md`，提取 P0 优先级的标签类控件清单
2. 为每个控件创建标准目录结构：
   ```
   examples/widgets/labels/<widget-name>/
   ├── CMakeLists.txt
   ├── <WidgetName>.h
   ├── <WidgetName>.cpp
   └── main.cpp
   ```
3. 实现要点：
   - status-led：圆形 LED 指示灯（多色、闪烁动画）
   - info-badge：角标/徽章（数字角标、状态点）
   - typing-label：打字机效果文本
   - elided-label：自动省略长文本标签
4. 每个 `main.cpp` 展示控件的各种状态和配置

## 涉及文件

- `examples/widgets/labels/status-led/`（新建）
- `examples/widgets/labels/info-badge/`（新建）
- `examples/widgets/labels/typing-label/`（新建）
- `examples/widgets/labels/elided-label/`（新建）
- `catalogs/01-widget.md`（读取）

## 参考资料

- QLabel: https://doc.qt.io/qt-6/qlabel.html
- QFontMetrics 省略: https://doc.qt.io/qt-6/qfontmetrics.html#elidedText
