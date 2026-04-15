---
id: "055"
title: "Qt 版本兼容性验证脚本"
category: automation
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [050]
blocks: []
estimated_effort: medium
---

## 目标

当 Qt 小版本升级时自动验证所有示例是否仍可编译。

## 验收标准

- [ ] 支持指定 Qt 版本号参数（如 `--qt-version 6.7.0`）
- [ ] 自动安装指定版本 Qt
- [ ] 逐个编译所有示例并记录兼容性结果
- [ ] 生成版本对比报告（与上一版本对比）

## 实施说明

1. 接受 `--qt-version` 参数，使用 `aqtinstall` 安装指定 Qt 版本
2. 基于已有的 `build_examples.py` 逻辑编译所有示例
3. 记录每个示例在新版本上的编译结果
4. 与上次版本的报告做 diff，标记回归项
5. 输出 JSON + Markdown 格式的兼容性报告

## 涉及文件

- `scripts/verify_compatibility.py`（新建）
- `scripts/build_examples.py`（依赖）

## 参考资料

- aqtinstall: https://github.com/miurahr/aqtinstall
- Qt 版本变更日志: https://wiki.qt.io/Qt_6
