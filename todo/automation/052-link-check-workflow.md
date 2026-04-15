---
id: "052"
title: "链接检查工作流"
category: automation
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: small
---

## 目标

使用 lychee 检查所有教程文档中的链接有效性。

## 验收标准

- [ ] PR 涉及 `tutorial/**` 时触发
- [ ] 每周一自动运行（schedule cron）
- [ ] 生成检查报告

## 实施说明

1. 使用 `lycheeverse/lychee-action` GitHub Action
2. 配置扫描路径为 `tutorial/`、`document/`、`docs/` 等文档目录
3. 设置 schedule 触发器：`cron: '0 2 * * 1'`（每周一 UTC 02:00）
4. PR 触发条件：paths 包含 `tutorial/**` 和 `document/**`
5. 配置排除规则：跳过本地链接和特定域名
6. 生成 Markdown 格式的检查报告，通过 artifact 上传

## 涉及文件

- `.github/workflows/link-check.yml`（新建）

## 参考资料

- lychee: https://github.com/lycheeverse/lychee
- lychee-action: https://github.com/lycheeverse/lychee-action
