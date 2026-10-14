---
title: "3.15 QAbstractItemView 基类进阶"
description: "入门篇把选择模式、委托机制、currentIndex/selectedIndexes 过了一遍，进阶篇看交互密集场景要用的四件事。InternalMove 拖拽排序的属性怎么配、移动靠模型哪三个方法完成，openPersistentEditor 开出的编辑器生命周期由谁管，canFetchMore/fetchMore 的虚拟列表怎么按需加载，最后 setViewport 把默认视口整个换掉，配套示例在 examples/advanced/03-qtwidgets/15-qabstractitemview-base-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.15——QAbstractItemView 基类进阶

[入门篇 QAbstractItemView 基础](../../beginner/03-qtwidgets/15-qabstractitemview-base-beginner.md) 把 setModel、选择模式、currentIndex/selectedIndexes、setItemDelegate 这些基础接口过了一遍，做一个静态展示的列表，入门篇的内容够用。控件基类部分走到第五篇，咱们看 QListView、QTableView、QTreeView 共同的基类 QAbstractItemView，它继承自上一篇的 QAbstractScrollArea，列表、树、表格的滚动正是从那里继承来的。咱们平时用的列表、表格、树视图都是它的子类，本篇讲的接口，在哪个子类身上都能直接调。可一旦做交互密集的界面，比如能拖拽排序的任务列表、每行嵌着进度条的下载管理器、上万条数据的日志面板，入门篇的接口就不够用了。

咱们把最容易把人问住的几个场面摆出来。视图上配好了 InternalMove，条目却拖不动。openPersistentEditor 开出来的编辑器不知道什么时候被销毁，指针拿在手里心里没底。十万条数据直接装进模型，内存和启动时间都吃不消。场面各不相同，病根也各不相同，本篇把四件事讲明白：InternalMove 的拖拽排序属性怎么配、移动靠模型哪些方法完成，持久编辑器的生命周期由谁管，虚拟列表怎么让视图只加载看得见的行，最后看 setViewport 怎么把默认视口换成 OpenGL。

## InternalMove 拖拽排序：框架在视图手里，数据搬运在模型手里

QAbstractItemView 内置了一套完整的拖拽框架，咱们不必自己处理 QDrag 和 dropEvent，把属性配好，视图就能自动处理拖拽排序。要配的属性有四个，setDragDropMode、setDefaultDropAction、setDragEnabled、setAcceptDrops。setDragDropMode 定方向，取值列成表：

| 取值 | 允许的方向 | 适合的场景 |
| -------- | -------- | -------- |
| NoDragDrop | 拖入拖出都不允许 | 默认值，禁用拖拽 |
| DragOnly | 只允许拖出 | 从列表拖到别处 |
| DropOnly | 只允许拖入 | 从别处拖进列表 |
| DragDrop | 拖入拖出都允许 | 与别的视图互拖，动作由 defaultDropAction 决定 |
| InternalMove | 视图内部移动 | 拖拽排序最常用 |

```cpp
auto *listView = new QListView();
listView->setModel(model);
listView->setDragEnabled(true);           // 允许拖拽
listView->setAcceptDrops(true);           // 允许接收拖放
listView->setDragDropMode(QAbstractItemView::InternalMove);
listView->setDefaultDropAction(Qt::MoveAction);
```

四行一起写，看着像谁都不能少，其实分工有主次。笔者翻了 qabstractitemview.cpp 的 setDragDropMode 实现，传 InternalMove 进去，setDragEnabled(true) 和 setAcceptDrops(true) 是它顺手带开的，两行显式调用写出来，是给自己和后来读代码的人看的。视图这一侧配的只是框架，数据搬运全在模型手里。InternalMove 的移动靠模型的 `mimeData`、`dropMimeData`、`removeRows` 三个方法协同完成：拖出去时 startDrag 调 mimeData 把选中项打包成 QMimeData，放下时 dropMimeData 把数据写回目标位置，拖放结束 drag->exec 返回 MoveAction，视图再走内部的 clearOrRemove 调 removeRows 把旧行删掉。QStandardItemModel 的三个方法都有默认实现，直接可用。换成自定义模型，mimeTypes()、mimeData()、dropMimeData()、supportedDragActions() 就得自己实现，缺了拖拽不会生效。咱们在 QListView 上配好 InternalMove 却发现拖拽没反应，用的又是自定义模型，头一个要查的就是上面几个方法。

另一个容易搅浑的点是 setDefaultDropAction 和 setDragDropMode 的关系。setDefaultDropAction 设置拖拽落下时默认的动作类型，Qt::MoveAction、Qt::CopyAction 或 Qt::LinkAction，它只在 DragDrop 模式下起作用。InternalMove 模式自动使用 MoveAction，不必额外设置，用了 DragDrop 模式，setDefaultDropAction(Qt::MoveAction) 才是必要的。dragDropMode 是总开关，defaultDropAction 是 DragDrop 模式下的子选项。咱们要是把 defaultDropAction 设成了 Qt::MoveAction，dragDropMode 却还留在 NoDragDrop，拖拽不会生效，开关压根没打开，子选项设得再对也没有用。

配套示例把拖拽做成了能跑的工程，文件在 examples/advanced/03-qtwidgets/15-qabstractitemview-base-advanced/，可执行文件 15-qabstractitemview-base-advanced。窗口上半部分是 DragDropListView，drag_drop_list_view.cpp 的构造函数里，模型填了 10 个 Task 条目，每个条目的 flags 显式补上 Qt::ItemIsDragEnabled 和 Qt::ItemIsDropEnabled，拖与放的许可同样记在条目的 flags 上。四行属性之后还有一句 setDropIndicatorShown(true)，拖动时插入位置会画出一条指示线。咱们跑一个实验，把 setDragEnabled(true) 和 setAcceptDrops(true) 两行注释掉，重新编译，拖拽照常工作，因为 InternalMove 已经把两个开关带开了。看完把注释去掉，恢复原状。

## 持久编辑器：进度条常驻单元格，生命周期交给视图

默认情况下，视图里的编辑器是临时的，用户双击单元格，编辑器弹出，编辑完成或取消后关闭销毁。有些场景要的偏偏是常驻，进度条、复选框、颜色选择器得一直显示在某个单元格上，这时用 `openPersistentEditor(const QModelIndex &index)`。视图收到调用，通过委托的 createEditor 为指定索引创建一个编辑器，之后不会在编辑完成时自动关闭，编辑器会一直存在，直到咱们调用 closePersistentEditor，或者索引对应的行被移除。

```cpp
// 给某一行的第 0 列打开持久进度条编辑器
for (int row = 0; row < model->rowCount(); ++row) {
    QModelIndex progressIndex = model->index(row, 0);
    listView->openPersistentEditor(progressIndex);
}
```

持久编辑器的数据同步由视图自动处理，模型数据变化时，视图调用委托的 setEditorData 更新编辑器的显示。反方向，用户在编辑器里改了值，编辑器要把新值送回模型，标准控件如 QProgressBar、QCheckBox，Qt 内部的委托机制会处理同步。自定义编辑器就得自己动手了，咱们在编辑器值变化的信号里手动调用 model->setData(index, newValue)。委托的 createEditor、setEditorData、setModelData 各自承担什么，[3.3 Model/View 进阶](./03-model-view-advanced.md) 讲过分工，本篇只看视图一侧的协作。

### 一道调试题：removeRow 之后，那一行的持久编辑器去哪了

咱们看一段代码，先自己想一遍再往下读：

```cpp
// 删除模型中的第 3 行
model->removeRow(3);
// 此时第 3 行的持久编辑器会怎样？
```

答案是，第 3 行的持久编辑器被关闭并销毁，后面行的持久编辑器不用咱们操心，视图内部拿 QPersistentModelIndex 记着每个编辑器的位置，行一删，索引自动偏移，原来的第 4 行变成第 3 行，编辑器跟着挪。被删的那行走的是销毁，rowsAboutToBeRemoved 在行删掉之前就把受影响行的编辑器清了出去，销毁调用委托的 destroyEditor，默认实现是 deleteLater。所以咱们要是把某个持久编辑器的裸指针存了下来，removeRow 之后它就是悬空的，永远不要持有持久编辑器的裸指针，需要时通过委托获取。

比 removeRow 更彻底的是整模型刷新。beginResetModel 和 endResetModel 一对调用发出去，视图的 reset() 会把所有编辑器，持久的也在内，全部释放。persistent 集合，也就是视图内部登记持久编辑器的那份名单，也一并清空。咱们在 reset 之前没记下哪些行开着编辑器，reset 之后就得重新遍历一遍 openPersistentEditor。做带常驻控件的列表，刷新模型之后重开一遍编辑器，是稳妥的做法。

示例的下半部分是下载面板，演示的就是常驻进度条。main.cpp 里 6 个文件条目各带一个初始进度，PersistentEditorDelegate 装在列表上，createEditor 返回一个 0 到 100 的 QProgressBar，setEditorData 从 DisplayRole 读值设给进度条，setModelData 把进度条的 value 以 EditRole 写回，updateEditorGeometry 让进度条正好盖住单元格。循环给每一行 openPersistentEditor 之后，进度条全部常驻。咱们点 Simulate Download Progress 按钮，槽函数挑一个没到 100 的任务，model->setData 把它推进 5 到 20。setData 一调用，dataChanged 跟着发出去，视图自动调 setEditorData，对应的进度条往前走。模型到编辑器方向的同步，按钮一点就能看见。想亲眼看行删除的偏移，在按钮的槽里加一行 model->removeRow(0)，重新编译再点，头一行的进度条连着条目一起消失，后面的进度条整体上移一行，就是索引自动重映射的样子。看完把新加的一行删掉。

## 虚拟列表：rowCount 报已加载的行数，滚到底再要一批

模型里躺着上万条甚至上百万条数据时，咱们把全部数据装进 QStandardItemModel 走不通，内存消耗太大，初始化也慢。QListView 配合自定义模型可以做虚拟列表，模型的 rowCount 只报已加载的行数，视图滚动到已加载部分的底部，再向模型要下一批，数据只在实际用到时才进内存。

实现虚拟列表要靠 QAbstractItemModel 的两个虚函数，canFetchMore(const QModelIndex &parent) 和 fetchMore(const QModelIndex &parent)。canFetchMore 返回 true 表示模型还有更多数据可以加载，fetchMore 负责加载一批新数据。视图一侧的配合，咱们在 qabstractitemview.cpp 里能看到两条路。一条由滚动条触发，垂直滚动条的值一到 maximum，视图就检查 canFetchMore，返回 true 就直接调 model 的 fetchMore，马上把下一批补进来。另一条挂在定时器上，视图重算几何时，updateGeometries 会启动一个零延迟的 fetchMore 定时器，到点先检查 canFetchMore，再看最后一行的 visualRect 是否已经进入视口，进来了才真的加载，避免提前把用不着的数据拉进来。

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

咱们把机制串一遍。rowCount 返回的是已加载的行数，不是总行数，虚拟列表的其余机制全部由此展开。canFetchMore 让视图知道底下还有数据，fetchMore 一次补一批，每次用 beginInsertRows/endInsertRows 把新增行的范围通知视图，视图只在需要显示时调用 data 获取具体内容。模型虚函数的完整分工和变更通知的调用约定，[3.3 Model/View 进阶](./03-model-view-advanced.md) 讲得全，本篇看的是视图怎么消费。

data 方法必须做到 O(1)，这是虚拟列表的性能底线。咱们要是把文件读取或数据库查询藏在 data 里而没有缓存，快速滚动时视图会疯狂调用 data，性能直接崩掉。常见做法是把已加载的数据缓存到一个 QVector 中，fetchMore 时批量读入缓存，data 直接从缓存返回。

虚拟列表配 scrollTo，咱们要记住一个前提，目标行得先在模型里。scrollTo 的 ScrollHint 参数有四个值：

| ScrollHint | 滚动后的位置 |
| -------- | -------- |
| PositionAtTop | 目标行贴视口顶部 |
| PositionAtCenter | 目标行居中 |
| PositionAtBottom | 目标行贴视口底部 |
| EnsureVisible | 仅当目标行不可见时才滚动 |

在大模型下，特别是用了 canFetchMore/fetchMore 的虚拟列表，目标行还没被加载时（rowCount 还没到那一行），scrollTo 会失败，它无法滚动到一个不存在的行。咱们要么在 scrollTo 之前循环调用 fetchMore，直到目标行被加载，要么封一个跳转函数，目标 row 大于等于 rowCount 时先补数据再滚动。

## setViewport：默认视口可以整个换掉

QAbstractScrollArea 提供了 `setViewport(QWidget *)`，允许咱们用自定义的 QWidget 替换默认的视口控件。用到的地方不多，特殊场景下正好合适，需要一个带 OpenGL 渲染能力的视口时，用 QOpenGLWidget 换掉默认 QWidget，需要一个自带双缓冲的自绘控件当视口时，也从这里换。

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

使用 setViewport 时有几件事要您当心。setViewport 适合在构造函数中调用一次，不要反复调用，旧视口是直接删掉的，换来换去只是白折腾。新视口的 parent 会被自动设置为 QAbstractScrollArea，不需要咱们手动设置。新视口的大小由 QAbstractScrollArea 自动管理，不要手动 resize 它。绘制要落在 viewport() 上，QPainter 开在 viewport() 而不是 this 上，换了自定义视口之后更容易忘。为什么是 viewport() 而不是 this，坐标系与滚动偏移怎么换算，[3.14 QAbstractScrollArea 基类进阶](./14-qabstractscrollarea-base-advanced.md) 里讲过完整的过程。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QAbstractItemView 在 QtWidgets 模块，拖拽部分涉及 QDrag/QMimeData，持久编辑器部分涉及委托的 createEditor/destroyEditor，虚拟列表部分涉及 QAbstractItemModel 的 canFetchMore/fetchMore 机制，所有内容在桌面平台上行为一致。

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- 视图基类，setDragDropMode/openPersistentEditor/scrollTo 接口

[Qt 文档 · QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html) -- 模型基类，canFetchMore/fetchMore/dropMimeData 接口

[Qt 文档 · QListView](https://doc.qt.io/qt-6/qlistview.html) -- 列表视图，支持虚拟列表和拖拽排序

[Qt 文档 · Drag and Drop](https://doc.qt.io/qt-6/dnd.html) -- Qt 拖放机制概述

---

视图管框架，模型管数据，本篇的几件事都在两边的配合上。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/15-qabstractitemview-base-advanced/，可执行文件 15-qabstractitemview-base-advanced）完整玩一轮，上半区把 Task 条目拖来拖去，看指示线落在哪、松手后顺序怎么变，下半区连点 Simulate Download Progress，看进度条怎么跟着 dataChanged 一次一次往前走。下一篇 [3.16 QAbstractSpinBox 基类进阶](./16-qabstractspinbox-base-advanced.md) 讲 QSpinBox、QDoubleSpinBox、QDateTimeEdit 的共同基类，微调框的步进、上下按钮与键盘交互都由它统管。
