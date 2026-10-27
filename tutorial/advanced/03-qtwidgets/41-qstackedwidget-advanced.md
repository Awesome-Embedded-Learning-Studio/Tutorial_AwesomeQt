---
title: "3.41 QStackedWidget 进阶"
description: "入门篇用 QStackedWidget 配 QComboBox 和 QListWidget 搭过导航界面，进阶篇回到页面容器那半边对着 6.9.1 读实现，QStackedWidget 继承 QFrame 且本体只是 QStackedLayout 的转发壳、三个信号靠构造里 connect 转发、setCurrentWidget 验明在册才放行而 setCurrentIndex 越界静默返回、切换全程包在 setUpdatesEnabled 里中间态不绘制所以本身不闪、缺的只是几何插值、焦点按三级规则移交、insertWidget 抬索引不发 currentChanged、removeWidget 走 delete takeAt 删的是包装条目页面本体无恙、删当前页按保位补前一位、widgetRemoved 带删除前索引、所有权留在堆叠件上复用要 re-parent、StackingMode 默认 StackOne、StackAll 全员可见当前页仅 raise、切回 StackOne 时 currentIndex 为 0 可见性整理被整个跳过、sizeHint 取全体页最大值窗口不为切页跳动、加滑动动画要遮蔽非 virtual 的 setCurrentIndex 且演完直调基类对齐状态，配套示例在 examples/advanced/03-qtwidgets/41-qstackedwidget-advanced/"
---

# 现代Qt开发教程（进阶篇）3.41——QStackedWidget 进阶

[3.40 QTabBar 进阶](./40-qtabbar-advanced.md) 收尾时咱们约好回到页面容器那半边，看堆叠页与标签栏怎么在各写各的代码里保持同构。QStackedWidget 在 39 里当过配角，qt_tabwidget_stackedwidget 这个对象名咱们照过面，QTabWidget 删标签，靠的也是堆叠件的 widgetRemoved 回流驱动。本篇把它扶正单独读，您会看到两边的机制处处对得上，删除当前页后的补位规则，与 QTabBar 的 SelectRightTab 同一套保位语义。源稿的重心放在切换动画上，容器本身的机制几乎空白，本篇按 6.9.1 源码重排，动画收进末节单独交代。

本篇引用以 qstackedlayout.cpp 为主，壳侧补 qstackedwidget.cpp 与 qstackedwidget.h，删除路径补 qlayout.cpp，继承链补 qframe.cpp。配套示例的完整玩法咱们放在结语，正文引用文件名加行号。

## 壳与芯：QStackedWidget 的每一行都在转发

继承链的头一行写在 qstackedwidget.h 第 16 行，QStackedWidget 继承 QFrame。它是 QFrame 后代这件事平时看不出来，默认 frameStyle 是 NoFrame 加 Plain，出处（qframe.cpp 第 20 行），边框一个像素都不画，但 setFrameShape、setLineWidth 和 QSS 的边框位都随时可用，[3.45 QFrame 进阶](./45-qframe-advanced.md) 咱们专门读它，这里挂链不展开。私有数据也顺着继承链走，QStackedWidgetPrivate 继承 QFramePrivate，成员只有一个布局指针，出处（qstackedwidget.cpp 第 12-18 行）。

构造体里拢共几句话（qstackedwidget.cpp 第 108-118 行）：new 一个 QStackedLayout 装进自己，随后三句 connect，把布局的 currentChanged、widgetRemoved、widgetAdded 逐一转发过来，信号名一个不改。此后 addWidget、insertWidget、removeWidget、count 全是一行委托，机制本体都在布局那边。读它的法子是把壳当目录、把 QStackedLayout 当正文，咱们下面直接进正文。widgetAdded 是 6.9 新增的信号（第 92-100 行），页面动态增删的同步逻辑从此不必拿 indexOf 轮询。

## 两扇门进同一间客厅：setCurrentWidget 验明在册，setCurrentIndex 静默返回

切换有两个入口。setCurrentWidget 在壳这边有独立守卫，indexOf 返回 -1 就发一句 qWarning 直接返回，警告原文是 “not contained in stack”，出处（qstackedwidget.cpp 第 220-228 行）。setCurrentIndex 没有这等待遇，越界索引走到布局里被 widget(index) 的 nullptr 挡回来，函数静默返回，一个字都不吭（qstackedlayout.cpp 第 277-280 行）。您调试时切页没反应，头一件事就是核对索引合法性。两扇门在 QStackedLayout::setCurrentIndex 汇合，setCurrentWidget 进来时，经布局的同名函数把指针换成索引，再进同一扇正门（第 349-357 行）。

正门里的切换比源稿写的“旧页 hide、新页 show，完事”厚实得多。函数开头取 prev 与 next，next 为空或者就是 prev，直接返回，切到当前页不重发信号（第 277-280 行）。切换全程包在父件的 setUpdatesEnabled(false) 与 true 之间，出处（第 282-288、329-330 行），中间态不进绘制，源稿说的一帧闪切并不存在，切页本身不闪，缺的只是几何插值，那是动画的活儿，末节咱们自己补。

骨架咱们摘出来对着看。

```cpp
// 切换的骨架（qstackedlayout.cpp 第 293-301 行）：
if (prev) {
    prev->clearFocus();
    if (d->stackingMode == StackOne)
        prev->hide();
}
d->index = index;
next->raise();
next->show();
```

顺序咱们逐行念下来：旧页让出焦点，StackOne 模式下才 hide，d->index 落位，新页 raise 再 show。hide 看模式，raise 不看，raise 这一步在 StackAll 模式里有特殊含义，下节展开。

焦点移交这段笔者停了半拍，它比想象中讲究。切换前记下窗口的焦点件，焦点原本落在旧页上的话，切完替它安排新家：头一个候选是新页的 focusWidget()（第 309-310 行），找不到就沿焦点链扫一圈，挑第一个能 Tab、可见、可用的件（第 313-318 行），再兜底给新页本体（第 323-324 行）。您做表单类页面，切页后焦点跟不跟手全看这一段。currentChanged 在末尾才发，参数是新索引（第 331 行）。

## 增删页：索引怎么挪，删掉的页面归谁

addWidget 是 insertWidget 的别名，转手传列表长度（qstackedlayout.cpp 第 179-183 行）。insertWidget 的规则在第 199-220 行：索引越界按追加算，负数同样处理（第 203-205 行）。堆叠还是空的时候，新页直接走 setCurrentIndex 上位，出处（第 209-210 行）。已有当前页时，插进的位置不晚于当前索引，d->index 就抬一格，当前件不换人（第 212-213 行）。您靠 currentChanged 同步导航控件的话，插入页面不会打扰它，信号认部件换人，索引挪动不入它的眼。StackOne 下新页被 hide，一律 lower 垫底（第 214-216 行），末尾发 widgetAdded（第 218 行）。

删除的路径多绕一站。QStackedWidget::removeWidget 委托给 QLayout::removeWidget，后者沿 itemAt 找到包装条目，delete takeAt(i)（qlayout.cpp 第 1297-1312 行），删掉的是 QWidgetItem 包装，页面本体无恙。后继规则在 takeAt 里（qstackedlayout.cpp 第 243-264 行），与 QTabBar 删当前标签的保位语义同构：删的不是当前页、位置又在当前索引之前，d->index 减一格，不发信号（第 257-259 行）。删的正是当前页，d->index 归 -1，列表还有人就让同位置的新面孔经 setCurrentIndex 补位，删的是末位则取前一位（第 249-253 行），删空了直接发 currentChanged(-1)，出处（第 254-256 行）。hide 之前还有个细节值得咱们多看一眼，删除发生在部件析构里时，hide 会被跳过，qt_wasDeleted 拿私有标志挡了一下（第 234-237、261 行），析构中的对象经不起再碰显示状态。

widgetRemoved 带着删除前的索引发出（第 260 行），咱们自己写的槽函数拿到的也是这个语义，39 篇 QTabWidget 删标签全靠它回流驱动。

所有权笔者单独核对过注释，removeWidget 不删除页面，父对象与父件都还是 QStackedWidget，想复用就自己 re-parent（qstackedwidget.cpp 第 163-177 行）。被删的页面几何停在旧位置、处于隐藏态，放着不管，它就以隐形孩子的身份留在堆叠件里，堆叠件析构时一并带走。想收回手动管理，removeWidget 之后补一句 setParent(nullptr)，这一步省不得。

## StackingMode：StackOne 只亮一页，StackAll 全员可见当前页压顶

咱们从默认值记起，私有构造把 stackingMode 初始化成 StackOne，出处（qstackedlayout.cpp 第 21 行），只亮当前页是常态。StackAll 反过来，全部页面可见，当前页只是被 raise 压到顶（枚举文档第 516-521 行），属性注释给的场景是图形编辑器的叠层绘制，半透明画布盖在内容上（第 524-533 行）。布局的 setGeometry 按模式分叉（第 452-467 行）：StackOne 只把矩形给当前页，StackAll 给所有页同一个矩形。咱们顺带记一笔，insertWidget 的 hide 只在 StackOne 下执行（第 214-216 行），StackAll 里新页插进来不藏，直接以 lower 的姿态垫在底下候场。这也埋下动画一节的伏笔，隐藏页的几何是陈旧的，轮到您把它切上场才被摆正。

切换模式的代码笔者读了两遍才敢信。切到 StackAll 时，当前页的几何被复制给全部页面再统一 setVisible(true)，叠层整整齐齐（第 559-569 行）。切回 StackOne 的分支请您看摘录。

```cpp
// 切回 StackOne 的分支（qstackedlayout.cpp 第 553-558 行）：
case StackOne:
    if (const int idx = currentIndex())
        for (int i = 0; i < n; ++i)
            if (QWidget *widget = d->list.at(i)->widget())
                widget->setVisible(i == idx);
    break;
```

条件拿 currentIndex() 的返回值直接当布尔值，当前页恰好是 0 号时整个整理循环被跳过，所有页面留在可见状态。您拿 6.9.1 做实验，从 StackAll 切回 StackOne 会看到几页内容叠在一起晾着，可见性的整理得自己补一句循环。另外 QStackedWidget 没把 stackingMode 暴露成属性，头文件里查无此名（qstackedwidget.h），要调 setStackingMode 得从 layout() 取出布局再 static_cast<QStackedLayout*> 转过去，壳转发了一切，唯独漏了这一项。

## sizeHint 取全体页的最大值，窗口不为切页跳动

sizeHint 从 (0,0) 起步，逐页 expandedTo 取最大（qstackedlayout.cpp 第 416-432 行），某页某轴的 sizePolicy 是 Ignored，那一维按 0 计（第 425-428 行）。minimumSize 同理，取全页最大的 qSmartMinSize，出处（第 437-447 行）。后果您一用就能感到：切到小页面窗口不缩，布局谈判的底牌是最大那页，导航类界面不会切一次页跳一次尺寸。

heightForWidth 有段源码注释值得念给您，它跳过 QLayoutItem，直接问 widget 本体，注释写明原因是 QWidgetItem::heightForWidth 对隐藏件会返回 -1，出处（第 493-505 行）。隐藏页照样参与高度计算，只是问法不同。反过来这也是约束，几页尺寸悬殊时堆叠件常驻最大页的地盘，想按当前页收缩，只能监听 currentChanged 自己重设尺寸策略，示例没做这一步，您扩展时可以补。

## 滑动动画：遮蔽非 virtual 的 setCurrentIndex，演完回基类对齐

给切换加动画，源稿的方向是对的，机制上有一处硬约束必须交代。setCurrentIndex 没有 virtual 修饰，就是普通槽（qstackedwidget.h 第 37-39 行），子类的同名函数只能算遮蔽，经基类指针或引用调用会绕过动画直达基类实现。您翻示例的类注释，这层后果写得很清楚，出处（slide_stacked_widget.h 第 32-36 行），注释同样写明 setCurrentWidget 没被遮蔽，直达基类实现，直接切不播动画，出处（文件头的类注释第 20-21 行）。

动画流程咱们照示例走。入口三道守卫，非法索引、同页请求、动画进行中一律返回，出处（slide_stacked_widget.cpp 第 22-29 行）。新页被 move 到视口外，show 加 raise 让它站上桌面（第 61-64 行）。两个 QPropertyAnimation 只动 pos 属性，旧页滑出、新页滑进，包在 QParallelAnimationGroup 里同步，出处（第 73-86 行）。收尾在 finished 回调里，咱们把摘录放在下面。

```cpp
// finished 收尾（slide_stacked_widget.cpp 第 89-98 行）：
connect(group, &QParallelAnimationGroup::finished, this,
        [this, toIndex, group]() {
            // QStackedWidget::setCurrentIndex (non-virtual call via scope resolution)
            // to update internal state without re-triggering our override
            QStackedWidget::setCurrentIndex(toIndex);
            m_animating = false;
            group->deleteLater();
        });
```

作用域解析直调基类的 setCurrentIndex，直调这一步不能省。动画期间咱们手动摆弄几何，堆叠件的 d->index 还是旧值，currentIndex() 会说谎，导航按钮的边界判断、currentChanged 的订阅者全跟着错。回到基类实现正好把状态收干净，raise 加 show 新页，hide 旧页，再发信号，一套齐全。

交互冻结用 m_animating 一个布尔就够，动画进行中新的切换请求直接丢弃，您舍不得丢就再记一个待办索引，finished 里看到就接着切。给整个堆叠件 setEnabled(false) 也是一路，控件变灰，用户知道系统在忙，代价是导航件在动画期间也点不动。

还有一个时段性风险，机制依据在 setGeometry 那段。动画进行中窗口被拖宽，布局重新分派矩形，StackOne 只认当前页（第 452-459 行），滑到半路的旧页被一把拽回完整矩形、站回 (0,0)，新页仍按旧宽度滑入，两页叠在一起。示例没处理 resize，您把时长压在两三百毫秒重合概率就小得多，要彻底处理就在 resizeEvent 里停动画、重摆几何、再续播。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，widgetAdded 信号要 6.9 起才有。QStackedWidget、QStackedLayout 与 QPropertyAnimation 同在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QStackedWidget](https://doc.qt.io/qt-6/qstackedwidget.html) -- 堆叠页面容器，removeWidget 的所有权说明与 setCurrentWidget 的前置条件定义处

[Qt 文档 · QStackedLayout](https://doc.qt.io/qt-6/qstackedlayout.html) -- 堆叠布局，setCurrentIndex 路径、sizeHint 聚合与增删后继规则的实现地

[Qt 文档 · QStackedLayout::stackingMode](https://doc.qt.io/qt-6/qstackedlayout.html#stackingMode-prop) -- 堆叠模式属性，StackOne 默认与 StackAll 叠层用法

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画，滑动切换里驱动 pos 的引擎

[Qt 文档 · QTabWidget](https://doc.qt.io/qt-6/qtabwidget.html) -- 标签页控件，壳内堆叠件的行为在 3.39 讲过

---

容器这半边读穿了，标签栏与堆叠页各写各的代码、共用同一套补位语义，40 篇开头的约定在这两篇里对上了号。下一程换分地盘的容器，[3.42 QSplitter 进阶](./42-qsplitter-advanced.md)，拖杆分尺寸的机制咱们接着看。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/41-qstackedwidget-advanced/，可执行文件 41-qstackedwidget-advanced），入口在 main.cpp 第 47-78 行，三页彩色堆叠加导航条一屏摆开。前进后退按钮直调 setCurrentIndex（main.cpp 第 91-105 行），页码指示靠 currentChanged 更新，出处（第 108-109 行）。您可以改 slide_stacked_widget.cpp 第 77、85 行的 QEasingCurve 换体感，或在动画播放时拖宽主窗口，观察旧页被布局拽回 (0,0) 的那一下，正是上文 setGeometry 分叉的现场。

完成标准照旧。setCurrentIndex 越界为什么一声不吭，您指得出拦截在哪一行。删除当前页后接位的是谁，末位与非末位两种情形您按 takeAt 推得出来。从 StackAll 切回 StackOne、当前页是 0 号会发生什么，您答得出第 554 行的条件毛病在哪。动画结束为什么直调基类 setCurrentIndex，您说得清不对齐会让谁说谎。
