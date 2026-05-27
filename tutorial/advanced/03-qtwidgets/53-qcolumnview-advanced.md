---
title: "3.53 QColumnView 进阶"
description: "入门篇我们用 QColumnView 搭了类似 macOS Finder 列视图的多列浏览器，掌握了 setModel/setPreviewWidget/clicked 信号以及基本的多列导航行为。"
---

# 现代Qt开发教程（进阶篇）3.53——QColumnView 进阶

## 1. 前言 / 当列视图需要更精细的控制

入门篇我们用 QColumnView 搭了类似 macOS Finder 列视图的多列浏览器，掌握了 setModel / setPreviewWidget / clicked 信号以及基本的多列导航行为。QColumnView 的核心交互很直观——点击一个有子项的节点，右侧展开新的一列显示子项；点击叶子节点，最右侧显示预览信息。对于简单的层级浏览，入门篇那一套够用了。但 QColumnView 在细节定制上有些特殊的地方——列宽策略和普通 QTreeView 不同，预览组件的更新时机需要特别处理，而且它和 QTreeView 共享 model 但行为差异不小。

今天我们把 QColumnView 的定制能力拆透。核心内容是三个方面：列宽控制与 setColumnWidths、预览组件的动态更新策略、以及 QColumnView 与 QTreeView 的行为差异。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QColumnView 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 列宽控制与 setColumnWidths

QColumnView 的每一列实际上是一个独立的 QListView。这些 QListView 水平排列在一个 QScrollArea 中，列数随着用户的导航深度动态增减。列宽的控制和 QTreeView 完全不同——你不能通过 QHeaderView 来管理列宽，因为每一列是独立的视图。

QColumnView::setColumnWidths(const QList<int> &sizes) 设置各列的初始宽度。列表中第 N 个值对应第 N 列的宽度。如果列表长度小于当前列数，剩余列使用默认宽度。

```cpp
// 设置前三列的宽度
columnView->setColumnWidths({200, 180, 160});
```

setColumnWidths 有一个行为需要注意：它设置的是初始宽度，不是固定宽度。用户仍然可以手动拖动列边界来调整宽度。如果你需要列宽固定不可调，需要获取每列的 QListView 然后设 setSizePolicy 或者在 resizeEvent 中强制恢复。

```cpp
// 获取各列的 QListView
for (int i = 0; i < columnView->model()->columnCount(); ++i) {
    if (auto *list = columnView->columnView(i)) {
        list->setMinimumWidth(150);
        list->setMaximumWidth(250);
    }
}
```

columnView(int index) 不在公共 API 中（它是 protected 方法）。如果你需要获取各列视图，可能需要遍历 QColumnView 的子对象。实际上，QColumnView 暴露的列宽控制接口非常有限——除了 setColumnWidths 之外几乎没有其他公共方法。这是 QColumnView 被认为"定制能力有限"的主要原因之一。

### 3.2 预览组件的动态更新

QColumnView::setPreviewWidget(QWidget *widget) 设置预览区域——当用户选中一个叶子节点时，这个 widget 显示在最后一列的右侧。预览组件的核心问题是：setPreviewWidget 只设置一次 widget 实例，你需要在选中项变化时更新这个 widget 的内容。

```cpp
auto *preview = new QWidget;
auto *preview_layout = new QVBoxLayout(preview);

auto *icon_label = new QLabel;
auto *name_label = new QLabel;
auto *detail_label = new QLabel;

preview_layout->addWidget(icon_label, 0, Qt::AlignCenter);
preview_layout->addWidget(name_label, 0, Qt::AlignCenter);
preview_layout->addWidget(detail_label);

columnView->setPreviewWidget(preview);

// 选中项变化时更新预览
connect(columnView->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this, [this](const QModelIndex &current) {
    if (!current.isValid()) return;

    // 更新预览内容
    QString name = current.data(Qt::DisplayRole).toString();
    QString detail = current.data(Qt::UserRole + 1).toString();
    QPixmap icon = current.data(Qt::DecorationRole).value<QPixmap>();

    name_label->setText(name);
    detail_label->setText(detail);
    icon_label->setPixmap(icon.scaled(64, 64,
        Qt::KeepAspectRatio, Qt::SmoothTransformation));
});
```

预览组件只在叶子节点（没有子节点的节点）被选中时显示。如果你选中了一个有子节点的中间节点，QColumnView 会展开它的子列而不是显示预览。这个行为是内置的——你不能让中间节点也显示预览，除非你在 model 的 hasChildren 中返回 false 来欺骗 QColumnView（但这会导致子列不展开）。

preview 的显示和隐藏由 QColumnView 内部控制。你不能手动 show/hide preview widget——QColumnView 会根据当前选中项是否为叶子节点来管理 preview 的可见性。

### 3.3 与 QTreeView 的行为差异

QColumnView 和 QTreeView 都继承自 QAbstractItemView，都使用相同的 model 接口。但它们的导航行为有根本区别。

QTreeView 在同一个视图中用缩进展示层级关系——子节点缩进在父节点下面。QColumnView 在不同的列中展示层级关系——每个层级一列。这意味着 QTreeView 的 currentIndex 只有一个，而 QColumnView 每一列都有自己的选中项。

在 model 层面，QColumnView 对 model 有一个隐含的要求：model 的 hasChildren(const QModelIndex &) 必须正确实现。如果 hasChildren 返回 true 但 rowCount 返回 0，QColumnView 会展开一个空列——用户体验很糟糕。反过来，如果 hasChildren 返回 false 但实际有子节点，QColumnView 不会展开新列——用户无法导航到子节点。QTreeView 对这个问题有更好的容错——它允许用户双击展开一个没有声明 hasChildren 的节点。

QColumnView 的 expand / collapse 概念也不同于 QTreeView。QTreeView 中 expand 展开一个节点的子节点（在同一视图中显示），collapse 折叠子节点。QColumnView 中"展开"意味着在右侧创建新列，"折叠"意味着移除右侧的列。当你选中一个不同的父节点时，它右侧的子列会被销毁重建。

## 4. 踩坑预防

第一个坑是 setPreviewWidget 的 widget 只能设一个。你不能为不同类型的节点设置不同的预览 widget——只有一个预览区域，你需要在这个 widget 内部根据选中项类型切换显示内容。后果是如果你有三种不同类型的叶子节点（文件、文件夹、快捷方式），你的预览 widget 内部需要用 QStackedWidget 来切换三套不同的布局。

第二个坑是列宽在窗口 resize 时可能不会按比例调整。QColumnView 的各列宽度是绝对值，不跟随窗口宽度自动伸缩。当窗口变宽时，右侧可能出现空白区域；当窗口变窄时，可能出现水平滚动条。如果你希望列宽跟随窗口宽度变化，需要重写 resizeEvent，根据新宽度重新分配列宽。

第三个坑是 model 的 hasChildren 不准确导致导航异常。hasChildren 返回 true 但 rowCount 返回 0 时，QColumnView 会展开一个空列。hasChildren 返回 false 但实际有子节点时，用户无法导航到子节点。后果是层级浏览不完整——用户能看到某些节点有子项的标记但展开后是空的。解决方案是确保 model 的 hasChildren 实现与 rowCount 保持一致。

## 5. 练习项目

练习项目：多层级文件类型浏览器。我们要实现一个按文件类型组织的列视图浏览器。

完成标准是：QColumnView 配合 QStandardItemModel。根级分类为："文档"、"图片"、"音频"、"视频"。每个分类下有 3-5 个文件名（叶子节点）。预览区域显示选中文件的文件名、类型图标（QLabel 显示纯色方块模拟）和文件大小（QLabel 显示模拟数据）。列宽设为 180px。叶子节点不可再展开。点击分类节点时右侧展开文件列表，点击文件节点时显示预览。

提示几个关键点：QStandardItem::setChild 构建层级；叶子节点不要设子项这样 hasChildren 自动返回 false；预览 widget 用 currentChanged 信号更新内容。

## 6. 官方文档参考链接

[Qt 文档 · QColumnView](https://doc.qt.io/qt-6/qcolumnview.html) -- 列视图控件，包含 setPreviewWidget/setColumnWidths 等接口

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- 抽象视图基类，selectionModel/currentChanged 信号

[Qt 文档 · QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 标准项模型，构建层级数据

---

到这里，QColumnView 的进阶内容就拆完了。QColumnView 的定制能力确实有限——列宽控制只有 setColumnWidths、预览组件只有一个实例需要内部切换、没有 QHeaderView 管理列宽。它的优势是开箱即用的多列导航体验，劣势是深度定制困难。如果你需要更灵活的列视图，可以考虑用多个 QListView + QHBoxLayout 自己搭。但对于标准的层级浏览器需求，QColumnView 仍然是最省事的选择。
