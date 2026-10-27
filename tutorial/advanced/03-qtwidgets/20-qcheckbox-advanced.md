---
title: "3.20 QCheckBox 进阶"
description: "入门篇会调 setTristate、checkStateChanged 与 m_updating 防递归，进阶篇把三态勾选搬进多层权限树。父节点状态由全部子孙综合算出，blockSignals 挡的是树而且一挡挡全部信号，向下传播走完再向上冒泡，深树用延迟与缓存两条路压开销，配套示例在 examples/advanced/03-qtwidgets/20-qcheckbox-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.20——QCheckBox 进阶

[入门篇 QCheckBox 基础](../../beginner/03-qtwidgets/20-qcheckbox-beginner.md) 把 setTristate 的三态模式、checkStateChanged 与 toggled 的取舍、全选/全不选连同层级复选过了一遍，摆扁平列表够用。[3.19 QRadioButton 进阶](./19-qradiobutton-advanced.md) 收尾预告过，三态的传播、PartiallyChecked 在树形结构里的冒泡、blockSignals 防递归都在本篇展开。为什么值得单独一篇，咱们把入门篇的“向下传播 + 向上冒泡”搬进一棵三层权限树，m_updating 守卫没跟着一起搬，勾一下角色节点想全选，程序当场没了动静，itemChanged 一层勾起一层，传播链自己咬住了自己。死循环只是最先碰到的一个难题，后面还有中间态算错、静默设值找错对象、深树冒泡拖慢，全都藏在多层嵌套里。

笔者把多层树里最容易把人问住的三个场面摆出来。想静默地给节点设状态，对 QTreeWidgetItem 调 blockSignals，编译器当场报错，树节点根本不是 QObject。点一个角色节点想全选，九个权限全亮了，角色节点却停在半选，算它状态的那次冒泡跑在子节点改完之前。用 blockSignals 把树的信号一挡到底做完批量更新，恢复后发现靠这些信号刷新的别处界面没了动静，block 期间被挡掉的不止 itemChanged 一个。三个场面各有一套根因，本篇讲四件事：父节点的状态怎么从子孙算出来，传播的次序怎么排，blockSignals 的正确对象与作用域怎么拿捏，深树频繁勾选时冒泡的开销怎么压下去。

## 第三态是算出来的：子孙全亮，父节点才给 Checked

咱们回到规则本身。入门篇的两层联动里，全选框看子项，子项全亮给 Checked、全灭给 Unchecked、有亮有灭给 PartiallyChecked，一层的布尔值就够算。树一旦多出层次，PartiallyChecked 自己也成了输入：一个子节点可能因为它的子孙混合了选中和未选中，自己就处于半选，那么它算选中还是没选中？拿 bool 去套就翻车了。[3.12 QAbstractButton 基类进阶](./12-qabstractbutton-base-advanced.md) 讲过第三态的来历，PartiallyChecked 在 isChecked() 眼里报 true，旧的 stateChanged 自 Qt 6.9 起废弃，由 checkStateChanged 接替，本篇的判断一律落在 Qt::CheckState 枚举上，Unchecked、PartiallyChecked、Checked 三个值，谁也不折算成 bool。

父节点该设成什么，咱们用一张表说全：

| 子节点的状态分布 | 父节点设成 |
| --------------- | --------- |
| 全部是 Checked | Checked |
| 全部是 Unchecked | Unchecked |
| 其余任何组合，混了选中与未选中，或子节点里有 PartiallyChecked | PartiallyChecked |

咱们把规则写成代码，就是一次递归汇总：

```cpp
Qt::CheckState computeParentState(QTreeWidgetItem* parent)
{
    if (parent->childCount() == 0) {
        return parent->checkState(0);
    }

    int checked = 0;
    int unchecked = 0;

    for (int i = 0; i < parent->childCount(); ++i) {
        auto childState = computeParentState(parent->child(i));
        if (childState == Qt::Checked) ++checked;
        else if (childState == Qt::Unchecked) ++unchecked;
        // PartiallyChecked 既不算 checked 也不算 unchecked
    }

    int total = parent->childCount();
    if (checked == total) return Qt::Checked;
    if (unchecked == total) return Qt::Unchecked;
    return Qt::PartiallyChecked;
}
```

computeParentState(parent->child(i)) 这一行把递归带了下去：子节点自己有孩子时，它先把子节点当成一个父节点，把孙辈全部算完再报上来。报上来的咱们叫它有效状态，无论树多深，每个节点拿到的都是整棵子树的综合结果，第三态的语义在每一层都成立。配套示例的树只有三层，角色、模块、权限各占一层，递归的好处看不出来，您在权限底下再垫一层新条目，模块那层照样算得对，汇总代码一行不用改。

次序是另一个容易出错的地方，开篇第二个场面就出在这里。向下传播与向上冒泡两步都得做，先做哪个却不能随便。咱们假如让冒泡跑在传播前面，父节点算状态时它名下的孩子还没改完，computeParentState 读到的是旧摊子，算出来的档位（Qt::CheckState 的取值）与传播后的实情对不上。传播做完又没人补算，错档就停在那里。明明子孙全亮，角色节点显示的可能是半选，也可能是空选，取决于冒泡那一瞬孩子改到了哪一步。配套示例在 onItemChanged 里给的次序是先用 propagateToChildren 把整棵子树改完，再用 bubbleToParents 从当前节点逐层向上算，两步各走一遍，冒泡读到的就永远是改完的终态。

配套示例（examples/advanced/03-qtwidgets/20-qcheckbox-advanced/，可执行文件 20-qcheckbox-advanced）就是一棵 Administrator、Editor、Viewer 三个角色下挂三个模块、每个模块下挂 View、Edit、Delete 三个权限的树，咱们拿三个实验把规则摸实。勾选 Administrator，九个权限一次全亮，角色节点保持全选，这是全 Checked 才给 Checked。勾亮某个模块下的 View，它所在的模块当场变成半选，Administrator 跟着半选，半选沿路径一层层冒上去，Editor 与 Viewer 纹丝不动。这个半选只是勾亮那一刻的瞬时态，把 View 再取消，模块与 Administrator 又退回全灭。再做一个破坏性实验，把 onItemChanged 里 propagateToChildren 与 bubbleToParents 两行对调，重新编译后点一个此前一项未选的模块节点，三个权限全亮了，角色节点却空着，冒泡先跑，computeParentState 读到的还是传播前的全灭权限层，给角色算了个全不选。要是点之前那个模块已有亮着的权限，角色停的就是半选，同样是旧摊子算出来的。改回来，一切如常。

## blockSignals 挡的是树，一挡就是全部信号

防递归这头，入门篇给的办法是 m_updating 标志位，咱们在信号处理器入口查标志，正在更新就提前返回。小树里它工作得很好，但标志是处理器手工维护的一件全局状态，多线程或嵌套调用的场景下可能状态不一致。Qt 另有现成的工具，QObject::blockSignals(bool)，传 true 让对象停止发射一切信号，传 false 恢复。

用之前有一个对象得认清楚，就是开篇第一个场面里编译报错的来处。QTreeWidgetItem 不是 QObject，qtreewidget.h 里它的声明连基类都没有，blockSignals 这个方法它天生没有，您对着 item 调，编译器当场拒绝。itemChanged 的发射方另有其人：您在节点上改状态，setCheckState 一路调到模型，模型的数据变更最后由 QTreeWidget 在 emitItemChanged 里把 itemChanged emit 出来，写在 qtreewidget.cpp 里。所以要静默的是树，不是节点：

```cpp
void setCheckStateSilent(QTreeWidgetItem* item, int column, Qt::CheckState state)
{
    // QTreeWidgetItem 不是 QObject，不能直接 blockSignals
    // 但 TreeWidget 本身是 QObject，我们可以 block TreeWidget 的信号
    auto *tree = item->treeWidget();
    tree->blockSignals(true);
    item->setCheckState(column, state);
    tree->blockSignals(false);
}
```

批量的活它也接得住。点一下“全选”要改几十上百个节点，与其让每个 itemChanged 都进处理器跑一遍标志判断，不如在开始更新前把树的信号 block 住，全部设完再放开，一条信号都不用白跑。配套示例是两件工具搭着用的：onItemChanged 入口先用 m_updating 挡住重入，propagateToChildren 与 bubbleToParents 内部的每一处 setCheckState 又各自用最小作用域的 blockSignals 包住，一道防整体重入，一道管单次设值的静默。您要是嫌两道累赘，留 blockSignals 一道也能防住，前提是作用域圈得对，圈不对就引出下一个麻烦。

blockSignals 的脾气是一挡挡全部。它拦的不只是 itemChanged，itemSelectionChanged、itemExpanded 一类信号在 block 期间同样发不出去，信号发射的必经之路 doActivate 进门第一件事就是查 blockSig 标志，查到被挡就直接返回。您要是让界面靠那些信号驱动别处的刷新，block 一大段，刷新就漏一段。开篇第三个场面就是它：批量更新一路 block 到底，恢复之后那块界面没了动静。对策是把作用域压到最小，只包住要静默的那一次设值：

```cpp
// 正确：最小作用域
tree->blockSignals(true);
parent->setCheckState(0, Qt::PartiallyChecked);
tree->blockSignals(false);

// 错误：作用域太大，其他信号也被阻止了
tree->blockSignals(true);
updateAllChildren(parent, state);   // 这里面可能触发了其他需要信号的操作
updateAllParents(parent->parent()); // 同上
tree->blockSignals(false);
```

### 一道调试题：逐层挡信号的冒泡，窗口开在哪

咱们再看一段向上冒泡的实现，它在 itemChanged 的处理器里被调用，跑起来偶尔会重入，您先自己找找毛病：

```cpp
void updateParentState(QTreeWidgetItem* parent)
{
    if (!parent) return;

    tree->blockSignals(true);
    parent->setCheckState(0, computeState(parent));
    tree->blockSignals(false);

    updateParentState(parent->parent());  // 递归向上冒泡
}
```

咱们把毛病定位到 unblock 的时机上。每一层递归都自己 block 再自己 unblock，unblock 一执行完，树就回到了可发信号的状态，而后面还有一整条递归路径要走。递归本身不改状态还好，路径上任何一层在 unblock 之后调了会改 checkState 的代码，itemChanged 立刻发射，处理器里没有防重入的判断，整条传播链重跑一遍，跑的过程中再有设值，就再进一次。修法是把挡信号的责任从每一层收拢到最外层入口，进了处理器先 blockSignals(true)，所有层全部做完再 blockSignals(false)，可发信号的窗口从头到尾就没开过。作用域的大小以一组联动为边界来定，向下传播加冒泡是一次不可分的更新，组内一次挡完，组外一概不挡，与前面说的最小作用域并不冲突，那里拦的是另一头，把与设值无关的动作圈进 block 区间，白闷掉别的信号。配套示例的 m_updating 加的正是同一道保险，只是它挡在处理器入口，连重入的机会都不给。

## 深树频繁勾选：把子树遍历换成沿路径计数

树一深，咱们就得把冒泡的代价算一算。叶子节点每变一次，从它到根的路径上每个祖先都要重算状态，深度 N 的树一次变化带来 O(N) 级别的计算量。权限树、配置树通常毫无压力，怕的是文件系统完整目录树那种非常深的树，用户频繁切换叶子的选中状态，冒泡一次接一次，开销就攒起来了。

优化有两条路。一条是延迟冒泡，变化发生时不立刻算祖先，只把祖先标成待更新，攒到下一次事件循环统一算，QMetaObject::invokeMethod 配合 Qt::QueuedConnection 就能搭出来。另一条是缓存，咱们给每个节点记两个数，子树里选中的叶子数与叶子总数，算父节点时看这两个数就定，不用再遍历子孙：

```cpp
struct NodeCache {
    int checked_count = 0;   // 子树中 checked 的叶子数量
    int total_count = 0;     // 子树中叶子总数
};

// 叶子节点变化时，只需沿路径向上更新缓存
void updateCache(QTreeWidgetItem* node, int delta)
{
    auto* cache = getNodeCache(node);
    cache->checked_count += delta;

    auto* parent = node->parent();
    if (parent) {
        updateCache(parent, delta);
    }
}

// 根据 O(1) 缓存计算父节点状态
Qt::CheckState stateFromCache(const NodeCache& cache)
{
    if (cache.checked_count == 0) return Qt::Unchecked;
    if (cache.checked_count == cache.total_count) return Qt::Checked;
    return Qt::PartiallyChecked;
}
```

缓存的收益在复杂度上看得清楚。逐层现算时，每到一个祖先都要把它的孩子各问一遍，深度 N、每个节点 M 个孩子的树，一次冒泡是 O(N * M) 的量级，换成缓存后每层只做 O(1) 的加减，整条路径 O(N) 收工。几千个节点的树，差别跑得出来。计数挂在哪儿，咱们也有现成的位置，QTreeWidgetItem 本身能存自定义数据，setData 塞一个 QVariant 进去，把 NodeCache 的两个整数装在里面随节点走，不用另开一张映射表。配套示例满打满算三十九个节点，直接现算就够快，缓存留给深树再上不迟。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QCheckBox 与 QTreeWidget 属于 QtWidgets 模块，QTreeWidgetItem 的身份与 itemChanged 的发射路径对照 qtbase 源码核过，示例可在任何支持 Qt 6 的桌面平台上编译运行。

[Qt 文档 · QCheckBox](https://doc.qt.io/qt-6/qcheckbox.html) -- 复选框，三态模式说明

[Qt 文档 · QTreeWidget](https://doc.qt.io/qt-6/qtreewidget.html) -- 树形控件，itemChanged 信号

[Qt 文档 · QTreeWidgetItem](https://doc.qt.io/qt-6/qtreewidgetitem.html) -- 树节点项，setCheckState / checkState

[Qt 文档 · QObject::blockSignals](https://doc.qt.io/qt-6/qobject.html#blockSignals) -- 信号阻塞方法

[Qt 文档 · Qt::CheckState](https://doc.qt.io/qt-6/qt.html#CheckState-enum) -- 复选状态枚举

---

19 篇预告的三态话头到这里兑现了。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/20-qcheckbox-advanced/，可执行文件 20-qcheckbox-advanced）完整玩一轮。勾选 Administrator 看九个权限一次点亮，再勾亮某个 View，看半选怎样从权限层冒到角色层，又随着取消退回全灭，路径一步步追得到。把 onItemChanged 里两行对调再编译，停错的角色状态摆在眼前，次序的分量不用多讲。想练缓存的，拿 setData 给每个节点挂一份计数，勾选前后 qDebug 打出来对一对。下一篇 [3.21 QCommandLinkButton 进阶](./21-qcommandlinkbutton-advanced.md) 讲按钮五连的收尾 QCommandLinkButton，平台样式适配与 description 的布局行为在那边展开。
