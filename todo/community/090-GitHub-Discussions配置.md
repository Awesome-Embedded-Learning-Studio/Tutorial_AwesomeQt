---
id: 090
title: "GitHub Discussions 配置"
category: community
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: small
---

# GitHub Discussions 配置

## 目标

启用 GitHub Discussions 并创建分类：Q&A、想法、展示与分享、通用、嵌入式专区，为社区提供讨论和交流的空间。

## 验收标准

- [ ] Discussions 已在仓库中启用
- [ ] 5 个讨论分类已创建
- [ ] README 中添加 Discussions 入口链接
- [ ] 分类描述清晰明了

## 实施说明

1. 在 GitHub 仓库 Settings 中启用 Discussions 功能
2. 创建以下分类：
   - **Q&A** - 问答区，用于教程相关提问
   - **想法** - Ideas，用于新教程建议和功能提议
   - **展示与分享** - Show and Tell，用于展示使用 Qt 开发的项目
   - **通用** - General，用于一般性讨论
   - **嵌入式专区** - Embedded，用于嵌入式 Qt 开发讨论
3. 在 README.md 中添加 Discussions 入口徽章和链接
4. 可配置 GitHub Actions 自动欢迎新讨论

## 涉及文件

- `README.md`（添加链接）
- GitHub 仓库 Settings（启用 Discussions）

## 参考资料

- [GitHub Discussions 文档](https://docs.github.com/en/discussions)
- [GitHub Discussions 分类配置](https://docs.github.com/en/discussions/managing-discussions-for-your-community/managing-categories-for-discussions)
