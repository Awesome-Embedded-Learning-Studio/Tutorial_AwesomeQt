---
id: "066"
title: "RK3568: Buildroot/Yocto Qt6 镜像构建"
category: embedded
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["005"]
blocks: [067, 068]
estimated_effort: large
---

## 目标

编写 RK3568 开发板的 Qt6 镜像构建教程，涵盖 BSP 配置、Mali GPU 驱动和 Wayland vs eglfs 平台选择。

## 验收标准

- [ ] RK3568 BSP 环境搭建
- [ ] Mali GPU 驱动集成
- [ ] Wayland 后端构建方案
- [ ] eglfs 后端构建方案
- [ ] Wayland vs eglfs 选择指南
- [ ] Qt6 SDK 构建与输出

## 实施说明

1. **BSP 环境**：获取 Rockchip 官方 BSP，配置 kernel 和 u-boot
2. **Mali GPU**：集成 Mali-G52 驱动（panfrost 开源驱动或 ARM 官方驱动）
3. **Wayland 方案**：
   - 构建 Weston compositor
   - 配置 Qt6 Wayland 平台插件
   - 适合多应用场景
4. **eglfs 方案**：
   - 直接通过 EGL/GBM 渲染
   - 单应用全屏场景，性能更优
5. **选择指南**：根据应用场景（多窗口 vs 全屏、GPU 需求、启动速度）给出建议
6. 输出可用的 Qt6 交叉编译 SDK

## 涉及文件

- `document/tutorials/embedded/rk3568-qt6-image-build.md`（新建）
- `examples/embedded/rk3568/`（配置文件）

## 参考资料

- RK3568 数据手册: https://www.rock-chips.com/a/en/products/RK3568_Series/
- Panfrost 驱动: https://docs.mesa3d.org/drivers/panfrost.html
- Qt Wayland: https://doc.qt.io/qt-6/qtwayland-index.html
