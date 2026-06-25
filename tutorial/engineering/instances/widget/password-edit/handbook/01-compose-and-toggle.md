---
title: "Step 1：组合骨架 + 显隐切换"
description: "PasswordEdit 继承 QWidget 组合 QLineEdit + QToolButton，定义 Strength 枚举，搭出编辑行布局，接 QToolButton::clicked 翻 textVisible。"
---

# Step 1：组合骨架 + 显隐切换

← [手册首页](./index.md) · 下一步 [Step 2 强度算法 + 色块染色](./02-strength-and-indicator.md) →

这一步把裸 QLineEdit + 裸按钮包进一个 `AwesomeQt::PasswordEdit` 类，定义 Strength 枚举，搭出编辑行布局，把按钮的 `clicked` 接到翻显隐态的逻辑上。先不要强度色块、不要信号透传——能点按钮在明文/密文之间切，按钮文案跟着变就行。强度是下一步的事。

## Step 1：组合骨架 + Strength + 编辑行 + 显隐切换

### 目标

得到一个 `PasswordEdit : public QWidget`，构造时 new 出 QLineEdit（`EchoMode::Password`）和 QToolButton（默认文案「显」），用 QHBoxLayout 横排成编辑行塞进 QVBoxLayout。定义 `enum class Strength { kWeak, kMedium, kStrong }` 加 `Q_ENUM`。实现 `textVisible()` / `setTextVisible(bool)`——后者改 QLineEdit 的 echoMode（`Normal`/`Password`）并把按钮文案同步成「隐」/「显」。把 `QToolButton::clicked` 连到一个翻 `textVisible_` 的 lambda。

### 提示

- **组合姿势**：`class PasswordEdit : public QWidget`，私有成员 `QLineEdit* edit_{nullptr}` 和 `QToolButton* toggle_btn_{nullptr}`。构造里 `edit_ = new QLineEdit(this)`（parent=this 对象树托管）、`toggle_btn_ = new QToolButton(this)`，再用 `new QVBoxLayout(this)` 当根布局，里面塞一个 QHBoxLayout 编辑行。**别继承 QLineEdit**——那会把一堆不该暴露的输入 API 放出去，组合才好控边界
- **密码模式**：`edit_->setEchoMode(text_visible_ ? QLineEdit::Normal : QLineEdit::Password)`。成员 `bool text_visible_{false}` 默认密文，构造时按它设 echoMode（`src/password_edit.cpp:30`）
- **按钮文案**：默认密文时按钮提示「显」（点了会显示）。建议把文案同步逻辑收进一个私有 `syncToggleText()`：明文时设「隐」、密文时设「显」（`src/password_edit.cpp:180`）。**用文字别用 emoji 图标**——省得扯 icon 资源、跨平台还可能缺字体
- **按钮体验**：`toggle_btn_->setFocusPolicy(Qt::NoFocus)`（别抢密码框焦点）、`setCursor(Qt::PointingHandCursor)`（手型光标提示可点）
- **Strength 枚举**：`enum class Strength { kWeak, kMedium, kStrong };` 紧跟 `Q_ENUM(Strength)`（`include/password_edit.h:38`）。这一步先定义上，下一步算法要用
- **clicked 连线**：`connect(toggle_btn_, &QToolButton::clicked, this, [this]{ setTextVisible(!text_visible_); })`（`src/password_edit.cpp:65`）。翻转发给 setTextVisible 处理，不在 lambda 里直接改 echoMode——单一改动入口
- **setTextVisible 早返**：入口 `if (text_visible_ == visible) return;`（`src/password_edit.cpp:84`）。这一步看着多余（点同一个态没变化），但 step 3 接上勾选框双向联动后这个早返是防无限递归的关键，现在就养成习惯

### 关键认知——为什么组合不继承

继承 QLineEdit 看着省事（直接拥有密码框所有方法），但你会把 `setText` / `setValidator` / `setMaxLength` 这堆内部 API 全暴露给外部，外部能绕过你的强度逻辑直接塞文本。组合后 `edit_` 是私有的，外部只能走你批准的接口（text / setText / setTextVisible），边界牢牢攥在自己手里。这也是本批 input 型组合控件的统一姿势——和 editable-table 把 QTableWidget 私有化是一个道理。

### 检查点

跑起来出现一个密码框 + 一个「显」按钮。输入字符显示成圆点（密码模式生效）；点「显」按钮，字符变明文、按钮文案变「隐」；再点「隐」，回到圆点、文案变「显」= 显隐切换对了。焦点始终在密码框、点按钮不抢焦点 = `NoFocus` 设对了。

> QLineEdit / EchoMode 不熟？[QLineEdit 入门](../../../../../beginner/03-qtwidgets/22-qlineedit-beginner.md)。组合控件思路？[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

### 对照答案

- 类定义 + Strength 枚举 + Q_ENUM：`include/password_edit.h:27` / `include/password_edit.h:38`
- 构造组合 QLineEdit + QToolButton + 布局：`src/password_edit.cpp:27`
- setTextVisible 改 echoMode + 早返：`src/password_edit.cpp:83`
- syncToggleText 同步按钮文案：`src/password_edit.cpp:180`
- clicked 连线翻显隐：`src/password_edit.cpp:65`

---

下一步是重头戏：[Step 2 强度算法 + 色块染色](./02-strength-and-indicator.md)。
