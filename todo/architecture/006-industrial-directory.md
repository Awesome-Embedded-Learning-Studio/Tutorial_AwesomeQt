---
id: 006
title: "创建工业实践目录结构"
category: architecture
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [001]
blocks: [077, 078, 079]
estimated_effort: small
---

## 目标

在 document/tutorials/ 下创建 project-practice/ 目录，包含 desktop-apps/industrial-hmi/automotive/smart-device-ui 子目录。同时在代码目录准备 industrial/ 骨架。

## 验收标准

- [ ] document/tutorials/project-practice/ 目录已创建
- [ ] 4 个子目录各有 .pages 文件

## 实施说明

1. 创建 document/tutorials/project-practice/ 主目录
2. 创建 4 个子目录：
   - desktop-apps/
   - industrial-hmi/
   - automotive/
   - smart-device-ui/
3. 每个子目录创建 .pages 文件

## 涉及文件

- `document/tutorials/project-practice/*`

## 参考资料

- MkDocs .pages 文件格式
- 工业实践常见分类
