---
id: "039"
title: "其他模块：QtCharts 数据可视化"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: medium
---

## 目标

掌握 QtCharts 模块的使用，学会创建各类图表 (折线图、柱状图、饼图、散点图、面积图等)，
实现数据可视化展示。理解图表组件的架构与自定义能力。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QtCharts 架构：QChart, QChartView, QSeries, QAxis
- QLineSeries / QSplineSeries：折线图/曲线图
- QBarSeries / QStackedBarSeries / QPercentBarSeries：柱状图
- QPieSeries：饼图
- QScatterSeries：散点图
- QAreaSeries：面积图
- 坐标轴配置：
  - QValueAxis 数值轴
  - QCategoryAxis 分类轴
  - QDateTimeAxis 时间轴
  - 轴范围、刻度、标签格式
- 图表外观定制：
  - 主题 (QChart::ChartTheme)
  - 动画选项
  - 图例 (QLegend) 配置
  - 系列颜色与样式
- 交互功能：缩放、平移、工具提示
- 实时数据更新

踩坑重点：
1. 大数据量 (10万+点) 未启用 OpenGL 加速导致渲染卡顿
2. 坐标轴范围未设置导致自动缩放不稳定
3. QChartView 的 setRenderHint 未设置抗锯齿导致图表锯齿

练习项目：实现一个实时数据监控面板，包含折线图显示温度变化、
柱状图显示各区域销量、饼图显示市场份额，支持动态数据更新。

## 涉及文件

- document/tutorials/beginner/05-other-modules/02-qtcharts-dataviz-beginner.md
- examples/beginner/05-other-modules/02-qtcharts-dataviz-beginner/

## 参考资料

- [Qt Charts Module](https://doc.qt.io/qt-6/qtcharts-index.html)
- [QChart Class Reference](https://doc.qt.io/qt-6/qchart.html)
- [QLineSeries Class Reference](https://doc.qt.io/qt-6/qlineseries.html)
