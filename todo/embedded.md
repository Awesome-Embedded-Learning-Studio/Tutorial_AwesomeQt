# embedded 线（核心特色）

> 中文 Qt 教程的真空地带，与组织定位最契合。scope：Layer1 公共基础 ~9 + Layer2 板级差异 ~4 ≈ 13 篇。

## gate：embedded 生产方式待定

开工前需定：如何在缺少硬件的环境下保证交叉编译、镜像烧写、板级行为可复现（验证方法、写作与校对流程）。revisit：定下方式后。

## Layer1 公共基础（~9 篇·⚠零条目·已知缺口）

板无关，下列主题尚无条目，开工后新建：
- 工具链 / sysroot
- Qt 源码交叉编译 + Lite 裁剪
- 渲染后端选型：EGLFS / Wayland / LinuxFB ★
- 输入：tslib / libinput
- GPU 驱动差异
- 部署自启
- 交叉调试
- B2Qt 决策
- 交叉 CMake

## Layer2 板级差异（~4 篇·只写独有）

- **iMX6ULL**：Yocto/Buildroot Qt6 镜像 · eglfs/linuxfb 部署 · tslib 触摸 · GPIO/I2C/SPI(sysfs/spidev) · Vivante GPU 调优 · 背光/休眠/看门狗
- **RK3568**：Buildroot/Yocto 镜像 · Mali GPU · Wayland/eglfs 选型 · QML 仪表盘 GPU 加速（偏 QML 深水区·B2 约束·深浅待定）
- **RK3588**：多显示器 + 4K · DRM/KMS · 视频叠加 · Mali-G610
- **Allwinner**：Tina Linux Qt6 交叉编译 · 精简配置 + 最小化二进制

> 重建时 Layer2 按新结构拆——渲染后端选型/GPU/输入/部署调试归 Layer1 公共，板级只留独有（多显4K、Tina 精简等）。

## pilot

工具链 + Qt 源码交叉编译（以一块板为例）。revisit：gate 解除后。
