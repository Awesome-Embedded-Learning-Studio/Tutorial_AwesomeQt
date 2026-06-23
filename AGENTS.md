# AGENTS.md

给**任何 AI 编程助手**（Claude Code / Cursor / Copilot / Codex / Windsurf 等）的项目入口，人也适用。一个 vendor-neutral 文件——所有 agent 都读它。如果你打算用 AI 助手参与 AwesomeQt 的贡献，先读这里。

## 这是什么

**AwesomeQt** —— 聚焦 C++ / QtWidgets 的 Qt 6 中文深度教程，三层递进（入门 137 ✅ / 进阶 134 ✅ / 专家 2 进行中，规划 102 篇）+ 可复用实例库（`widget/` `app/` `model/` `industrial/`，`AwesomeQt::` 命名空间），VitePress 建站。隶属 Awesome-Embedded-Learning-Studio。不摊 QML 深度 / PySide6。

## 目录结构

```
tutorial/{beginner,advanced,expert,engineering}/   三层教程 + 实战栏目
examples/{beginner,advanced,expert,experiment}/    教程配套示例（每目录独立 CMake，约 275 个）
widget/ app/ model/ industrial/                    实例库代码根（AwesomeQt:: 命名空间）
site/.vitepress/config/shared.ts                   站点配置单一真相源（改 markdown 插件/head 只改这里）
site/.vitepress/{plugins,theme}/                   自定义 markdown 插件与主题
scripts/build.ts                                   分卷并行构建 + 增量缓存 + 搜索索引合并
scripts/document/check_links.py                    死链检查
qt_src/qt6.9.1/                                    Qt 6.9.1 源码（gitignore，专家层取证用，不入库）
```

## 金科玉律（所有 agent 必读）

- **源码证据**：专家层任何涉及 Qt 源码的结论，必须带 `qt_src/qt6.9.1` 的「文件:行号」可复现证据，禁止凭记忆断言 Qt 内部实现。断言 Qt 行为前，先编译实测或查 Qt 官方文档（doc.qt.io）并标 Qt 版本（基线 6.9.1）。
- **踩坑必须写后果**：不只写错误做法，必须写清会造成什么后果（double free / vtable 错误 / 信号不触发 / 时序错乱等）。只写真碰到的坑，不编造。
- **文档禁止完整可运行代码**：教程只给伪代码和关键片段，完整工程交 `examples/`。每个 `examples/` 工程最少五件套（`widget.h` + `widget.cpp` + `main.cpp` + `CMakeLists.txt` + `.gitignore`），且 `cmake -B build && cmake --build build` 直接成功。
- **链接必须真实**：不编造 URL，宁可不贴也不瞎写。外部链接指向 Qt 官方文档并标 Qt 版本。
- **文章五段结构**：前言 / 环境说明 / 核心概念讲解（含穿插随堂测验）/ 踩坑预防 / 练习项目 / 官方文档参考链接。
- **构建 / 校验**：`pnpm install` → `pnpm dev`（热更新）/ `pnpm build`（分卷并行）；示例 `cmake -B build && cmake --build build`。

## 你来做什么？（按场景路由）

| 场景 | 去哪 |
|---|---|
| 贡献文章 / 代码示例 / 实例库 | [CONTRIBUTING.md](CONTRIBUTING.md)（五段结构、五件套、PR 清单） |
| 构建与校验命令 | `pnpm dev` / `pnpm build` / `cmake -B build && cmake --build build` |
| 专家层源码查证 | 本地 `qt_src/qt6.9.1/`（不入库，gitignore） |
| 死链检查 | `python3 scripts/document/check_links.py` |

> 本文件只描述项目规则与入口，不依赖任何私有配置。专家层进度 2/102（进行中）。
