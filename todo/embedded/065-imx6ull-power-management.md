---
id: "065"
title: "iMX6ULL: 亮屏休眠、看门狗与电源管理"
category: embedded
priority: P2
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["061"]
blocks: []
estimated_effort: medium
---

## 目标

编写 iMX6ULL 上的亮屏休眠控制、看门狗集成和电源管理教程。

## 验收标准

- [ ] 屏幕背光控制（brightness sysfs）
- [ ] 自动休眠/唤醒策略
- [ ] 看门狗定时器配置与 Qt 集成
- [ ] 低功耗模式切换
- [ ] Qt 应用优雅处理系统挂起/恢复事件

## 实施说明

1. **背光控制**：通过 `/sys/class/backlight/` 节点控制亮度，封装为 Qt 类
2. **休眠策略**：
   - 使用 QTimer 监控用户活动，超时后降低亮度或关闭屏幕
   - 监听输入事件重置计时器
   - 调用 Linux 的 mem standby 模式
3. **看门狗**：配置 `/dev/watchdog`，在 Qt 事件循环中定期喂狗
4. **电源管理**：
   - CPU 频率调节（cpufreq）
   - Qt 应用响应 `QEvent::ApplicationStateChange`
   - 系统挂起前保存状态，恢复后重建资源
5. 提供完整的电源管理封装类

## 涉及文件

- `document/tutorials/embedded/imx6ull-power-management.md`（新建）
- `examples/embedded/imx6ull/power-management/`（示例代码）

## 参考资料

- Linux 电源管理: https://www.kernel.org/doc/Documentation/power/
- Qt 应用状态: https://doc.qt.io/qt-6/qevent.html
