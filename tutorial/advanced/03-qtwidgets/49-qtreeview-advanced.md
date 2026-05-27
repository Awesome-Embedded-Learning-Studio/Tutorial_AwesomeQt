---
title: "3.49 QTreeView 进阶"
description: "入门篇我们用 QTreeView 配合 QStandardItemModel 展示了树形数据，掌握了 setModel/setCurrentIndex/expanded/collapsed 信号、setHeaderHidden 隐藏表头、以及 QStandardItem 的基本操作。"
---

# 现代Qt开发教程（进阶篇）3.49——QTreeView 进阶

## 1. 前言 / 当树的外观和行为都需要深度定制

入门篇我们用 QTreeView 配合 QStandardItemModel 展示了树形数据，掌握了 setModel / setCurrentIndex / expanded / collapsed 信号、setHeaderHidden 隐藏表头、以及 QStandardItem 的基本操作。对于一个标准的文件目录树或者分类导航，入门篇那一套够用了。但当你需要定制 QTreeView 的视觉细节——比如把默认的小三角形展开指示器换成自定义图标、让整行都被选中而不是只有第一列、给不同的层级设置不同的缩进——你会发现 QTreeView 提供的 API 比想象中分散，很多效果需要通过 QStyle 或者 delegate 来实现。

今天我们把 QTreeView 的视觉定制和交互增强拆透。核心内容是三个方面：自定义展开指示器和整行选中、indentation 和 rootIsDecorated 控制层级缩进、以及 QHeaderView 与树列的协作。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QTreeView 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。自定义绘制涉及 QStyledItemDelegate。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 整行选中与自定义展开指示器

默认情况下 QTreeView 的选中只覆盖第一列——点击某行的第一列会选中该列的文本区域，第二列及之后不会被高亮。这在多列树中看起来很不自然——用户期望点击整行的任何位置都能选中整行。

```cpp
// 开启整行选中
treeView->setSelectionBehavior(
    QAbstractItemView::SelectRows);
```

setSelectionBehavior(SelectRows) 让选中覆盖整行宽度——不只是有内容的列，而是从第一列到最后一列的完整行高。

自定义展开指示器（就是那个 ▶/▼ 三角形）需要通过 QStyle 或者 QStyledItemDelegate 来实现。QTreeView 使用 QStyle::PE_IndicatorBranch 原语来绘制展开/折叠指示器。你可以通过 QSS 来替换默认图标：

```css
QTreeView::branch:has-children:!has-siblings:closed,
QTreeView::branch:closed:has-children:has-siblings {
    border-image: none;
    image: url(:/icons/branch-closed.png);
}

QTreeView::branch:open:has-children:!has-siblings,
QTreeView::branch:open:has-children:has-siblings  {
    border-image: none;
    image: url(:/icons/branch-open.png);
}

QTreeView::branch:has-children:!has-siblings:closed:hover,
QTreeView::branch:closed:has-children:has-siblings:hover {
    image: url(:/icons/branch-closed-hover.png);
}
```

QSS 的 `QTreeView::branch` 选择器支持以下伪状态：`:has-children` 有子节点，`:!has-children` 无子节点，`:has-siblings` 有兄弟节点，`:!has-siblings` 无兄弟节点，`:open` 展开，`:closed` 折叠，`:hover` 悬停。通过组合这些伪状态，你可以为不同层级的节点设置不同的展开图标。

如果 QSS 不够灵活（比如你想根据节点数据动态切换图标），可以在 QStyledItemDelegate 的 paint 方法中手动绘制展开指示器——检查 index 的 child 是否存在，画对应的图标。但这种方式需要处理 hitTest 来拦截点击事件，复杂度较高。

### 3.2 indentation 和 rootIsDecorated

QTreeView 默认每层缩进 20 像素。你可以通过 setIndentation(int) 改变这个值——较小的值让树更紧凑，较大的值让层级关系更清晰。

```cpp
treeView->setIndentation(24);  // 每层缩进 24 像素
```

setRootIsDecorated(bool) 控制顶层节点是否显示展开指示器。默认为 true——顶层节点也有三角形指示器。设为 false 后顶层节点看起来像普通列表项，只有子节点有缩进和指示器。这在"根节点只是分类标题、子节点才是实际数据"的场景下很有用——根节点不需要可展开的外观。

```cpp
// 根节点不显示展开指示器
treeView->setRootIsDecorated(false);
```

setRootIsDecorated(false) 不会影响实际的可展开行为——根节点仍然可以有子节点，仍然可以展开折叠。它只是视觉上去掉了展开指示器。用户双击根节点仍然可以展开它。如果你真的想禁止根节点展开，需要在 model 中控制。

setAnimated(bool) 开启展开/折叠的过渡动画。默认为 false。开启后展开时子节点会从 0 高度动画到完整高度，折叠时反向动画。动画时长由 setAnimationDuration(int) 控制（Qt 6.5+ 新增），默认约 400ms。

```cpp
treeView->setAnimated(true);
treeView->setIndentation(20);
```

setAnimated(true) 的动画效果在节点较少（几十个）时看起来不错，但节点较多（几百个子节点同时展开）时动画帧率会下降，甚至出现卡顿。如果你的树节点可能有很多子节点，谨慎开启动画或者用 setAnimationDuration 设一个较短的时长（100-200ms）。

### 3.3 QHeaderView 与列宽管理

QTreeView 顶部有一个 QHeaderView 控制列宽和列标题。QHeaderView 的行为直接影响树列的视觉呈现。

```cpp
auto *header = treeView->header();

// 交互式调整列宽（默认开启）
header->setSectionResizeMode(QHeaderView::Interactive);

// 第一列自动拉伸占满剩余空间
header->setSectionResizeMode(0, QHeaderView::Stretch);

// 其他列按内容自动调整
header->setSectionResizeMode(1, QHeaderView::ResizeToContents);

// 禁止用户拖动调整列宽
header->setSectionsMovable(false);

// 隐藏最后一列的后面空间
header->setStretchLastSection(true);
```

setSectionResizeMode 是最常用的配置。Stretch 模式让列自动填满剩余宽度——适合"名称"列。ResizeToContents 让列宽自动适应内容——适合"大小"、"日期"这种内容较短的列。Interactive 是默认模式——用户可以手动拖动列边界调整宽度。

setSectionsMovable(bool) 控制用户是否可以拖动列头来重新排列列顺序。默认为 true。如果你的树的列顺序有业务含义（比如第一列必须是名称），设为 false 防止用户打乱。

一个常见的布局需求是：第一列（树列）占满剩余空间，其他列按内容自适应。这种布局下，当窗口缩小时，只有第一列变窄——其他列保持内容宽度不变。

```cpp
header->setSectionResizeMode(0, QHeaderView::Stretch);
for (int i = 1; i < header->count(); ++i) {
    header->setSectionResizeMode(i,
        QHeaderView::ResizeToContents);
}
```

headerData 的 Qt::TextAlignmentRole 可以控制列标题的对齐方式——默认左对齐，数值列可以设为右对齐。

## 4. 踩坑预防

第一个坑是 SelectRows 和 setItemWidget 的交互问题。开启 SelectRows 后选中高亮覆盖整行，但如果你用 setIndexWidget 在某列嵌入了一个 QPushButton 之类的交互控件，点击按钮区域不会触发行的选中——按钮吞掉了鼠标事件。后果是用户点击按钮后那行看起来没被选中（视觉上高亮没出现），但点击行的其他位置又能选中。解决方案是给 setIndexWidget 的控件安装事件过滤器，在 mousePressEvent 中手动触发 treeView->setCurrentIndex。

第二个坑是 QSS branch 图标在某些平台上不生效。在 macOS 上 QTreeView 使用原生的展开指示器（NSOutlineView 风格），QSS 的 branch 样式可能被原生样式覆盖。如果你发现 QSS 设了 branch 图标但显示的还是默认三角形，检查是否在 macOS 上运行——需要在 QApplication 构造前设置 `QApplication::setAttribute(Qt::AA_DontUseNativeDialogs)` 或者使用 Fusion 风格来确保 QSS 生效。

第三个坑是 setAnimated(true) 在大量子节点展开时的性能问题。展开动画需要对每个子项的高度做插值。如果一个节点有 500 个子项，动画期间每帧都要计算 500 个项的位置。后果是展开大节点时动画掉帧明显。解决方案是限制同时展开的子项数量，或者关闭动画改用即时展开。

## 5. 练习项目

练习项目：项目文件浏览器。我们要实现一个类似 IDE 项目面板的树形视图。

完成标准是：QTreeView 配合 QStandardItemModel 显示一个模拟的项目结构。根节点是项目名"MyProject"，包含三个顶级分组："Sources"（包含 5 个 .cpp 文件）、"Headers"（包含 5 个 .h 文件）、"Resources"（包含 3 个 .qrc 文件）。整行选中模式。根节点不显示展开指示器（setRootIsDecorated(false)），三个分组节点显示自定义展开图标（QSS branch）。每行有 4 列：名称、大小、类型、修改日期。第一列 Stretch，其他列 ResizeToContents。开启展开动画。点击文件节点在状态栏显示"已选择: [文件路径]"。

提示几个关键点：QStandardItem::setChild 构建层级结构；QStandardItem 的 setEditable(false) 禁止双击编辑；header 的 setSectionResizeMode 分别设置每列。

## 6. 官方文档参考链接

[Qt 文档 · QTreeView](https://doc.qt.io/qt-6/qtreeview.html) -- 树视图，包含 setIndentation/setRootIsDecorated/setAnimated/expandAll 等接口

[Qt 文档 · QHeaderView](https://doc.qt.io/qt-6/qheaderview.html) -- 表头控件，setSectionResizeMode 控制列宽策略

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 自定义项绘制委托

---

到这里，QTreeView 的进阶内容就拆完了。整行选中一行代码搞定（SelectRows），自定义展开图标走 QSS branch 选择器，缩进和根节点装饰各有独立开关。QHeaderView 的列宽策略要按列分别设置——Stretch 给树列，ResizeToContents 给数据列。展开动画好看但对大节点有性能影响。掌握了这些，你就能做出视觉精致、交互流畅的树形视图——而不只是"能展开折叠的列表"。
