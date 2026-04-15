---
id: "056"
title: "Pre-commit 钩子配置"
category: automation
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: small
---

## 目标

配置 pre-commit 钩子，自动检查代码风格和文档格式。

## 验收标准

- [ ] 配置 clang-format 检查 C++ 代码风格
- [ ] 配置 black/isort 检查 Python 代码风格
- [ ] 配置 markdownlint 检查文档格式
- [ ] 配置 trailing-whitespace 和 end-of-file-fixer

## 实施说明

1. 创建 `.pre-commit-config.yaml` 配置文件
2. 引入以下 hooks：
   - `clang-format`：C/C++ 代码格式化
   - `black` + `isort`：Python 代码格式化
   - `markdownlint`：Markdown 文档格式检查
   - `trailing-whitespace`：去除行尾空格
   - `end-of-file-fixer`：确保文件以换行符结尾
   - `check-yaml`：YAML 语法检查
3. 添加项目级 `.clang-format` 和 `.markdownlint.json` 配置
4. 在 README 中说明安装方式：`pre-commit install`

## 涉及文件

- `.pre-commit-config.yaml`（新建）
- `.clang-format`（新建）
- `.markdownlint.json`（新建）

## 参考资料

- pre-commit: https://pre-commit.com/
- clang-format: https://clang.llvm.org/docs/ClangFormat.html
