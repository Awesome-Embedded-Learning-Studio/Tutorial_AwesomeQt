---
id: "043"
title: "其他模块：QtPdf 文档处理"
category: content
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: medium
---

## 目标

掌握 QtPdf 模块的使用，包括 PDF 文档的加载、渲染、导航。
学会在 Qt 应用中集成 PDF 查看功能。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QPdfDocument：PDF 文档操作
  - load / close
  - status / error 信号
  - pageCount
  - pagePointSize
  - getAllText / textAtPosition
  - pageLabels
- QPdfPageRenderer：页面渲染
  - renderPage / requestPage
  - 渲染分辨率与缩放
- QPdfPageNavigation：页面导航
  - currentPage / pageCount
  - nextPage / previousPage
  - currentPageChanged 信号
- 自定义 PDF 查看控件构建
- 缩放、平移、翻页交互
- 缩略图导航
- CMake 配置：find_package(Qt6 REQUIRED COMPONENTS Pdf)

踩坑重点：
1. 大型 PDF 文件同步渲染导致界面卡顿，需使用异步渲染
2. PDF 加载失败但未检查 status 导致后续操作崩溃
3. 渲染分辨率设置过低导致文字模糊不清

练习项目：实现一个 PDF 查看器，支持文档打开、页面导航、
缩放控制、全屏阅读、缩略图侧栏导航功能。

## 涉及文件

- document/tutorials/beginner/05-other-modules/06-qtpdf-documents-beginner.md
- examples/beginner/05-other-modules/06-qtpdf-documents-beginner/

## 参考资料

- [Qt PDF Module](https://doc.qt.io/qt-6/qtpdf-index.html)
- [QPdfDocument Class Reference](https://doc.qt.io/qt-6/qpdfdocument.html)
- [Qt PDF Widgets Module](https://doc.qt.io/qt-6/qtpdfwidgets-index.html)
