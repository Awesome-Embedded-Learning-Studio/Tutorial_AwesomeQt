---
title: "卡住怎么办"
description: "按症状查：cpp 报 incomplete type QLabel、senderIndex 未定义、按点号下段冒出多余点、setText 触发一堆 textChanged、段首退格跳不回、Tab 进控件不亮——给方向指向成品 file:行号，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/ip-edit/`，对照着看。

## cpp 编译报 `invalid use of incomplete type 'class AwesomeQt::QLabel'`（`new QLabel` 那行）

- 头文件里是不是写了 `namespace AwesomeQt { class QLabel; }` 前向声明？编译器会把 `QLabel` 当成 `AwesomeQt` 命名空间内的类，和全局 `::QLabel` 不是同一类型，cpp 里 `new QLabel` 时类型不完整。→ 删掉命名空间内的前向声明，头文件顶部直接 `#include <QLabel>`（QLabel 是非模板 Qt 公有类，include 安全）：`include/ip_edit.h:8`
- 同理检查 `QLineEdit` 也别前向声明——它一样要直接 include：`include/ip_edit.h:9`
- 进阶排查：[QLabel](https://doc.qt.io/qt-6/qlabel.html) / [QLineEdit](https://doc.qt.io/qt-6/qlineedit.html)

## cpp 编译报 `'senderIndex' was not declared in this scope`

- 是不是在 `onOctetTextChanged` 槽里调了 `senderIndex()` 想取发送者段索引？那函数压根没定义。→ 改在 `connect(textEdited, ...)` 的 lambda 里**按值捕获段索引 `i`**，满位判断内联进 lambda，删掉槽声明与定义：`src/ip_edit.cpp:37`
- 别用 `sender()` 反查再 `qobject_cast`——构造循环里捕获 `i` 干净得多，也避免运行时类型转换
- 进阶排查：[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

## 按 `.` 跳下段后，下一段里冒出一个多余的 `.`

- `keyPressEvent` 的 `Key_Period` 分支是不是只 `focusNextOctet(current)` 没 `event->accept()`？没 accept 事件继续冒泡，子 `QLineEdit` 默认处理把点号当字符输入。→ 跳段后 `event->accept()` 消费掉点号：`src/ip_edit.cpp:220`
- 检查 `Key_Comma`（兼容小键盘/中文输入习惯）分支也 accept 了：`src/ip_edit.cpp:216`
- 进阶排查：[QKeyEvent](https://doc.qt.io/qt-6/qkeyevent.html)

## 调一次 `setText` 外部接到 4 次 `textChanged`（回显闪好几次）

- `setText` 循环里每段 `edit->setText()` 是不是没屏蔽信号？每段触发子段 `textChanged`、step 1 那条转发 lambda 各 emit 一次整体地址，4 段就 4 次假通知（中间还是半成品地址）。→ 每段回填前包 `QSignalBlocker blocker(edit)`（RAII 出作用域自动恢复），循环结束后统一 `emit textChanged(text())` 一次：`src/ip_edit.cpp:104` / `src/ip_edit.cpp:117`
- `clear()` 同样要屏蔽，否则清 4 段也发 4 次：`src/ip_edit.cpp:155`
- 进阶排查：[QSignalBlocker](https://doc.qt.io/qt-6/qsignalblocker.html)

## 段首退格跳不回上段，或跳回了却删错位

- 三个条件是不是漏了一个？必须同时满足 `edit->text().isEmpty()`（段空）+ `edit->cursorPosition() == 0`（光标在段首）+ `current > 0`（非首段）才跳。漏判「段空」会导致段里有字也被跳走。→ `src/ip_edit.cpp:233`
- 跳回上段后是不是没把光标置段末？`prev->setCursorPosition(prev->text().length())` 把光标移到段末，`prev->backspace()` 才能删掉末位。不置段末，`backspace()` 删的位置不对。→ `src/ip_edit.cpp:237`
- 进阶排查：[QLineEdit::backspace](https://doc.qt.io/qt-6/qlineedit.html#backspace)

## Tab 进控件后 4 段全不亮，得再按一次 Tab 才进第一段

- 壳 `QWidget` 没设焦点代理，Tab 聚焦到壳本身而非子段。→ 构造末尾 `setFocusProxy(octets_[0])` + `setFocusPolicy(Qt::StrongFocus)`：`src/ip_edit.cpp:69`
- 检查 `octets_[0]` 在 `setFocusProxy` 调用时已经建好了（在构造循环之后调）：`src/ip_edit.cpp:28` 的循环跑完才有 `octets_[0]`
- 进阶排查：[QWidget::setFocusProxy](https://doc.qt.io/qt-6/qwidget.html#setFocusProxy)

## 子段里能敲进字母（以为校验失效）

- 每段是不是漏挂 `QIntValidator`？挂上后非数字根本敲不进。→ 构造里 `edit->setValidator(new QIntValidator(0, 255, this))`：`src/ip_edit.cpp:26`（4 段共用同一个 validator，循环外建一次）
- 注意：`QIntValidator` 只在**用户键盘输入**时拦截；程序化 `setText("abc")` 它拦不住，得靠 `setText` 内部 `toInt` 失败补 0 兜底。→ `src/ip_edit.cpp:108`
- 进阶排查：[QIntValidator](https://doc.qt.io/qt-6/qintvalidator.html)

## `setText("999.1.1.1")` 显示成 `999.1.1.1`（越界没夹）

- `setText` 循环里是不是漏了 `std::clamp(val, 0, 255)`？→ 每段取值后 `val = std::clamp(val, 0, 255)`：`src/ip_edit.cpp:111`
- 别忘了 cpp 顶部 `#include <algorithm>`，否则 `std::clamp` 未声明：`src/ip_edit.cpp:9`
- 缺段（如 `"1.2.3"`）补 0 也别漏：`i >= parts.size()` 分支 `edit->setText("0")`：`src/ip_edit.cpp:114`

## moc 报错（Q_PROPERTY 不认识）

- 类里**有没有 `Q_OBJECT`**？IpEdit 有 Q_PROPERTY，漏了 moc 不生成元对象。→ `include/ip_edit.h:23`
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- Q_PROPERTY 的 `placeholderHint` 三件套（READ/WRITE/NOTIFY）是不是齐全且名字一致？→ `include/ip_edit.h:26`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
