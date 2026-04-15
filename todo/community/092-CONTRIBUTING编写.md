---
id: 092
title: "CONTRIBUTING.md 编写"
category: community
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: [093]
estimated_effort: medium
---

# CONTRIBUTING.md 编写

## 目标

编写完整的贡献指南，包含仓库结构说明、教程编写规范、代码提交规范、PR 流程，为贡献者提供清晰的指引。

## 验收标准

- [ ] CONTRIBUTING.md 包含目录结构说明
- [ ] 教程编写规范参考 AWESOMEQT_GUIDE.md
- [ ] 代码风格规范已定义
- [ ] PR 检查清单已包含
- [ ] 提交信息格式规范已定义
- [ ] 分支命名规范已定义

## 实施说明

1. **仓库结构说明**：
   - 介绍主要目录（document/tutorials/, codes/, todo/ 等）
   - 说明各目录的作用和约定

2. **教程编写规范**：
   - 引用 AWESOMEQT_GUIDE.md 作为 AI 辅助生成规范
   - 定义手动编写教程的格式要求
   - 说明 front matter 元数据字段

3. **代码提交规范**：
   - Conventional Commits 格式
   - 分支命名：`feat/`, `fix/`, `docs/`, `translate/` 前缀
   - 提交消息示例

4. **PR 流程**：
   - 创建 PR 前的检查清单
   - 审核流程说明
   - 合并策略

## 涉及文件

- `CONTRIBUTING.md`

## 参考资料

- [GitHub CONTRIBUTING.md 指南](https://docs.github.com/en/communities/setting-up-your-project-for-healthy-contributions/setting-guidelines-for-repository-contributors)
- [Conventional Commits](https://www.conventionalcommits.org/)
- `AWESOMEQT_GUIDE.md`（项目内部规范）
