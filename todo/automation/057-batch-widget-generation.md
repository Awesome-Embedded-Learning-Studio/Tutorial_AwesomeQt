---
id: "057"
title: "批量生成 Widget 代码脚本"
category: automation
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: [070, 071, 072, 073, 074]
estimated_effort: large
---

## 目标

根据 `catalogs/01-widget.md` 中的清单，批量生成 Widget 代码骨架。

## 验收标准

- [ ] 读取 `catalogs/01-widget.md` 中的 Widget 清单
- [ ] 为每个 Widget 生成标准目录结构（CMakeLists.txt, .h, .cpp, main.cpp）
- [ ] 生成的代码包含基本骨架和注释占位
- [ ] 支持指定生成范围（如 `--category button`）

## 实施说明

1. 解析 `catalogs/01-widget.md` 中的 Widget 清单，提取名称、分类、描述
2. 使用 Jinja2 模板引擎渲染代码骨架
3. 模板包含：CMakeLists.txt（标准 Qt 项目配置）、头文件（类声明）、实现文件（骨架代码）、main.cpp（演示入口）
4. 支持 `--category` 参数过滤特定分类
5. 支持 `--output` 参数指定输出根目录（默认 `examples/`）
6. 生成后自动运行 `clang-format` 格式化

## 涉及文件

- `scripts/generate_widget.py`（新建）
- `catalogs/01-widget.md`（读取）
- `scripts/templates/`（模板目录，新建）

## 参考资料

- Jinja2: https://jinja.palletsprojects.com/
- Qt Widget 基类: https://doc.qt.io/qt-6/qwidget.html
