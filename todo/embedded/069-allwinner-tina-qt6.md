---
id: "069"
title: "全志: Tina Linux Qt6 交叉编译"
category: embedded
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["005"]
blocks: []
estimated_effort: large
---

## 目标

编写全志（Allwinner）平台 Tina Linux Qt6 交叉编译教程，涵盖 Tina SDK 配置、buildroot overlay、Qt6 精简配置和最小化二进制。

## 验收标准

- [ ] Tina SDK 环境搭建
- [ ] buildroot overlay 自定义配置
- [ ] Qt6 精简配置（仅启用必要模块）
- [ ] 最小化二进制大小优化
- [ ] 目标板部署与验证

## 实施说明

1. **Tina SDK**：
   - 获取全志 Tina Linux SDK
   - 选择目标平台（如 V853、T113 等）
   - 配置交叉编译工具链
2. **buildroot overlay**：
   - 创建自定义 overlay 目录
   - 配置 Qt6 包构建规则
   - 集成必要的系统库（libc、libm、libpthread）
3. **Qt6 精简**：
   - 使用 `qt-configure-module` 裁剪不需要的功能
   - 禁用 GUI 不必要的 image format 插件
   - 静态链接可选模块减小体积
   - 使用 `-skip` 跳过不需要的 Qt 模块
4. **最小化**：
   - 使用 `strip` 去除符号信息
   - 使用 UPX 压缩可执行文件（可选）
   - 目标：qtbase 最小二进制 < 5MB
5. 部署到目标板并运行验证

## 涉及文件

- `document/tutorials/embedded/allwinner-tina-qt6.md`（新建）
- `examples/embedded/allwinner/`（配置文件）

## 参考资料

- Tina Linux: https://tina.100ask.net/
- Allwinner SDK: https://www.allwinnertech.com/
- Qt 配置选项: https://doc.qt.io/qt-6/configure-options.html
