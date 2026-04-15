# TODO 追踪系统

本目录是 Tutorial_AwesomeQt 仓库的 TODO 追踪系统，按方向分类管理所有待办事项。

## 目录结构

```
todo/
├── catalogs/        # 内容目录（控件/应用/模式/QML/工业模板的完整规划清单）
├── architecture/    # 架构和重构 TODO（001-009，P0 优先）
├── content/         # 教程内容创建 TODO（010-049，P0-P1 优先）
├── automation/      # CI/CD 和自动化 TODO（050-059，P0-P1 优先）
├── embedded/        # 嵌入式 Qt TODO（060-069，P0-P1 优先）
├── code-library/    # 控件/应用/模式实现 TODO（070-079，P0-P1 优先）
├── mkdocs/          # MkDocs 优化 TODO（080-089，P1-P2 优先）
├── community/       # 社区和贡献 TODO（090-094，P1-P2 优先）
├── translation/     # 翻译流水线 TODO（095-098，P1-P2 优先）
├── interactive/     # 交互式元素 TODO（100-103，P2 优先）
└── archive/         # 已完成的 TODO 归档
```

## 优先级定义

| 级别 | 含义 | 示例 |
|------|------|------|
| P0 | 必须先做，阻塞其他工作 | 目录迁移、GUI 入门教程、CI 编译验证 |
| P1 | 重要，影响下一阶段内容 | 嵌入式板级教程、QML 入门、构建脚本 |
| P2 | 显著价值提升 | 工业模板、翻译流水线、Mermaid 增强 |
| P3 | 锦上添花 | 在线沙箱、练习交互系统 |

## TODO 文件规范

每个 TODO 是一个独立的 Markdown 文件，使用以下 frontmatter：

```yaml
---
id: XXX                          # 唯一编号，与文件名编号一致
title: "描述性标题"
category: architecture|content|automation|embedded|code-library|mkdocs|community|translation|interactive
priority: P0|P1|P2|P3
status: pending|in-progress|blocked|done
created: YYYY-MM-DD
assignee: charliechen
depends_on: []                   # 依赖的 TODO ID 列表
blocks: []                       # 被本 TODO 阻塞的 TODO ID 列表
estimated_effort: small|medium|large|epic
---
```

文件体包含：目标、验收标准（可勾选）、实施说明、涉及文件、参考资料。

## 状态说明

| 状态 | 含义 |
|------|------|
| pending | 未开始 |
| in-progress | 进行中 |
| blocked | 被阻塞（等待依赖完成） |
| done | 已完成 |

## 编号规则

| 范围 | 类别 | 说明 |
|------|------|------|
| 001-009 | `architecture/` | 架构重组、构建系统、目录结构 |
| 010-049 | `content/` | 教程内容创建（按模块/层级分组） |
| 050-059 | `automation/` | CI/CD、构建脚本、AI 生成流水线 |
| 060-069 | `embedded/` | 嵌入式 Qt 板级教程、交叉编译 |
| 070-079 | `code-library/` | 控件/应用/模式/工业模板实现 |
| 080-089 | `mkdocs/` | 文档站点增强、导航、插件 |
| 090-094 | `community/` | Discussions、贡献者系统、指南 |
| 095-098 | `translation/` | AI 翻译流水线、双语站点 |
| 100-103 | `interactive/` | 在线沙箱、图表、GIF、练习系统 |

## catalogs/ 目录

`catalogs/` 目录保存完整的规划清单，是所有代码项目的参考来源：

- `00-overview.md` — 项目总览和编码标准
- `01-widget.md` — 537 项自定义控件清单
- `02-app.md` — 200 项完整应用清单
- `03-model.md` — 317 项设计模式清单
- `04-qml.md` — 52 项 QML 项目清单
- `05-industrial.md` — 工业级模板清单

这些文件回答"我们最终要构建什么"，而 TODO 任务文件回答"下一步做什么"。

## 归档

完成的 TODO 文件从对应分类目录移入 `archive/`，保留原始内容便于回溯。归档文件的 status 字段标记为 `done`，验收标准中已完成的项标记 `[x]`。
