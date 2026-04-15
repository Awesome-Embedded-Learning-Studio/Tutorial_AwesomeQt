---
id: "070"
title: "实现 P0 按钮类控件（toggle-switch/radio-card等）"
category: code-library
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016", "027"]
blocks: []
estimated_effort: large
---

## 目标

根据 `catalogs/01-widget.md` 中的 P0 按钮类控件清单批量实现，包含 toggle-switch、radio-card、icon-button、gradient-button 等。

## 验收标准

- [ ] 每个 Widget 独立目录，含 `CMakeLists.txt`、`.h`、`.cpp`、`main.cpp`
- [ ] 每个 Widget 独立可编译运行
- [ ] 包含 `demo.gif` 演示动图
- [ ] 遵循项目代码规范（命名、注释、格式）

## 实施说明

1. 读取 `catalogs/01-widget.md`，提取 P0 优先级的按钮类控件清单
2. 为每个控件创建标准目录结构：
   ```
   examples/widgets/buttons/<widget-name>/
   ├── CMakeLists.txt
   ├── <WidgetName>.h
   ├── <WidgetName>.cpp
   └── main.cpp
   ```
3. 实现要点：
   - 继承 `QWidget` 或 `QAbstractButton`
   - 支持 `paintEvent` 自定义绘制
   - 支持样式自定义（颜色、大小、动画）
   - 提供完整的信号槽接口
4. 每个控件的 `main.cpp` 作为独立 demo，展示所有功能
5. 录制 `demo.gif` 展示交互效果

## 涉及文件

- `examples/widgets/buttons/toggle-switch/`（新建）
- `examples/widgets/buttons/radio-card/`（新建）
- `examples/widgets/buttons/icon-button/`（新建）
- `examples/widgets/buttons/gradient-button/`（新建）
- `catalogs/01-widget.md`（读取）

## 参考资料

- Qt 自定义 Widget: https://doc.qt.io/qt-6/widgets-tutorials-addressbook.html
- QAbstractButton: https://doc.qt.io/qt-6/qabstractbutton.html
