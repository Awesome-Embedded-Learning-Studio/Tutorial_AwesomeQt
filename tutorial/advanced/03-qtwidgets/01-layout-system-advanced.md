---
title: "3.1 布局系统进阶"
description: "stretch 设了 1 控件却纹丝不动，动态加完控件取 geometry 拿到旧值，嵌套四五层之后 resize 掉帧，页面切换缺一个顺滑的过渡，这些现象的根子都在布局系统的内层机制里。本篇讲明白六种 QSizePolicy 策略怎么参与空间分配、脏标记与 activate 怎么决定重算时机、QPropertyAnimation 怎么驱动页面滑动切换，以及 resize 卡顿为什么来自递归的 sizeHint 计算。配套示例在 examples/advanced/03-qtwidgets/01-layout-system-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.1——布局系统进阶

[入门篇布局基础](../../beginner/03-qtwidgets/01-layout-system-beginner.md) 把五大布局管理器过了一遍：QHBoxLayout、QVBoxLayout、QGridLayout 怎么拼界面，stretch 和 spacing 怎么调。做静态界面，这些对咱们确实够用。可界面一旦动起来，这些就不太够用了。动态添加一组控件之后布局错乱，嵌套四五层之后拖窗口明显掉帧，明明设了 stretch 系数控件就是不按比例分空间，页面切换还常常缺一个顺滑的过渡动画。本篇就沿着这些现象往下挖，从一段让人疑惑的代码开始。

```cpp
auto *layout = new QHBoxLayout;
auto *label = new QLabel("标题");
label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
layout->addWidget(label, 1);  // stretch = 1
auto *edit = new QTextEdit;
edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
layout->addWidget(edit, 1);   // stretch = 1
```

label 和 edit 的 stretch 都是 1，按直觉，resize 窗口时两者该按比例分享多余空间。实际跑起来，label 的宽度纹丝不动，额外空间全被 edit 吃掉。咱们把这个反差记在心里，往下看布局怎么分空间，答案自己会浮出来。

## 空间分配的三轮计算：stretch 只对过了筛子的策略生效

入门篇说过，sizePolicy 决定控件“愿不愿意接受额外空间”，当时没有展开它具体怎么影响空间分配，现在咱们把这件事算清楚。

QSizePolicy 定义了六种策略，布局管理器对每一种的处理逻辑都不一样，咱们把语义摆齐：

| 策略      | 语义                                                                               |
| --------- | ---------------------------------------------------------------------------------- |
| Fixed     | 控件只要 sizeHint 指定的大小，多了不要，少了不行                                   |
| Minimum   | 控件至少需要 sizeHint 的大小，但可以更大                                           |
| Maximum   | 控件最多接受 sizeHint 的大小，不能再大                                             |
| Preferred | 控件希望得到 sizeHint 的大小，比它大或比它小都能接受，绝大多数控件的默认策略就是它 |
| Expanding | 和 Preferred 类似，但它会主动要求尽可能多的空间                                    |
| Ignored   | 完全忽略 sizeHint，控件的大小完全由布局决定                                        |

空间的分配大致分三轮，咱们跟着走一遍。第一轮，布局遍历所有子项，收集每个子项的 sizeHint，连同 sizePolicy 和 stretch 一起记下。第二轮做筛选，看哪些子项可以接受额外空间，Expanding 和 Preferred 参与分配，Fixed 不参与，然后按 stretch 比例把剩余空间分给这些子项。第三轮，检查分配结果是否满足每个子项的 minimumSizeHint 和 maximumSize 约束，有冲突就做二次调整。

现在咱们回头看开篇那段代码。label 是 Fixed 策略，第二轮的筛选没把它算进去，它根本进不了“按 stretch 比例分配”的环节，stretch=1 对它没有意义。stretch 只对愿意接受额外空间的策略生效。您要是确实想让 label 分到一定比例的空间，把它的水平策略改成 Preferred 或 Minimum，stretch 才有参与的资格。

stretch 的作用范围还有一条边界，它只作用于一个布局的直接子项。嵌套布局里这一点特别容易出问题，因为 setStretch 设错了不报任何错误，stretch 就是安静地不生效。比如 QVBoxLayout 里套了一个 QHBoxLayout，您对 QVBoxLayout 调 setStretch(1, 2)，设置的是内层 QHBoxLayout 这个子布局在垂直方向的 stretch，影响不到 QHBoxLayout 里面的任何控件。发现 stretch 不生效时，排查分两步：先确认自己操作的是正确的那个布局对象，再确认目标控件的 sizePolicy 确实允许它接受额外空间，别是 Fixed。

六种策略之外，还有两个函数也参与布局和控件的尺寸协商，咱们来看 expandingDirections()。水平方向的 Expanding 策略意味着 expandingDirections() 会包含 Qt::Horizontal，布局管理器看到这个标记，就知道要优先给这个控件分配水平方向的额外空间。hasHeightForWidth() 管的是另一件事，它告诉布局这个控件的高度会不会随宽度变化，自动换行的文本控件是典型场景。QTextEdit 默认在垂直方向是 Expanding，也是同一个道理，它需要尽可能多的纵向空间来显示文本。

### Ignored 的失效方式：没有下限的控件能被压到 0

六种策略里 Ignored 最特殊，也最容易让界面凭空少一个控件。“完全忽略 sizeHint，大小由布局决定”听起来没毛病，但您给一个没有内容的控件（比如空文本的 QLabel）设了 Ignored，它的 sizeHint 是 (0, 0)，minimumSizeHint 也是 (0, 0)，布局在没有任何约束的情况下会把它的尺寸压到 0，控件直接从界面上消失。更糟的是它要是跟别的控件并排在一个水平布局里，消失之后布局重排，整行看起来都可能不对。笔者的建议是，只在确实需要布局完全控制大小的场景用 Ignored，并且给它一个 minimumSize 兜底，或者干脆改用 Minimum，Minimum 至少保证 sizeHint 作为下限。

配套示例在 `examples/advanced/03-qtwidgets/01-layout-system-advanced/`，示例程序叫 SizePolicyDemo，`cmake -B build && cmake --build build` 构建后运行，窗口里有三个演示区，每个演示区下方都有自己的说明标签。最上面一排六根色条对应六种策略，咱们拖宽窗口就能看到红色 Fixed 条宽度不变，蓝色 Expanding 条把多余空间吃掉。

中间的演示区可以下拉切换中间那根色条的 sizePolicy，把它切到 Fixed 再拖窗口，它会立刻从跟着伸缩变成定住不动。最下面三根色条都是 Expanding，旁边的三个 spinbox 控制各自的 stretch，默认 1:2:3。咱们把三个值全归零，三根条平分空间，改成 1:1:8，第三根明显占大头。下方的说明标签还会顺带提示一句：stretch 只对 Expanding、Preferred 这类策略生效，对 Fixed 无效。

## 增删控件之后：重算发生在下一次事件循环

静态界面的布局一次就完事，咱们在实际项目里会大量遇到动态增删：设置面板添加新分组，表格动态插入行，向导页面切换内容。QBoxLayout 提供 `insertWidget(int index, QWidget*)` 在指定位置插入控件，对应的 `removeWidget(QWidget*)` 移除控件，QGridLayout 的 `addItem()` 还能在指定行列放置子项。接口本身不难记，难的是弄清楚操作完之后，布局什么时候真的重算。

布局管理器内部维护着一套脏标记机制。咱们添加或移除子项时，布局只是被标记成“需要重新计算”，并不会立即执行计算。计算发生在下一次事件循环迭代时，或者您主动调用 `invalidate()` / `activate()` 时。`invalidate()` 清空所有缓存的几何信息，让布局在下一次布局时机重新计算。`activate()` 更进一步，立即触发一次布局计算并应用结果。

这个设计意味着，咱们在一个函数里连续添加五个控件，布局不会计算五次，它会等函数返回后的下一次事件循环统一算一次。合并计算是合理的，避免了频繁重算。但要是您需要在添加控件之后立刻拿到它的实际几何位置，比如用它定位一个覆盖层（overlay），就必须手动调用 `activate()` 强制立即计算：

```cpp
layout->addWidget(new_panel);
layout->activate();  // 立即计算，之后才能拿到 new_panel 的正确 geometry
auto pos = new_panel->pos();  // 否则这里拿到的是旧值或零值
```

反过来，addWidget 之后什么都不做，也有它隐蔽的风险。新控件可能在下一次事件循环之前完全不显示，或者显示在错误的位置。这在同步代码里最容易误判，您以为函数执行完控件就该出现了，实际布局还没来得及重算。新控件的位置或大小要是还参与后续的逻辑判断（比如根据它的位置来放置另一个元素），拿到的就是错误的值。选择也简单：需要立即取几何信息，调 `layout->activate()`，只是希望控件正确显示，`layout->invalidate()` 让事件循环代劳就够了。

配套示例里就有一处现成的用法。stretch 演示区的 `updateStretchValues()` 在三个 spinbox 变动之后先 `invalidate()` 再 `activate()`，把新的 stretch 立刻反映到三根色条上。咱们拨一下 spinbox，色条宽度当场变化，这两行调用就是直接原因。

还有一个细节值得您留意：`removeWidget()` 之后，控件去了哪儿。布局只是不再管理它，它的所有权还和加入时一样，仍挂在原来的父对象上，并且不会自动隐藏，会停在原来的位置继续显示。咱们接下来怎么处置它，得看需求。只想让它从界面上消失，显式调用 hide() 就够。控件确实不要了，就手动 delete，或者用 `QObject::deleteLater()` 延迟删除。您的代码要是还在其他地方引用这个控件，延迟删除更安全。

## 页面切换滑起来：让新旧两个 geometry 一起动

动态增删说的是内容变了之后布局怎么追上来，页面切换也一样，只是换的是整页的内容。入门篇用 QStackedLayout 做过页面切换，切换是瞬间完成的，没有任何过渡效果。产品级的界面通常需要一个滑动或淡入淡出的动画来衔接。Qt 没有直接提供带动画的 StackedLayout，咱们可以用 QPropertyAnimation 手动实现一个。

咱们的做法是绕开 QStackedLayout 的 setCurrentIndex，自己接管页面切换：切换时让旧页面和新页面同时可见，咱们再用 QPropertyAnimation 驱动两者的 geometry 属性从旧位置滑动到新位置，动画结束后再隐藏旧页面。

```cpp
void slideToWidget(QWidget* target, int direction)
{
    // direction: +1 向左滑（显示右侧页面），-1 向右滑（显示左侧页面）
    auto *anim_old = new QPropertyAnimation(current_widget, "geometry");
    auto *anim_new = new QPropertyAnimation(target, "geometry");

    QRect base_rect = container->rect();
    anim_old->setStartValue(base_rect);
    anim_old->setEndValue(base_rect.translated(-direction * base_rect.width(), 0));
    anim_new->setStartValue(base_rect.translated(direction * base_rect.width(), 0));
    anim_new->setEndValue(base_rect);

    target->show();
    target->raise();
    anim_old->start(QAbstractAnimation::DeleteWhenStopped);
    anim_new->start(QAbstractAnimation::DeleteWhenStopped);

    // 动画结束后隐藏旧页面，更新 current_widget
    connect(anim_new, &QPropertyAnimation::finished, this, [this, target]() {
        current_widget->hide();
        current_widget = target;
    });
}
```

这个方案靠的是同时操作两个页面的 geometry，让一个滑出去的同时另一个滑进来。direction 参数控制滑动方向，实现左进右出或右进左出。动画期间两个控件都必须可见，所以您需要用 raise() 控制层叠顺序。动画的时长建议设在 200 到 300 毫秒，再长用户会觉得卡，再短看不清过渡。QEasingCurve::OutCubic 是一个比较自然的缓动曲线，适合页面切换。

## resize 卡顿的来源：每帧一次的递归 sizeHint 计算

咱们大概见过这样的界面：设置面板里布局套布局，四五个层级往下叠，拖动窗口边缘时刷新明显掉帧。表象是卡，咱们把 activate() 在嵌套结构里的行为一层层看下去，原因就清楚了。

布局的 activate() 会触发一次递归 sizeHint 计算，从顶层一直算到底层。每一层布局得先知道所有子项的 sizeHint，才能计算自己的 sizeHint，然后一层层向上汇报。N 层嵌套，每次 activate 就是一次 O(N * M) 的计算，M 是每层的平均子项数。咱们再把它放进 resize 的场景：窗口拖动时，这个计算每一帧执行一次。

想给这样的结构减负，最直接的一招是减少嵌套层级，能用一个 QGridLayout 排下的版面，就别用多层 QBoxLayout 套出来。接着，咱们可以给固定大小的控件用 `setFixedSize()`，而不是只设 minimumSize，固定大小的控件在 sizeHint 计算时可以跳过某些约束检查，计算量小一些。真到了特别复杂的场景，比如列表项里嵌了大量控件，可以考虑用自定义布局类替代多层标准布局嵌套：直接重写 QLayoutItem 的 sizeHint 和 setGeometry，在一次计算中完成所有子项的定位，避免递归。

还有一个批量增删时能用的招：临时禁用布局。调用 `setEnabled(false)`，禁用中的布局如同不存在，期间不会响应增删和尺寸变化去做重排，等全部变更完成，再 `setEnabled(true)`，补一次 `activate()`，把所有变更合并成一轮计算。不过这套开关得咱们自己配对好，中间忘了重新启用，界面就停在不刷新的状态，实测确实碰到明显的性能问题时再用它也不迟。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，依赖 QtWidgets 模块，动画部分额外用到 QtCore 的 QPropertyAnimation，示例可在任何支持 Qt6 的桌面平台上编译运行。

[Qt 文档 · QSizePolicy](https://doc.qt.io/qt-6/qsizepolicy.html) -- 六种空间策略的完整定义和 stretch 因子说明

[Qt 文档 · QLayout](https://doc.qt.io/qt-6/qlayout.html) -- 布局基类接口，包含 activate / invalidate / setGeometry 等核心方法

[Qt 文档 · QStackedLayout](https://doc.qt.io/qt-6/qstackedlayout.html) -- 页面切换布局，setCurrentIndex 和 currentChanged 信号

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画类，用于驱动 geometry 等属性做插值动画

[Qt 文档 · QBoxLayout](https://doc.qt.io/qt-6/qboxlayout.html) -- 水平/垂直布局基类，insertWidget / removeWidget / addStretch 接口

[Qt 文档 · Layout Management](https://doc.qt.io/qt-6/layout.html) -- Qt 布局系统概述，包含空间分配算法的高层说明

---

布局系统的内层机制就看到这里。想再动手的，请您把 SizePolicyDemo 的三个演示区挨个玩一遍，尤其试试在动态切换区把六种策略轮着选，再对照每个演示区下方的说明标签，看咱们预判的现象和标签的解释对不对得上。下一篇 [3.2 事件处理进阶](./02-event-handling-advanced.md) 讲事件处理与传播机制，也就是 Qt 应用怎么响应用户操作的那一层。
