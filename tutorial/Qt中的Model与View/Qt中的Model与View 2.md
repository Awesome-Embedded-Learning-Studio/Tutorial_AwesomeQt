# Qt中的Model与View 2

​	您可以看完概念后看一下笔者翻译的[Model/View Programming | Qt Widgets 6.8.0](https://doc.qt.io/qt-6/model-view-programming.html)，文章地址在：[Qt中的Model与View（Qt官网翻译版）-CSDN博客](https://blog.csdn.net/charlie114514191/article/details/143460640)

## 让我们消化一下概念...

​	[Model/View Programming | Qt Widgets 6.8.0](https://doc.qt.io/qt-6/model-view-programming.html)中已经详细的介绍了Qt框架下的Model View是如何工作的，我们进一步按照应用的方式仔细学习：

1. 我们说，源数据就是我们的想要展示的数据，直接扔上去是不好看的（没人做好看的界面扔原始数据！）
2. 视图是我们看到的样子，我们看到的数据的展示形式就是View来进行控制
3. 模型是原始数据的格式化封装，这是为了跟视图进行配合
4. 代理是一种编辑器，当我们想要跟数据进行交互编辑的时候，就是Delegate准备发力！举个例子：我们在TableWidget中双击编辑数据的时候，Delegate会生成一个LineEdit供我们编辑，这就是代理的作用！

​	如此，我们基本上把Qt MV模式说完了基本的组成要素，我们下面为了更加详细的描述，必须先引出一些概念：

1. 模型索引：ModelIndex。我们直到想要拿到一个Model下的数据，因为是根据View进行配合，我们按照View拿到数据的时候就是依照模型的索引：在哪里？行列如何？我们就是这样发问指挥程序拿到我们的Index，把里头的数据取出来，或者控制这个Model的行为。当然正如我所说：**按照View拿到数据**就意味着只会在当前显示的样子有效，如果我们拿到这个索引后改变了这个视图，我们不确定按照之前的方法还能不能再次索引到之前的Model了！

2. 行号列号：上面我说的在哪里，这个就是行号和列的事情

3. 父项：父对象！Qt中的父对象是怎么个回事，这里就是怎么个回事！一般在我们使用树形控件访问Index的时候就需要使用父项这个概念找到一个节点下的子节点的位置。

4. 项的角色：项也是有角色的！我们看看Qt提供了哪些Role:

   | 常量                          | 值     | 角色数据类型           | 描述                                                   |
   | ----------------------------- | ------ | ---------------------- | ------------------------------------------------------ |
   | Qt::DisplayRole               | 0      | QString                | 用于渲染为文本的关键数据。                             |
   | Qt::DecorationRole            | 1      | QColor, QIcon, QPixmap | 用于渲染为装饰（图标）的数据。                         |
   | Qt::EditRole                  | 2      | QString                | 适合在编辑器中编辑的数据格式。                         |
   | Qt::ToolTipRole               | 3      | QString                | 在项的工具提示中显示的数据。                           |
   | Qt::StatusTipRole             | 4      | QString                | 在状态栏中显示的数据。                                 |
   | Qt::WhatsThisRole             | 5      | QString                | 在“这是什么？”模式下为项目显示的数据。                 |
   | Qt::SizeHintRole              | 13     | QSize                  | 将提供给视图的项的大小提示。                           |
   | Qt::FontRole                  | 6      | QFont                  | 用于使用默认代理呈现的项的字体。                       |
   | Qt::TextAlignmentRole         | 7      | Qt::Alignment          | 用于使用默认代理呈现的项的文本对齐方式。               |
   | Qt::BackgroundRole            | 8      | QBrush                 | 用于使用默认代理呈现的项的背景画刷。                   |
   | Qt::ForegroundRole            | 9      | QBrush                 | 用于使用默认代理呈现的项的前景画刷（文本颜色，通常）。 |
   | Qt::CheckStateRole            | 10     | Qt::CheckState         | 用于获取项的选中状态。                                 |
   | Qt::InitialSortOrderRole      | 14     | Qt::SortOrder          | 用于获取表头视图部分的初始排序顺序。                   |
   | Qt::AccessibleTextRole        | 11     | QString                | 供辅助功能扩展和插件（如屏幕阅读器）使用的文本。       |
   | Qt::AccessibleDescriptionRole | 12     | QString                | 项的描述，用于辅助功能目的。                           |
   | Qt::UserRole                  | 0x0100 | 依据应用需求而定       | 第一个可以用于应用程序特定用途的角色。                 |

​	我们访问数据，有的时候关心的是一个角色的数据（请注意，搞Qt的MV编程的时候，你需要的思考方式是：我想要XX角色的数据，所以我按照XX角色访问数据）

​	在上一篇博客中，在最后给出了一些Qt的类，这里我们进一步给出参考和指引：

## QAbstractItemModel

​	所有Model的爹！规定了我们的Model的接口规范是什么，以便能够与模型/视图架构中的其他组件进行互操作。我们需要从设计的层次来观察这个Model是如何的！

​	如果需要一个与项目视图（例如 QML 的列表视图元素或 C++ 小部件 QListView 或 QTableView）一起使用的模型，我们应该考虑子类化 QAbstractListModel 或 QAbstractTableModel 而不是此类。底层数据模型以表格层次结构的形式向视图和委托公开。如果我们不使用层次结构，则模型是一个简单的行和列表。每个项目都有一个由 QModelIndex 指定的唯一索引。

​	我们可以通过模型访问的每个数据项都有一个关联的模型索引。我们可以使用 index() 函数获取此模型索引。每个索引可能有一个 brothers() 索引；子项有一个 parent() 索引。每个项都有许多与之关联的数据元素，可以通过为模型的 data() 函数指定角色（请参阅 Qt::ItemDataRole）来检索它们。可以使用 itemData() 函数同时获取所有可用角色的数据。每个角色的数据都使用特定的 Qt::ItemDataRole 设置。各个角色的数据使用 setData() 单独设置，也可以使用 setItemData() 为所有角色设置。可以使用 flags()（请参阅 Qt::ItemFlag）查询项，以查看是否可以选择、拖动或以其他方式操作它们。

​	如果某项有子对象，hasChildren() 将为相应索引返回 true。该模型对层次结构的每个级别都有一个 rowCount() 和一个 columnCount()。可以使用 insertRows()、insertColumns()、removeRows() 和 removeColumns() 插入和删除行和列。

​	模型发出信号以指示更改。例如，每当模型提供的数据项发生更改时，都会发出 dataChanged()。对模型提供的标题的更改会导致发出 headerDataChanged()。

​	如果基础数据的结构发生变化，模型可以发出 layoutChanged() 以指示任何附加视图，它们应该重新显示显示的任何项目，并考虑新结构。可以使用 match() 函数搜索模型中可用的项目以查找特定数据。要对模型进行排序，可以使用 sort()。一般是升序的！

### 子类化

注意：模型子类化参考中提供了一些有关子类化模型的一般准则。

​	在对 QAbstractItemModel 进行子类化时，至少必须实现 index()、parent()、rowCount()、columnCount() 和 data()。这些函数用于所有只读模型，并构成可编辑模型的基础。

​	我们还可以重新实现 hasChildren() 来为 rowCount() 实现成本高昂的模型提供特殊行为。这使模型可以限制视图请求的数据量，并可用作实现模型数据惰性填充的一种方式。要在模型中启用编辑，我们还必须实现 setData()，并重新实现 flags() 以确保返回 ItemIsEditable。我们还可以重新实现 headerData() 和 setHeaderData() 来控制模型标题的呈现方式。

​	重新实现 setData() 和 setHeaderData() 函数时，必须分别明确发出 dataChanged() 和 headerDataChanged() 信号。

​	自定义模型需要创建模型索引以供其他组件使用。为此，请使用项目的合适行号和列号以及它的标识符（指针或整数值）调用 createIndex()。这些值的组合对于每个项目必须是唯一的。自定义模型通常在其他重新实现的函数中使用这些唯一标识符来检索项目数据并访问有关该项目的父级和子级的信息。有关唯一标识符的更多信息，请参阅简单树模型示例。

​	**没有必要支持 Qt::ItemDataRole 中定义的每个角色。**根据模型中包含的数据类型，实现 data() 函数可能只对一些更常见的角色返回有效信息有用。大多数模型至少为 Qt::DisplayRole 提供项目数据的文本表示，行为良好的模型还应为 Qt::ToolTipRole 和 Qt::WhatsThisRole 提供有效信息。支持这些角色使模型可以与标准 Qt 视图一起使用。但是，对于处理高度专业化数据的某些模型，仅为用户定义的角色提供数据可能是合适的。

​	提供可调整大小数据结构接口的模型可以提供 insertRows()、removeRows()、insertColumns() 和 removeColumns 的实现()。在实现这些函数时，重要的是在模型维度发生更改之前和之后通知所有连接的视图：

- insertRows() 实现必须在将新行插入数据结构之前调用 beginInsertRows()，并在之后立即调用 endInsertRows()。

- insertColumns() 实现必须在将新列插入数据结构之前调用 beginInsertColumns()，并在之后立即调用 endInsertColumns()。
- removeRows() 实现必须在从数据结构中删除行之前调用 beginRemoveRows()，并在之后立即调用 endRemoveRows()。
- removeColumns() 实现必须在从数据结构中删除列之前调用 beginRemoveColumns()，并在之后立即调用 endRemoveColumns()。

​	这些函数发出的私有信号使附加组件有机会在任何数据变得不可用之前采取行动。使用这些 begin 和 end 函数封装插入和删除操作还使模型能够正确管理持久模型索引。如果我们希望正确处理选择，则必须确保调用这些函数。如果我们插入或删除带有子项的项目，则无需为子项调用这些函数。换句话说，父项将负责其子项。

​	要创建增量填充的模型，我们可以重新实现 fetchMore() 和 canFetchMore()。如果 fetchMore() 的重新实现向模型添加了行，则必须调用 beginInsertRows() 和 endInsertRows()。

### 线程安全

作为 QObject 的子类，QAbstractItemModel 不是线程安全的。任何与 QAbstractItemModel 模型相关的 API 都应仅从模型对象所在的线程调用。如果 QAbstractItemModel 与视图相连，则意味着 GUI 线程，因为视图就位于该线程，它将从 GUI 线程调用模型。使用后台线程填充或修改模型的内容是可能的，但需要小心，因为后台线程不能直接调用任何与模型相关的 API。相反，我们应该将**更新排队并在主线程中应用它们。**这可以通过QueueConnnection来完成。

### 如果你喜欢看表格总结...

#### 基本的信息

| 项目             | 内容                                                         |
| ---------------- | ------------------------------------------------------------ |
| **类定义**       | `QAbstractItemModel` 定义了项模型与模型/视图架构中其他组件互操作所需的标准接口。该类不应直接实例化，应该通过子类化来创建新的模型。 |
| **类别**         | 属于 Qt 的模型/视图框架，是模型/视图类之一。可以作为 QML 的项视图元素或 Qt Widgets 模块中的项视图类的底层数据模型。 |
| **建议子类**     | 对于使用项视图（如 QML 的 List View 或 C++ 的 QListView、QTableView）的模型，建议考虑子类化 `QAbstractListModel` 或 `QAbstractTableModel`。 |
| **数据模型结构** | 模型以表格层次结构的形式暴露给视图和委托。如果不使用层次结构，则模型是一个简单的行列表。每个项通过唯一的索引 `QModelIndex` 来指定。 |
| **索引获取**     | 每个可通过模型访问的数据项都有一个关联的模型索引，可以使用 `index()` 函数获得。每个索引可能有一个兄弟索引；子项有一个父索引。 |
| **数据访问**     | 每个项有多个数据元素，使用角色（参见 `Qt::ItemDataRole`）来指定模型的 `data()` 函数获取数据。可通过 `itemData()` 函数同时获取所有可用角色的数据。 |
| **数据设置**     | 使用特定的 `Qt::ItemDataRole` 设置每个角色的数据。可以使用 `setData()` 单独设置每个角色，或使用 `setItemData()` 为所有角色设置数据。 |
| **项标志查询**   | 可以通过 `flags()` 查询项的标志（参见 `Qt::ItemFlag`），了解它们是否可以被选择、拖动或以其他方式操作。 |
| **子项检查**     | 如果项有子对象，`hasChildren()` 返回对应索引的 true。        |
| **行列计数**     | 模型有 `rowCount()` 和 `columnCount()` 用于每个层级的计数。行和列可以通过 `insertRows()`, `insertColumns()`, `removeRows()` 和 `removeColumns()` 来插入和移除。 |
| **信号发射**     | 模型发出信号以指示更改。例如，`dataChanged()` 在数据项更改时发出，`headerDataChanged()` 在头部更改时发出，`layoutChanged()` 在底层数据结构更改时发出。 |
| **数据搜索**     | 可使用 `match()` 函数搜索模型中的特定数据。                  |
| **排序**         | 可使用 `sort()` 对模型进行排序。                             |

#### 子类化指南

| 项目             | 内容                                                         |
| ---------------- | ------------------------------------------------------------ |
| **基本实现**     | 子类化 `QAbstractItemModel` 时，至少需实现 `index()`, `parent()`, `rowCount()`, `columnCount()` 和 `data()`。这些函数用于所有只读模型，并构成可编辑模型的基础。 |
| **特殊行为实现** | 可以重新实现 `hasChildren()` 提供特殊行为，特别是当 `rowCount()` 的实现昂贵时。这允许模型限制视图请求的数据量，可以实现延迟填充模型数据。 |
| **编辑功能**     | 若要启用编辑，必须实现 `setData()`，并重新实现 `flags()` 返回 `ItemIsEditable`。可重新实现 `headerData()` 和 `setHeaderData()` 控制模型头部的展示。 |
| **信号发射**     | 在重新实现 `setData()` 和 `setHeaderData()` 时，必须显式发射 `dataChanged()` 和 `headerDataChanged()` 信号。 |
| **模型索引创建** | 自定义模型需要创建模型索引供其他组件使用，使用 `createIndex()` 调用，提供合适的行、列号和唯一标识符（指针或整数值）。组合值必须对每个项唯一。 |
| **角色支持**     | 并非必须支持 `Qt::ItemDataRole` 中定义的每个角色。根据模型包含的数据类型，可能只需实现 `data()` 返回常用角色的有效信息。大多数模型至少为 `Qt::DisplayRole` 提供文本表示。 |
| **增量填充**     | 可重新实现 `fetchMore()` 和 `canFetchMore()` 来创建增量填充的模型。如果 `fetchMore()` 添加行，必须调用 `beginInsertRows()` 和 `endInsertRows()`。 |
| **线程安全**     | `QAbstractItemModel` 作为 `QObject` 的子类，不是线程安全的。所有与模型相关的 API 应仅在模型对象所在的线程中调用。使用后台线程填充或修改模型内容时，需小心处理，不能直接调用任何模型相关的 API，而是应排队更新并在主线程中应用。 |

### API

| 函数签名                                                     | 描述                                   |
| ------------------------------------------------------------ | -------------------------------------- |
| QAbstractItemModel(QObject *parent = nullptr)                | 构造函数                               |
| virtual ~QAbstractItemModel()                                | 析构函数                               |
| virtual QModelIndex buddy(const QModelIndex &index) const    | 获取与给定索引相关联的索引。           |
| virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const | 检查是否可以放置 MIME 数据。           |
| virtual bool canFetchMore(const QModelIndex &parent) const   | 检查是否可以获取更多数据。             |
| bool checkIndex(const QModelIndex &index, QAbstractItemModel::CheckIndexOptions options = CheckIndexOption::NoOption) const | 检查索引的有效性。                     |
| virtual bool clearItemData(const QModelIndex &index) (since 6.0) | 清除指定索引的数据。                   |
| virtual int columnCount(const QModelIndex &parent = QModelIndex()) const = 0 | 获取指定父项的列数。                   |
| virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const = 0 | 获取指定索引的数据。                   |
| virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) | 放置 MIME 数据。                       |
| virtual void fetchMore(const QModelIndex &parent)            | 获取更多数据。                         |
| virtual Qt::ItemFlags flags(const QModelIndex &index) const  | 获取指定索引的项标志。                 |
| virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const | 检查指定父项是否有子项。               |
| bool hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const | 检查指定索引是否有效。                 |
| virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const | 获取表头数据。                         |
| virtual QModelIndex index(int row, int column, const QModelIndex &parent) const = 0 | 获取指定行和列的索引。                 |
| bool insertColumn(int column, const QModelIndex &parent = QModelIndex()) | 插入一列。                             |
| virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) | 插入多列。                             |
| bool insertRow(int row, const QModelIndex &parent = QModelIndex()) | 插入一行。                             |
| virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) | 插入多行。                             |
| virtual QMap<int, QVariant> itemData(const QModelIndex &index) const | 获取指定索引的所有角色的数据。         |
| virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const                  |
| virtual QMimeData *mimeData(const QModelIndexList &indexes) const | 获取与指定索引列表相关联的 MIME 数据。 |
| virtual QStringList mimeTypes() const                        | 获取支持的 MIME 类型列表。             |
| bool moveColumn(const QModelIndex &sourceParent, int sourceColumn, const QModelIndex &destinationParent, int destinationChild) | 移动列。                               |
| virtual bool moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent, int destinationChild) | 移动多列。                             |
| bool moveRow(const QModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationChild) | 移动行。                               |
| virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) | 移动多行。                             |
| virtual void multiData(const QModelIndex &index, QModelRoleDataSpan roleDataSpan) const (since 6.0) | 获取多个角色的数据。                   |
| virtual QModelIndex parent(const QModelIndex &index) const = 0 | 获取指定索引的父索引。                 |
| bool removeColumn(int column, const QModelIndex &parent = QModelIndex()) | 移除一列。                             |
| virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) | 移除多列。                             |
| bool removeRow(int row, const QModelIndex &parent = QModelIndex()) | 移除一行。                             |
| virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) | 移除多行。                             |
| virtual QHash<int, QByteArray> roleNames() const             | 获取角色名称。                         |
| virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0 | 获取指定父项的行数。                   |
| virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) | 设置指定索引的数据。                   |
| virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) | 设置表头数据。                         |
| virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) | 设置指定索引的角色数据。               |
| virtual QModelIndex sibling(int row, int column, const QModelIndex &index) const | 获取指定索引的兄弟索引。               |
| virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) | 对指定列进行排序。                     |
| virtual QSize span(const QModelIndex &index) const           | 获取指定索引的跨越尺寸。               |
| virtual Qt::DropActions supportedDragActions() const         | 获取支持的拖放操作。                   |
| virtual Qt::DropActions supportedDropActions() const         | 获取支持的放置操作。                   |

| 信号名称                                                     | 描述                   |
| ------------------------------------------------------------ | ---------------------- |
| void columnsAboutToBeInserted(const QModelIndex &parent, int first, int last) | 列即将被插入的通知。   |
| void columnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationColumn) | 列即将被移动的通知。   |
| void columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last) | 列即将被移除的通知。   |
| void columnsInserted(const QModelIndex &parent, int first, int last) | 列已被插入的通知。     |
| void columnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationColumn) | 列已被移动的通知。     |
| void columnsRemoved(const QModelIndex &parent, int first, int last) | 列已被移除的通知。     |
| void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>()) | 数据已改变的通知。     |
| void headerDataChanged(Qt::Orientation orientation, int first, int last) | 表头数据已改变的通知。 |
| void layoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(), QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint) | 布局即将改变的通知。   |
| void layoutChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(), QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint) | 布局已改变的通知。     |
| void modelAboutToBeReset()                                   | 模型即将被重置的通知。 |
| void modelReset()                                            | 模型已重置的通知。     |
| void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end) | 行即将被插入的通知。   |
| void rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow) | 行即将被移动的通知。   |
| void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last) | 行即将被移除的通知。   |
| void rowsInserted(const QModelIndex &parent, int first, int last) | 行已被插入的通知。     |
| void rowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow) | 行已被移动的通知。     |
| void rowsRemoved(const QModelIndex &parent, int first, int last) | 行已被移除的通知。     |

| 函数签名                                                     | 描述                 |
| ------------------------------------------------------------ | -------------------- |
| void beginInsertColumns(const QModelIndex &parent, int first, int last) | 开始插入列的操作。   |
| void beginInsertRows(const QModelIndex &parent, int first, int last) | 开始插入行的操作。   |
| bool beginMoveColumns(const QModelIndex &sourceParent, int sourceFirst, int sourceLast, const QModelIndex &destinationParent, int destinationChild) | 开始移动列的操作。   |
| bool beginMoveRows(const QModelIndex &sourceParent, int sourceFirst, int sourceLast, const QModelIndex &destinationParent, int destinationChild) | 开始移动行的操作。   |
| void beginRemoveColumns(const QModelIndex &parent, int first, int last) | 开始移除列的操作。   |
| void beginRemoveRows(const QModelIndex &parent, int first, int last) | 开始移除行的操作。   |
| void beginResetModel()                                       | 开始重置模型的操作。 |
| void changePersistentIndex(const QModelIndex &from, const QModelIndex &to) | 更改持久索引。       |
| void changePersistentIndexList(const QModelIndexList &from, const QModelIndexList &to) | 更改持久索引列表。   |
| QModelIndex createIndex(int row, int column, const void *ptr = nullptr) const | 创建索引。           |
| QModelIndex createIndex(int row, int column, quintptr id) const | 创建索引。           |
| void endInsertColumns()                                      | 结束插入列的操作。   |
| void endInsertRows()                                         | 结束插入行的操作。   |
| void endMoveColumns()                                        | 结束移动列的操作。   |
| void endMoveRows()                                           | 结束移动行的操作。   |
| void endRemoveColumns()                                      | 结束移除列的操作。   |
| void endRemoveRows()                                         | 结束移除行的操作。   |
| void endResetModel()                                         | 结束重置模型的操作。 |
| QModelIndexList persistentIndexList() const                  | 获取持久索引列表。   |

| 函数签名                         | 描述           |
| -------------------------------- | -------------- |
| virtual void resetInternalData() | 重置内部数据。 |

## QAbstractItemView

​	这个也是所有View的爹，说白了，看这个就看懂了View要做什么它提供了一个标准接口，通过信号和槽机制与模型进行交互，使子类能够保持与其模型的同步。此类提供了对键盘和鼠标导航、视口滚动、项目编辑和选择的标准支持。

​	下面是默认的一些行为：

| 键                  | 功能描述                                           |
| ------------------- | -------------------------------------------------- |
| 箭头键              | 改变当前项目并选择它。                             |
| Ctrl + 箭头键       | 改变当前项目但不选择它。                           |
| Shift + 箭头键      | 改变当前项目并选择它，之前选中的项目不会取消选择。 |
| Ctrl + 空格         | 切换当前项目的选择状态。                           |
| Tab / Backtab       | 切换到下一个/上一个项目。                          |
| Home / End          | 选择模型中的第一个/最后一个项目。                  |
| Page Up / Page Down | 上下滚动视图中显示的行数。                         |
| Ctrl + A            | 选择模型中的所有项目。                             |

请注意，以上表格假设选择模式允许这些操作。**如果选择模式为 `QAbstractItemView::NoSelection`，则无法选择项目。**

​	`QAbstractItemView` 类是 Qt 的模型/视图框架的一部分。继承自 `QAbstractItemView` 的视图类只需要实现其特定功能，例如绘制项目、返回项目的几何形状、查找项目等。此类提供了公共槽，例如 `edit()` 和 `setCurrentIndex()`，还提供了许多受保护的槽，包括 `dataChanged()`、`rowsInserted()`、`rowsAboutToBeRemoved()`、`selectionChanged()` 和 `currentChanged()`。

​	根项目通过 `rootIndex()` 返回，当前项目通过 `currentIndex()` 获取。要确保项目可见，可以使用 `scrollTo()`。

​	与滚动相关的一些函数包括 `setHorizontalScrollMode()` 和 `setVerticalScrollMode()`。例如，可以重新实现视图的 `resizeEvent()` 函数来设置滚动条的范围：

```
void MyView::resizeEvent(QResizeEvent *event) {
    horizontalScrollBar()->setRange(0, realWidth - width());
    ...
}
```

​	请注意，**范围在小部件显示之前不会更新。**许多其他函数涉及选择控制，例如 `setSelectionMode()` 和 `setSelectionBehavior()`。此类提供了一个默认的选择模型（`selectionModel()`），但可以使用 `setSelectionModel()` 替换为 `QItemSelectionModel` 的实例。要完全控制项目的显示和编辑，可以使用 `setItemDelegate()` 指定一个委托

​	下面是一些常见的属性介绍：

#### QAbstractItemView::CursorAction

此枚举描述了在项目之间导航的不同方式：

| 常量                            | 值   | 描述                           |
| ------------------------------- | ---- | ------------------------------ |
| QAbstractItemView::MoveUp       | 0    | 移动到当前项目上方的项目。     |
| QAbstractItemView::MoveDown     | 1    | 移动到当前项目下方的项目。     |
| QAbstractItemView::MoveLeft     | 2    | 移动到当前项目左侧的项目。     |
| QAbstractItemView::MoveRight    | 3    | 移动到当前项目右侧的项目。     |
| QAbstractItemView::MoveHome     | 4    | 移动到左上角的项目。           |
| QAbstractItemView::MoveEnd      | 5    | 移动到右下角的项目。           |
| QAbstractItemView::MovePageUp   | 6    | 移动到当前项目上方一页的项目。 |
| QAbstractItemView::MovePageDown | 7    | 移动到当前项目下方一页的项目。 |
| QAbstractItemView::MoveNext     | 8    | 移动到当前项目后的项目。       |
| QAbstractItemView::MovePrevious | 9    | 移动到当前项目前的项目。       |

#### QAbstractItemView::DragDropMode

描述视图可以响应的各种拖放事件。默认情况下，视图不支持拖放（`NoDragDrop`）。

| 常量                            | 值   | 描述                                     |
| ------------------------------- | ---- | ---------------------------------------- |
| QAbstractItemView::NoDragDrop   | 0    | 不支持拖动或放置。                       |
| QAbstractItemView::DragOnly     | 1    | 视图支持拖动其自身的项目。               |
| QAbstractItemView::DropOnly     | 2    | 视图接受放置。                           |
| QAbstractItemView::DragDrop     | 3    | 视图同时支持拖动和放置。                 |
| QAbstractItemView::InternalMove | 4    | 视图仅接受来自自身的移动（不复制）操作。 |

请注意，使用的模型需要提供对拖放操作的支持。

#### QAbstractItemView::DropIndicatorPosition

指示当前鼠标位置的索引相关的放置指示器的位置：

| 常量                          | 值   | 描述                                                         |
| ----------------------------- | ---- | ------------------------------------------------------------ |
| QAbstractItemView::OnItem     | 0    | 项目将被放置在索引上。                                       |
| QAbstractItemView::AboveItem  | 1    | 项目将被放置在索引上方。                                     |
| QAbstractItemView::BelowItem  | 2    | 项目将被放置在索引下方。                                     |
| QAbstractItemView::OnViewport | 3    | 项目将被放置在视口的无项目区域。视图如何处理放置在视口上的项目取决于所用模型的行为。 |

#### QAbstractItemView::EditTrigger

描述将启动项目编辑的操作。

| 常量                               | 值   | 描述                           |
| ---------------------------------- | ---- | ------------------------------ |
| QAbstractItemView::NoEditTriggers  | 0    | 不允许编辑。                   |
| QAbstractItemView::CurrentChanged  | 1    | 当前项目变化时开始编辑。       |
| QAbstractItemView::DoubleClicked   | 2    | 双击项目时开始编辑。           |
| QAbstractItemView::SelectedClicked | 4    | 点击已选项目时开始编辑。       |
| QAbstractItemView::EditKeyPressed  | 8    | 按下平台编辑键时开始编辑。     |
| QAbstractItemView::AnyKeyPressed   | 16   | 在项目上按下任意键时开始编辑。 |
| QAbstractItemView::AllEditTriggers | 31   | 所有以上操作均可启动编辑。     |

`EditTriggers` 类型是 `QFlags<EditTrigger>` 的别名，存储 `EditTrigger` 值的 OR 组合。

#### QAbstractItemView::ScrollHint

指示如何滚动视图：

| 常量                                | 值   | 描述                         |
| ----------------------------------- | ---- | ---------------------------- |
| QAbstractItemView::EnsureVisible    | 0    | 滚动以确保项目可见。         |
| QAbstractItemView::PositionAtTop    | 1    | 滚动以将项目放在视口的顶部。 |
| QAbstractItemView::PositionAtBottom | 2    | 滚动以将项目放在视口的底部。 |
| QAbstractItemView::PositionAtCenter | 3    | 滚动以将项目放在视口的中心。 |

#### QAbstractItemView::ScrollMode

描述滚动条的行为：

| 常量                              | 值   | 描述                   |
| --------------------------------- | ---- | ---------------------- |
| QAbstractItemView::ScrollPerItem  | 0    | 视图每次滚动一个项目。 |
| QAbstractItemView::ScrollPerPixel | 1    | 视图每次滚动一个像素。 |

#### QAbstractItemView::SelectionBehavior

描述选择行为：

| 常量                             | 值   | 描述           |
| -------------------------------- | ---- | -------------- |
| QAbstractItemView::SelectItems   | 0    | 选择单个项目。 |
| QAbstractItemView::SelectRows    | 1    | 仅选择行。     |
| QAbstractItemView::SelectColumns | 2    | 仅选择列。     |

#### QAbstractItemView::SelectionMode

描述视图如何响应用户选择：

| 常量                                   | 值   | 描述                                                         |
| -------------------------------------- | ---- | ------------------------------------------------------------ |
| QAbstractItemView::SingleSelection     | 1    | 用户选择项目时，任何已选项目变为未选。                       |
| QAbstractItemView::ContiguousSelection | 4    | 用户选择项目时，之前的选择清除并选择新的项目。按 Shift 键可以选择多个项目。 |
| QAbstractItemView::ExtendedSelection   | 3    | 用户选择项目时，之前的选择清除并选择新的项目。按 Ctrl 键可以切换选择状态。 |
| QAbstractItemView::MultiSelection      | 2    | 用户选择项目时，选择状态切换，其他项目保持不变。             |
| QAbstractItemView::NoSelection         | 0    | 不允许选择项目。                                             |

常用的选择模式是 `SingleSelection` 和 `ExtendedSelection`。

#### QAbstractItemView::State

描述视图可以处于的不同状态。这通常在重新实现自定义视图时比较重要：

| 常量                                  | 值   | 描述                     |
| ------------------------------------- | ---- | ------------------------ |
| QAbstractItemView::NoState            | 0    | 默认状态。               |
| QAbstractItemView::DraggingState      | 1    | 用户正在拖动项目。       |
| QAbstractItemView::DragSelectingState | 2    | 用户正在选择项目。       |
| QAbstractItemView::EditingState       | 3    | 用户正在编辑项目。       |
| QAbstractItemView::ExpandingState     | 4    | 用户正在打开项目的分支。 |
| QAbstractItemView::CollapsingState    | 5    | 用户正在关闭项目的分支。 |
| QAbstractItemView::AnimatingState     | 6    | 项目视图正在执行动画。   |



### API

| 函数签名                                                     | 详细用途                                       |
| ------------------------------------------------------------ | ---------------------------------------------- |
| `QAbstractItemView(QWidget *parent = nullptr)`               | 构造函数，通过父级控件初始化视图。             |
| `virtual ~QAbstractItemView()`                               | 析构函数，清理资源。                           |
| `bool alternatingRowColors() const`                          | 获取是否启用交替行颜色。                       |
| `int autoScrollMargin() const`                               | 获取自动滚动的边距设置。                       |
| `void closePersistentEditor(const QModelIndex &index)`       | 关闭指定索引的持久编辑器。                     |
| `QModelIndex currentIndex() const`                           | 获取当前选中的索引。                           |
| `Qt::DropAction defaultDropAction() const`                   | 获取默认的拖放动作。                           |
| `QAbstractItemView::DragDropMode dragDropMode() const`       | 获取当前的拖放模式。                           |
| `bool dragDropOverwriteMode() const`                         | 获取拖放时是否覆盖模式。                       |
| `bool dragEnabled() const`                                   | 获取拖动功能是否启用。                         |
| `QAbstractItemView::EditTriggers editTriggers() const`       | 获取编辑触发器设置。                           |
| `bool hasAutoScroll() const`                                 | 获取是否支持自动滚动。                         |
| `QAbstractItemView::ScrollMode horizontalScrollMode() const` | 获取水平滚动模式。                             |
| `QSize iconSize() const`                                     | 获取图标的大小。                               |
| `virtual QModelIndex indexAt(const QPoint &point) const = 0` | 获取指定点的模型索引（纯虚函数，需要实现）。   |
| `QWidget *indexWidget(const QModelIndex &index) const`       | 获取与指定索引关联的控件。                     |
| `bool isPersistentEditorOpen(const QModelIndex &index) const` | 检查指定索引的持久编辑器是否打开。             |
| `QAbstractItemDelegate *itemDelegate() const`                | 获取当前的项委托。                             |
| `QAbstractItemDelegate *itemDelegateForColumn(int column) const` | 获取指定列的项委托。                           |
| `virtual QAbstractItemDelegate *itemDelegateForIndex(const QModelIndex &index) const` | 获取指定索引的项委托（自 Qt 6.0 起）。         |
| `QAbstractItemDelegate *itemDelegateForRow(int row) const`   | 获取指定行的项委托。                           |
| `virtual void keyboardSearch(const QString &search)`         | 根据搜索字符串进行键盘搜索。                   |
| `QAbstractItemModel *model() const`                          | 获取当前使用的模型。                           |
| `void openPersistentEditor(const QModelIndex &index)`        | 打开指定索引的持久编辑器。                     |
| `void resetHorizontalScrollMode()`                           | 重置水平滚动模式。                             |
| `void resetVerticalScrollMode()`                             | 重置垂直滚动模式。                             |
| `QModelIndex rootIndex() const`                              | 获取模型的根索引。                             |
| `virtual void scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint = EnsureVisible) = 0` | 滚动到指定索引（纯虚函数，需要实现）。         |
| `QAbstractItemView::SelectionBehavior selectionBehavior() const` | 获取选择行为设置。                             |
| `QAbstractItemView::SelectionMode selectionMode() const`     | 获取选择模式设置。                             |
| `QItemSelectionModel *selectionModel() const`                | 获取当前选择模型。                             |
| `void setAlternatingRowColors(bool enable)`                  | 设置是否启用交替行颜色。                       |
| `void setAutoScroll(bool enable)`                            | 设置是否启用自动滚动。                         |
| `void setAutoScrollMargin(int margin)`                       | 设置自动滚动的边距。                           |
| `void setDefaultDropAction(Qt::DropAction dropAction)`       | 设置默认的拖放动作。                           |
| `void setDragDropMode(QAbstractItemView::DragDropMode behavior)` | 设置拖放模式。                                 |
| `void setDragDropOverwriteMode(bool overwrite)`              | 设置拖放时是否覆盖模式。                       |
| `void setDragEnabled(bool enable)`                           | 设置拖动功能的启用状态。                       |
| `void setDropIndicatorShown(bool enable)`                    | 设置是否显示拖放指示器。                       |
| `void setEditTriggers(QAbstractItemView::EditTriggers triggers)` | 设置编辑触发器。                               |
| `void setHorizontalScrollMode(QAbstractItemView::ScrollMode mode)` | 设置水平滚动模式。                             |
| `void setIconSize(const QSize &size)`                        | 设置图标的大小。                               |
| `void setIndexWidget(const QModelIndex &index, QWidget *widget)` | 设置与指定索引关联的控件。                     |
| `void setItemDelegate(QAbstractItemDelegate *delegate)`      | 设置全局项委托。                               |
| `void setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate)` | 设置指定列的项委托。                           |
| `void setItemDelegateForRow(int row, QAbstractItemDelegate *delegate)` | 设置指定行的项委托。                           |
| `virtual void setModel(QAbstractItemModel *model)`           | 设置模型（纯虚函数，需要实现）。               |
| `void setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior)` | 设置选择行为。                                 |
| `void setSelectionMode(QAbstractItemView::SelectionMode mode)` | 设置选择模式。                                 |
| `virtual void setSelectionModel(QItemSelectionModel *selectionModel)` | 设置选择模型（纯虚函数，需要实现）。           |
| `void setTabKeyNavigation(bool enable)`                      | 设置 Tab 键导航的启用状态。                    |
| `void setTextElideMode(Qt::TextElideMode mode)`              | 设置文本省略模式。                             |
| `void setVerticalScrollMode(QAbstractItemView::ScrollMode mode)` | 设置垂直滚动模式。                             |
| `bool showDropIndicator() const`                             | 获取是否显示拖放指示器。                       |
| `virtual int sizeHintForColumn(int column) const`            | 获取指定列的建议大小（纯虚函数，需要实现）。   |
| `QSize sizeHintForIndex(const QModelIndex &index) const`     | 获取指定索引的建议大小。                       |
| `virtual int sizeHintForRow(int row) const`                  | 获取指定行的建议大小（纯虚函数，需要实现）。   |
| `bool tabKeyNavigation() const`                              | 获取 Tab 键导航的启用状态。                    |
| `Qt::TextElideMode textElideMode() const`                    | 获取文本省略模式。                             |
| `QAbstractItemView::ScrollMode verticalScrollMode() const`   | 获取垂直滚动模式。                             |
| `virtual QRect visualRect(const QModelIndex &index) const = 0` | 获取指定索引的可视区域（纯虚函数，需要实现）。 |

| 函数签名                                                     | 详细用途         |
| ------------------------------------------------------------ | ---------------- |
| `virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const override` | 处理输入法查询。 |

| 函数签名                                              | 详细用途                           |
| ----------------------------------------------------- | ---------------------------------- |
| `void clearSelection()`                               | 清除当前选择。                     |
| `void edit(const QModelIndex &index)`                 | 编辑指定的索引。                   |
| `virtual void reset()`                                | 重置视图。                         |
| `void scrollToBottom()`                               | 滚动到视图底部。                   |
| `void scrollToTop()`                                  | 滚动到视图顶部。                   |
| `virtual void selectAll()`                            | 选择所有项目。                     |
| `void setCurrentIndex(const QModelIndex &index)`      | 设置当前索引。                     |
| `virtual void setRootIndex(const QModelIndex &index)` | 设置根索引（纯虚函数，需要实现）。 |
| `void update(const QModelIndex &index)`               | 更新指定索引。                     |

| 信号名称                                       | 详细用途               |
| ---------------------------------------------- | ---------------------- |
| `void activated(const QModelIndex &index)`     | 发射当项目被激活时。   |
| `void clicked(const QModelIndex &index)`       | 发射当项目被点击时。   |
| `void doubleClicked(const QModelIndex &index)` | 发射当项目被双击时。   |
| `void entered(const QModelIndex &index)`       | 发射当鼠标进入项目时。 |
| `void iconSizeChanged(const QSize &size)`      | 发射当图标大小变化时。 |
| `void pressed(const QModelIndex &index)`       | 发射当项目被按下时。   |
| `void viewportEntered()`                       | 发射当视口被进入时。   |

| 项目           | 内容                                                         |
| -------------- | ------------------------------------------------------------ |
| **受保护类型** | `enum CursorAction { MoveUp, MoveDown, ... }`: 光标操作类型。 |
|                | `enum DropIndicatorPosition { OnItem, AboveItem, ... }`: 拖放指示器位置。 |
|                | `enum State { NoState, DraggingState, ... }`: 状态类型。     |
| **受保护函数** | `QPoint dirtyRegionOffset() const`: 获取脏区域偏移量。       |
|                | `QAbstractItemView::DropIndicatorPosition dropIndicatorPosition() const`: 获取拖放指示器位置。 |
|                | `virtual bool edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)` |
|                | `void executeDelayedItemsLayout()`: 执行延迟项布局。         |
|                | `virtual int horizontalOffset() const = 0`: 获取水平偏移量（纯虚函数，需要实现）。 |
|                | `virtual bool isIndexHidden(const QModelIndex &index) const = 0`: 检查索引是否隐藏（纯虚函数）。 |
|                | `virtual QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) = 0`: 移动光标（纯虚函数）。 |
|                | `void scheduleDelayedItemsLayout()`: 调度延迟项布局。        |
|                | `void scrollDirtyRegion(int dx, int dy)`: 滚动脏区域。       |
|                | `virtual QModelIndexList selectedIndexes() const`: 获取选中的索引列表（纯虚函数）。 |
|                | `virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event = nullptr) const`: 获取选择命令（纯虚函数）。 |
|                | `void setDirtyRegion(const QRegion &region)`: 设置脏区域。   |
|                | `virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) = 0`: 设置选择区域（纯虚函数）。 |
|                | `void setState(QAbstractItemView::State state)`: 设置视图状态。 |
|                | `virtual void startDrag(Qt::DropActions supportedActions)`：开始拖动（纯虚函数）。 |
|                | `QAbstractItemView::State state() const`: 获取当前状态。     |
|                | `virtual int verticalOffset() const = 0`: 获取垂直偏移量（纯虚函数，需要实现）。 |
|                | `virtual QRegion visualRegionForSelection(const QItemSelection &selection) const = 0`: 获取选择区域的可视区域（纯虚函数）。 |

| 函数签名                                                     | 详细用途               |
| ------------------------------------------------------------ | ---------------------- |
| `virtual void dragEnterEvent(QDragEnterEvent *event) override` | 处理拖动进入事件。     |
| `virtual void dragLeaveEvent(QDragLeaveEvent *event) override` | 处理拖动离开事件。     |
| `virtual void dragMoveEvent(QDragMoveEvent *event) override` | 处理拖动移动事件。     |
| `virtual void dropEvent(QDropEvent *event) override`         | 处理放置事件。         |
| `virtual bool event(QEvent *event) override`                 | 处理通用事件。         |
| `virtual bool eventFilter(QObject *object, QEvent *event) override` | 事件过滤器处理。       |
| `virtual void focusInEvent(QFocusEvent *event) override`     | 处理获得焦点事件。     |
| `virtual bool focusNextPrevChild(bool next) override`        | 处理焦点切换事件。     |
| `virtual void focusOutEvent(QFocusEvent *event) override`    | 处理失去焦点事件。     |
| `virtual void inputMethodEvent(QInputMethodEvent *event) override` | 处理输入法事件。       |
| `virtual void keyPressEvent(QKeyEvent *event) override`      | 处理按键按下事件。     |
| `virtual void mouseDoubleClickEvent(QMouseEvent *event) override` | 处理鼠标双击事件。     |
| `virtual void mouseMoveEvent(QMouseEvent *event) override`   | 处理鼠标移动事件。     |
| `virtual void mousePressEvent(QMouseEvent *event) override`  | 处理鼠标按下事件。     |
| `virtual void mouseReleaseEvent(QMouseEvent *event) override` | 处理鼠标释放事件。     |
| `virtual void resizeEvent(QResizeEvent *event) override`     | 处理窗口调整大小事件。 |
| `virtual void timerEvent(QTimerEvent *event) override`       | 处理定时器事件。       |
| `virtual bool viewportEvent(QEvent *event) override`         | 处理视口事件。         |
| `virtual QSize viewportSizeHint() const override`            | 获取视口的建议大小。   |

| 函数签名                                                     | 详细用途             |
| ------------------------------------------------------------ | -------------------- |
| `virtual void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)` | 关闭编辑器。         |
| `virtual void commitData(QWidget *editor)`                   | 提交编辑数据。       |
| `virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous)` | 当前索引变化时发射。 |
| `virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>())` | 数据变化时发射。     |
| `virtual void editorDestroyed(QObject *editor)`              | 编辑器销毁时发射。   |
| `virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)` | 行即将被移除时发射。 |
| `virtual void rowsInserted(const QModelIndex &parent, int start, int end)` | 行插入时发射。       |
| `virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)` | 选择变化时发射。     |
| `virtual void updateGeometries()`                            | 更新几何形状时发射。 |