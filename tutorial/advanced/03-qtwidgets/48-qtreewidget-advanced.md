---
title: "3.48 QTreeWidget 进阶"
description: "入门篇我们用 QTreeWidget 展示了层级数据，掌握了 addTopLevelItem/addChild/setColumnCount/currentItemChanged 信号、QTreeWidgetItem 的展开折叠控制，以及多列树的基本用法。"
---

# 现代Qt开发教程（进阶篇）3.48——QTreeWidget 进阶

## 1. 前言 / 当树不再"一次性全部加载"

入门篇我们用 QTreeWidget 展示了层级数据，掌握了 addTopLevelItem / addChild / setColumnCount / currentItemChanged 信号、QTreeWidgetItem 的展开折叠控制，以及多列树的基本用法。对于一个目录浏览器或者分类列表这种层级不超过三层的简单场景，入门篇那一套完全够用——所有节点在启动时一次性 addChild 完毕，展开折叠只是视觉上的显隐切换。但如果你要展示的树有一万多个节点（比如一个大型项目的目录结构），一次性加载所有节点不仅启动慢，内存占用也大——每个 QTreeWidgetItem 都是独立的堆对象，包含多个列的数据、字体、图标、自定义 role 数据等。更糟糕的是，如果数据源是远程的（比如网络文件系统），你不可能在启动时就遍历整个目录树。

今天我们把 QTreeWidget 的延迟加载机制和大数据优化拆透。核心内容是三个方面：QTreeWidgetItem 的 childIndicatorPolicy 和 expanded 信号实现懒加载、大量节点的批量操作优化、以及 QTreeWidget 的排序与过滤。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QTreeWidget 和 QTreeWidgetItem 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 懒加载——只在展开时加载子节点

懒加载的核心思路是：在创建父节点时先不加载任何子节点，只在用户展开这个节点时才去加载子数据。QTreeWidgetItem 提供了一个关键属性 childIndicatorPolicy 来控制展开指示器（那个小的三角形/加号）的显示行为。

```cpp
// 创建一个"待加载"的占位节点
auto *item = new QTreeWidgetItem({"加载中..."});
item->setChildIndicatorPolicy(
    QTreeWidgetItem::ShowIndicator);  // 强制显示展开指示器
```

childIndicatorPolicy 有三个值。ShowIndicator 始终显示展开指示器，即使该项没有子节点。DontShowIndicator 始终不显示。DontShowIndicatorWhenChildless 是默认值——有子节点时显示，没有时不显示。懒加载的关键就是用 ShowIndicator——即使节点还没有子节点，也显示展开指示器，让用户知道"这里可以展开"。

然后在 QTreeWidget 的 itemExpanded 信号中执行实际的子节点加载：

```cpp
connect(treeWidget, &QTreeWidget::itemExpanded,
        this, [this](QTreeWidgetItem *parent) {
    // 检查是否已经加载过
    if (parent->data(0, Qt::UserRole).toBool()) {
        return;  // 已加载，跳过
    }

    // 移除占位项（如果有的话）
    while (parent->childCount() > 0) {
        delete parent->takeChild(0);
    }

    // 加载实际子节点
    QStringList children = loadChildren(parent->text(0));
    for (const auto &name : children) {
        auto *child = new QTreeWidgetItem({name});
        // 如果子节点也可能有子节点，给它也设 ShowIndicator
        child->setChildIndicatorPolicy(
            QTreeWidgetItem::ShowIndicator);
        parent->addChild(child);
    }

    // 标记已加载
    parent->setData(0, Qt::UserRole, true);
});
```

这里有几个关键细节。第一是"已加载"标记——用 setData(Qt::UserRole, true) 标记这个节点的子节点已经加载过了。用户折叠后再展开不应该重新加载。如果你需要"每次展开都刷新子节点"的行为（比如远程文件系统内容可能变化），就不用设这个标记，每次展开时先 deleteAllChild 再重新加载。

第二是占位项的处理。有些人喜欢在创建父节点时加一个虚拟的子节点（文字为"加载中..."），这样默认的 childIndicatorPolicy 就会显示展开指示器。展开后删掉占位项再加载真实子节点。但如果你用了 ShowIndicator，就不需要占位项了。

第三是性能——itemExpanded 信号的槽函数在 UI 线程中同步执行。如果你的 loadChildren 需要做磁盘 IO 或者网络请求，必须在后台线程中执行，否则展开操作会卡住 UI。一个常见的模式是用 QFuture + QThreadPool 在后台加载子数据，加载完成后通过信号槽在 UI 线程中添加子节点。

### 3.2 大量节点的批量操作优化

QTreeWidget 的每个操作（addTopLevelItem、addChild、insertTopLevelItem 等）都会触发 model 的 rowsInserted 信号，进而触发 QTreeWidget 的布局重算。如果你一次性添加几千个节点，每次 addChild 都触发一次重算，性能会非常差。

```cpp
// 差的做法：逐个添加
for (int i = 0; i < 5000; ++i) {
    treeWidget->addTopLevelItem(
        new QTreeWidgetItem({QString("Item %1").arg(i)}));
}
// 触发了 5000 次布局重算

// 好的做法：批量添加
QList<QTreeWidgetItem*> items;
items.reserve(5000);
for (int i = 0; i < 5000; ++i) {
    items.append(
        new QTreeWidgetItem({QString("Item %1").arg(i)}));
}
treeWidget->addTopLevelItems(items);
// 只触发 1 次布局重算
```

addTopLevelItems(QList) 和 addChildren(QList) 是批量操作版本——它们只触发一次 rowsInserted 信号，不管列表里有多少项。对于大数据量的初始化，性能差距可能是几十倍。

同样的原则适用于删除——用 takeTopLevelItem 逐个删除比 clear() 慢得多。clear() 一次性清空所有顶层项，只触发一次 model reset。

```cpp
// 清空整个树
treeWidget->clear();  // 一次性，最快

// 只删除顶层项中的某几个
QList<QTreeWidgetItem*> to_remove;
for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
    auto *item = treeWidget->topLevelItem(i);
    if (shouldRemove(item)) {
        to_remove.append(item);
    }
}
qDeleteAll(to_remove);  // 批量删除
```

### 3.3 排序与过滤

QTreeWidget 提供了 setSortingEnabled(bool) 来开启列头点击排序。开启后点击列头会按该列的内容对所有可见项排序。排序使用 QTreeWidgetItem::operator< 做比较，默认是按字符串字典序排序。

```cpp
treeWidget->setSortingEnabled(true);
treeWidget->sortByColumn(0, Qt::AscendingOrder);
```

排序有一个容易忽略的行为：setSortingEnabled(true) 开启后，后续 addTopLevelItem / addChild 时 QTreeWidget 会自动把新项插入到排序后的正确位置，而不是添加到末尾。这意味着你不能假设项的索引和添加顺序一致——插入位置取决于排序结果。如果你需要按照特定顺序添加项，先关闭排序、添加完再开启。

```cpp
treeWidget->setSortingEnabled(false);
// 按特定顺序添加
for (const auto &name : orderedNames) {
    treeWidget->addTopLevelItem(
        new QTreeWidgetItem({name}));
}
treeWidget->setSortingEnabled(true);
treeWidget->sortByColumn(0, Qt::AscendingOrder);
```

过滤方面 QTreeWidget 没有内置的过滤功能（不像 QListView 有 QSortFilterProxyModel）。如果你需要搜索/过滤树节点，需要自己实现。最直接的方式是遍历所有项，根据匹配结果设置 hidden 属性。

```cpp
void filterTree(QTreeWidget *tree, const QString &keyword)
{
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        filterItem(tree->topLevelItem(i), keyword);
    }
}

bool filterItem(QTreeWidgetItem *item, const QString &keyword)
{
    bool has_match = item->text(0).contains(
        keyword, Qt::CaseInsensitive);
    bool child_match = false;

    for (int i = 0; i < item->childCount(); ++i) {
        if (filterItem(item->child(i), keyword)) {
            child_match = true;
        }
    }

    // 如果子节点匹配，父节点也应该显示
    item->setHidden(!(has_match || child_match));

    return has_match || child_match;
}
```

这段过滤逻辑有一个重要的细节：如果某个子节点匹配了关键词，它的所有祖先节点都不应该被隐藏——否则匹配的子节点虽然存在但不可见（因为父节点被隐藏了）。所以 filterItem 返回 bool 表示"该项或其后代是否有匹配"，如果有匹配则该项不隐藏。

这种遍历过滤的方式在节点数量较大时（超过几千个）会有明显的性能问题——每次关键词变化都要遍历整棵树。如果你的树很大，考虑用 QSortFilterProxyModel 配合 QTreeView（而不是 QTreeWidget）来实现过滤——QSortFilterProxyModel 有内置的增量过滤优化。

## 4. 踩坑预防

第一个坑是懒加载中 itemExpanded 槽函数执行磁盘 IO 导致界面卡死。itemExpanded 信号在 UI 线程中同步触发。如果你的 loadChildren 做了磁盘遍历或者网络请求，展开操作会阻塞 UI 直到加载完成。后果是用户点击展开后界面冻结数百毫秒甚至数秒。解决方案是把加载逻辑放到后台线程，展开时立即显示"加载中..."占位项，加载完成后替换为真实子节点。

第二个坑是批量添加后忘记 setUpdatesEnabled。即使你用了 addTopLevelItems 批量添加，如果 treeWidget 本身嵌在一个正在频繁重绘的父容器中，每次 model 变化可能触发父容器的布局重算。用 setUpdatesEnabled(false) / setUpdatesEnabled(true) 包裹批量操作更保险。

第三个坑是排序开启时项的索引不稳定。setSortingEnabled(true) 后，添加项的顺序和最终索引不一致。如果你通过 index 定位项（比如 topLevelItem(i)），索引可能不是你期望的。解决方案是通过 item 的 data(Qt::UserRole) 存储唯一标识符，用标识符来查找而不是靠索引。

## 5. 练习项目

练习项目：文件系统浏览器。我们要实现一个类似文件管理器的树形目录浏览器。

完成标准是：QTreeWidget 展示本地文件系统目录树。根节点是用户主目录（QDir::homePath()）。每个目录节点使用懒加载——展开时才读取子目录和文件。目录节点显示文件夹图标，文件节点显示文件图标。文件节点没有展开指示器。双击文件节点在状态栏显示文件路径和大小。顶部一个 QLineEdit 作为搜索框，输入关键词后过滤树——只显示文件名包含关键词的文件节点及其所有祖先目录节点。过滤逻辑在 200ms 防抖后执行（用 QTimer::singleShot）。

提示几个关键点：懒加载用 setChildIndicatorPolicy(ShowIndicator) + itemExpanded 信号；QDir::entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot) 获取目录内容；过滤用递归遍历 setHidden；防抖用 QTimer 单次触发。

## 6. 官方文档参考链接

[Qt 文档 · QTreeWidget](https://doc.qt.io/qt-6/qtreewidget.html) -- 树形控件，包含 addTopLevelItems/itemExpanded/setSortingEnabled 等接口

[Qt 文档 · QTreeWidgetItem](https://doc.qt.io/qt-6/qtreewidgetitem.html) -- 树节点，包含 setChildIndicatorPolicy/addChild/data 等方法

[Qt 文档 · QDir](https://doc.qt.io/qt-6/qdir.html) -- 目录操作类，entryInfoList 用于列举目录内容

---

到这里，QTreeWidget 的进阶内容就拆完了。懒加载的套路是 ShowIndicator + itemExpanded + UserRole 标记——不要一次性加载整棵树。批量操作用 addTopLevelItems/addChildren 而不是逐个 addChild。排序开启后索引不可靠，用 UserRole 存标识符定位。过滤需要递归处理父子关系——子节点匹配时祖先节点不能隐藏。把这些搞透后，你就能做出流畅的大树浏览器——而不只是"几十个节点的分类列表"。
