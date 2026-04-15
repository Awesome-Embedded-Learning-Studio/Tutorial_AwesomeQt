---
id: 008
title: "创建完整项目实战目录结构"
category: architecture
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [006]
blocks: []
estimated_effort: small
---

## 目标

在代码目录下创建工业模板骨架目录（hmi-dashboard/scada-panel/automotive-cluster/smart-home-controller/medical-monitor/pos-terminal），每个包含 README.md 和 CMakeLists.txt 模板。

## 验收标准

- [ ] 6 个工业模板目录已创建
- [ ] 每个包含 README.md 和 CMakeLists.txt 骨架

## 实施说明

1. 创建 6 个工业模板目录：
   - hmi-dashboard/
   - scada-panel/
   - automotive-cluster/
   - smart-home-controller/
   - medical-monitor/
   - pos-terminal/
2. 每个目录创建 README.md（项目简介和说明）和 CMakeLists.txt（基础构建模板）

## 涉及文件

- `industrial/*/`

## 参考资料

- Qt 工业应用常见模板
- CMake 项目模板最佳实践
