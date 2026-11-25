---
title: 旧版教程（已停止维护）
description: 重构之前的 AwesomeQt 教程，原样保留，停止维护。
sidebar: false
aside: false
lastUpdated: false
---

# 旧版教程（已停止维护）

这里是重构之前的教程，内容原样保留，方便想查阅的读者。它已经停止维护，新版教程从零开发，重构的起因和进展见 [issue #35](https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt/issues/35)。

- [入门教程](./beginner/)
- [进阶教程](./advanced/)
- [专家教程](./expert/)
- [实例文档](./engineering/)

## 已知的问题

旧文里的错误不再逐篇修正。读到下面两处时请留意：

- 示例工程的路径。进阶教程的多篇文章里写的 `examples/...` 是迁移前的旧路径，工程现在都在仓库的 `src/examples/` 下面，前面补上 `src/` 就能找到。
- 布局入门里提到的单整数写法 `setContentsMargins(int all)` 并不存在。`QLayout` 和 `QWidget` 的 `setContentsMargins` 都只有四个 int 参数和传入 `QMargins` 这两种重载（按 Qt 6.12 的头文件核对）。
