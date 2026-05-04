# 实战练习 · ConfirmDialog — 做一个比 QMessageBox 好看的确认弹窗

## 前言：QMessageBox 够用，但不体面

Qt 给我们提供了 QMessageBox，一行代码就能弹一个确认对话框，方便是真方便。但方便的代价就是千篇一律——它用的是系统原生风格，你改不了按钮文字的样式、改不了图标大小、改不了整体配色。在工业软件或者有设计要求的项目里，你几乎一定会需要一个"自定义的确认对话框"，让它和你的应用主题统一。

这篇练习我们要做的就是这样一个东西：继承 QDialog，自己画一个带有标题、消息、图标、确认/取消按钮的确认弹窗，最后再提供一个静态方法 `ConfirmDialog::confirm()`，让调用方一行代码就能用——和 QMessageBox 一样方便，但长相由你说了算。

---

## 出发前的装备清单

- **QDialog** — 对话框基类，exec() 启动模态事件循环，accept()/reject() 关闭并返回结果。
- **QDialogButtonBox** — 标准按钮容器，自动处理 Ok/Cancel 的布局和平台差异。
- **QLabel** — 用来显示图标、标题文本和消息内容。
- **QStyle::standardPixmap** — 获取平台原生的标准图标（警告、问号、信息等）。

---

## 我们的目标长什么样

```
┌─────────────────────────────────────┐
│                                     │
│   ⚠  确认删除                       │
│                                     │
│   您确定要删除这个文件吗？           │
│   此操作不可撤销。                  │
│                                     │
│           [确认]    [取消]           │
│                                     │
└─────────────────────────────────────┘
```

一个约 380px 宽的对话框，左侧一个图标，右侧标题文字，下面是消息内容（可换行），底部是两个按钮。并且我们可以通过一行静态方法调用：

```cpp
int result = ConfirmDialog::confirm("确认删除", "您确定要删除这个文件吗？", this);
if (result == QDialog::Accepted) {
    // 用户点了确认
}
```

完成标准：静态方法调用能正确弹出对话框；点确认返回 Accepted，点取消或按 Esc 返回 Rejected；标题、消息、按钮文字都可以自定义。

---

## 第一步 — 对话框类骨架

### 思考题

QDialog 有两种显示方式：exec() 和 show()。前者是模态的（弹出后其他窗口无法操作，直到对话框关闭），后者是非模态的。对于确认对话框，我们应该用哪种？为什么？

### 动手写

```cpp
#pragma once

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>

class ConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfirmDialog(const QString &title,
                           const QString &message,
                           QWidget *parent = nullptr);

    // TODO: 静态便捷方法
    //       static int confirm(const QString &title,
    //                          const QString &message,
    //                          QWidget *parent = nullptr);

    // TODO: 可选的自定义方法
    //       void setConfirmText(const QString &text);
    //       void setCancelText(const QString &text);
    //       void setIcon(QStyle::StandardPixmap icon);

private:
    void setupUi(const QString &title, const QString &message);

    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_messageLabel;
    QDialogButtonBox *m_buttonBox;
};
```

构造函数接收标题和消息文本，把它们传给 setupUi 来构建界面。

### 检查点

编译一下确认类骨架没有语法错误。暂时不需要运行——还没写 setupUi。

---

## 第二步 — 标题区域：图标 + 文字

### 思考题

Qt 的 QStyle::standardPixmap 提供了一系列平台原生图标：SP_MessageBoxWarning（警告三角）、SP_MessageBoxQuestion（问号）、SP_MessageBoxInformation（i 字母）、SP_MessageBoxCritical（叉号）。我们默认用哪个比较适合"确认对话框"？另外，你知道怎么把 QStyle::StandardPixmap 转换成一个可以放在 QLabel 里的 QPixmap 吗？

### 动手写

标题区域用一个 QHBoxLayout：左边图标 QLabel，右边标题 QLabel。

```cpp
// 在 setupUi 里：
// TODO: 创建一个 QWidget 作为标题区域的容器
//       或者直接用 QHBoxLayout 包裹

// 图标：
// TODO: m_iconLabel = new QLabel
//       用 style()->standardIcon(QStyle::SP_MessageBoxWarning) 获取 QIcon
//       然后用 QIcon::pixmap(size) 转成 QPixmap
//       设置到 QLabel::setPixmap()
//       建议图标大小 32x32 或 48x48

// 标题文字：
// TODO: m_titleLabel = new QLabel(title)
//       设置粗体大号字体
//       设置 wordWrap(true) 以防标题太长
```

图标大小的选择取决于你想要的视觉风格。32px 比较紧凑，48px 比较醒目。你可以先设 40px 然后根据效果调整。

### 检查点

如果暂时只想看效果，可以在 main.cpp 里直接创建 ConfirmDialog 并 exec()——你应该能看到对话框顶部有一个警告图标和一个粗体标题。

---

## 第三步 — 消息区域：自动换行的文本

### 思考题

QLabel 默认不会自动换行——如果文本很长，它会撑得很宽。你知道怎么让 QLabel 在固定宽度内自动换行吗？提示：有一个方法叫 setWordWrap，但它通常需要配合 setMinimumWidth 或 setMaximumWidth 才能正常工作，否则 QLabel 会认为它有无限宽度，根本不需要换行。

### 动手写

```cpp
// 消息区域：
// TODO: m_messageLabel = new QLabel(message)
//       setWordWrap(true)
//       setMinimumWidth(280)   ← 这很关键，没有这个 wordWrap 不生效
//       setMaximumWidth(380)
//       设置正常字体（比标题小一号）
```

### 你可能会遇到的坑

setWordWrap(true) + 不设宽度 = 换行不生效。这个坑几乎每个 Qt 初学者都踩过。QLabel 的换行逻辑是"宽度不够了才换"，如果你不限制宽度，QLabel 觉得自己有无限宽，一行就能放下所有文字，就不换行了。必须用 setMinimumWidth 或 setFixedWidth 告诉它"你就这么宽，放不下就换行"。

### 检查点

传一段比较长的消息文本，消息应该自动换行显示，而不是把对话框撑到屏幕那么宽。

---

## 第四步 — 按钮区域：QDialogButtonBox

### 动手写

```cpp
// 按钮区域：
// TODO: m_buttonBox = new QDialogButtonBox(
//           QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
//
//       连接信号：
//       connect(m_buttonBox, &QDialogButtonBox::accepted,
//               this, &QDialog::accept);
//       connect(m_buttonBox, &QDialogButtonBox::rejected,
//               this, &QDialog::reject);
```

QDialogButtonBox 的好处是自动适配平台风格。比如在 macOS 上，按钮的默认顺序和 Windows 上不同——QDialogButtonBox 帮你处理了这些差异。如果手动放两个 QPushButton，你就得自己考虑跨平台按钮顺序的问题。

你可能还想改按钮文字。默认的 Ok/Cancel 是英文的，中文环境下可能不够友好。可以通过 `m_buttonBox->button(QDialogButtonBox::Ok)->setText("确认")` 来修改，或者用 setConfirmText/setCancelText 方法封装一下。

### 检查点

对话框底部应该有两个按钮。点击确认按钮对话框关闭（exec 返回 Accepted），点击取消对话框也关闭（exec 返回 Rejected）。按 Esc 键也应该关闭对话框并返回 Rejected——这是 QDialog 的默认行为，不需要你写代码。

---

## 第五步 — 静态便捷方法：一行代码搞定

### 思考题

静态方法 `confirm()` 内部会调用 exec()。exec() 是一个阻塞调用——它会启动一个局部事件循环，直到对话框关闭才返回。这意味着在 exec() 期间，主窗口的事件循环是被"挂起"的吗？还是说对话框有自己的事件循环在处理事件？提示：exec() 内部会调用 QEventLoop::exec()，它确实在处理事件（绘制、输入、定时器等），只是不会返回到调用者——这就是为什么对话框弹出后主窗口虽然不能操作但不会"卡死"。

### 动手写

```cpp
int ConfirmDialog::confirm(const QString &title,
                            const QString &message,
                            QWidget *parent)
{
    // TODO: 创建 ConfirmDialog 实例（栈上，不用 new）
    //       ConfirmDialog dialog(title, message, parent);
    //       return dialog.exec();
    //       exec() 返回 QDialog::Accepted 或 QDialog::Rejected
}
```

就这么简单。因为对话框是模态的，exec() 返回时对话框已经关闭了，栈上的 dialog 对象自动销毁，不会有内存泄漏。

### 检查点

在 main.cpp 里测试：

```cpp
int result = ConfirmDialog::confirm("测试标题", "这是一条测试消息。");
qDebug() << "返回值:" << result
         << (result == QDialog::Accepted ? "确认" : "取消");
```

控制台应该正确输出你的选择。

---

## 最终组装

写一个稍微完整的 main.cpp 来测试各种场景：

```cpp
#include <QApplication>
#include <QDebug>
#include "confirm_dialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // TODO: 测试基本确认
    //       调用 confirm()，打印结果

    // TODO: 测试长消息文本（自动换行效果）

    // TODO: 测试修改按钮文字

    return 0;  // 不需要 app.exec()，因为 exec() 已经处理了事件循环
}
```

等等，上面这个 main.cpp 不需要 `app.exec()` 吗？确实不需要。`QDialog::exec()` 内部会启动自己的事件循环，不需要外层的 `app.exec()`。但如果你在 exec() 之后还需要继续处理事件（比如弹第二个对话框），那还是需要 `app.exec()`。保险起见用 `app.exec()` 也行，不影响。

---

## 验收标准

静态方法 confirm() 一行调用即可弹出对话框。点确认按钮返回 QDialog::Accepted。点取消按钮返回 QDialog::Rejected。按 Esc 或点窗口关闭按钮也返回 Rejected。长消息文本自动换行，不会撑爆对话框宽度。按钮文字可以自定义（比如改成中文"确认"/"取消"）。

---

## 进阶挑战

给对话框加一个"不再提示"复选框——查一下怎么在按钮区域上方加一个 QCheckBox，以及怎么通过返回值把勾选状态传递给调用者。或者加一个倒计时自动确认功能：5 秒后如果用户没操作就自动 accept()——用 QTimer::singleShot 实现。再或者，给对话框加上自定义 QSS 样式，比如圆角、阴影、按钮渐变色。

---

## 踩坑预防清单

> **坑 #1：QLabel 换行不生效**
> setWordWrap(true) 必须配合 setMinimumWidth 才能真正换行。不然 QLabel 会认为自己有无限宽度，永远不会换行。这是 Qt 文档里写了但很多人不看的经典陷阱。

> **坑 #2：静态方法里 new 了对话框但忘记 delete**
> 如果用 `new ConfirmDialog` 创建，exec() 返回后对话框确实关闭了但对象还占着内存。用栈上创建（不用 new）是最安全的做法，出了作用域自动销毁。

> **坑 #3：exec() 的"阻塞"误解**
> exec() 阻塞的是**调用线程的代码执行**，不是**事件处理**。在 exec() 期间，定时器、网络请求、绘制等事件都在正常处理。所以不用担心"对话框打开时主窗口会卡死"——它只是不能被操作（模态），不是不响应。

---

## 官方文档参考

- [QDialog Class](https://doc.qt.io/qt-6/qdialog.html) — exec、accept、reject
- [QDialogButtonBox Class](https://doc.qt.io/qt-6/qdialogbuttonbox.html) — 标准按钮
- [QStyle::StandardPixmap](https://doc.qt.io/qt-6/qstyle.html) — 标准图标枚举

到这里就大功告成了。这篇练习让你掌握了自定义对话框的核心模式——一个 exec() + accept()/reject() 的简洁协议。下一篇我们回到控件方向，挑战一个更复杂的自绘控件：iOS 风格的开关按钮。
