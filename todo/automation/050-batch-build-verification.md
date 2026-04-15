---
id: "050"
title: "批量编译验证脚本"
category: automation
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: [051]
estimated_effort: small
---

## 目标

创建 Python 脚本 `scripts/build_examples.py`，扫描所有 `examples/` 项目，逐个编译验证。

## 验收标准

- [ ] 脚本扫描 `examples/` 下所有 `CMakeLists.txt`
- [ ] 逐个执行 `cmake` + `build`
- [ ] 生成 JSON 报告（包含编译状态、耗时、错误信息）
- [ ] 支持 `--host` 参数跳过嵌入式示例

## 实施说明

1. 使用 `pathlib` 递归扫描 `examples/` 目录下所有 `CMakeLists.txt`
2. 对每个项目创建临时 build 目录，执行 `cmake` 配置和 `cmake --build`
3. 收集编译结果（成功/失败/跳过），汇总为 JSON 报告
4. `--host` 参数用于过滤掉需要交叉编译工具链的嵌入式示例
5. 输出报告路径默认为 `build_report.json`

## 涉及文件

- `scripts/build_examples.py`（新建）

## 参考资料

- CMake 命令行文档: https://cmake.org/cmake/help/latest/manual/cmake.1.html
- Python subprocess 模块: https://docs.python.org/3/library/subprocess.html
