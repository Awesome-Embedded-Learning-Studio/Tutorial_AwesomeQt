---
title: "LogViewer 手搓手册"
description: "从一只只读 QPlainTextEdit 一行行搓出 LogViewer：6 步打通组合控件骨架、Q_PROPERTY、级别染色、自动滚底、行数上限裁旧。"
---

# LogViewer 手搓手册

> **source**：成品答案在 `widget/log-viewer/`（做完对照）· **related**：只读文本组合控件递进链第 1 环　·　教程层 [QPlainTextEdit 入门](../../../../../beginner/03-qtwidgets/24-qplaintextedit-beginner.md) / [日志进阶](../../../../../advanced/01-qtbase/14-logging-advanced.md)

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 LogViewer，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **组合控件骨架**：继承 QWidget + 持有 QPlainTextEdit + 挂布局，不自绘
- **QTextCursor 操纵富文本**：movePosition / insertText / removeSelectedText，按插入设色
- **Q_PROPERTY 全套**：READ / WRITE / NOTIFY 三件，让属性被外部 / Designer 驱动
- **QPlainTextEdit 原生 blockCount 裁旧**：不维护自己的计数器，让文档当真相源
- **主题无关的着色策略**：用无效 QColor 表示「默认前景色」，不在代码里硬编码黑 / 白

## 1. 起点

先有个能跑的空壳——一只只读 QPlainTextEdit 弹出来。新建最小 Qt Widgets 工程，main 里弹个窗：

```cpp
#include <QApplication>
#include <QPlainTextEdit>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QPlainTextEdit edit;
    edit.setReadOnly(true);
    edit.setPlainText("hello log");
    edit.resize(380, 220);
    edit.show();
    return app.exec();
}
```

弹出一只只读文本框、显示 hello log = 环境通了，往下走。QPlainTextEdit 不熟先看 [QPlainTextEdit 入门](../../../../../beginner/03-qtwidgets/24-qplaintextedit-beginner.md)。

## 2. 任务清单

分 6 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | LogViewer 骨架：组合 QPlainTextEdit（只读 + 等宽 + NoWrap） | [01](./01-compose-and-append.md) |
| 2 | append 染色：Level 枚举 + QTextCursor 按级别套前景色 | [01](./01-compose-and-append.md) |
| 3 | 时间戳前缀 + 便捷重载（appendInfo / Warning / Error） | [01](./01-compose-and-append.md) |
| 4 | 行为开关升级为 Q_PROPERTY（maxLines / autoScroll / showTimestamp） | [02](./02-autoscroll-and-trim.md) |
| 5 | 自动滚底（autoScroll 开关） | [02](./02-autoscroll-and-trim.md) |
| 6 | 行数上限裁旧（maxLines + trimOldBlocks） | [03](./03-cap-and-polish.md) |

成品对照：`widget/log-viewer/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **加级别过滤**：给 LogViewer 加一个 `setLevelFilter(QSet<Level>)`，append 时被过滤的级别不进 document（或进了但隐藏）。思考：过滤是 append 时拦截好，还是全存进 document 再靠可见性筛好？各自的取舍。
- **加搜索高亮**：接一个搜索框，把匹配关键字的行用另一套 QTextCharFormat（比如加背景色）标出来。提示：QTextCursor 能遍历文档、QTextDocument 有 `find` 方法。
- **多通道分流**：把 stdout 和 stderr 分两路 append，左侧加分通道勾选。思考：这要不要从「单 view」升级成「按通道多 view / tab」，边界在哪。
- **下一站**：带 model 后端的日志控件——把数据源从直接 append 换成 QAbstractListModel，view 换成 QListView + 自定义 delegate，支持百万行虚拟化。
