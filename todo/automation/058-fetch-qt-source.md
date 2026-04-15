---
id: "058"
title: "按需下载 Qt 源码脚本"
category: automation
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: small
---

## 目标

创建 shell 脚本从 Qt 服务器下载指定版本的源码到 `qt_src/`。

## 验收标准

- [ ] 支持 `--version` 参数指定 Qt 版本
- [ ] 支持 `--mirror` 参数指定镜像源
- [ ] 下载后自动解压到 `qt_src/` 目录
- [ ] 支持增量下载（已下载的模块跳过）
- [ ] 支持指定模块列表（如 `--modules base,declarative,multimedia`）

## 实施说明

1. 使用 `curl` 或 `wget` 从 Qt 官方镜像下载源码 tar 包
2. 默认镜像：`https://download.qt.io/official_releases/qt/`
3. 支持 `--mirror` 参数切换国内镜像（如清华、阿里云）
4. 使用 MD5/SHA256 校验下载完整性
5. 解压到 `qt_src/<version>/` 目录，保留模块目录结构
6. 记录已下载模块清单，支持增量下载

## 涉及文件

- `scripts/fetch_qt_source.sh`（新建）

## 参考资料

- Qt 源码下载: https://download.qt.io/official_releases/qt/
- 清华镜像: https://mirrors.tuna.tsinghua.edu.cn/qt/
