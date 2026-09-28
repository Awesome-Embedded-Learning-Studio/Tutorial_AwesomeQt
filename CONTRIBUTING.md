# 贡献指南

感谢你对 AwesomeQt 的关注！这是一套聚焦 C++ / QtWidgets 的 Qt 6 中文深度教程，隶属 [Awesome-Embedded-Learning-Studio](https://github.com/Awesome-Embedded-Learning-Studio)。欢迎任何形式的贡献：修正错别字、改进代码示例、补充踩坑、完善现有内容、新增章节等。

## 快速开始

```bash
# 1. Fork 仓库后克隆到本地
git clone https://github.com/<你的用户名>/Tutorial_AwesomeQt.git
cd Tutorial_AwesomeQt

# 2. 安装站点依赖
pnpm install

# 3.（可选但推荐）安装提交前自检钩子
pnpm hooks:install

# 4. 创建特性分支
git switch -c fix/typo-signal-slot

# 5. 本地预览（访问 http://localhost:5173/Tutorial_AwesomeQt/）
pnpm dev

# 6. 提交并推送
git commit -m "fix: 修正信号槽章节的错别字"
git push origin fix/typo-signal-slot

# 7. 在 GitHub 上创建 Pull Request
```

## 提交前自检

本仓库用 [pre-commit](https://pre-commit.com/) 在每次 `git commit` 前自动检查（配置在 `.pre-commit-config.yaml`）：

- **markdownlint**：检查 `tutorial/` 下文章的 markdown 格式（规则在 `.markdownlint.json`）
- **clang-format**：对暂存的 C/C++ 源文件执行格式化（复用根 `.clang-format`）
- **check-added-large-files**：防止大文件（>1.5MB）误入 git 历史
- **基础卫生**：末尾换行、尾随空格、YAML 语法

安装钩子：`pnpm hooks:install`（或 `bash scripts/setup_precommit.sh`）。依赖本机有 `pre-commit`、`python3`、`clang-format`。如果钩子修改了文件，它会中止本次提交，请检查改动后重新 `git add` 再提交。紧急情况可 `git commit --no-verify` 跳过，但不要在 PR 中长期绕过。日常可用 `pre-commit run --all-files` 对全仓跑一遍。

## 文章规范

### 文章结构

每篇教程覆盖**五个方向**，落位形态自由（小节 / 段落 / 块引用皆可），不设编号骨架（2026-09-28 改版裁决，细则见 [ROADMAP_BASE_RESTYLE.md](ROADMAP_BASE_RESTYLE.md)）：

```markdown
---
title: "信号与槽"
description: "关键词密度摘要：机制要点 + 各坑崩法 + 示例路径 + cmake 命令"
---

# 信号与槽

## 为什么需要信号与槽          ← 来路：真实问题开场，不设「前言」栏目
## 在什么环境上验证            ← 地基：Qt 版本基线一句带过 + 转诊链接（可并入收尾）
## connect 与跨线程投递        ← 机制：认知顺序 + 断言三锚 + 检查点穿插
## 漏写 Q_OBJECT 信号为什么不响   ← 翻车：坑穿插行文，三要素齐（场景/崩法/解法）
## 官方文档参考                ← 收尾：链接真实可达、标 Qt 版本
```

- H2 一律机制句或观点句，**不编号**；H1 与 title 不带「现代Qt开发教程（新手篇）N.N——」类前缀（编号由文件名承担，sidebar 按文件名排序）
- **检查点**（随堂测验形态）：每篇 1–2 个，内联在对应机制小节后——A 动手实验（默认，在配套 example 里改一行预测再跑）/ B 预测题 / C 找错题；块引用 + 粗体「检查点：」，答案 `<details>` 折叠
- **不设练习栏目**（2026-09-28 裁决：练习整体删除，动手义务由检查点承担）
- 教程正文引号用中文弯引号 “”（代码块与行内代码内不动），禁直角引号「」

### Frontmatter 元数据

每篇文章开头必须包含：

| 字段 | 必填 | 说明 |
|------|------|------|
| `title` | 是 | 文章标题（侧边栏自动扫描依赖此字段） |
| `description` | 是 | 一句话描述（用于站点搜索与社交分享） |

> 其余字段（如 chapter/order/tags）尚未启用，请勿自行添加。

### 文件命名

`序号-知识点名-层级.md`，例如 `01-signal-slot-beginner.md` / `01-signal-slot-advanced.md` / `01-signal-slot-expert.md`。专家层专属章节直接命名（如 `17-moc-internals-expert.md`），无需在其他层级占位。

### 写作风格

1. 语言清晰简洁的中文，首次出现的技术术语可附英文原文
2. 代码注释用中文
3. 标题层级不超过 4 级
4. **专家层**：每条涉及 Qt 源码的结论必须带「文件:行号」可复现证据（指向 `qt_src/qt6.9.1`）；断言 Qt 行为前先编译实测或查 Qt 官方文档，并标 Qt 版本（基线 6.9.1）
5. **踩坑**：只写真碰到的坑，不编造；每条必须写清后果（崩溃 / 内存泄漏 / 信号不触发 / 时序错乱等）；坑穿插在行文中间，不设独立栏目，小节标题写机制本身

## 代码示例规范

`examples/` 下每个示例最少五件套，且 `cmake -B build && cmake --build build` 必须直接成功：

```
01-signal-slot-beginner/
├── widget.h
├── widget.cpp
├── main.cpp
├── CMakeLists.txt
└── .gitignore
```

- **C++17**，遵循根 `.clang-format`（LLVM / 4 空格 / 100 列）
- 一个示例只展示一个核心知识点，**禁止 TODO 占位**和无关代码
- `find_package(Qt6 REQUIRED COMPONENTS ...)` 指明用到的 Qt 模块
- 实例库（`widget/` `app/` `model/` `industrial/`）统一用 `AwesomeQt::` 命名空间

## 本地预览

```bash
pnpm dev          # 热更新开发服务器
pnpm build        # 生产构建（分卷并行，与部署一致）
pnpm preview      # 预览生产产物
```

## PR 检查清单

提交前请确认：

- [ ] frontmatter 的 `title` / `description` 完整
- [ ] 文章覆盖五方向（来路 / 地基 / 机制 / 翻车 / 收尾），坑穿插行文且写了具体后果，机制方向至少 1 个检查点，无练习栏目
- [ ] 如涉及示例/实例库，`cmake -B build && cmake --build build` 通过
- [ ] 专家层涉及源码处带「文件:行号」证据
- [ ] 内部链接有效，外部链接指向 Qt 官方文档并标版本
- [ ] `pre-commit run` 无报错

## 行为准则

请尊重所有贡献者，给出建设性反馈，专注对项目最有利的事。

---

- 使用 AI 编程助手参与贡献？请先读 [AGENTS.md](AGENTS.md)。
- 报错或提建议？请用 [Issue 模板](https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt/issues/new/choose)。
