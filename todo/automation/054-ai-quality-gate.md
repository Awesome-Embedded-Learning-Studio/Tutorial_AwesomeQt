---
id: "054"
title: "AI 质量门禁检查"
category: automation
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: [053]
blocks: []
estimated_effort: medium
---

## 目标

创建自动检查脚本 `scripts/quality_gate.py`，验证 AI 生成的教程是否符合质量标准。

## 验收标准

- [ ] 检查 7 节结构完整性（目标、知识点、代码、踩坑、拓展、小结、测验）
- [ ] 检查踩坑条目数 >= 3
- [ ] 检查测验数 >= 3
- [ ] 扫描禁用短语列表（如"简单地说"、"不难发现"等 AI 味重表达）
- [ ] 验证文档中的外部链接返回 HTTP 200
- [ ] 检查字数范围（2000-8000 字）

## 实施说明

1. 解析 Markdown 文件，提取各节标题，验证 7 个必要节是否完整
2. 统计"踩坑"节下的条目数量，要求 >= 3
3. 统计"测验"节下的题目数量，要求 >= 3
4. 维护禁用短语列表文件（`config/banned_phrases.txt`），扫描全文匹配
5. 提取所有外部链接，使用 `requests` 逐个验证 HTTP 状态码
6. 统计全文中文字数，验证是否在 2000-8000 范围内
7. 输出质量报告，标记不达标项

## 涉及文件

- `scripts/quality_gate.py`（新建）
- `config/banned_phrases.txt`（新建，禁用短语列表）

## 参考资料

- Markdown 解析: https://python-markdown.github.io/
- Python requests: https://docs.python-requests.org/
