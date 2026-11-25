---
title: "3.43 QToolBox 进阶"
description: "入门篇用 QToolBox 搭过折叠面板导航，进阶篇把标题列与页的对号关系对着 6.9.1 读实现，标题按钮是文件内私有的 QToolBoxButton 继承 QAbstractButton 与 QToolButton 无关而 QTabWidget 走公开类 QTabBar、addItem 只是 insertItem(-1) 的内联壳且越界索引一律落队尾空指针返回负一、每页被包进去边框的 QScrollArea 且 widgetResizable 为真、第一页入册自动成为当前页、中段插入删掉整个布局重建并补一次 setCurrentIndex 校正、setCurrentIndex 重复设同页静默早退所以点当前页标题没有信号、最后一页被删 currentChanged 发负一而删当前页跳 0 号、removeItem 只挪父不删本体、逐项属性全部直接存在按钮身上没有独立元数据表、禁用当前页会向下一页一页试向上同步折返找最近可用页接盘而全员禁用时禁用页继续挂当前态、标题绘制走 CE_ToolBoxTab 且样式表再分成 Shape 与 Label 两个盒子、伪状态有 :selected 加 :first :last :middle :only-one :next-selected :previous-selected 六个位置邻接态、按钮 sizeHint 图标按 PM_SmallIconSize 计宽补 2 像素文字纵向补 8 像素而 QToolBox 自己没有 sizeHint 重写、动画方案里 blockSignals 拦不住切换因为显隐在函数体里无条件执行、收尾须把 maximumHeight 放回 QWIDGETSIZE_MAX 即 (1<<24)-1，配套示例在 examples/advanced/03-qtwidgets/43-qtoolbox-advanced/"
---

# 现代Qt开发教程（进阶篇）3.43——QToolBox 进阶

[3.42 QSplitter 进阶](./42-qsplitter-advanced.md) 收尾说下一程收进一个抽屉里翻页，标签列配折叠页，本篇的 QToolBox 干的就是这个活。一列标题从上往下摞，点谁谁的面板撑开，其余收起，互斥切换。入门篇您用 addItem 与 setCurrentIndex 搭过侧边栏导航，进阶篇咱们对着 6.9.1 把它读穿，标题按钮是谁造的，页与索引怎么对号，逐项属性存在哪，QSS 能伸到哪里。源稿把重心压在外观定制与过渡动画上，机制层几乎空白，还有几处断言与源码对不上，本篇按源码重排，改准的地方随文给出行号。

本篇引用以 qtoolbox.cpp 为主，内联重载补 qtoolbox.h，QSS 的落点补 qstylesheetstyle.cpp，与 QTabWidget 的对照补 qtabwidget.cpp。配套示例的完整玩法咱们放在结语，正文引用文件名加行号。

## 标题列的真身：QToolBoxButton，跟 QToolButton 没有血缘

名字相近的两类控件值得咱们花一段辨清。QToolBox 的标题栏不用 QToolButton，也不用 QTabBar，而是 qtoolbox.cpp 文件内私有的 QToolBoxButton，继承 QAbstractButton（第 25 行）。构造函数里它把背景角色设成 Window，尺寸策略 Preferred 配 Minimum，焦点策略 NoFocus（第 29-35 行），不抢键盘焦点算是它当导航栏的自觉。笔者把 QTabWidget 那边对照着翻了，标签列是公开类 QTabBar，qtabwidget.cpp 第 171 行持着指针，第 201 行 new 出来。QToolBox 没有等价物，标题按钮不导出，公共头文件里查无此类。

私有结构 Page 咱们翻开看，每页攥着三样东西，标题按钮、一个 QScrollArea、您的页面本体（qtoolbox.cpp 第 57-76 行）。滚动区的来龙去脉放到 insertItem 一节讲，这里记一笔存储结构就够，text、icon、tooltip 这些逐项属性没有一个独立的元数据表，全部直接存在按钮身上，Page 只做转发（第 63-70 行）。

找按钮的公共路子没有，tabButton 一类的接口不存在，您只能遍历 children 按类名认。源稿说遍历顺序和面板索引一致，咱们核对后发现只对追加式使用成立，按钮在 insertItem 被调时创建（第 319 行），children 列表排的是创建序，中段插入的新页落进页列表的 index 位置（第 340 行），按钮却造在列表尾巴上，按位置对号会错位。想要稳定对号，公开接口里 indexOf 换算比裸遍历靠得住。

## addItem 是内联壳，insertItem 才是正身

四个添加接口里只有带图标的 insertItem 有函数体，其余三个是头文件里的内联转发，addItem(widget, text) 就是 insertItem(-1, item, QIcon(), text)（qtoolbox.h 第 77-83 行）。索引语义由此定下，负一与越界一律落到队尾（qtoolbox.cpp 第 332-338 行），您传 999 与传 -1 同效。空指针直接返回负一，一个异常都不抛（第 310-311 行）。

每页入册时的包装咱们看摘录。

```cpp
// 每页的滚动区包装（qtoolbox.cpp 第 323-327 行）
c.sv = new QScrollArea(this);
c.sv->setWidget(widget);
c.sv->setWidgetResizable(true);
c.sv->hide();
c.sv->setFrameStyle(QFrame::NoFrame);
```

您的页面被塞进一个去边框的 QScrollArea，页面超高时滚动条由它出面，QToolBox 自己不管滚动。第一页入册时自动成为当前页（第 337-338 行），哪怕您本意只是摆几个空壳，第一页也会被点亮。按钮的对象名统一是 qt_toolbox_toolboxbutton（第 320 行），人人同名，对象树里按名字找按钮只能找到头一个。

中段插入走另一条分支，整个布局删掉重建，按钮与滚动区按新页序重新进 QVBoxLayout（第 340-341 行，relayout 本体在第 405-415 行），当前页的索引被挤动时，还会补一次 setCurrentIndex，把当前页校正回来（第 342-349 行）。收尾统一执行 button->show、updateTabs，再回调 itemInserted(index)（第 352-356 行），您想在页进页出时挂自己的逻辑，itemInserted 与 itemRemoved 两个虚函数，就是官方留的口子（第 691-705 行）。

## 重复设当前索引静默早退，点当前页的标题没有信号

切换的正身咱们看摘录。

```cpp
// 切换的完整动作（qtoolbox.cpp 第 390-402 行）
QToolBoxPrivate::Page *c = d->page(index);
if (!c || d->currentPage == c)
    return;

c->button->setSelected(true);
if (d->currentPage) {
    d->currentPage->sv->hide();
    d->currentPage->button->setSelected(false);
}
d->currentPage = c;
d->currentPage->sv->show();
d->updateTabs();
emit currentChanged(index);
```

动作依次是选中态搬家，旧页的滚动区隐藏，新页的滚动区显示，末尾发 currentChanged。开头那个早退值得您留意，索引越界静默返回，重复设当前索引也静默返回。您点着当前页的标题再点一次，clicked 一路到 _q_buttonClicked 转成 setCurrentIndex（第 359-372 行），在早退处返回，信号一个不发。“再点一次收起面板”的交互在这里走不通，得在外面另做。

updateTabs 里藏着一段小机关，咱们翻出来看，当前页之后的标题会借用各自页面的背景色（第 135-143 行），配色跟页面走，视觉上标题与页面连成一体。样式表换风格时，changeEvent 收到 StyleChange，也会把它重跑一遍（第 677-683 行）。

删页面有两条路。直接 delete 页面本体，destroyed 信号触发 _q_widgetDestroyed 清理（第 423-437 行），删的是最后一页时 currentChanged 发的是负一（第 439-441 行），删的是当前页则跳到 0 号（第 442-445 行）。removeItem 是温和版，页面被挪出滚动区重新认 QToolBox 当父对象，本体不删（第 453-463 行），返回后页面对象还在您手里，想彻底放手得自己 delete。

## 逐项属性存在按钮身上，禁当前页会自动跳走

setItemText、setItemIcon、setItemToolTip 三个接口长得像在维护一张表，实现却是挨个转发给按钮（第 587-618 行）。isItemEnabled 也一样，读的是按钮的 isEnabled（第 625-630 行）。setItemEnabled 的看点在禁用当前页时的一段接盘搜索，按钮本身的可用状态改完（第 551 行），循环向下一页一页试，向上同步折返（第 556-569 行），头一个可用页接盘成为新当前页。全员禁用时循环走空，原索引传回 setCurrentIndex，被同页早退挡下，禁用页继续挂着当前态。禁用后的标题收不到点击，反复点它没反应，您做可用性提示仍然是正事，只是提示的立足点是按钮关了门，别把它说成程序卡了。

标题按钮自己的建议尺寸也有基准可查，sizeHint 里图标部分按 PM_SmallIconSize 计宽再补 2 像素间距，文字纵向另补 8 像素（第 149-159 行）。咱们翻了整份 qtoolbox.cpp，QToolBox 自己没有 sizeHint 重写，对外的建议尺寸由构造时建的 QVBoxLayout 聚合而来（第 264-265 行），隐藏的滚动区不占布局，高度主要看当前页。

## QSS 的可达面：两个盒子与七个伪状态

标题的绘制入口在 paintEvent，走 CE_ToolBoxTab 交给样式（第 202-209 行）。样式表接手后把它分成两个盒子，CE_ToolBoxTabShape 管背景边框，CE_ToolBoxTabLabel 管文字字体与调色，出处是 qstylesheetstyle.cpp 第 4321-4350 行，所以 QToolBox::tab 一条规则同时管到形状与字。伪状态笔者数了一遍，:selected 来自选中态（qtoolbox.cpp 第 174-175 行置 State_Selected），样式层把位置信息映射成 :first、:last、:middle、:only-one，邻接信息映射成 :next-selected 与 :previous-selected，出处是 qstylesheetstyle.cpp 第 2123-2139 行，数据源头在 initStyleOption，它填的 position 与 selectedPosition，正是映射要吃的数据（qtoolbox.cpp 第 184-199 行）。首尾标题加圆角、给当前页的邻页让一点空隙，这些活 QSS 都接得住。可达面的边界也摆在那里，图标永远画在文字左边，标题里塞不进第三个控件，布局结构 QSS 动不了，深度定制只剩子类化或换自绘方案。

配套示例里的渐变标题就是照可达面写的（toolbox_demo.cpp 第 68-92 行）。这里有个容易想当然的边界，min-height 与 icon-size 写进 ::tab 都不生效，几何尺寸消费不了子规则（qstylesheetstyle.cpp 第 2627-2661 行只吃控件本体的规则），能落地的只有几样：background、border、padding，加上 font 与 color。您把 min-height 压到 20px 再跑，标题栏的高度纹丝不动，背景倒是立刻换色，这就是可达面的边界现场。

## 想要过渡动画，blockSignals 拦不住切换

源稿的动画思路是 QPropertyAnimation 驱动页面的 maximumHeight，切换瞬间完成这件事也属实，setCurrentIndex 里，隐藏与显示两步是无条件执行的（第 394-400 行）。收尾把最大值放回 QWIDGETSIZE_MAX，即 qwidget.h 第 921 行的 (1<<24)-1，确实必须，留在动画终值上的上限会拦住面板在窗口变大后的伸展。但源稿说可以用 blockSignals 阻止默认切换，笔者对着调用链核过，它拦不住。点击标题走 clicked 到 `_q_buttonClicked` 再到 setCurrentIndex（第 321、359-372 行），显隐切换在函数体里直接做，blockSignals 只能压住末尾那句 emit currentChanged（第 402 行），面板照样瞬间换。真要接管切换，官方没有留别的口子，子类化拦在 setCurrentIndex，或者干脆用一组自绘折叠面板替代 QToolBox。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，全部行为在 6.9.1 源码核对。QToolBox 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。动画部分涉及 QtCore 的 QPropertyAnimation。

[Qt 文档 · QToolBox](https://doc.qt.io/qt-6/qtoolbox.html) -- 折叠工具箱容器，addItem 与逐项属性接口的定义处

[Qt 文档 · QToolBox::currentChanged](https://doc.qt.io/qt-6/qtoolbox.html#currentChanged) -- 切换信号，最后一页被删时携带负一的定义处

[Qt 文档 · QToolBox::itemInserted](https://doc.qt.io/qt-6/qtoolbox.html#itemInserted) -- 页进页出的虚函数口子

[Qt 文档 · QStyleOptionToolBox](https://doc.qt.io/qt-6/qstyleoptiontoolbox.html) -- 标题绘制的数据载体，position 字段喂给 QSS 伪状态

[Qt 文档 · QScrollArea](https://doc.qt.io/qt-6/qscrollarea.html) -- 每页内层的滚动包装，下一篇的主角

---

抽屉读穿，按钮的真身、切换的早退、逐项属性的存储与 QSS 的七个伪状态都对上了号，42 篇递过来的话到这里收梢。页面们各自住在 QScrollArea 里这件事，本篇只是路过打了个照面，下一程把它本体读穿，[3.44 QScrollArea 进阶](./44-qscrollarea-advanced.md)，视口怎么裁剪、构件怎么布局，咱们接着看。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/43-qtoolbox-advanced/，可执行文件 43-qtoolbox-advanced）。三页初始内容各带一个标准图标入册（toolbox_demo.cpp 第 105、112、119 行），底部两个按钮演示动态增删，Add Page 入册后立刻 setCurrentIndex 跳到新页（第 152 行），Remove Last Page 守着至少留一页的底线（第 163-166 行），移除后 delete 放手（第 170-172 行），状态栏跟着 currentChanged 走，刷新页数与页码（第 46-51 行）。

完成标准照旧。QToolBoxButton 的父类您说得出口。中段插入一页后按 children 顺序找按钮为什么会错位，您指得出创建序与页序的分岔在哪一行。点当前页的标题为什么没有信号，您找得到那个早退。禁用当前页时接盘者怎么找，您描得出向下试探与向上折返怎么交替。七个伪状态里您至少叫得出 :selected 与 :first。
