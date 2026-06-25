---
title: "Step 3：信号透传 + setter 去重 + Q_PROPERTY"
description: "透传 textChanged 给外部用，补 textVisible/placeholderText/strength 三个 Q_PROPERTY，demo 用勾选框和显隐态双向联动——靠 setter 去重早返防无限递归。"
---

# Step 3：信号透传 + setter 去重 + Q_PROPERTY

← [Step 2](./02-strength-and-indicator.md) · [手册首页](./index.md) →

前两步搭好了「能输入、能切显隐、能看强度」的密码框。这一步把它变成「外部能干净地监听 + 能被属性系统驱动」的正式控件：把内部 `QLineEdit::textChanged` 透传出去给 demo 回显用，补上三个 Q_PROPERTY，再把 demo 里勾选框和显隐态的双向联动接好——这里有个无限递归的坑，靠 setter 去重早返来填。搓完这三件，控件就生产可用了。

## Step 3：textChanged 透传 + 三 Q_PROPERTY + 双向联动

### 目标

实现四个东西：

1. **`textChanged(QString)` 信号透传**：在 PasswordEdit 加一个 `textChanged` 信号，构造时把内部 `QLineEdit::textChanged` 连到它，让外部不用 `findChild` 抠内部控件就能监听文本变化
2. **三个 Q_PROPERTY**：`textVisible(bool READ textVisible WRITE setTextVisible NOTIFY textVisibleChanged)`、`placeholderText(QString READ ... WRITE setPlaceholderText NOTIFY placeholderTextChanged)`、`strength(Strength READ strength NOTIFY strengthChanged)`
3. **placeholderText 完整读写**：`placeholderText()` 透传 `edit_->placeholderText()`、`setPlaceholderText()` 透传 `edit_->setPlaceholderText()` 且做去重早返 + emit
4. **demo 双向联动**：勾选框 `visible_check_` 和 PasswordEdit 显隐态互连——`textVisibleChanged → setChecked` 和 `toggled → setTextVisible`，靠 setter 去重早返防递归

### 提示（按顺序）

1. **textChanged 信号声明 + 透传**：在 signals 段加 `void textChanged(const QString& text)`（`include/password_edit.h:76`）。构造里 `connect(edit_, &QLineEdit::textChanged, this, &PasswordEdit::textChanged)`（`src/password_edit.cpp:64`）。**无条件透传**——和 `onTextChanged`（去重的强度重算）是两条独立连线，文本变化要每一个字符都通知外部，不能去重
2. **Q_PROPERTY 三件声明**：在类里 `Q_OBJECT` 后加三条 Q_PROPERTY（`include/password_edit.h:31`）。READ/WRITE/NOTIFY 都指向已有的函数和信号。注意 `strength` 是只读属性（只有 READ + NOTIFY，没 WRITE）——强度是算法算出来的，外部不能直接设
3. **placeholderText 去重早返**：`if (edit_->placeholderText() == text) return;` 再透传 + emit（`src/password_edit.cpp:101`）。和 setTextVisible 一个套路——属性 setter 入口先判等，相同就不动不发信号
4. **双向联动连线（demo）**：`connect(edit_, &PasswordEdit::textVisibleChanged, visible_check_, &QCheckBox::setChecked)` + `connect(visible_check_, &QCheckBox::toggled, edit_, &PasswordEdit::setTextVisible)`（`demo/password_edit_window.cpp:61`）。两条线方向相反，构成「点眼睛按钮 → 勾选框跟着变 / 点勾选框 → 显隐态跟着变」的双向同步
5. **为什么不会无限递归**：假设用户点眼睛按钮 → `setTextVisible(true)` → emit `textVisibleChanged(true)` → 勾选框 `setChecked(true)` → 勾选框 emit `toggled(true)` → 又调 `setTextVisible(true)`……但第二次进 `setTextVisible` 时 `text_visible_` 已经是 true，入口 `if (text_visible_ == visible) return`（`src/password_edit.cpp:84`）直接早返，环断掉。**这个早返是双向联动的命根子**，少了就栈溢出
6. **demo 回显用透传信号**：`connect(edit_, &PasswordEdit::textChanged, this, [this](const QString& text){ echo_label_->setText(text.isEmpty() ? "(空)" : text); })`（`demo/password_edit_window.cpp:81`）。密文态下回显 label 也显示真实文本，验证 `text()` 在密文时也拿得到真值——别用 `findChild<QLineEdit*>` 那种脆弱 hack
7. **strengthChanged 回显档位**：demo 里把 `strengthChanged` 连到一个 lambda，switch 档位设 strength_label 文案「弱/中/强」（`demo/password_edit_window.cpp:64`）

### 关键认知——为什么信号透传不能省、为什么去重早返是双向联动的命根

**信号透传不能省**：组合控件把内部 QLineEdit 私有化了，外部拿不到 `edit_` 指针。但「监听文本变化」是密码框最常见的下游需求（回显、校验、联动）。如果你不给 PasswordEdit 加 `textChanged` 信号，外部只能 `findChild<QLineEdit*>()` 去抠内部控件——这把外部代码和你的内部实现死死耦合：哪天你把 QLineEdit 换成自定义输入框，所有 `findChild` 的地方全崩，且 `findChild` 还可能返回空指针。透传一个信号成本就一行 connect，换来的是外部只依赖你的公开接口、内部随便重构。这是「组合控件要把内部信号的常用需求透传出去」的标准姿势。

**去重早返是双向联动的命根**：两个状态（显隐态 + 勾选态）双向同步时，A 变通知 B、B 变又通知 A，天然成环。如果没有「值相同就早返」的闸门，每一边的 setter 都会 emit 信号触发另一边 setter，无限递归直到栈溢出。闸门就在 setter 入口的 `if (current == new) return`——环走到第二次时值已经同步，直接 return，环自然断。这不光是 PasswordEdit 的事，凡是「两个状态双向联动」的场景（spinbox 和 slider 互连、两个 checkbox 互斥联动）都要靠这个早返防递归。养成习惯：**凡是会被双向连线的 setter，入口一律判等早返**。

### 检查点

- 打字时「实时回显」label 实时显示真实文本（密文态也显示真值，不是圆点）= textChanged 透传对了
- 点眼睛按钮切显隐：勾选框跟着勾上/取消 = `textVisibleChanged → setChecked` 对了
- 点勾选框：显隐态跟着切、眼睛按钮文案跟着变 = `toggled → setTextVisible` 对了
- 反复快速点勾选框 / 眼睛按钮：**不崩、不卡死** = setter 去重早返防住递归了（要是少了早返这里会栈溢出）
- 强度档变化时「当前强度」label 实时变「弱/中/强」= strengthChanged 对了
- 在 Designer 或外部用 `setProperty("textVisible", true)` 能驱动显隐切换 = Q_PROPERTY 注册对了

> 信号槽 / 去重机制不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。Q_PROPERTY？[属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

### 对照答案

- textChanged 信号声明：`include/password_edit.h:76`
- textChanged 透传连线：`src/password_edit.cpp:64`
- Q_PROPERTY 三件声明：`include/password_edit.h:31`
- setTextVisible 去重早返：`src/password_edit.cpp:83`
- setPlaceholderText 去重 + 透传 + emit：`src/password_edit.cpp:101`
- demo 双向联动连线：`demo/password_edit_window.cpp:61`
- demo textChanged 回显：`demo/password_edit_window.cpp:81`
- demo strengthChanged 档位回显：`demo/password_edit_window.cpp:64`

---

搓完了。跑 demo 对照成品：打字时色块从红变黄变绿、回显 label 实时显示真值、眼睛按钮和勾选框怎么点都双向同步且不崩 = 你搓的和 repo 一致。

想再深？回 [手册首页](./index.md) 看进阶挑战（强度加权 / 色块改进度条 / 确认密码二次输入 / 显隐态持久化 / 下一站金额框）。
