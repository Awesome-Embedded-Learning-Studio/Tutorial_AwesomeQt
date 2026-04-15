---
id: 093
title: "Issue 和 PR 模板"
category: community
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [092]
blocks: []
estimated_effort: small
---

# Issue 和 PR 模板

## 目标

创建 bug-report.yml、feature-request.yml、question.yml 和 PULL_REQUEST_TEMPLATE.md，规范化 Issue 和 PR 提交流程。

## 验收标准

- [ ] 3 个 Issue 模板已创建（bug-report.yml, feature-request.yml, question.yml）
- [ ] question.yml 配置重定向到 Discussions
- [ ] PR 模板包含变更类型检查清单
- [ ] 模板在 GitHub 上可正常显示和使用

## 实施说明

1. **Bug Report 模板** (`.github/ISSUE_TEMPLATE/bug-report.yml`)：
   - 问题标题
   - 问题描述
   - 复现步骤
   - 期望行为 vs 实际行为
   - 环境（Qt 版本、操作系统、编译器）
   - 相关教程/代码链接

2. **Feature Request 模板** (`.github/ISSUE_TEMPLATE/feature-request.yml`)：
   - 功能描述
   - 使用场景
   - 建议实现方式
   - 相关教程模块

3. **Question 模板** (`.github/ISSUE_TEMPLATE/question.yml`)：
   - 配置重定向到 Discussions
   - 引导用户到 Q&A 分类

4. **PR 模板** (`.github/PULL_REQUEST_TEMPLATE.md`)：
   - 变更类型复选框（教程新增/修改、代码修复、翻译、文档）
   - 变更描述
   - 关联 Issue
   - 自测检查清单

## 涉及文件

- `.github/ISSUE_TEMPLATE/bug-report.yml`
- `.github/ISSUE_TEMPLATE/feature-request.yml`
- `.github/ISSUE_TEMPLATE/question.yml`
- `.github/PULL_REQUEST_TEMPLATE.md`
- `.github/ISSUE_TEMPLATE/config.yml`（配置 question 重定向）

## 参考资料

- [GitHub Issue 模板语法](https://docs.github.com/en/communities/using-templates-to-encourage-useful-issues-and-pull-requests/syntax-for-githubs-form-schema)
- [GitHub 配置 Issue 模板](https://docs.github.com/en/communities/using-templates-to-encourage-useful-issues-and-pull-requests/configuring-issue-templates-for-your-repository)
