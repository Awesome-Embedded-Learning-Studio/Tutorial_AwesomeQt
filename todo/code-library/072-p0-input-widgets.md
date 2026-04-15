---
id: "072"
title: "实现 P0 输入类控件（search-edit/ip-input等）"
category: code-library
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["016", "028"]
blocks: []
estimated_effort: medium
---

## 目标

根据 `catalogs/01-widget.md` 中的 P0 输入类控件清单批量实现，包含 search-edit、ip-input、password-strength、tag-input 等。

## 验收标准

- [ ] 每个 Widget 独立目录，含 `CMakeLists.txt`、`.h`、`.cpp`、`main.cpp`
- [ ] 每个 Widget 独立可编译运行
- [ ] 包含 `demo.gif` 演示动图
- [ ] 输入验证逻辑完整

## 实施说明

1. 读取 `catalogs/01-widget.md`，提取 P0 优先级的输入类控件清单
2. 为每个控件创建标准目录结构：
   ```
   examples/widgets/inputs/<widget-name>/
   ├── CMakeLists.txt
   ├── <WidgetName>.h
   ├── <WidgetName>.cpp
   └── main.cpp
   ```
3. 实现要点：
   - search-edit：带搜索图标、清除按钮、下拉建议的搜索框
   - ip-input：IP 地址格式化输入，自动跳转下一段
   - password-strength：密码输入 + 强度指示条
   - tag-input：标签输入框，支持回车添加、删除标签
4. 每个 `main.cpp` 演示输入交互和验证逻辑

## 涉及文件

- `examples/widgets/inputs/search-edit/`（新建）
- `examples/widgets/inputs/ip-input/`（新建）
- `examples/widgets/inputs/password-strength/`（新建）
- `examples/widgets/inputs/tag-input/`（新建）
- `catalogs/01-widget.md`（读取）

## 参考资料

- QLineEdit: https://doc.qt.io/qt-6/qlineedit.html
- QValidator: https://doc.qt.io/qt-6/qvalidator.html
- QCompleter: https://doc.qt.io/qt-6/qcompleter.html
