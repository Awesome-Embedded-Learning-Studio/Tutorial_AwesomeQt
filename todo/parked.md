# 延后池（不在此阶段投入·一句话留指针）

> 每条带 revisit 触发。查「这事现在做不做」看这。

## community（34 stars 阶段过早）
GitHub Discussions + 5 分类 · all-contributors bot · CONTRIBUTING.md（须按现 CLAUDE.md+.claude 重写，旧引用 document/tutorials 已失效）· Issue/PR 模板。revisit：专家层立住 + 内容密度够厚。

## translation（远期）
中英翻译流水线 · 工作流文档 · 双语站点（旧写 mkdocs-static-i18n，须改 VitePress）· 中英术语对照表(≥200条，可单独做编写用 glossary)。revisit：社区化阶段。

## interactive（P2-P3）
在线沙箱(Godbolt iframe，Qt GUI 不适用) · Mermaid 架构图集（已被/将并入专家篇正文图示）· GIF 自动生成 · 练习题交互系统。revisit：社区化阶段。注：旧实现锚 mkdocs.yml/javascripts/，已迁 VitePress，复活须重写。

## 站点增强（命名遗留·实际 VitePress·多数旧路径已失效）
标签系统 · 进度小部件 · Mermaid 模板 · Qt5/6 对比标签页 · minify/offline（VitePress 已有 build:single 覆盖）· SEO/jieba（VitePress 用 flexsearch）。可抢救：GIF 规范(≤800px/5-15s/15fps/≤2MB)、Mermaid 4 模板。revisit：实例库铺量时重提，宿主改 VitePress。

## QML 深度（远期）
V4 引擎(JIT/GC) · QQmlEngine 类型桥接。维护者非 QML 方向。revisit：方向变化时。

## 实例库 1112 全量（愿景）
远超产能，靠广度做发现。首波39见 [instance-library.md](instance-library.md)。revisit：流水账判定持续运作后看有效存量。

## instances 实战栏目 + 桌面工程化（CI/单测/部署·P3 暂缓）
原 engineering/session1 已废，2026-06-14 已删（含 nav/build/tutorial-index/engineering-index 引用清理，build volume 改指 instances）。桌面 CI/单测/部署 跟 instances 一起往后放。revisit：专家层主力推进到一定阶段。

## 专家层 CI（构建/用词/死链/结构检查·暂不落地）
落地前作者本地 cmake 验示例。revisit：量上来后。
