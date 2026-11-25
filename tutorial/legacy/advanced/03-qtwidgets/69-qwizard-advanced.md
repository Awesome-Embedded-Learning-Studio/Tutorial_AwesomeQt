---
title: "3.69 QWizard 进阶"
description: "进阶篇把 QWizard 的机制对到源码行上：registerField 在页面入册前走暂存队列、setPage 时统一进全局字段表（qwizard.cpp 第 3888-3898、2184-2187 行），QWizardPage 的 field 与 setField 只是一层转发（第 3813-3844 行），mandatory 的判定是当前值与注册初值的比较而非查空（第 3604-3608 行），completeChanged 只在三态翻转时发出（第 471-478 行），nextId 默认找键序里更大的那页，与 id 加一无关（第 3767-3787 行），addPage 发号是 lastKey 加一（第 2143-2151 行），next() 的链路是 validateCurrentPage 到 nextId 再查 history、已进过的页直接拒（第 3028-3049 行），initializePage 只在前进首次进入时调用、currentIdChanged 在一切收拾完之后发出（第 781-788、828 行），cleanupPage 把字段拨回注册初值（第 3546-3558 行），点 Finish 的验证藏在 done 里（第 3214-3225 行），九个按钮角色都是 QPushButton、MacStyle 默认连取消钮都不建（第 613-653、1359 行），WizardOption 全集 17 项、没有 NoSubpixelRendering（qwizard.h 第 63-81 行），示例在 examples/advanced/03-qtwidgets/69-qwizard-advanced/"
---

# 现代Qt开发教程（进阶篇）3.69——QWizard 进阶

对话框家族的大件到这儿收尾。咱们在 [3.6 对话框进阶](./06-dialog-advanced.md) 里把 QWizard 搁在了一边，说非线性跳转在它里面实现起来别扭，不如自己搭状态机加共享数据容器。这个判断笔者一直想讨个说法，于是把 Qt 6.9.1 的 QWizard 源码（qwizard.cpp，3914 行）从头读到尾，读完可以报告。动态页序的口子官方专门留着，类文档写明“要提供依赖用户选择的动态页序，必须重写 QWizardPage::nextId()”（qwizard.cpp 第 1995-1997 行）。数据流也不用自己搬，QWizard 内置了一张全局字段表，正是 3.6 那篇手搭的共享数据容器的官方版。别扭不别扭，咱们把这趟源码走完再下判断。

引用文件交代清楚。主战场是 qwizard.cpp 与 qwizard.h（src/widgets/dialogs），Windows 的 Aero 特效另住在 qwizard_win.cpp，咱们不进去。配套示例是一只四页分支向导，完整玩法放结语。旧稿的几处断言与源码对不上，笔者在正文里挨个给出行号。

## fields 机制：页面入册那一刻，注册的字段才进全局表

registerField 的身体只有十行（qwizard.cpp 第 3888-3898 行）。页面还没进向导时，字段进页面的暂存队列 pendingFields，等 setPage 或 addPage 把页面插进来，暂存队列统一冲进向导的字段表（第 2184-2187 行）。所以咱们在页面构造函数里 registerField 完全合法，注册的时机与页面是否显示过毫无关系，咱们后面拿它推翻一个流传说法。

字段名带星号的秘密在 QWizardField 的构造函数里（第 167-177 行），名字尾部是星号就砍掉一位并置 mandatory 为真。属性和变化信号可以不填，留空时 resolve 拿 fallbackProperties 表补齐（第 101-110 行），表里七类控件各有默认属性，QComboBox 认的是 currentIndex 而不是 currentData，配套的信号表就在旁边（第 113-135 行）。您想存 currentData，属性就得显式传。您还能用 setDefaultProperty 给表添条目或改条目，见第 2838-2852 行。

resolve 还干一件要紧事，把入册那一刻控件身上的值，存成 initialValue（第 179-184 行）。咱们记着这个值，mandatory 的判定全靠它。

读取是一条转发链，咱们从页面这头顺过去。QWizardPage::field 与 setField 的身体就是一句 wizard() 的转调（第 3813-3844 行），落到 QWizard::field 查 fieldIndexMap（第 2432-2443 行），名字没注册过，控制台收到 qWarning “No such field”，返回值才是空 QVariant。重名注册也会被 addField 拒掉并警告（第 726-729 行）。顺着整条链咱们能推翻旧稿的一个说法，跳过的页面字段照样在册，值停在注册初值上，空 QVariant 只属于从未注册过的名字。

## mandatory 的判定：跟注册初值比，不是查空

咱们把默认 isComplete 的核心五行请出来（qwizard.cpp 第 3604-3608 行）：

```cpp
const QWizardField &field = *it;
if (field.page == this && field.mandatory) {
    QVariant value = field.object->property(field.property);
    if (value == field.initialValue)
        return false;
}
```

判定写得很直白，当前值等于注册初值就算没填好。QLineEdit 的星号字段初值多半是空串，效果上等于非空检查。QCheckBox 的星号字段初值多半是未勾，勾上才算数。您要是给初值填了非空文本，用户清空输入框反而算“改过”，页面照样算完成。mandatory 的判定只看值变没变，与空不空无关。QLineEdit 与 QAbstractSpinBox 另有一道 hasAcceptableInput 复核（第 3610-3620 行），验证器和输入掩码在这里生效。

按钮启停的整条链 Qt 替您接好了。addField 给 mandatory 字段把 changedSignal 接到页面私有的 _q_maybeEmitCompleteChanged（第 733-735 行），这个槽重跑一次 isComplete，三态没翻转，就不发 completeChanged（第 471-478 行）。信号一到向导这边，setPage 早就把 completeChanged 接到 _q_updateButtonStates（第 2189 行），后者拿 isComplete 的结果，去决定 Next、Commit、Finish 三个钮的启停，判定在第 1629-1631 行。您重写 isComplete 时，这层自动接线罩不住自定义条件，得自己把相关控件的信号连到 completeChanged，文档在第 3585-3591 行叮嘱过这句。

## 页序：nextId 默认找键序里更大的那页

默认 nextId 遍历 pageMap，跳过自己，返回键序里排在当前页后面的第一页（qwizard.cpp 第 3767-3787 行），这是笔者逐行核对过的找法。setPage 挂 1 和 10 两页，第 1 页的默认下一页就是第 10 页，“默认返回 id 加一”的说法在连号时碰巧对，一留空号就露馅。页号怎么来，咱们看 addPage 的实现，它发的是 lastKey 加一（第 2143-2151 行），纯 addPage 连加确实是 0、1、2 往上走，中间插一个 setPage(100)，下一个 addPage 拿到 101。分支向导要稳定的 id，setPage 配枚举最踏实。

next() 的链路咱们按序数（第 3028-3049 行）。validateCurrentPage 打头，默认转发当前页的 validatePage（第 3286-3296 行），验证不过就地停留。过了才问 nextId，返回 -1 就到头。接着两道关卡，目标页已在 history 里，qWarning “Page %d already met” 后直接返回（第 3038-3041 行），同一页一次运行进不去第二次，想回起点走 restart()。目标页不在 pageMap 里，同样警告返回（第 3042-3045 行）。全过了才 switchToPage 前进。

switchToPage 是页间跳转的唯一通道，身体在第 756-829 行。前进方向，目标页的 initialized 标志没立过，就调 initializePage，再把它记进 history，第 781-788 行。随后算 canContinue，依据是 nextId 是否返回 -1（第 791 行），canFinish 看 isFinalPage，按钮状态与焦点都收拾完，currentIdChanged 才在队尾发出（第 828 行）。您在 currentIdChanged 的槽里拿到的已经是初始化完的新页，时机不用猜。

“initializePage 每次显示前都调用”是误传，咱们顺着方向分支看就清楚。返回方向不调它，从 B 退回 A，A 的状态原样保留。退离 B 时 initialized 被清掉（第 763-769 行），再次前进到 B 它才会重新初始化。back() 本身不设验证（第 3012-3019 行），历史里退一步就到位。isFinalPage 的判定是显式 setFinalPage 压过默认，默认看 nextId 是否 -1（第 3660-3676 行）。

## 返回与重来：cleanupPage 把字段拨回入册那一刻

cleanupPage 的默认实现遍历本页字段，逐个 setProperty 拨回 initialValue，见 qwizard.cpp 第 3546-3558 行。两件事咱们当场认清。拨的是入册那一刻的值，构造与入册之间您改过的数不算数。清理只在退离与重来时发生，前进时跳过的页面不动它。

IndependentPages 选项把退离时的清理与去初始化一并免掉（第 766-770 行），页面状态全程保留，代价是一致性自己管。咱们把重来的路也走一遍，向导首次亮相时 setVisible 发现 current 是 -1 就调 restart（第 2900-2907 行），restart 内部走 reset（第 686-703 行），对 history 里的页挨个 cleanupPage，清 initialized，发一趟 currentIdChanged(-1)（第 701 行），再切到起始页。起始页默认取键序最小的那页，setStartId 可改。

点 Finish 的验证藏在 done 里，这是笔者想请您留意的位置。done 的 Rejected 分支调 reset 收拾现场，Accepted 分支要再过一次 validateCurrentPage，不过就返回、向导不关（第 3214-3225 行）。Finish 钮接的是 accept，验证挂在 done 上，next 不做这道关，两条路最终都汇到 validatePage。

## 按钮与外观：九个角色都是 QPushButton，MacStyle 默认连取消钮都不建

WizardButton 枚举给九个角色，外加一个 Stretch 占位，见 qwizard.h 第 29-44 行。init 只急造四个，Back、Next、Commit、Finish，见 qwizard.cpp 第 660-663 行，其余按需 ensureButton，造出来的就是 QPushButton（第 1359 行），带 objectName，文本出自 buttonDefaultText（第 613-639 行），Back 是 “< &Back”，Next 是 “&Next >”，Finish 是 “&Finish”，MacStyle 下 Back 换成 “Go Back”。您拿 button(QWizard::NextButton) 出来，加图标、设提示语都行。换整只按钮用 setButton，旧钮被删，新钮按 buttonSlots 接线（第 832-853 行），Next 与 Commit 都接 next()，Finish 接 accept()，Help 直接转发 helpRequested 信号。Aero 样式下 button(BackButton) 返回的，是 Vista 标题栏那一枚，第 2721-2731 行。

默认钮这件事也归 _q_updateButtonStates 管，Next、Commit、Finish 谁当期谁 setDefault（第 1650-1655 行），NoDefaultButton 选项把它关掉。样式本身的默认值从样式提示 SH_WizardStyle 来（第 648 行），属性文档写了四路（第 2449-2461 行），Windows Vista 开启 alpha 合成时走 AeroStyle，其余看控件样式，QMacStyle 配 MacStyle，QWindowsStyle 配 ModernStyle，咱们在 Linux 上跑，Fusion 当道，默认就是 ClassicStyle。旧稿写的 “AquaStyle” 在枚举里不存在（qwizard.h 第 54-60 行只有四个样式），macOS 上它叫 MacStyle。样式还带默认选项，MacStyle 默认 NoDefaultButton 加 NoCancelButton，见 qwizard.cpp 第 649-653 行，所以 macOS 的向导默认没有取消钮，ModernStyle 则默认 HelpButtonOnRight。

WizardOption 全集 17 项，登记在 qwizard.h 第 63-81 行，头一项是 IndependentPages，末一项是 NoCancelButtonOnLastPage。旧稿写到的 NoSubpixelRendering 不在其中，那个名字是凭印象编的，您别在代码里等它。路上咱们已经见过几项的实际作用，IndependentPages 免清理，NoDefaultButton 关默认钮，HaveFinishButtonOnEarlyPages 让非末页也亮出 Finish，可见性判定在 qwizard.cpp 第 1643-1644 行。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QWizard](https://doc.qt.io/qt-6/qwizard.html) -- 向导对话框类，样式与选项的属性定义处

[Qt 文档 · QWizardPage](https://doc.qt.io/qt-6/qwizardpage.html) -- 向导页基类，registerField 与四个虚函数的文档

[Qt 文档 · QWizard::WizardOption](https://doc.qt.io/qt-6/qwizard.html#WizardOption-enum) -- 17 项选项标志的完整说明

---

源码走完，开篇那桩判断可以下了：动态页序有 nextId 的官方口子，数据流有内置字段表，3.6 说的别扭，收回。咱们把对话框家族的大件走完了，下一站是 [3.70 QSplashScreen 进阶](./70-qsplashscreen-advanced.md)。它管的是主窗口亮相之前那块启动画面，加载太快闪一下就走、进度条画在 pixmap 上靠不靠谱，那边接着聊。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/69-qwizard-advanced/，可执行文件 69-qwizard-advanced）。四页分支向导，页面 id 走枚举经 setPage 注册（wizard_demo.cpp 第 163-166 行），选择页的 nextId 按下拉框当前项分流（第 39-48 行），两条配置页都直汇 Summary（第 76-80、102-105 行），Summary 的 initializePage 按 choiceIndex 字段拼汇总文本（第 125-145 行），按钮布局也是定制的，Back 在最左，中间 Stretch 占位，右边依次是 Next、Finish、Cancel，见第 172-174 行。您留神看选择页的 Next 钮，打开示例时它是灰的，因为 choiceIndex 挂了星号而 currentIndex 还停在注册初值 0（第 36 行），把下拉框换到 Type B 它就亮，换回 Type A 又灰回去，这正是“跟初值比较”的判定在示例里的实际表现。走 Type B 一趟，Summary 只报 Module name，Type A 的 projectNameA 字段在册但没人碰过，读出来还是注册初值。

完成标准照旧。mandatory 的判定拿什么比，您答得出。nextId 的默认找法与 addPage 的发号方式，您说得出。next() 的链路顺序与 already met 拦截，您数得清。Finish 点击的验证在哪个函数里，您指得出。
