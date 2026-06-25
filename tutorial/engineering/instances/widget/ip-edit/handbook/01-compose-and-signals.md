---
title: "Step 1：组合骨架 + 4 段构造 + 信号接线"
description: "IpEdit 继承 QWidget 组合 4 个 QLineEdit + 3 个 QLabel，构造里建段、挂 QIntValidator、连 textEdited/textChanged/editingFinished 三条信号，先把 text() 地址拼出来。"
---

# Step 1：组合骨架 + 4 段构造 + 信号接线

← [手册首页](./index.md) · 下一步 [Step 2 跨段焦点流转](./02-focus-flow.md) →

这一步把起点那堆裸 `QLineEdit` 包进一个 `AwesomeQt::IpEdit` 类，构造里循环建出 4 段、每段挂 `QIntValidator(0,255)`、连上三条信号。先不要跨段跳焦——那是 step 2 的事；这一步目标是「`text()` 能从 4 个子段现拼出地址、外部能接到 `textChanged`」。

## Step 1：组合骨架 + 4 段构造 + QIntValidator + 信号接线

### 目标

得到一个 `IpEdit : public QWidget`，头文件声明 `static constexpr int kOctetCount = 4`、私有成员 `QLineEdit* octets_[kOctetCount]{}` 和 `QLabel* dots_[kOctetCount - 1]{}`，公有 `text()` / `setText()` / `isValid()` / `clear()` 四件套，信号 `textChanged(QString)` / `editingFinished()` / `placeholderHintChanged(QString)`。构造里循环 4 次建 `QLineEdit`：`setMaxLength(3)` + `setAlignment(Qt::AlignCenter)` + `setValidator(new QIntValidator(0,255,this))` + `setFixedWidth(40)`，存进 `octets_[i]`；段间插 `QLabel(".")`。三条信号接线到位：`textEdited`（满 3 位判断，step 2 才接跳焦，这一步可先空）、`textChanged`（转发成整体地址）、`editingFinished`（末段才发）。

### 提示

- **组合姿势**：`class IpEdit : public QWidget`，加 `Q_OBJECT`。构造里 `new QHBoxLayout(this)` + `setContentsMargins(0,0,0,0)` + `setSpacing(0)`，循环建段 `addWidget(edit)`、段间 `addWidget(dot)`。**别继承 `QLineEdit`**——你要的是 4 段拼起来，不是一段
- **validator 共享**：`new QIntValidator(0, 255, this)` 在循环外建一次，4 段共用同一个（parent=this 对象树托管）。挂上后用户在段里敲字母直接被拒，省得 `text()` 再判
- **头文件别前向声明 Qt 类**：直接 `#include <QLabel>` / `<QLineEdit>`（非模板 Qt 公有类 include 安全），别在 `namespace AwesomeQt` 里写 `class QLabel;`——那会被编译器当成命名空间内的类，cpp 里 `new QLabel` 报 `incomplete type`（见踩坑①）
- **`textChanged` 转发**：`connect(edit, &QLineEdit::textChanged, this, [this](const QString&){ emit textChanged(text()); })`——子段一变就重拼整体地址发出去。注意 lambda 捕获 `this`，别捕获 `edit`（构造循环里 `edit` 是局部变量）
- **`editingFinished` 末段才发**：`connect(edit, &QLineEdit::editingFinished, this, [this, i](){ if (i == kOctetCount - 1) emit editingFinished(); })`——按值捕获 `i`，别用不存在的 `senderIndex()`（见踩坑②）

### 关键认知——为什么地址状态不存成员

看着诱人的写法是维护一个 `QString address_` 成员，每次段变了更新它。但这会和 4 个子段的 `text()` 产生两份真相，稍有遗漏就不同步（典型 bug：程序化 `setText` 忘了同步 `address_`、或用户粘贴没触发更新）。成品做法是 `address_` 根本不存——`text()` 每次调用都遍历 4 段现拼，地址永远是子段的真实投影。4 段遍历的开销可忽略，换来的是「永远一致」。

### 检查点

跑起来出现 4 个居中对齐的输入框、中间用点隔开。在某段敲 `255` 能敲进去、敲 `999` 敲到 `99` 还行第三个 `9` 被 `maxLength(3)` 顶住、敲字母直接进不去（被 `QIntValidator` 拒）。段里打字时外部能接到 `textChanged`（先放个 `qDebug` 或 demo 里挂个 label 回显看）。这一步还没跳焦，满 3 位不会自动跳下段——正常，step 2 才接。

> QLineEdit / 信号槽不熟？[QLineEdit 入门](../../../../../beginner/03-qtwidgets/22-qlineedit-beginner.md) / [信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。

### 对照答案

- 类定义 + `kOctetCount` + 成员数组：`include/ip_edit.h:22` / `include/ip_edit.h:31` / `include/ip_edit.h:78`
- 头文件直接 include Qt 类（不前向声明）：`include/ip_edit.h:8`
- 构造循环建段 + validator + 信号接线：`src/ip_edit.cpp:28`
- `textEdited` 满位判断（step 2 接跳焦）：`src/ip_edit.cpp:37`
- `textChanged` 转发整体地址：`src/ip_edit.cpp:47`
- `editingFinished` 末段才发：`src/ip_edit.cpp:50`
- 段间插点 `QLabel`：`src/ip_edit.cpp:59`

---

下一步是重头戏：[Step 2 重写 keyPressEvent 做跨段焦点流转](./02-focus-flow.md)。
