---
id: 003
title: "Qt 源码管理策略"
category: architecture
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: small
---

## 目标

确认 qt_src/qt6.9.1/ 已在 .gitignore 中，创建按需下载脚本。仅保留专家层源码分析需要的部分头文件引用路径。

## 验收标准

- [ ] 确认 qt_src/ 在 .gitignore 中
- [ ] 创建 scripts/fetch_qt_source.sh 可按需下载
- [ ] AWESOMEQT_GUIDE.md 中记录 qt_src 的使用方法

## 实施说明

1. 检查 .gitignore 中是否已包含 qt_src/，若未包含则添加
2. 创建 scripts/fetch_qt_source.sh 脚本，支持按需下载 Qt 源码
3. 在 AWESOMEQT_GUIDE.md 中补充 qt_src 目录的使用说明

## 涉及文件

- `.gitignore`
- `scripts/fetch_qt_source.sh`

## 参考资料

- Qt 源码下载官方镜像
- .gitignore 模式匹配规则
