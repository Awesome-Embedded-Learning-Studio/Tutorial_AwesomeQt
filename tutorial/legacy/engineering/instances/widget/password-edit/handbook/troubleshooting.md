---
title: "卡住怎么办"
description: "按症状查：cpp 漏引布局类、显隐切换不生效、强度档不变色块狂闪、双向联动栈溢出、首屏色块异常、findChild hack 耦合——给方向指向成品 file:行号，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/password-edit/`，对照着看。

## cpp 编译报 `expected type-specifier before 'QHBoxLayout'` / `'QVBoxLayout'`

- 构造里 `new QHBoxLayout` / `new QVBoxLayout`，但 cpp **只引了 `QLineEdit` / `QToolButton`，漏了布局类头**。Q_OBJECT 类的 cpp 不像头文件那样顺手引布局类，新加布局代码要单独引。→ `src/password_edit.cpp:9` / `src/password_edit.cpp:13`
- 同理检查 `QLabel`（色块要用）在 cpp 里引了吗？→ `src/password_edit.cpp:10`
- 进阶排查：[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)

## 点眼睛按钮没反应 / echoMode 没切

- `QToolButton::clicked` **真的连上了吗**？构造里漏了 `connect(toggle_btn_, &QToolButton::clicked, ...)`。→ `src/password_edit.cpp:65`
- clicked 连的 lambda 里**翻的是 `text_visible_` 吗**？写成 `setTextVisible(visible)`（没取反）就永远只设一个方向。正确是 `setTextVisible(!text_visible_)`
- setTextVisible 里**改的是 echoMode 吗**？漏了 `edit_->setEchoMode(visible ? Normal : Password)`。→ `src/password_edit.cpp:88`
- 进阶排查：[QLineEdit 入门](../../../../../beginner/03-qtwidgets/22-qlineedit-beginner.md)

## 打字过程中色块疯狂闪烁 / demo 接到一堆重复 strengthChanged

- `onTextChanged` **是不是无条件刷新了**？正确姿势是先 `computeStrength`，只有 `new_strength != strength_` 才刷新 + emit。→ `src/password_edit.cpp:153`
- 信号去重机制不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

## 勾选框和显隐态双向联动后，点一下程序卡死 / 崩

- 这是**双向连线成环 + 没有去重早返**导致无限递归栈溢出。`textVisibleChanged → setChecked → toggled → setTextVisible → textVisibleChanged` 死循环。→ setTextVisible 入口必须 `if (text_visible_ == visible) return` 早返，环走到第二次值已相同直接 return。→ `src/password_edit.cpp:84`
- 双向连线两条都接了吗？`textVisibleChanged → setChecked` + `toggled → setTextVisible` 缺一条就不叫双向，但两条都接就得有早返防环。→ `demo/password_edit_window.cpp:61`

## 刚启动、还没打字，色块显示异常 / `strength()` 返回脏值

- 成员 `strength_` **有没有给初值**？声明应是 `Strength strength_{Strength::kWeak}`。→ `include/password_edit.h:97`
- 构造**末尾调了 `updateStrengthIndicator()` 吗**？没调首屏色块状态不确定。→ `src/password_edit.cpp:67`

## 空文本时强度档返回意外值

- computeStrength **有没有在入口显式处理空文本**？空串 length=0，应直接 `return Strength::kWeak`。→ `src/password_edit.cpp:111`
- 别依赖 `length < 6` 这种后续逻辑碰巧覆盖空串——显式兜底才稳

## demo 想回显真实文本，用了 `edit_->findChild<QLineEdit*>()` 拿不到 / 崩

- 这是**把 demo 和控件内部成员耦合**的脆弱 hack。`findChild` 可能返回空指针解引用崩，且内部一重构 demo 就废。→ 给 PasswordEdit 加 `textChanged(QString)` 信号，构造透传内部 `QLineEdit::textChanged`，demo 改用 `&PasswordEdit::textChanged`。→ `src/password_edit.cpp:64` / `demo/password_edit_window.cpp:81`
- 同理 demo 想读当前文本应调 `edit_->text()`（公开 API），别 findChild 内部控件

## 强度算法算出的档和预期不符

- 种类统计是不是**把符号漏了**？`isLower/isUpper/isDigit` 之外的字符（标点、空格、中文）一律算符号，用 else 兜。→ `src/password_edit.cpp:125`
- 档位判定顺序对吗？「种类 >= 3 但长度 < 8」应落 kMedium 不是 kStrong——`classes >= 3 && length >= 8` 两个条件都要满足才是 kStrong。→ `src/password_edit.cpp:141`
- 「种类 == 2」是不是没单独判？种类 2 直接 kMedium，别让它掉到下面的 kStrong 判定。→ `src/password_edit.cpp:137`

## moc 报错（Q_PROPERTY / Q_ENUM 不认识）

- 类**有没有 `Q_OBJECT`**？→ `include/password_edit.h:28`
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- Q_ENUM 的 Strength **是不是在 PasswordEdit 类里**、Q_ENUM 紧跟其后？→ `include/password_edit.h:38` / `include/password_edit.h:39`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
