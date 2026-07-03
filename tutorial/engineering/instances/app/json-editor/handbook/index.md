---
title: "JSON Editor 手搓手册"
description: "从空 QMainWindow 一行行搓出 JSON 编辑器：QPlainTextEdit 编辑区 + QTreeWidget 树视图、QJsonDocument 解析校验、递归填充、格式化/紧凑、UTF-8 读写。"
---

# JSON Editor 手搓手册

> **source**：成品答案在 `app/01-dev-tools/json-editor/`（做完对照）· **related**：app 栏整机应用范式（与 image-viewer 并列，复用同一份 QMainWindow 骨架）

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 JSON 编辑器，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **QMainWindow 整机装配**：菜单栏 / 工具栏 / 状态栏 / QAction 在菜单和工具栏间复用、QAction 配快捷键和标准图标
- **QPlainTextEdit**：纯文本编辑区、等宽字体、toPlainText / setPlainText、QTextCursor 光标位置保护
- **QJsonDocument 解析**：fromJson + QJsonParseError 拿错误类型与字节偏移、toJson 在 Indented / Compact 两种格式间互转
- **QJsonValue 递归**：switch 七种值类型，object 遍历 keys、array 用索引复用同一套递归
- **QTreeWidget**：三列填充、addChild 递归、虚拟根统一顶层形态
- **QSplitter**：水平分栏 + setStretchFactor 拉伸比
- **QFileDialog + UTF-8 读写**：打开/保存对话框、QFile + QTextStream 显式指定编码、记上次打开的目录

## 1. 起点

先有个能跑的空主窗口。最小 Qt Widgets 工程，main 里弹个 QMainWindow：

```cpp
#include <QApplication>
#include <QMainWindow>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QMainWindow w;
    w.resize(960, 640);
    w.show();
    return app.exec();
}
```

弹出空白主窗口 = 环境通了。QMainWindow 不熟先看 [QMainWindow 主窗口](../../../../../beginner/03-qtwidgets/55-qmainwindow-beginner.md)。

## 2. 任务清单

分 5 步（归到 3 个阶段文件），每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 中央区 QSplitter + 编辑区/树两栏 + 五个 QAction 装菜单和工具栏 | [01](./01-layout-and-actions.md) |
| 2 | Validate：QJsonDocument 解析 + QJsonParseError 报错 + 状态栏绿/红字 | [02](./02-parse-and-validate.md) |
| 3 | 树递归填充：虚拟根 + fillValue 吃 object/array/scalar 七种类型 | [02](./02-parse-and-validate.md) |
| 4 | Format / Compact：解析后 toJson 写回编辑区，保光标位置 | [03](./03-format-and-fileio.md) |
| 5 | Open / Save：QFileDialog 读写 .json，UTF-8 编码，记上次目录 | [03](./03-format-and-fileio.md) |

成品对照：`app/01-dev-tools/json-editor/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **编辑区联动树点击**：现在树只是只读展示。提示：`QTreeWidget::itemClicked` 取出被点节点的路径（沿 parent 链回溯 key/索引），在编辑区里高亮对应文本段（QTextCursor + `setExtraSelections`）。难点是「树节点 ↔ 文本偏移」的映射，需要 parse 时记录每个节点的 source offset。
- **行号 + 折叠**：QPlainTextEdit 自带行号要自己画（重写 `LineNumberArea` + `blockCountChanged`），JSON 的 `{}` 折叠要解析括号配对。提示：参考 Qt 官方 Code Editor Example。
- **语法高亮**：键名/字符串/数字/布尔/null 用不同颜色。提示：QSyntaxHighlighter + QRegularExpression，object key 和 string value 用不同规则区分。
- **错误偏移跳转**：状态栏的 `@ offset N` 做成可点击，点了把光标跳到 offset 对应的字节位置。提示：QPlainTextEdit 的光标按字符位置算，offset 按字节算，UTF-8 多字节字符要做转换。
- **增量校验**（debounce）：实时校验会卡，但加 300ms 防抖可以两全。提示：textChanged 启动/重置一个 QTimer::singleShot(300)，用户停手 300ms 才真校验。
- **下一站**：app 栏的 image-viewer / sqlite-browser——换皮复用 QMainWindow 整机骨架，但引入自绘画布 / QSqlTableModel。
