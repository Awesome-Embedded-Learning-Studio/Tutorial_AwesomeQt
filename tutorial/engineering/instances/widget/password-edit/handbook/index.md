---
title: "PasswordEdit 手搓手册"
description: "从空壳 QLineEdit 一行行搓出 PasswordEdit：3 步打通组合控件骨架、强度算法 + 色块染色、显隐切换 + 信号透传 + setter 去重。"
---

# PasswordEdit 手搓手册

> **source**：成品答案在 `widget/password-edit/`（做完对照）· **related**：input/display 组合控件递进链第 1 环　·　教程层 [QLineEdit 入门](../../../../../beginner/03-qtwidgets/22-qlineedit-beginner.md) / [信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 PasswordEdit，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **组合而非继承**：把 QLineEdit 当私有成员挂进 QWidget，不重写 paintEvent——input 型组合控件的正确组装姿势
- **EchoMode 切显隐**：QLineEdit 的 `Password`/`Normal` 模式切换，外加一个 QToolButton 当开关（step 1 搭骨架）
- **纯静态算法 + 状态去重**：`computeStrength` 做成静态纯函数独立可测，`onTextChanged` 只在档位真变时刷新 + 发信号（step 2 重头）
- **stylesheet 染色色块**：用 QLabel + `setStyleSheet` 给 3 个色块按强度染色，不自绘（step 2）
- **信号透传 + setter 去重**：把内部 `QLineEdit::textChanged` 透传出去给外部用；setter 入口判等早返，让勾选框和显隐态的双向联动不陷入无限递归（step 3）

## 1. 起点

先有个能跑的空壳：一个 QWidget 里塞个密码模式的 QLineEdit，右侧贴个按钮。main 里弹窗：

```cpp
#include <QApplication>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget w;
    auto* layout = new QHBoxLayout(&w);
    auto* edit = new QLineEdit(&w);
    edit->setEchoMode(QLineEdit::Password);
    auto* btn = new QToolButton(&w);
    btn->setText(QStringLiteral("显"));
    layout->addWidget(edit);
    layout->addWidget(btn);
    w.resize(300, 40);
    w.show();
    return app.exec();
}
```

弹出一个密码框 + 一个「显」按钮 = 环境通了，往下走。这一步用的是裸 QLineEdit + 裸按钮，还没封装——下一步我们就把它包进 PasswordEdit 类，再把强度色块加上。QLineEdit 不熟先看 [QLineEdit 入门](../../../../../beginner/03-qtwidgets/22-qlineedit-beginner.md)。

## 2. 任务清单

分 3 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 组合骨架 + Strength 枚举 + 显隐切换（QLineEdit 密码模式 + QToolButton + 布局） | [01](./01-compose-and-toggle.md) |
| 2 | 强度算法 + 色块染色（computeStrength 纯函数 + 3 个 QLabel + setStyleSheet） | [02](./02-strength-and-indicator.md) |
| 3 | 信号透传 + setter 去重 + Q_PROPERTY（textChanged 透传 / 双向联动防递归） | [03](./03-signals-and-properties.md) |

成品对照：`widget/password-edit/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **强度算法加权**：现在只看「长度 + 种类数」。想更贴近真实安全评估，可以给每种字符加权（符号比数字分高）、设长度阈值分档（< 8 直接弱、>= 12 加分）。思考：算法变了，色块染色逻辑要不要改？（答：不用，染色只认 `Strength` 枚举，算法是上游）
- **色块改连续进度条**：现在 3 档离散色块。想做成 0-100 的连续进度条 + 颜色渐变，可以把 QLabel 换成自绘 QWidget 或者用 QProgressBar 套 stylesheet。提示：这就跨进了自绘领域，参考 [status-led 成品导览](../../status-led/) 的自绘范式。
- **加「确认密码」二次输入**：注册页常见——两个 PasswordEdit，第二个要和第一个比对一致才放行。提示：复用本件的 `textChanged` 透传信号做比对，不需要新控件。
- **密码可见性记忆**：显隐态切换后想跨会话记住用户偏好（上次选了明文，这次默认明文）。提示：把 `textVisible` 这个 Q_PROPERTY 持久化到 QSettings，构造时读回。
- **下一站**：带输入格式限制的金额框——复用本件的组合骨架 + setter 去重，把强度算法换成金额校验（小数位、千分位、范围）。
