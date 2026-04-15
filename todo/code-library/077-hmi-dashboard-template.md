---
id: "077"
title: "工业模板：HMI 实时数据仪表盘"
category: code-library
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["008", "018", "019", "022"]
blocks: []
estimated_effort: epic
---

## 目标

创建工业 HMI 仪表盘项目模板，包含实时数据可视化、报警管理和趋势图功能。

## 验收标准

- [ ] 实时数据仪表盘界面（温度、压力、流量、转速等）
- [ ] 报警管理系统（阈值报警、历史报警记录）
- [ ] 实时趋势图（数据曲线、缩放、平移）
- [ ] 数据模拟源（可切换为真实数据源）
- [ ] 主题切换（亮色/暗色/工业风）
- [ ] 项目模板可复用，提供清晰的扩展接口

## 实施说明

1. **架构设计**：
   - MVC 模式：数据模型、视图、控制器分离
   - 数据源抽象层：支持模拟数据/串口数据/网络数据/OPC UA
   - 插件化布局：可拖拽排列仪表组件
2. **仪表盘组件**：
   - 圆形仪表（gauge）
   - 数值显示面板
   - 状态指示灯
   - 实时曲线图
3. **报警系统**：
   - 可配置阈值规则
   - 报警等级（信息/警告/严重）
   - 声音提示
   - 报警历史记录查询
4. **趋势图**：
   - 多通道数据曲线
   - 时间轴缩放
   - 数据导出（CSV）
5. 使用 Qt Widget + QPainter 绘制，不依赖第三方图表库

## 涉及文件

- `examples/templates/hmi-dashboard/`（新建，完整项目目录）
- `examples/templates/hmi-dashboard/src/`
- `examples/templates/hmi-dashboard/resources/`

## 参考资料

- Qt Model/View: https://doc.qt.io/qt-6/model-view-programming.html
- OPC UA: https://doc.qt.io/qt-6/qtopcua-index.html
- Qt Serial Bus: https://doc.qt.io/qt-6/qtserialbus-index.html
