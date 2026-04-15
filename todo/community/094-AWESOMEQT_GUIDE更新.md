---
id: 094
title: "AWESOMEQT_GUIDE.md 更新"
category: community
priority: P0
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: [092]
estimated_effort: medium
---

# AWESOMEQT_GUIDE.md 更新

## 目标

更新 AI 生成规则文档，反映新的目录结构（document/tutorials/）、新增的嵌入式和工业模板内容，确保 AI 辅助生成教程时遵循最新规范。

## 验收标准

- [ ] 目录路径已更新为 document/tutorials/
- [ ] 嵌入式教程生成规则已添加
- [ ] 工业模板生成规则已添加
- [ ] Qt 5/6 对比标签页规则已添加
- [ ] tags 元数据生成规则已添加
- [ ] 现有规则与新结构兼容

## 实施说明

1. **目录路径更新**：
   - 将所有 `docs/tutorials/` 引用更新为 `document/tutorials/`
   - 更新示例路径和命令

2. **嵌入式教程规则**：
   - 定义嵌入式教程的 front matter 格式
   - 添加板级分类标签规范
   - 说明交叉编译相关注意事项

3. **工业模板规则**：
   - 定义工业模板的生成格式
   - 添加代码规范和安全注意事项

4. **Qt 5/6 对比规则**：
   - 定义 `pymdownx.tabbed` 标签页使用规范
   - 说明何时需要双版本对比

5. **tags 元数据规则**：
   - 定义 tags 字段格式
   - 列出推荐标签词汇

## 涉及文件

- `AWESOMEQT_GUIDE.md`

## 参考资料

- 现有 `AWESOMEQT_GUIDE.md` 内容
- 项目当前目录结构
- 084-Qt5-6对比标签页 任务规范
- 080-标签系统配置 任务规范
