---
id: "060"
title: "iMX6ULL: Yocto/Buildroot 构建 Qt6 镜像"
category: embedded
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["005"]
blocks: [061]
estimated_effort: large
---

## 目标

编写完整的 iMX6ULL 开发板 Qt6 镜像构建教程。

## 验收标准

- [ ] 包含 Yocto 和 Buildroot 两种构建方案
- [ ] 交叉编译工具链配置说明
- [ ] Qt6 SDK 构建步骤
- [ ] rootfs 集成 Qt6 库
- [ ] 首次启动验证 Qt6 环境

## 实施说明

1. **Yocto 方案**：
   - 使用 `meta-qt6` 层，配置 `local.conf` 添加 Qt6 支持
   - 配置 `meta-freescale` 和 `meta-freescale-3rdparty` 层
   - 构建包含 `qtbase`、`qtdeclarative` 的完整镜像
   - 输出 SDK 供交叉编译使用

2. **Buildroot 方案**：
   - 配置 Buildroot defconfig 启用 Qt6 包
   - 选择正确的交叉编译工具链（arm-linux-gnueabihf）
   - 自定义 rootfs overlay 集成 Qt6 运行时库
   - 构建 SD 卡镜像

3. **验证步骤**：
   - 烧录 SD 卡，启动开发板
   - 运行 `qtdiag` 确认 Qt6 环境
   - 运行标准 Qt 示例验证渲染

## 涉及文件

- `document/tutorials/embedded/imx6ull-qt6-image-build.md`（新建）
- `examples/embedded/imx6ull/yocto/`（配置文件）
- `examples/embedded/imx6ull/buildroot/`（配置文件）

## 参考资料

- iMX6ULL 数据手册: https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/i-mx-applications-processors/i-mx-6-processors/i-mx-6ull-single-board-computers
- meta-qt6: https://code.qt.io/yocto/meta-qt6.git
- Buildroot Qt6: https://buildroot.org/downloads/manual/manual.html
