---
id: "053"
title: "AI 内容生成流水线"
category: automation
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: [054]
estimated_effort: large
---

## 目标

创建 Claude API 批量生成教程的 Python 脚本 `scripts/generate_tutorial.py`，支持 `--module/--level/--topic/--batch` 参数。

## 验收标准

- [ ] 读取 `AWESOMEQT_GUIDE.md` 作为系统上下文
- [ ] 支持 Claude Batch API
- [ ] 自动调用 Example Agent 生成配套代码
- [ ] 支持单篇模式（`--module --level --topic`）
- [ ] 支持批量模式（`--batch` 指定 JSON 批处理文件）

## 实施说明

1. 定义命令行参数：`--module`（模块名）、`--level`（初级/中级/高级）、`--topic`（具体主题）、`--batch`（JSON 批处理文件路径）
2. 读取 `AWESOMEQT_GUIDE.md` 作为 system prompt 上下文，确保生成内容符合教程规范
3. 单篇模式：根据参数构建 prompt，调用 Claude API 生成单篇教程
4. 批量模式：读取 JSON 文件中的任务列表，使用 Claude Batch API 并行处理
5. 生成代码部分时，调用 Example Agent 生成可编译的示例代码
6. 输出文件按约定路径保存至 `document/tutorials/` 对应目录

## 涉及文件

- `scripts/generate_tutorial.py`（新建）
- `AWESOMEQT_GUIDE.md`（读取）

## 参考资料

- Claude API 文档: https://docs.anthropic.com/en/docs
- Claude Batch API: https://docs.anthropic.com/en/docs/build-with-claude/batch-api
