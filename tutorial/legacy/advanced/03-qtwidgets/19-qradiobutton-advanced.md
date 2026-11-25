---
title: "3.19 QRadioButton 进阶"
description: "入门篇会调同 parent 自动互斥和 QButtonGroup 分组，进阶篇把互斥的归属问到源码行上。进了组的按钮 autoExclusive 不起作用，运行时切换 exclusive 不修正现状，移除选中按钮后 checkedId 报 -1，idClicked 只认点击，整数 ID 的映射靠 setId 与 button(int) 维系，配套示例在 examples/advanced/03-qtwidgets/19-qradiobutton-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.19——QRadioButton 进阶

[入门篇 QRadioButton 基础](../../beginner/03-qtwidgets/19-qradiobutton-beginner.md) 把同 parent 的自动互斥、QButtonGroup 跨 parent 分组、toggled 与 idClicked 的取舍连同 QSS 圆形美化过了一遍，摆一组固定的选项够用。[3.18 QToolButton 进阶](./18-qtoolbutton-advanced.md) 结尾留了个话头，说互斥的事比入门篇看到的要深。深在哪里，咱们直接看一个会把人问住的场面：同一排 QRadioButton 全部塞进 setExclusive(false) 的 QButtonGroup，照 autoExclusive 默认开着的思路去想，同 parent 的它们还得互斥，要真想多选就得挨个 setAutoExclusive(false)。实际跑起来，两个按钮可以同时亮着，setAutoExclusive 一行都不用写。互斥到底归谁管，答案在官方文档的一句话和 qabstractbutton.cpp 的几行代码里，本篇把它翻出来，动态增删按钮的状态管理、idClicked 与 idToggled 的分工也一并摆开。

笔者把三个容易把人问住的场面摆出来。同一排单选按钮进了非互斥的组，两个同时亮着，autoExclusive 与 exclusive 两个词都指着互斥，第一反应多半是再挨个 setAutoExclusive(false) 补一道保险，补了才发现是白补。设备列表运行时移除当前选中的按钮，checkedId() 变成 -1，拿它当索引用的后续逻辑当场落空。构造函数里 setChecked 设好默认选中，指望 idClicked 的槽刷新详情区，程序跑起来详情区空着，用户点一下才有内容。三个场面各有一套根因，本篇讲三件事：互斥的归属在源码里怎么分派，动态增删按钮时组与按钮的状态怎么分家，idClicked 与 idToggled 怎么分工、整数 ID 的映射怎么管。

## 互斥归组管：进了 QButtonGroup，parent 的扫描轮不上

QButtonGroup 出厂就是互斥的，exclusive 默认为 true，组里同一时间只认一个选中。setExclusive(false) 一调，组这边撒手，不再取消任何人的选中。到这一步都直白，让您拿不准的是另一头：QRadioButton 的 autoExclusive 出厂也是 true，qradiobutton.cpp 的构造函数里一行 setAutoExclusive(true) 白纸黑字，[3.12 QAbstractButton 基类进阶](./12-qabstractbutton-base-advanced.md) 讲过它的生效范围是同一个 parent。两个开关都开着，按钮又同 parent 又同组，互斥听谁的？

官方文档在 autoExclusive 的说明里写过一句明话，“The property has no effect on buttons that belong to a button group”，进了组的按钮，autoExclusive 不起作用。机制咱们去 qabstractbutton.cpp 的 notifyChecked 里看：互斥的执行写成一个 if/else，按钮有组，走组分支，组先记下新的选中按钮，组互斥时把旧的选中取消。没有组，才轮到 autoExclusive 分支去扫同 parent 的兄弟。进了组的按钮，parent 扫描那整条路径不会执行。queryButtonList 把界线又补了一道，没进组的按钮扫 parent 找兄弟时，进了组的兄弟会被从候选里剔除。两套机制不叠加，是让位关系，组优先。

咱们拿一个最小的实验直接看结果。两个同 parent 的 QRadioButton 塞进 setExclusive(false) 的组，先后 setChecked(true)，两个都亮着，谁也没取消谁。12 篇给过工程建议，互斥这件事上 autoExclusive 与 QButtonGroup 二选一，本篇把边界划到了源码行上，比当时更清楚：按钮一旦进了组，互斥只由组的 exclusive 说了算，autoExclusive 连候选名单都进不去。想让同 parent 的单选按钮保持互斥，要么一个组都不给它们，要么给一个互斥的组。组不互斥又都进组，就没有互斥，挨个 setAutoExclusive(false) 是多余动作。

咱们把组在运行时切来切去，它不修正现状的脾气就冒出来了。setExclusive 的函数体只有一行赋值，qbuttongroup.cpp 里看得见，切过去时不检查组里谁亮着。非互斥期间攒下的两个选中，切回互斥后照样都亮。互斥要等下一次按钮状态变化才重新执行，而且只取消组记录着的那一个旧选中，另一个多出来的选中没人管，会一直亮到程序去处理它。

互斥的归属按按钮的处境列一张表，您写代码前扫一眼：

| 按钮的处境 | 互斥由谁执行 |
| --------- | --------- |
| 进了组，exclusive 为 true | 组在每次新的选中时取消旧选中 |
| 进了组，exclusive 为 false | 没有互斥，同 parent 也不互斥 |
| 没进组，autoExclusive 为 true（QRadioButton 出厂状态） | 同 parent 的兄弟互相排他 |
| 没进组，autoExclusive 为 false | 没有互斥 |

配套示例第一个分区是动态设备选择器，咱们拿它把归属过一遍。开局传感器 A 选中，点传感器 B，A 自动熄灭，底部信息条的 checkedId 从 0 跳到 1。按一下“切换 Exclusive”按钮把组切到 OFF，再点传感器 A，A、B 同时亮着，checkedId 报 0，非互斥的组不取消旧选中，autoExclusive 又因为按钮进了组而整段不起作用。再点 A 一下，A 熄灭，单选按钮平时点了不能取消的限制跟互斥绑在一起，组不互斥，点一下选中项就把它点灭了。第二个分区把颜色与尺寸六个单选按钮摆进同一个容器，颜色三个进一个组，尺寸三个进另一个组。开局亮着的尺寸选项是“小”，点“大”，熄灭的是“小”，颜色那边的“红色”纹丝不动。六个按钮同一个 parent，光靠 autoExclusive 只能活一个，现在两组互不牵连，组成员身份把 parent 扫描挡在了外面。做表单时多个单选组共用一个容器是安全的，组本身就是界线。

## 动态增删按钮：移除选中的那个，checkedId() 变 -1

静态界面在构造函数里把按钮建齐、进组、连信号，一次完事。设备列表跟着上下线增减，模板列表跟着数据改，选项就得运行时动。动态添加不难，建 QRadioButton，addButton(radio, id) 进组，addWidget 进布局，配套示例用自增计数给每个新设备发 ID，从 0 起往上走，button(id) 随时按 ID 找回按钮。有一个细节值得您记着，新按钮如果在进组前就已经选中，addButton 见到组互斥、按钮又是选中态，会当场执行一次互斥，把组里原来的选中顶掉。想让新选项安静地进组，建它的时候别急着 setChecked。

动态移除才有状态要处理。removeButton 把按钮从组里摘出去，不碰按钮自己的 checked，组这边发现记录着的选中被摘走，直接清空记录，checkedId() 从此报 -1。摘走的按钮马上 deleteLater 倒也罢了，有的场合只是从组里摘出来、控件还留在界面上，按钮亮着选中，组却不认它，两边状态分了家。咱们拿 checkedId() 当索引用的话，-1 得当正常值处理，不能假设它永远有效。补选还是放空是产品决定，设备场景停在一个已经下线的旧设备上，不如自动跳到第一个可用的，配置场景里“无选中”本身可以是合法状态，让用户重选就行。

咱们看配套示例的移除逻辑，它对被移除的正是选中项的情况备了两手：

```cpp
void removeOption(int id)
{
    auto *btn = m_group->button(id);
    if (btn == nullptr) return;

    bool wasChecked = btn->isChecked();
    m_group->removeButton(btn);

    // 从布局中移除并删除
    m_layout->removeWidget(btn);
    btn->deleteLater();

    // 如果被移除的是选中项，需要处理"无选中"的状态
    if (wasChecked) {
        // 方案一：自动选中剩余的第一个按钮
        if (auto *first = m_group->button(0)) {
            first->setChecked(true);
        }
        // 方案二：保持"无选中"状态，让用户重新选择
    }
}
```

补选的写法有一处 ID 上的讲究。方案一里的 button(0) 是按 ID 找按钮，被移除的恰好是 0 号时它返回 nullptr，补选落空，界面停在无选中。配套示例换成 buttons().first()，取组内按钮列表的头一个，跟 ID 怎么分配脱钩，组里还有按钮就一定补得上。咱们把断点停在 removeButton 之后、补选之前，调试器里读 checkedId() 就是 -1，那个值真实存在过，只是被补选立刻盖掉了。信息条上您看到的是换号后的结果，从被移除的号换成剩余头一个的号。

## idClicked 只认点击：程序设的默认选中，槽一次不进

QButtonGroup 的信号成对出现，id 系列带整数参数，button 系列带按钮指针，触发条件两边同一套，qabstractbutton.cpp 的 emitClicked 里 idClicked 与 buttonClicked 挨着连发。入门篇推荐过 idClicked，一个整数就能定位选项，省下指针到业务的转换。id 还是 button 只决定参数形态，触发上的分别藏在 clicked 与 toggled 两个词之间，咱们列一张表：

| 信号 | 参数 | 什么时候发 |
| --------- | --------- | --------- |
| idClicked | int | 真正的点击发生时，鼠标、键盘空格、快捷键都算，程序调 click() 与 animateClick() 也算 |
| buttonClicked | QAbstractButton* | 与 idClicked 同一套触发条件，参数换成按钮指针 |
| idToggled | int 与 bool | 选中状态起了任何变化就发，程序 setChecked 引起的也算 |
| buttonToggled | QAbstractButton* 与 bool | 与 idToggled 同一套触发条件，参数换成按钮指针与选中标志 |

分工落到业务上，咱们按需求挑就行。要区分用户主动选的和程序设置的，连 clicked 系列，初始化的 setChecked 不会惊动它。要响应全部状态变化，改连 toggled 系列，程序设的、用户点的、互斥被连带取消的，全都到。互斥取消旧选中时，旧按钮那条 idToggled(oldId, false) 也会发，连 toggled 的槽记得把 false 的分支也写上，只处理 true 的话，切换一次选中会漏掉半边。

咱们自己给组接上四个信号探针（示例没接，idPressed、idToggled、idReleased、idClicked 各写一行 connect），就能看一次完整点击的信号次序。传感器 A 选中时用户点了传感器 B，到的顺序是 idPressed(B)，然后 idToggled(A, false)、idToggled(B, true) 连着两条，再 idReleased(B)，最后 idClicked(B)。取消旧选中与设置新选中都发生在按下与释放之间。有一个对写槽有利的细节，在 idToggled(A, false) 的槽里读 checkedId()，拿到的已经是 B 的 ID，组在取消旧选中之前就把新选中记好了，中间不存在 checkedId() 报 -1 的空档，槽里尽管放心读。程序 setChecked 切换选中时，来的只有两条 idToggled，顺序同样是旧 false 在前、新 true 在后，pressed、released、clicked 一概没有。

### 一道调试题：默认选中设好了，详情区却是空的

咱们再看一段初始化代码，程序跑起来详情区一片空白，用户随便点一个选项它才出内容，您先自己找找原因：

```cpp
m_group = new QButtonGroup(this);
m_group->setExclusive(true);
for (int i = 0; i < 3; ++i) {
    auto *radio = new QRadioButton(options[i], this);
    m_group->addButton(radio, i);
}
m_group->button(0)->setChecked(true);  // 想让"基础配置"默认选中

connect(m_group, &QButtonGroup::idClicked, this, [this](int id) {
    detailLabel->setText(optionDetail(id));  // 选中变化时刷新详情区
});
```

咱们把毛病定位到 setChecked 的路径上。idClicked 由 emitClicked 发出，emitClicked 只在点击路径的末尾被调用，setChecked 走的是状态设置路径，从头到尾碰不到它，初始化那次默认选中只发了一条 idToggled(0, true)，槽连的是 idClicked，自然一声不吭。入门篇 3.19 提醒过 setChecked 不触发 clicked，当时它是个免打扰的特性，这里反过来咬了需要刷新的人。修法两条，初始化末尾手动刷一次详情区，或者改连 idToggled，槽里判断 checked 为 true 再刷新，用户点的、程序设的就都接得住了。

剩下的疑问在整数 ID 本身。addButton 的第二个参数给按钮定 ID，任何整数都行，唯独 -1 不行，它是保留值，button() 查不到、id() 查不在组里的按钮、checkedId() 没有选中时，报的都是 -1，setId 收到 -1 会直接忽略。不传第二个参数，组自动分配，从 -2 起往更小的数走，笔者实测连加三个按钮，拿到 -2、-3、-4。自定 ID 用正数，跟自动分配天然错开，官方文档也是这么建议的。ID 不必连续、不必从 0 开始，拿它对齐枚举值或者数据库主键都行，槽里拿到 ID 直接当索引用。

setId 可以在进组之后改 ID，改完旧 ID 立即失效，button(旧ID) 返回 nullptr。代码里要是缓存过按钮的 ID，setId 一调缓存全部作废，ID 的分配与修改收拢到一处，集中在一个枚举或常量定义里管，别散在各个槽里手工发号。两个按钮分到同一个 ID 的话，button() 返回哪一个没有保证，唯一性得咱们自己守：

```cpp
auto *group = new QButtonGroup(this);
auto *r1 = new QRadioButton("A");
auto *r2 = new QRadioButton("B");
auto *r3 = new QRadioButton("C");

group->addButton(r1, 10);  // ID = 10
group->addButton(r2, 20);  // ID = 20
group->addButton(r3, 30);  // ID = 30

group->setId(r2, 25);      // 把 r2 的 ID 从 20 改为 25

qDebug() << group->id(r1);   // 10
qDebug() << group->id(r2);   // 25
qDebug() << group->button(20);  // nullptr（ID 20 已经不存在了）
qDebug() << group->button(25);  // r2
```

配套示例第三个分区摆着选项 X、Y、Z，下面两条计数标签分别连 idClicked 与 buttonClicked。程序启动时选项 X 顶着 setChecked 设的默认选中，两条计数都是 0，程序设置的那次选中，clicked 系列不认。咱们点一下选项 Y，两条计数一起加一，一条报最后 ID，一条报按钮文字，同时到、参数各报各的。idClicked 与 buttonClicked 的差别到参数为止，触发上的区分度在 clicked 与 toggled 之间，选信号时先选系列，再挑参数。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QRadioButton 与 QButtonGroup 属于 QtWidgets 模块，互斥归属与信号次序这两件事都对照 qtbase（Qt 官方源码仓库，QtWidgets 就在其中）核过，示例可在任何支持 Qt 6 的桌面平台上编译运行。

[Qt 文档 · QButtonGroup](https://doc.qt.io/qt-6/qbuttongroup.html) -- 按钮分组，exclusive 属性和信号说明

[Qt 文档 · QRadioButton](https://doc.qt.io/qt-6/qradiobutton.html) -- 单选按钮，autoExclusive 机制

[Qt 文档 · QAbstractButton](https://doc.qt.io/qt-6/qabstractbutton.html) -- 按钮基类，setChecked 的信号触发规则

---

18 篇留下的互斥话头到这里落了地。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/19-qradiobutton-advanced/，可执行文件 19-qradiobutton-advanced）完整玩一轮。切换 Exclusive 按钮按一下，再点第二个设备按钮，两个同时亮的样子对着正文里的归属表看。移除选中的设备，补选把 checkedId 拉回有效值之前，它刚当过一次 -1，调试器停在 removeButton 之后就能看见。信号对比区先看启动时那两个 0，再点一个选项，程序设的选中与用户点的选中，差别就摆在计数上。下一篇 [3.20 QCheckBox 进阶](./20-qcheckbox-advanced.md) 讲按钮五连的第四个 QCheckBox，三态的传播、PartiallyChecked 在树形结构里的冒泡、blockSignals 防递归都在那边展开。
