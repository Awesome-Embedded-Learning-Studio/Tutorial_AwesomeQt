---
title: "3.46 QListWidget 进阶"
description: "入门篇我们用 QListWidget 展示了字符串列表、图标列表、多选和单选操作，掌握了 addItem/setCurrentRow/currentItemChanged 信号以及 QSS 定制列表项外观。"
---

# 现代Qt开发教程（进阶篇）3.46——QListWidget 进阶

## 1. 前言 / 当列表不只是"一行一行的文字"

入门篇我们用 QListWidget 展示了字符串列表、图标列表、多选和单选操作，掌握了 addItem / setCurrentRow / currentItemChanged 信号以及 QSS 定制列表项外观。对于简单的选择列表——比如字体列表、颜色列表、国家列表——入门篇那一套完全够用。但一旦你需要做"拖拽调整顺序"、"给每一项嵌入自定义控件（比如带头像和状态指示器的联系人项）"、"或者把列表项导出为文件路径"这类需求，你就会发现 addItem(QString) 那个接口远远不够。

今天我们把 QListWidget 的进阶能力拆透。核心内容是四个方面：setDragDropMode 实现内部拖放排序、setItemWidget 给列表项嵌入自定义 QWidget、MIME 数据的自定义与外部拖放交互，以及大量列表项的性能优化。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QListWidget 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。拖放涉及 QMimeData 和 QDrag（QtGui）。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 setDragDropMode 实现内部拖放排序

QListWidget 内置了拖放支持，通过 setDragDropMode 配置拖放行为。最常用的模式是 InternalMove——允许用户在列表内部拖拽项目来调整顺序。

```cpp
listWidget->setDragDropMode(QAbstractItemView::InternalMove);
listWidget->setDefaultDropAction(Qt::MoveAction);
listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
```

InternalMove 模式下，用户按住某个列表项拖动时，QListWidget 会在拖动位置显示一个插入指示线（drop indicator），松开鼠标后项目被移动到新位置。整个过程中 QListWidget 自动处理 QModelIndex 的更新——你不需要手动移动数据。

但有一个关键点：InternalMove 只移动了项的显示顺序，它不会自动同步你维护的外部数据结构。比如你有一个 `QVector<QString> m_items` 和 QListWidget 的显示一一对应，用户通过拖放调整了列表顺序后，`m_items` 的顺序并没有变。你需要在模型变化后读取新的顺序来同步。

```cpp
connect(listWidget->model(), &QAbstractItemModel::rowsMoved,
        this, [this]() {
    // 重新同步外部数据
    m_items.clear();
    for (int i = 0; i < listWidget->count(); ++i) {
        m_items.append(listWidget->item(i)->text());
    }
});
```

rowsMoved 信号在拖放完成后触发。你在这个槽里遍历 QListWidget 的所有项，按照新的显示顺序重建你的外部数据。如果你需要更细粒度的控制——比如限制某些项目不能被拖到某些位置——你可以子类化 QListView（QListWidget 的基类）并重写 dropEvent。

setDragEnabled(bool) 控制项目是否可以拖出，setAcceptDrops(bool) 控制是否接受外部拖入。默认 InternalMove 模式下两者都是 true。如果你只想允许拖出但不允许拖入（比如把列表项拖到另一个窗口），设 setAcceptDrops(false) 并把 dragDropMode 改为 QAbstractItemView::DragOnly。

### 3.2 setItemWidget 嵌入自定义控件

QListWidgetItem 默认只能显示文字和图标。如果你需要在列表项中嵌入更复杂的控件——比如一个联系人卡片包含头像、名称和在线状态指示器，或者一个音乐列表项包含播放按钮和进度条——就需要用 setItemWidget。

```cpp
auto *item = new QListWidgetItem(listWidget);
listWidget->addItem(item);

auto *widget = new QWidget;
auto *layout = new QHBoxLayout(widget);
layout->setContentsMargins(8, 4, 8, 4);

auto *avatar = new QLabel;
avatar->setPixmap(QPixmap(":/avatar.png").scaled(40, 40,
    Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
layout->addWidget(avatar);

auto *info = new QVBoxLayout;
auto *name = new QLabel("张三");
name->setStyleSheet("font-weight: bold;");
auto *status = new QLabel("在线");
status->setStyleSheet("color: #4caf50;");
info->addWidget(name);
info->addWidget(status);
layout->addLayout(info, 1);

listWidget->setItemWidget(item, widget);
```

setItemWidget 接受一个 QListWidgetItem 和一个 QWidget，把 widget 关联到该项。QListWidget 在渲染该项时不再使用默认的委托绘制，而是直接显示你提供的 widget。

setItemWidget 有几个重要的限制需要你心里有数。第一是性能——setItemWidget 为每个列表项创建一个完整的 QWidget 层级（包括布局管理器、子控件等），如果列表有上千项，内存和渲染开销会非常大。setItemWidget 适合几十到几百项的场景，不适合大数据列表。大数据场景应该用 QListView + 自定义 QStyledItemDelegate，delegate 只负责绘制（不创建控件），性能好得多。

第二是 sizeHint——使用 setItemWidget 后，QListWidgetItem 的 sizeHint 需要手动设置，否则列表项可能显示高度不够，widget 的下半部分被截断。

```cpp
item->setSizeHint(widget->sizeHint());
```

如果你在 setItemWidget 之后动态改变了 widget 的大小（比如展开/折叠内容），需要重新设置 sizeHint。

第三是选择和交互。setItemSelected / setCurrentItem 仍然能正常工作——选中状态由 QListWidget 管理，不影响 setItemWidget 的 widget。但 widget 上的子控件（QPushButton、QLineEdit 等）会正常接收鼠标事件，不会触发列表项的选中。这意味着如果你在 widget 上放了一个按钮，点击按钮不会选中列表项——点击按钮区域外的部分才会选中。如果你希望点击 widget 的任何位置都选中列表项，需要自己处理——比如在 widget 上安装事件过滤器，把鼠标事件转发给 QListWidget。

### 3.3 MIME 数据与外部拖放

QListWidget 的拖放不限于内部——你可以把列表项拖到其他应用，或者从其他应用拖入数据。这通过 MIME 类型机制实现。

默认情况下 QListWidget 的内部拖放使用 `application/x-qabstractitemmodeldatalist` 这个私有 MIME 类型。如果你想让列表项可以被拖到外部应用（比如文件管理器），需要子类化 QListWidget 并重写 mimeData 方法，把项的数据编码为标准的 MIME 格式。

```cpp
QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override
{
    auto *mime = new QMimeData;
    QList<QUrl> urls;
    for (auto *item : items) {
        // 假设 item->data(Qt::UserRole) 存储了文件路径
        QString path = item->data(Qt::UserRole).toString();
        urls.append(QUrl::fromLocalFile(path));
    }
    mime->setUrls(urls);
    return mime;
}
```

这段代码把选中的列表项转换为文件 URL 列表，拖到文件管理器后就是复制/移动文件的操作。反过来，如果你想让 QListWidget 接受外部拖入的文件，需要重写 dropMimeData 并处理 `text/uri-list` MIME 类型。

```cpp
bool dropMimeData(int index, const QMimeData *data,
                  Qt::DropAction action) override
{
    if (data->hasUrls()) {
        for (const auto &url : data->urls()) {
            auto *item = new QListWidgetItem(
                url.toLocalFile());
            insertItem(index, item);
            ++index;
        }
        return true;
    }
    return QListWidget::dropMimeData(index, data, action);
}
```

supportedDropActions() 控制支持的拖放动作类型。如果你只允许复制不允许移动，返回 Qt::CopyAction。

### 3.4 大量列表项的性能优化

QListWidgetItem 是重量级的——每个项都是一个独立的 QListWidgetItem 对象，包含文字、图标、字体、背景色、自定义数据等。当列表项数量达到数千时，QListWidget 的性能会明显下降，特别是初始化阶段（逐个 addItem）和滚动阶段（每个项都需要渲染）。

优化方向有几个。第一是批量添加时用 setUpdatesEnabled(false) 包裹，防止每添加一项都触发一次重绘。

```cpp
listWidget->setUpdatesEnabled(false);
for (int i = 0; i < 5000; ++i) {
    listWidget->addItem(QString("项目 %1").arg(i));
}
listWidget->setUpdatesEnabled(true);
```

第二是设置 uniformItemSizes 属性。如果你的所有列表项大小相同（这是最常见的情况），把这个属性设为 true 可以让 QListView 跳过逐项计算 sizeHint 的步骤，直接用固定尺寸来布局。

```cpp
listWidget->setUniformItemSizes(true);
```

第三是 setItemWidget 的替代方案。如果你需要自定义外观但不需要交互控件（按钮、输入框等），用 QStyledItemDelegate 重写 paint 方法代替 setItemWidget。delegate 只负责绘制，不创建 QWidget，性能提升一个数量级。

如果你的列表真的需要处理上万条数据，QListWidget 已经不是合适的工具了——你应该用 QListView + 自定义 QAbstractListModel，利用 QListView 内置的懒加载机制（只渲染可见区域的项）。这属于 QListView 进阶的内容，我们下一篇会详细讲。

## 4. 踩坑预防

第一个坑是 InternalMove 拖放后外部数据不同步。QListWidget 的拖放只更新了自身的显示顺序，不会自动同步你维护的外部数据结构。后果是用户拖放调整了顺序，但你的 `m_items[index]` 还是旧顺序——下次操作（比如删除第 N 项）会删错项。解决方案是在 rowsMoved 信号中重建外部数据，或者在每次需要访问数据时直接从 QListWidget 读取（不维护外部副本）。

第二个坑是 setItemWidget 后 sizeHint 不够导致 widget 被截断。QListWidget 使用 QListWidgetItem 的 sizeHint 来决定列表项的高度，但 setItemWidget 不会自动更新 sizeHint。如果你在 setItemWidget 后没有手动设 `item->setSizeHint(widget->sizeHint())`，widget 会按默认的列表项高度显示——内容超出的部分直接被裁切。后果是看起来只有文字的上面一半，头像只露出个头顶。

第三个坑是大量 setItemWidget 导致内存和渲染性能问题。每个 setItemWidget 都创建了一个完整的 QWidget 层级。1000 个列表项就是 1000 个 QWidget + 1000 个 Layout + 大量子控件。滚动时所有经过可见区域的项都需要 show/hide，内存占用可能达到数百 MB。后果是列表滚动时明显卡顿，窗口 resize 时更卡。解决方案是超过 100 项就不要用 setItemWidget，改用 QStyledItemDelegate 自定义绘制。

## 5. 练习项目

练习项目：可拖拽排序的播放列表。我们要实现一个类似音乐播放器的列表界面。

完成标准是：QListWidget 包含 10 首歌曲信息。每个列表项通过 setItemWidget 显示：左侧一个 32x32 的封面图占位（用 QLabel 显示一个纯色方块，颜色从预定义列表取），中间上方是歌曲名（粗体），中间下方是歌手名（灰色），右侧是时长标签（如"3:45"）。列表开启 InternalMove 拖拽排序，拖拽后同步一个外部的 `QVector<SongInfo>` 数据结构。双击列表项播放（打印"正在播放: [歌曲名]"）。右键菜单包含"移除"和"移到顶部"两个操作。

提示几个关键点：setItemWidget 后要设 sizeHint；rowsMoved 信号里遍历 count() 重建 QVector；右键菜单用 setContextMenuPolicy(Qt::CustomContextMenu) + customContextMenuRequested 信号。

## 6. 官方文档参考链接

[Qt 文档 · QListWidget](https://doc.qt.io/qt-6/qlistwidget.html) -- 列表控件，包含 addItem/setItemWidget/setDragDropMode 等接口

[Qt 文档 · QListWidgetItem](https://doc.qt.io/qt-6/qlistwidgetitem.html) -- 列表项，包含 setData/sizeHint/flags 等

[Qt 文档 · QAbstractItemView::DragDropMode](https://doc.qt.io/qt-6/qabstractitemview.html#DragDropMode-enum) -- 拖放模式枚举

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 自定义绘制委托（setItemWidget 的替代方案）

---

到这里，QListWidget 的进阶内容就拆完了。InternalMove 一行代码就能开启拖拽排序，但别忘了同步外部数据。setItemWidget 能在列表项里嵌入任意控件，但它有明确的性能上限——几百项是极限，再多了换 delegate。MIME 数据机制让列表项可以和外部应用交互，拖文件进拖文件出都能实现。掌握这些后，你就能做出专业级的列表界面——而不只是"一列文字能选"的基本列表。
