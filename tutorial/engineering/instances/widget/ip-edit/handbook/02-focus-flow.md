---
title: "Step 2：跨段焦点流转（重写 keyPressEvent）"
description: "IpEdit 重写 keyPressEvent：按 '.' 跳下段并 accept() 消费点号、段首退格且段空跳上段继续删末位、回车非末段跳下末段发 editingFinished。满 3 位自动跳下段接进 textEdited lambda。"
---

# Step 2：跨段焦点流转（重写 keyPressEvent）

← [手册首页](./index.md) · 上一步 [Step 1 组合骨架](./01-compose-and-signals.md) · 下一步 [Step 3 地址往返](./03-address-roundtrip.md) →

这一步是 IpEdit 的灵魂——让 4 段「连起来用」而不是 4 个孤立输入框。重写 `IpEdit::keyPressEvent`，自己定位当前聚焦段、按按键类型分发：`.` 跳下段（消费掉点号）、段首退格且段空跳上段（继续删上段末位）、回车非末段跳下段末段发 `editingFinished`。再把 step 1 留空的 `textEdited` 接上「满 3 位自动跳下段」。

## Step 2：keyPressEvent + focusNextOctet/focusPrevOctet + 满位自动跳

### 目标

`include/ip_edit.h` 的 `protected` 区声明 `void keyPressEvent(QKeyEvent* event) override;`，`private` 区声明 `bool focusNextOctet(int from_index);` / `bool focusPrevOctet(int from_index);` / `QLineEdit* octet(int index) const;`（带边界保护）。`keyPressEvent` 实现四件事：①遍历 `octets_` 找 `hasFocus()` 的段作 `current`，找不到就交父类默认处理；②`Key_Period`/`Key_Comma` 非末段 `focusNextOctet(current)` + `event->accept()`，末段 `emit editingFinished()` + `accept()`；③`Key_Backspace` 且当前段空且光标在段首且非首段，`focusPrevOctet` 后 `prev->backspace()`；④`Key_Return`/`Key_Enter` 非末段跳下、末段发 `editingFinished`。step 1 的 `textEdited` lambda 补上「`t.length()>=3` 且 `toInt` 在 0-255 则 `focusNextOctet(i)`」。

### 提示

- **定位当前段**：`keyPressEvent` 入口遍历 `octets_`，`octet(i)->hasFocus()` 命中的就是 `current`。`current < 0`（焦点不在任何段上）直接 `QWidget::keyPressEvent(event)` 交默认，别硬处理
- **`.` 必须消费掉**：跳下段后 `event->accept()`。不 accept 的话事件继续冒泡、子 `QLineEdit` 默认处理会把点号当字符输入，下一段里就冒出一个多余的 `.`。这是本步最容易漏的一行
- **段首退格的三个条件缺一不可**：`edit->text().isEmpty()`（段空）+ `edit->cursorPosition() == 0`（光标在段首）+ `current > 0`（非首段），三者都满足才跳上段。跳上段后要 `prev->setCursorPosition(prev->text().length())` 把光标置段末、再 `prev->backspace()` 删上段末位——光标不置段末，`backspace()` 删的位置不对
- **`focusNextOctet` 进段即全选**：跳到下段后 `next->selectAll()`，用户接着输入直接覆盖整段，不用先清空。这是「顺手」的小细节
- **`octet(index)` 带边界保护**：`index < 0 || index >= kOctetCount` 返回 `nullptr`，`focusNextOctet`/`focusPrevOctet` 拿到 null 直接返回 false——避免越界访问 `octets_`
- **满位自动跳接进 `textEdited` 不接 `textChanged`**：`textEdited` 只在用户输入时触发，`textChanged` 程序化 `setText` 也触发。自动跳焦只该响应用户输入，所以连 `textEdited`，lambda 里按值捕获 `i`

### 关键认知——为什么不用 focusNextChild

`QWidget::focusNextChild()` 走的是 Qt 内置 Tab 焦点链，它干不了我们要的三件事：不会拦截 `.`（点号照样被输入）、不会判断「段首退格」上下文（它只往前跳一格）、回车也不归它管。所以必须自己在 `keyPressEvent` 里接管——`focusNextChild` 留给真正的 Tab 键（默认分支不拦，事件交父类，Tab 自然走焦点链）。控制权分层：跨段流转自己管，Tab 走链子。

### 检查点

跑起来在第一段敲 `192`，满 3 位自动跳到第二段、光标为空（或全选态）。敲完第三段按 `.`，跳到第四段且点号没被输入。在第四段按退格删到空、再按退格，跳回第三段并删掉第三段末位。任意段按回车，非末段跳下、末段（第四段）回车触发 `editingFinished`（外部 label 回显「editingFinished 触发」）。敲字母还是被 `QIntValidator` 拒（这一步不该破坏 step 1 的校验）。

> 事件处理不熟？[事件处理进阶](../../../../../advanced/03-qtwidgets/02-event-handling-advanced.md) / [事件系统入门](../../../../../beginner/01-qtbase/07-event-system-beginner.md)。

### 对照答案

- `keyPressEvent` 声明 + 焦点辅助声明：`include/ip_edit.h:66` / `include/ip_edit.h:72`
- `keyPressEvent` 实现（定位段 + 按键分发）：`src/ip_edit.cpp:198`
- `Key_Period`/`Key_Comma` 跳下段 + `accept()` 消费点号：`src/ip_edit.cpp:215`
- `Key_Backspace` 段首退格跳上段 + `backspace()`：`src/ip_edit.cpp:231`
- `Key_Return`/`Key_Enter` 回车分发：`src/ip_edit.cpp:245`
- `focusNextOctet` 进段全选：`src/ip_edit.cpp:267`
- `focusPrevOctet` 置光标段末：`src/ip_edit.cpp:277`
- `octet(index)` 边界保护：`src/ip_edit.cpp:287`
- `textEdited` 满位自动跳：`src/ip_edit.cpp:37`

---

下一步收尾：[Step 3 地址拼装回填 + 边界夹值 + 信号抑制](./03-address-roundtrip.md)。
