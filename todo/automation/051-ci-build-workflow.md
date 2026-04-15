---
id: "051"
title: "CI 构建验证工作流"
category: automation
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [050]
blocks: []
estimated_effort: small
---

## 目标

创建 GitHub Actions 工作流 `.github/workflows/build-examples.yml`，PR 和 push 涉及 `code/**` 时自动编译验证。

## 验收标准

- [ ] 触发条件为 PR/push 涉及 `code/**` 或 `examples/**`
- [ ] 使用 `install-qt-action` 安装 Qt6
- [ ] 调用 `build_examples.py` 执行编译验证
- [ ] 上传编译报告 artifact

## 实施说明

1. 使用 `paths` 过滤器仅在与代码相关文件变更时触发
2. 使用 `jurplel/install-qt-action` 安装 Qt 6.x 及必要模块（qtbase, qtdeclarative 等）
3. 在 Ubuntu runner 上运行，设置必要的系统依赖
4. 调用 `python scripts/build_examples.py` 生成报告
5. 使用 `actions/upload-artifact` 上传 `build_report.json`

## 涉及文件

- `.github/workflows/build-examples.yml`（新建）
- `scripts/build_examples.py`（依赖）

## 参考资料

- install-qt-action: https://github.com/jurplel/install-qt-action
- GitHub Actions 文档: https://docs.github.com/en/actions
