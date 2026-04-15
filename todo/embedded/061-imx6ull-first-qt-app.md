---
id: "061"
title: "iMX6ULL: 第一个 Qt 应用部署与运行"
category: embedded
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["060"]
blocks: [062, 063, 064, 065]
estimated_effort: medium
---

## 目标

编写在 iMX6ULL 上运行第一个 Qt Widget 应用的教程。

## 验收标准

- [ ] 交叉编译示例应用
- [ ] 通过 eglfs/linuxfb 平台插件运行
- [ ] 环境变量配置说明（QT_QPA_PLATFORM 等）
- [ ] 远程调试设置（gdbserver + VSCode/Qt Creator）

## 实施说明

1. 编写简单的 Qt Widget 示例（Hello World，含按钮交互）
2. 使用 060 中构建的 SDK 进行交叉编译
3. 通过 `scp`/`nfs` 部署到开发板
4. 配置运行时环境变量：
   - `QT_QPA_PLATFORM=eglfs` 或 `linuxfb`
   - `QT_QPA_EGLFS_FB=/dev/fb0`
   - `QT_QPA_GENERIC_PLUGINS=evdevmouse`
5. 配置 gdbserver 远程调试环境
6. 在 Qt Creator 中配置远程部署和调试

## 涉及文件

- `document/tutorials/embedded/imx6ull-first-qt-app.md`（新建）
- `examples/embedded/imx6ull/hello-qt/`（示例代码）

## 参考资料

- Qt eglfs 平台插件: https://doc.qt.io/qt-6/embedded-linux.html
- Qt 远程调试: https://doc.qt.io/qt-6/creator-embedded.html
