---
id: 004
title: "共享 CMake 预设和模块"
category: architecture
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [002]
blocks: []
estimated_effort: small
---

## 目标

在项目根目录创建 CMakePresets.json 和 cmake/ 共享模块，统一 Qt6 路径、C++23 标准、警告级别等配置。

## 验收标准

- [ ] CMakePresets.json 定义 Qt6 路径和 C++23
- [ ] cmake/Qt6Defaults.cmake 包含共享配置
- [ ] 至少一个示例成功使用预设编译

## 实施说明

1. 创建根目录 CMakePresets.json，定义 Qt6 路径、C++23 标准、编译警告级别
2. 创建 cmake/Qt6Defaults.cmake，封装共享编译选项
3. 选择一个现有示例验证预设编译是否正常

## 涉及文件

- `CMakePresets.json`
- `cmake/Qt6Defaults.cmake`

## 参考资料

- CMakePresets.json 规范
- Qt6 CMake 集成指南
