---
id: "075"
title: "实现 P0 开发工具类应用（code-counter/json-formatter等）"
category: code-library
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: ["022"]
blocks: []
estimated_effort: large
---

## 目标

实现 P0 开发工具类应用，包含 code-counter（代码统计）、json-formatter（JSON 格式化）、regex-tester（正则测试器）等。

## 验收标准

- [ ] 每个工具独立项目目录，含完整 CMake 构建文件
- [ ] 独立可编译运行
- [ ] 包含 `demo.gif` 演示动图
- [ ] 功能完整，可作为日常开发工具使用

## 实施说明

1. **code-counter**：
   - 扫描目录，统计代码行数、注释行数、空行数
   - 按文件类型分类统计
   - 支持导出统计报告
2. **json-formatter**：
   - JSON 语法高亮编辑器
   - 格式化/压缩 JSON
   - JSON 路径查询
   - 语法错误定位
3. **regex-tester**：
   - 正则表达式实时测试
   - 匹配结果高亮
   - 常用正则模板库
   - 捕获组展示
4. 每个工具使用 Qt Widget 实现，遵循统一的 UI 风格

## 涉及文件

- `examples/tools/code-counter/`（新建）
- `examples/tools/json-formatter/`（新建）
- `examples/tools/regex-tester/`（新建）

## 参考资料

- QSyntaxHighlighter: https://doc.qt.io/qt-6/qsyntaxhighlighter.html
- QRegularExpression: https://doc.qt.io/qt-6/qregularexpression.html
- QJsonDocument: https://doc.qt.io/qt-6/qjsondocument.html
