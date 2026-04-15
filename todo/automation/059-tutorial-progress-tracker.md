---
id: "059"
title: "教程进度自动追踪"
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

扫描 `document/tutorials/` 统计完成情况，更新首页进度数据。

## 验收标准

- [ ] 扫描 `document/tutorials/` 下所有教程文件
- [ ] 统计各模块的完成数量和总数
- [ ] 生成进度 JSON 数据
- [ ] 自动更新首页 README 中的进度徽章/表格

## 实施说明

1. 递归扫描 `document/tutorials/` 目录，识别教程 Markdown 文件
2. 解析每个文件的 frontmatter，提取模块分类和状态
3. 按模块汇总：总数、已完成数、进行中数、未开始数
4. 生成 `progress.json` 文件供其他工具使用
5. 读取 `README.md`，找到进度标记区域，替换为最新数据
6. 支持 CI 集成，作为 GitHub Action 步骤运行

## 涉及文件

- `scripts/update_progress.py`（新建）
- `document/tutorials/`（扫描目录）
- `README.md`（更新目标）

## 参考资料

- Python Frontmatter 解析: https://github.com/eyeseast/python-frontmatter
- Shields.io 徽章: https://shields.io/
