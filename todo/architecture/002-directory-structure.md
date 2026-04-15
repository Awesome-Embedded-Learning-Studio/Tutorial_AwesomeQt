---
id: 002
title: "优化现有代码目录结构"
category: architecture
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [001]
blocks: [050]
estimated_effort: medium
---

## 目标

优化 examples/widget/app/model 等代码目录的组织结构，添加共享 CMake 配置。代码目录保持原位置不动。

## 验收标准

- [ ] examples/ 下每个示例有独立 CMakeLists.txt
- [ ] 添加 examples/CMakePresets.json 共享配置
- [ ] 空目录（widget/app/model）准备好目录结构骨架

## 实施说明

1. 检查 examples/ 下现有示例，确保每个都有独立 CMakeLists.txt
2. 创建 examples/CMakePresets.json 定义共享编译配置
3. 为 widget/、app/、model/ 等空目录准备目录结构骨架（含 README.md 占位）
4. 不移动代码目录位置，仅优化内部组织

## 涉及文件

- `examples/`
- `widget/`
- `app/`
- `model/`

## 参考资料

- CMake Presets 文档
- Qt 项目 CMake 最佳实践
