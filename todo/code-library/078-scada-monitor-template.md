---
id: "078"
title: "工业模板：SCADA 监控面板"
category: code-library
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["008", "044"]
blocks: []
estimated_effort: epic
---

## 目标

创建 SCADA 风格监控面板项目模板，集成 Modbus 协议通信，支持工业现场数据采集与控制。

## 验收标准

- [ ] SCADA 风格主界面（工艺流程图、设备状态）
- [ ] Modbus TCP/RTU 协议集成
- [ ] 设备状态监控（在线/离线/故障）
- [ ] 远程控制功能（开关、阀门、电机启停）
- [ ] 历史数据存储与查询
- [ ] 用户权限管理

## 实施说明

1. **SCADA 主界面**：
   - 工艺流程图绘制（管道、阀门、泵、储罐等图元）
   - 图元可配置数据绑定
   - 实时数据更新动画
   - 报警闪烁效果
2. **Modbus 集成**：
   - 使用 `QModbusTcpClient` 和 `QModbusRtuSerialMaster`
   - 支持 Read Holding Registers、Read Input Registers、Write Single Register 等功能码
   - 自动重连机制
   - 通信日志记录
3. **设备管理**：
   - 设备列表管理（添加/删除/配置）
   - 设备状态轮询
   - 故障诊断面板
4. **数据存储**：
   - SQLite 存储历史数据
   - 数据压缩策略（小时/天/月聚合）
   - CSV 导出
5. **权限系统**：
   - 操作员/工程师/管理员三级权限
   - 关键操作二次确认

## 涉及文件

- `examples/templates/scada-monitor/`（新建，完整项目目录）
- `examples/templates/scada-monitor/src/`
- `examples/templates/scada-monitor/resources/`

## 参考资料

- Qt Modbus: https://doc.qt.io/qt-6/qtmodbus-index.html
- Qt SQL: https://doc.qt.io/qt-6/qtsql-index.html
- SCADA 设计模式: https://en.wikipedia.org/wiki/SCADA
