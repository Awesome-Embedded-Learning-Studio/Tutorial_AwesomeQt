---
id: 005
title: "创建嵌入式教程目录结构"
category: architecture
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [001]
blocks: [060, 061, 062, 063, 064, 065, 066, 067, 068, 069]
estimated_effort: small
---

## 目标

在 document/tutorials/ 下创建 embedded/ 目录，包含 imx6ull/rk3568-rk3588/allwinner/qt-for-mcu/cross-compile-patterns 子目录。

## 验收标准

- [ ] document/tutorials/embedded/ 目录已创建
- [ ] 5 个子目录各有 .pages 文件
- [ ] 目录结构与计划一致

## 实施说明

1. 创建 document/tutorials/embedded/ 主目录
2. 创建 5 个子目录：
   - imx6ull/
   - rk3568-rk3588/
   - allwinner/
   - qt-for-mcu/
   - cross-compile-patterns/
3. 每个子目录创建 .pages 文件用于 MkDocs 导航配置

## 涉及文件

- `document/tutorials/embedded/*/.pages`

## 参考资料

- MkDocs .pages 文件格式
- 嵌入式 Qt 开发常见平台分类
