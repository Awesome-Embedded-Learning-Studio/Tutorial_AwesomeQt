---
id: 082
title: "Mermaid 图表增强"
category: mkdocs
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: medium
---

# Mermaid 图表增强

## 目标

自定义 Mermaid 主题 + 创建可复用图表模板（对象树、信号槽序列图、事件链、嵌入式构建流水线），提升教程中技术图表的视觉一致性。

## 验收标准

- [ ] Mermaid 主题匹配 Material 明暗模式配色
- [ ] 创建对象树图表模板
- [ ] 创建信号槽序列图模板
- [ ] 创建事件链图表模板
- [ ] 创建嵌入式构建流水线图表模板
- [ ] 至少 4 种可复用模板可供教程引用

## 实施说明

1. 在 `javascripts/mermaid-config.js` 中定义自定义 Mermaid 主题
2. 在 `mkdocs.yml` 中引入 Mermaid 配置
3. 分别为以下场景创建 Mermaid 模板：
   - Qt 对象树（parent-child 关系）
   - 信号槽连接序列图
   - Qt 事件处理链路
   - 嵌入式交叉编译构建流水线
4. 确保主题在 light/dark 模式切换时自动适配

## 涉及文件

- `mkdocs.yml`
- `javascripts/mermaid-config.js`

## 参考资料

- [Mermaid Theme Configuration](https://mermaid.js.org/config/theming.html)
- [MkDocs Material Mermaid 集成](https://squidfunk.github.io/mkdocs-material/reference/diagrams/)
