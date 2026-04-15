---
id: 001
title: "教程迁移：tutorial/ → document/tutorials/"
category: architecture
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: [003, 004, 005, 006, 007]
estimated_effort: epic
---

## 目标

将现有 tutorial/ 目录内容迁移到 document/tutorials/，保持 beginner/advanced/expert 分层结构。使用 git mv 保留历史。需要更新 mkdocs.yml 的 docs_dir 配置。

## 验收标准

- [ ] tutorial/ 内容全部迁移到 document/tutorials/
- [ ] git log --follow 可追踪文件历史
- [ ] mkdocs build 正常构建
- [ ] 旧 tutorial/ 目录不存在

## 实施说明

1. 使用 `git mv` 逐文件/目录迁移，保留完整 git 历史
2. 迁移完成后更新 mkdocs.yml 中的 docs_dir 和 nav 配置
3. 验证 mkdocs build 无报错
4. 确认旧 tutorial/ 目录已完全移除

## 涉及文件

- `tutorial/*` -> `document/tutorials/*`
- `mkdocs.yml`

## 参考资料

- git mv 用法：保留文件重命名历史
- MkDocs docs_dir 配置文档
