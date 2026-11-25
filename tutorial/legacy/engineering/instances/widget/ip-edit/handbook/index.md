---
title: "IpEdit 手搓手册"
description: "从空壳 4 个 QLineEdit 一行行搓出 IpEdit：3 步打通组合控件骨架 + 跨段跳焦、0-255 校验与按键流转、地址拼装回填 + 信号抑制。"
---

# IpEdit 手搓手册

> **source**：成品答案在 `widget/ip-edit/`（做完对照）· **related**：组合控件递进链第 2 环（上一站 status-led）　·　教程层 [QLineEdit 入门](../../../../../beginner/03-qtwidgets/22-qlineedit-beginner.md) / [事件处理进阶](../../../../../advanced/03-qtwidgets/02-event-handling-advanced.md)

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 IpEdit，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **组合多个子控件成一个复合输入控件**：4 个 `QLineEdit` + 3 个 `QLabel` 挂进一个 `QWidget`，地址状态不存成员、永远从子段现拼
- **重写 `keyPressEvent` 做跨子控件焦点流转**：拦截 `.`/`BackSpace`/`Return`，自己定位当前段、决定跳下段还是跳上段（step 2 重头）
- **`textEdited` + lambda 按值捕获段索引**：满 3 位自动跳下段，绕开 `sender()` 反查和未定义槽的坑
- **地址拼装与回填**：`text()` 空段补 0、`setText` 用 `SkipEmptyParts` 拆分 + `std::clamp` 夹越界 + 缺段补 0（step 3）
- **程序化回填抑制中间信号**：`QSignalBlocker` 包住每段子 `setText`，末尾统一发一次 `textChanged`，避免外部接到一堆假通知

## 1. 起点

先有个能跑的空壳：一个 `QWidget` 里塞 4 个 `QLineEdit`、中间夹 3 个点。main 里弹窗：

```cpp
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    auto* layout = new QHBoxLayout(&w);
    for (int i = 0; i < 4; ++i) {
        auto* edit = new QLineEdit(&w);
        edit->setMaxLength(3);
        edit->setFixedWidth(40);
        layout->addWidget(edit);
        if (i < 3) {
            layout->addWidget(new QLabel(".", &w));
        }
    }
    w.resize(240, 32);
    w.show();
    return app.exec();
}
```

弹出 4 个能各自打字的输入框、中间有点隔开 = 环境通了，往下走。这一步用的是裸 `QLineEdit`，还没封装、还不能跳焦——下一步我们就把它包进 `IpEdit` 类并接上跨段流转。QLineEdit 不熟先看 [QLineEdit 入门](../../../../../beginner/03-qtwidgets/22-qlineedit-beginner.md)。

## 2. 任务清单

分 3 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 组合骨架 + 4 段构造 + 信号接线 + QIntValidator（先把地址拼出来） | [01](./01-compose-and-signals.md) |
| 2 | 跨段焦点流转（重写 keyPressEvent：`.` 跳下段消费点、段首退格跳上段、回车分发） | [02](./02-focus-flow.md) |
| 3 | 地址拼装回填 + 边界夹值 + 信号抑制（text/setText/isValid/clear） | [03](./03-address-roundtrip.md) |

成品对照：`widget/ip-edit/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **支持 IPv6**：IPv6 是 8 段十六进制（`::` 缩写），段长从 3 变 4、分隔符从 `.` 变 `:`、validator 从 `QIntValidator(0,255)` 换成正则 `[0-9a-fA-F]{1,4}`。思考：`::` 缩写在 `text()`/`setText()` 里怎么表达？要不要展开成完整 8 段存？
- **粘贴整段地址自动分发**：现在粘贴 `"192.168.1.1"` 进某一格只会塞进那一格。想在任一段粘贴时自动拆分填进 4 段——重写 `keyPressEvent` 拦 `Ctrl+V`，或连 `QLineEdit` 的自定义子类重写 `insert`。提示：用 `QApplication::clipboard()->text()` 取剪贴板内容再 `setText`。
- **从 `QLineEdit` 子类化做单段**：现在焦点流转全在 `IpEdit::keyPressEvent` 里靠「遍历找 `hasFocus()` 的段」定位当前段。若把单段提成 `class OctetEdit : public QLineEdit`，每个段自己处理自己的按键，`IpEdit` 只管段间跳转——职责更清晰。思考：跨段退格时上段的 `backspace()` 由谁触发？
- **下一站**：MAC 地址框（6 段十六进制）、版本号框（语义化版本 `major.minor.patch`）——复用本件的跳焦骨架，换段数和 validator 即可。
