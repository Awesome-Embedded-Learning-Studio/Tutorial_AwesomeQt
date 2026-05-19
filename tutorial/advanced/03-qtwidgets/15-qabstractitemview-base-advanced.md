---
title: "3.15 QAbstractItemView 基类进阶"
description: "入门篇我们把选择模式、委托机制、currentIndex/selectedIndexes 这些基础接口过了一遍。进阶篇要解决的是你在工程实战中一定会撞上的进阶场景：拖拽排序、持久编辑器、大数据量虚拟列表优化、以及自定义 viewport 的正确姿势。"
---

# 现代Qt开发教程（进阶篇）3.15——QAbstractItemView 基类进阶

## 1. 前言 / 为什么 QAbstractItemView 的进阶内容这么重要

入门篇我们用 QAbstractItemView 搭建了一个基本的表格视图，学会了选择模式、委托、选择模型这些东西。说实话，那些知识做一个静态展示的表格绰绰有余。但当你开始做交互密集型的列表——比如一个可以拖拽排序的任务列表、一个带内嵌进度条的下载管理器、或者一个展示上万条数据的高性能日志面板——入门篇的内容就捉襟见肘了。拖拽排序的 API 组合复杂、持久编辑器的生命周期管理容易踩坑、大数据量下 QListView 的默认行为会直接把界面拖卡。这篇进阶就是来解决这些问题的。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。依赖 QtWidgets 模块，拖拽部分涉及 QDrag/QMimeData，持久编辑器部分涉及委托的 createEditor/destroyEditor，虚拟列表部分涉及 QAbstractItemModel 的 canFetchMore/fetchMore 机制。所有内容在桌面平台上行为一致。

## 3. 核心概念讲解

### 3.1 拖拽排序——setDragDropMode 的完整配置

QAbstractItemView 内置了一套完整的拖拽框架。你不需要自己处理 QDrag 和 dropEvent——只需要配置好几个属性，视图就能自动处理拖拽排序。核心属性有四个：`setDragDropMode`、`setDefaultDropAction`、`setDragEnabled`、`setAcceptDrops`。

setDragDropMode 有四个值。NoDragDrop 禁用拖拽（默认）。DragOnly 只允许拖出，不允许拖入——适合做"从列表拖到别的地方"的场景。DropOnly 只允许拖入，不允许拖出——适合做"从别的地方拖到列表"的场景。InternalMove 允许在视图内部拖拽移动——这是实现排序最常用的模式。InternalMove 模式下，用户拖拽一个项目到另一个位置时，视图会自动调用模型的 `dropMimeData` 方法来执行移动操作。

```cpp
auto *listView = new QListView();
listView->setModel(model);
listView->setDragEnabled(true);           // 允许拖拽
listView->setAcceptDrops(true);           // 允许接收拖放
listView->setDragDropMode(QAbstractItemView::InternalMove);
listView->setDefaultDropAction(Qt::MoveAction);
```

但这里有一个关键的细节：InternalMove 的"移动"操作最终是通过模型的 `mimeData`、`dropMimeData` 和 `removeRows` 三个方法协同完成的。如果你用的是 QStandardItemModel，这三个方法都有默认实现，直接可用。如果你用的是自定义模型，你必须实现 `mimeTypes()`、`mimeData()`、`dropMimeData()` 和 `supportedDragActions()` 这几个方法，否则拖拽操作不会生效。很多朋友在 QListView 上配了 InternalMove 但发现拖拽没反应，原因就是自定义模型没有实现这些方法。

另外一个容易混淆的点是 setDefaultDropAction 和 setDragDropMode 的关系。setDefaultDropAction 设置的是拖拽时默认的 drop action——Qt::MoveAction、Qt::CopyAction 或 Qt::LinkAction。这个值只在 DragDrop 模式下有效，InternalMove 模式会自动使用 MoveAction，不需要额外设置。如果你用的是 DragDrop 模式而不是 InternalMove，setDefaultDropAction(Qt::MoveAction) 才是必要的。

### 3.2 持久编辑器——openPersistentEditor / closePersistentEditor

默认情况下，视图中的编辑器是临时的——用户双击一个单元格，编辑器弹出，编辑完成（或取消）后编辑器关闭销毁。但有些场景你需要编辑器始终显示在某个单元格上，比如进度条、复选框、颜色选择器。这时候就需要用 `openPersistentEditor(const QModelIndex &index)`。

openPersistentEditor 会让视图为指定索引创建一个编辑器（通过委托的 createEditor），并且不会在编辑完成后自动关闭。编辑器会一直存在，直到你调用 closePersistentEditor 或者索引对应的行被移除。

```cpp
// 给某一行的第 0 列打开持久进度条编辑器
for (int row = 0; row < model->rowCount(); ++row) {
    QModelIndex progressIndex = model->index(row, 0);
    listView->openPersistentEditor(progressIndex);
}
```

持久编辑器的数据同步由视图自动处理——当模型数据变化时，视图会调用委托的 `setEditorData` 来更新编辑器的显示。反过来，当用户在编辑器中修改了值，编辑器需要通过某种方式通知模型更新。对于标准控件（QProgressBar、QCheckBox），Qt 内部的委托机制会处理这个同步。但对于自定义编辑器，你需要在编辑器的值变化信号中手动调用 `model->setData(index, newValue)`。

现在有一道调试题给大家。看下面这段代码：

```cpp
// 删除模型中的第 3 行
model->removeRow(3);
// 此时第 3 行的持久编辑器会怎样？
```

答案是——第 3 行的持久编辑器会被关闭和销毁，但第 4 行及以后的行如果有持久编辑器，它们对应的索引会自动偏移（原来的第 4 行变成第 3 行）。视图会自动处理这个索引重映射。但如果你在 removeRow 之前保存了某个持久编辑器的指针，removeRow 之后那个指针就悬空了——因为 destroyEditor 被调用后编辑器被 delete 了。所以永远不要持有持久编辑器的裸指针，需要时通过委托获取。

### 3.3 虚拟列表优化——大数据量下的 QListView

当你的模型有上万甚至上百万条数据时，直接把全部数据加载到 QStandardItemModel 中是不可行的——内存消耗太大，初始化太慢。QListView 配合自定义模型可以实现"虚拟列表"：模型只告诉视图"总共有多少条数据"，视图只请求当前可见的那些行的数据。

实现虚拟列表的关键是 QAbstractItemModel 的 `canFetchMore(const QModelIndex &parent)` 和 `fetchMore(const QModelIndex &parent)` 这两个虚函数。canFetchMore 返回 true 表示模型还有更多数据可以加载，fetchMore 负责加载一批新数据。QListView 在滚动到底部时会检查 canFetchMore，如果返回 true 就调用 fetchMore 加载更多行。

```cpp
class VirtualLogModel : public QAbstractListModel
{
    Q_OBJECT

public:
    int rowCount(const QModelIndex & = {}) const override
    {
        return m_loadedCount;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role != Qt::DisplayRole || !index.isValid())
            return {};

        // 按需加载：只在这里读取第 index.row() 条数据
        return fetch_log_line(index.row());
    }

    bool canFetchMore(const QModelIndex &) const override
    {
        return m_loadedCount < m_totalCount;
    }

    void fetchMore(const QModelIndex &) override
    {
        int batchSize = 50;
        int remainder = m_totalCount - m_loadedCount;
        int toFetch = qMin(batchSize, remainder);

        beginInsertRows(QModelIndex(), m_loadedCount, m_loadedCount + toFetch - 1);
        m_loadedCount += toFetch;
        endInsertRows();
    }

private:
    int m_totalCount = 100000;    // 总数据量
    int m_loadedCount = 0;        // 已加载数量

    QString fetch_log_line(int row) const
    {
        // 从文件或数据库按行号读取，不全部加载到内存
        // 实际实现通常用文件 seek + 缓存
        return QString("Log entry #%1").arg(row);
    }
};
```

这个模式的核心思想是：rowCount 返回的是"已加载"的行数，不是总行数。视图通过 canFetchMore 知道还有更多数据，通过 fetchMore 一批一批地加载。每次 fetchMore 调用 beginInsertRows/endInsertRows 通知视图新增了哪些行。视图只在需要显示时调用 data 获取具体内容。

这里有一个非常重要的性能细节：data 方法必须是 O(1) 的。如果你在 data 里面做了文件读取或者数据库查询而没有缓存，快速滚动时视图会疯狂调用 data，性能会直接崩掉。常见做法是把已加载的数据缓存到一个 QVector 中，fetchMore 时把新数据批量读入缓存，data 直接从缓存返回。

### 3.4 setViewport——自定义视口控件

QAbstractScrollArea 提供了 `setViewport(QWidget *)` 方法，允许你用自定义的 QWidget 替换默认的视口控件。这个功能很少用到，但在某些特殊场景下非常有用——比如你需要一个带 OpenGL 渲染能力的视口（用 QOpenGLWidget 替换默认 QWidget），或者你需要一个自带双缓冲的自定义绘制控件作为视口。

```cpp
class GLViewport : public QOpenGLWidget
{
    // 自定义 OpenGL 渲染视口
    // ...
};

class GLItemView : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit GLItemView(QWidget *parent = nullptr)
        : QAbstractItemView(parent)
    {
        // 用 OpenGL 视口替换默认的 QWidget 视口
        setViewport(new GLViewport(this));
    }
};
```

使用 setViewport 时有几个注意事项。第一，setViewport 会接管旧视口的子控件和布局，旧视口会被删除。所以 setViewport 只应该在构造函数中调用一次，不要反复调用。第二，新视口的 parent 会被自动设置为 QAbstractScrollArea，你不需要手动设置 parent。第三，新视口的大小由 QAbstractScrollArea 自动管理，不要手动 resize 它。第四，你的 paintEvent 应该在 viewport() 上绘制，而不是在 this 上——这一点和入门篇讲的一样，但用了自定义视口后更容易忘记。

## 4. 踩坑预防

第一个坑是持久编辑器在行被移除后没有被正确清理。如果你用 openPersistentEditor 打开了某一行的编辑器，然后调用 removeRow 删除了那行，视图的默认行为会关闭并销毁该行的持久编辑器。但如果你用的是 beginResetModel/endResetModel 来刷新整个模型，所有持久编辑器都会被关闭——你在 reset 之前没有记录哪些行有持久编辑器，reset 之后就需要重新遍历并打开。最佳实践是在模型数据变化后重新调用一次 openPersistentEditor 来确保编辑器状态正确。

第二个坑是 dragDropMode 和 defaultDropAction 的混淆。dragDropMode 设置的是拖拽模式（是否允许拖/放/内部移动），defaultDropAction 设置的是拖拽操作的动作类型（移动/复制/链接）。很多朋友把 defaultDropAction 设成了 Qt::MoveAction 但 dragDropMode 还是 NoDragDrop，然后奇怪为什么拖拽不生效。记住：dragDropMode 是总开关，defaultDropAction 是模式为 DragDrop 时的子选项。如果你用 InternalMove 模式，不需要设置 defaultDropAction——InternalMove 自动使用 MoveAction。

第三个坑是 scrollTo 在大模型下配合 ScrollHint 不生效。scrollTo 的 ScrollHint 参数有 PositionAtTop、PositionAtCenter、PositionAtBottom 和 EnsureVisible 四个值。在大模型下（特别是用了 canFetchMore/fetchMore 的虚拟列表），如果目标行还没有被加载（rowCount 还没到那一行），scrollTo 会失败——它无法滚动到一个不存在的行。解决方案是在 scrollTo 之前先 fetchMore 直到目标行被加载，或者在 scrollTo 中加入判断：如果目标 row >= rowCount，先循环调用 fetchMore。

## 5. 练习项目

练习项目：可拖拽排序的虚拟日志查看器。窗口中央是一个 QListView，绑定一个自定义的 VirtualLogModel，模型从文件按需加载日志条目（模拟即可，不需要真读文件）。列表支持 InternalMove 拖拽排序——用户可以拖动日志条目重新排列。列表中每条日志的左侧显示时间戳，右侧显示一个彩色标签（通过委托的 paint 方法绘制）。窗口底部有一个"跳转到第 N 条"的输入框和按钮，点击后 scrollTo 到指定行，使用 PositionAtCenter 滚动提示。

完成标准是加载 10000 条数据时界面启动不超过 500ms，快速滚动时无明显卡顿，拖拽排序后数据正确更新。提示几个关键点：VirtualLogModel 用 canFetchMore/fetchMore 分批加载，data 方法从缓存中直接返回；拖拽排序需要模型实现 dropMimeData；scrollTo 之前检查目标行是否已加载，未加载则先 fetchMore。

## 6. 官方文档参考链接

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- 视图基类，setDragDropMode/openPersistentEditor/scrollTo 接口

[Qt 文档 · QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) -- 模型基类，canFetchMore/fetchMore/dropMimeData 接口

[Qt 文档 · QListView](https://doc.qt.io/qt-6/qlistview.html) -- 列表视图，支持虚拟列表和拖拽排序

[Qt 文档 · Drag and Drop](https://doc.qt.io/qt-6/dnd.html) -- Qt 拖放机制概述

---

到这里，QAbstractItemView 的进阶内容就搞定了。拖拽排序的四个属性配置搞清楚后就不会再对着一个拖不动的列表发呆，持久编辑器的生命周期管理知道了就不会再泄漏编辑器控件，虚拟列表的 canFetchMore/fetchMore 机制是实现大数据量高性能列表的关键，而 setViewport 给了你替换视口底层实现的能力。这些知识点在做一个交互复杂的列表或表格控件时会反复出现，值得多练几遍。
