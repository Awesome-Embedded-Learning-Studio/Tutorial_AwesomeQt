# 实战练习 · SearchEdit — 用"组合"而不是"自绘"造控件

## 前言：换个姿势造轮子

上一篇 StatusLed 我们走的是"自绘"路线——继承 QWidget，重写 paintEvent，用 QPainter 一笔一笔画。这条路很强大，但说实话，不是每个自定义控件都需要从零画起。很多时候，Qt 已经给了你几乎够用的零件，你要做的只是把它们拼起来，再稍加修饰。

搜索输入框就是最典型的例子。Qt 有 QLineEdit，功能已经很完整了——文本编辑、光标、选择、撤销、信号槽，全都有。我们要做的只是给它加一个左侧搜索图标和右侧清除按钮，然后套一层样式表让它看起来不那么朴素。整个过程**不需要重写 paintEvent**，不需要碰 QPainter，完全靠组合已有控件和 QSS 样式来实现。

这种"组合"的方式在工程里其实比"自绘"用得更频繁，因为它开发快、风险低、和原生控件的兼容性也更好。所以我们这篇练习的核心目标不只是做一个搜索框，而是掌握"如何不碰 paintEvent 就造出一个看起来很专业的控件"。

---

## 出发前的装备清单

- **QLineEdit** — Qt 的单行文本输入框，我们直接继承它。如果你用过 Qt 的输入控件，它应该很眼熟了。这次我们会用到它的一些"隐藏"功能，比如 addAction。
- **QAction** — 通常我们把它跟菜单和工具栏联系在一起，但它其实可以嵌入到 QLineEdit 里作为图标按钮来用。这是个很多人不知道的技巧。
- **QStyle** — Qt 的风格系统，可以获取平台原生的标准图标（比如搜索放大镜），不用自己找图标文件。
- **QSS (Qt Style Sheets)** — 类似 CSS 的样式系统，用来控制边框、圆角、内边距、焦点效果等视觉属性。

---

## 我们的目标长什么样

我们要做一个这样的搜索输入框：

```
┌─────────────────────────────────┐
│ 🔍  输入搜索关键词...        ✕  │
└─────────────────────────────────┘
```

左侧有一个搜索图标（放大镜），中间是文本输入区域，右侧有一个清除按钮（×），但清除按钮**只在有文字时才显示**。输入框有圆角边框，获得焦点时边框变色，placeholder 文本显示"搜索..."。

完成标准：在 main.cpp 里创建一个 SearchEdit，输入文字时清除按钮自动出现，点击清除按钮后文本清空且焦点回到输入框，按回车键能在控制台打印当前搜索文本。

---

## 第一步 — 继承 QLineEdit：先有个能用的壳

### 思考题

我们选择直接继承 QLineEdit，而不是继承 QWidget 再在里面放一个 QLineEdit。想想为什么？提示：继承 QLineEdit 意味着所有 QLineEdit 的原始 API（text()、setText()、placeholder、maxLength、信号等等）都自动可用，外部代码不需要做任何适配。反过来，如果你在外面套了一层 QWidget，外面要访问文本就得你自己转发一堆方法。这就是"继承"vs"组合"在设计上的取舍——前者省事但耦合紧，后者灵活但啰嗦。对于搜索框这种简单场景，继承是更务实的选择。

### 动手写

头文件骨架：

```cpp
#pragma once

#include <QLineEdit>

class SearchEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit SearchEdit(QWidget *parent = nullptr);

    // TODO: 你觉得需不需要自定义信号？
    //       QLineEdit 本身已经有 textChanged、returnPressed 等信号。
    //       看看它们是否已经够用。

private:
    // TODO: 你需要持有哪些子对象的引用？
    //       提示：清除按钮的 QAction 或者 QToolButton。

    // TODO: 需要哪些私有方法？
    //       比如初始化 UI 的 setupUi() 之类的。
};
```

构造函数里先调用 QLineEdit 的构造函数，把 parent 传上去。然后调用一个私有的 setupUi() 方法来做初始化——把初始化逻辑从构造函数里拆出来是个好习惯，构造函数保持干净。

```cpp
SearchEdit::SearchEdit(QWidget *parent)
    : QLineEdit(parent)
{
    // TODO: 调用 setupUi()
    // TODO: 设置一些默认属性，比如 placeholder 文本
}
```

### 检查点

编译运行。在 main.cpp 里创建一个 SearchEdit 并 show 出来。你现在看到的应该就是一个普通的 QLineEdit——没关系，接下来我们一步步加东西。

---

## 第二步 — 左侧搜索图标：QLineEdit 的隐藏技能

### 思考题

QLineEdit 有一个很多人不知道的方法叫 `addAction(QAction*, ActionPosition)`。去 Qt 文档里查一下这个方法，看看 ActionPosition 有哪些值，以及添加 action 后 QLineEdit 会在视觉上产生什么变化。

### 动手写

在 setupUi() 里，我们要做这么一件事：创建一个 QAction，给它设置一个搜索图标，然后添加到 QLineEdit 的 LeadingPosition（左侧）。这个图标有两种获取方式：用 QStyle 的标准图标（跨平台风格统一），或者用 QIcon 加载自定义图标文件。我们先用 QStyle 的标准图标，简单又不依赖外部资源。

```cpp
void SearchEdit::setupUi()
{
    // TODO: 创建一个 QAction
    //       用 addAction() 添加到 QLineEdit::LeadingPosition
    //
    //       图标的获取方式：
    //       QStyle::StandardPixmap 枚举里找找看有没有搜索相关的图标。
    //       用 style()->standardIcon(...) 获取。
    //       如果找不到合适的，QStyle::SP_DialogResetButton 或者
    //       自定义一个 Unicode 字符（比如 U+1F50D）也行。
}
```

一个小细节：addAction 返回的 QAction* 你可以存也可以不存，取决于你以后要不要操作它（比如响应点击）。对于搜索图标，通常只是装饰性的，用户不会去点它，所以不存也行。

### 你可能会遇到的坑

如果你用的是 QStyle 标准图标，不同平台上的图标长得不一样——Windows 上是一种风格，Linux 上是另一种，macOS 又是一种。这是 Qt 风格系统的工作方式，不是 bug。如果你想要完全一致的跨平台图标，就得自己准备图标资源文件了。

### 检查点

运行程序，搜索输入框左侧应该出现了一个小图标。如果没看到，检查一下 QAction 是否正确创建并添加到了 LeadingPosition。

---

## 第三步 — 右侧清除按钮：条件显示

### 思考题

清除按钮只在有文字时才出现，文本为空时隐藏。这意味着我们需要监听文本变化事件。QLineEdit 有哪个信号适合做这件事？`textChanged` 还是 `textEdited`？区别是什么？提示：一个在程序调用 setText 时也会触发，另一个只在用户输入时才触发。对于这个场景，你觉得用哪个更合适？

### 动手写

清除按钮的实现有两种常见方案。第一种方案是再用一个 QAction 放在 TrailingPosition（右侧），通过 `QLineEdit::addAction`。第二种方案是自己创建一个 QToolButton 作为 QLineEdit 的子控件，手动定位。第一种更简单也更"正统"，我们用这种。

```cpp
// 在 setupUi() 里继续：
// TODO: 创建第二个 QAction，作为清除按钮
//       添加到 QLineEdit::TrailingPosition
//       设置一个合适的图标（关闭/清除相关的 StandardPixmap）
//       初始状态设为不可见

// TODO: 连接 textChanged 信号到一个 lambda
//       lambda 逻辑：如果 text().isEmpty()，隐藏清除 action；否则显示。
//       提示：QAction 用 setVisible() 控制显隐。

// TODO: 连接清除 action 的 triggered 信号
//       逻辑：clear() 清空文本，然后 setFocus() 让输入框重新获得焦点
```

这里有个设计上的考量：`clear()` 会触发 `textChanged` 信号，而 `textChanged` 又会把清除按钮隐藏。所以清除按钮的点击流程是：用户点击 → clear() → textChanged 触发 → 清除按钮隐藏。这个链条是自动的，不需要你手动管理清除按钮的显隐——信号槽帮你搞定了。

### 你可能会遇到的坑

清除按钮点击后，焦点会被按钮抢走，输入框失去焦点。如果你想让用户能继续输入，必须在 clear() 之后调用 `setFocus()`。不然用户点完清除按钮还得再点一下输入框才能继续打字，体验很烂。

### 检查点

输入一些文字，清除按钮应该自动出现。点击清除按钮，文字消失，清除按钮隐藏，光标回到输入框里可以继续打字。清空后清除按钮应该消失。

---

## 第四步 — QSS 美化：让它看起来像搜索框

### 思考题

一个普通的 QLineEdit 看起来就是一个方方正正的输入框，和"搜索框"的视觉预期差得远。QSS 能做哪些事情让它更专业？提示：圆角边框、内边距（给图标腾空间）、焦点时的边框高亮、placeholder 文本颜色。

### 动手写

在构造函数或 setupUi 里加上样式表。QSS 的语法和 CSS 很像，但不是所有 CSS 属性都支持。Qt 文档里有一个完整的 QSS 属性列表，建议翻一下。

```cpp
// 在 setupUi() 的最后：
// TODO: 用 setStyleSheet() 设置样式
//
// 你需要考虑的样式属性：
// 1. border — 边框样式（圆角用 border-radius）
// 2. padding — 内边距，左侧要给搜索图标留空间
// 3. background — 背景色
// 4. selection-background-color — 选中文字的背景色
// 5. :focus 伪状态 — 获得焦点时的样式变化（边框颜色变深或变蓝）
```

关于 padding 的计算：左侧 padding 需要足够容纳搜索图标的宽度加上一点间距。你可以先设一个比较大的值（比如 24px），然后运行看效果再调整。

一个容易忽略的细节是 `setMinimumWidth()`——搜索框太窄的话图标和文字会挤在一起，很难看。建议设置一个合理的最小宽度。

### 你可能会遇到的坑

QSS 有选择器优先级的概念，和 CSS 类似。如果你在 QApplication 级别设了全局样式，又在这里设了局部样式，两者的冲突可能会导致一些意料之外的效果。调试 QSS 最好的办法是先去掉全局样式，确认局部样式正确，再慢慢加回来。

### 检查点

现在你的搜索框应该有了圆角边框，获得焦点时边框颜色会变化，placeholder 文本是浅灰色的。左侧搜索图标和右侧清除按钮不会被文字遮挡（padding 足够）。整体看起来像一个真正的搜索框，而不是裸奔的 QLineEdit。

---

## 最终组装

main.cpp 很简单——创建一个 QWidget 或 QMainWindow，放一个 SearchEdit 进去，再连一下 returnPressed 信号方便测试：

```cpp
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>

#include "search_edit.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // TODO: 创建主窗口 QWidget
    // TODO: 创建 QVBoxLayout，放入 SearchEdit
    // TODO: 连接 returnPressed 信号，打印当前文本到 qDebug
    // TODO: 设置窗口大小，show

    return app.exec();
}
```

---

## 验收标准

输入文字时清除按钮自动出现，清空文字后清除按钮自动消失。点击清除按钮后文本清空、焦点回到输入框、清除按钮隐藏，三件事同时发生。按回车键控制台正确打印当前搜索文本。输入框有圆角边框，获得焦点时视觉上有变化。placeholder 文本"搜索..."在空输入时可见，输入内容后消失。窗口缩放时搜索框正常跟随布局调整。

---

## 进阶挑战

给你的 SearchEdit 加一个 QCompleter，输入时自动弹出补全建议列表。或者加一个"搜索历史"功能：每次按回车就把搜索词存下来，下次输入时可以通过 QCompleter 看到历史记录。再或者，让清除按钮有一个淡入淡出的过渡效果而不是突然出现消失——提示查一下 QWidget 的 QGraphicsOpacityEffect 和 QPropertyAnimation。

---

## 踩坑预防清单

> **坑 #1：addAction 的 QAction 对象生命周期**
> QAction 必须 setParent 或通过合适的方式管理生命周期。如果你在函数局部作用域里 `new QAction` 但没有设置 parent，函数结束后它就变成野指针了。解决：创建时传入 this 作为 parent，或者用成员变量持有。

> **坑 #2：QSS 选择器没生效**
> 如果你写 `QLineEdit { ... }` 但没效果，可能是因为样式被更高优先级的规则覆盖了。对于继承自 QLineEdit 的自定义类，你可以用类名作为选择器：`SearchEdit { ... }`。

> **坑 #3：清除按钮点击后焦点丢失**
> 上面已经提过，但再强调一遍：clear() 之后必须 setFocus()。这真的是个超常见的 UX 反模式，很多"专业"软件也在犯这个错。

---

## 官方文档参考

- [QLineEdit Class](https://doc.qt.io/qt-6/qlineedit.html) — addAction、placeholder、textChanged 信号
- [QAction Class](https://doc.qt.io/qt-6/qaction.html) — 图标和触发
- [QStyle Class](https://doc.qt.io/qt-6/qstyle.html) — standardIcon、StandardPixmap 枚举
- [Qt Style Sheets](https://doc.qt.io/qt-6/stylesheet.html) — QSS 语法和属性

到这里就大功告成了。上一篇我们用自绘的方式画了一个 LED，这篇我们用组合的方式拼了一个搜索框——两个练习下来，你已经掌握了自定义控件的两大主流手法。下一篇我们换个方向，从"造控件"转向"摆控件"，练一练 Qt 里最实用的布局模式：表单布局。
