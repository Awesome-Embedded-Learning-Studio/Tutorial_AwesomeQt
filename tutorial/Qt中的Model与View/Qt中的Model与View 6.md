# QTreeView与QFileSystemModel

## QTreeView

`QTreeView` 实现了来自模型的项目的树形表示。这个类用于提供标准的层次结构列表，这些列表之前由 `QListView` 类提供，但使用了 Qt 的模型/视图架构提供的更灵活的方法。

`QTreeView` 类是模型/视图框架的一部分，并实现了 `QAbstractItemView` 类定义的接口，以允许其显示由 `QAbstractItemModel` 类派生的模型提供的数据。

构造一个显示模型数据的树视图非常简单。以下示例中，目录的内容由 `QFileSystemModel` 提供，并以树形结构显示：

```
QFileSystemModel *model = new QFileSystemModel;
model->setRootPath(QDir::currentPath());
QTreeView *tree = new QTreeView(splitter);
tree->setModel(model);
```

模型/视图架构确保树视图的内容在模型更改时得到更新。

具有子项的项目可以处于展开（子项可见）或折叠（子项隐藏）状态。当此状态发生变化时，将发出 `collapsed()` 或 `expanded()` 信号，并携带相关项目的模型索引。

用于指示层次结构级别的缩进量由 `indentation` 属性控制。

树视图的头部是使用 `QHeaderView` 类构造的，可以通过 `header()->hide()` 隐藏。请注意，每个头部的 `stretchLastSection` 属性被设置为 `true`，确保视图不会浪费分配给头部的空间。如果此值设置为 `true`，则该属性将覆盖头部最后一部分的调整模式。

默认情况下，树视图中的所有列都是可移动的，除了第一列。要禁用这些列的移动，可以使用 `QHeaderView` 的 `setSectionsMovable()` 函数。有关重新排列部分的更多信息，请参见“移动头部部分”。

`QTreeView` 支持一组键绑定，使用户能够在视图中导航并与项目的内容进行交互：

| 键       | 动作                                                         |
| -------- | ------------------------------------------------------------ |
| Up       | 将光标移动到同一列的上一行项目。                             |
| Down     | 将光标移动到同一列的下一行项目。                             |
| Left     | 折叠当前项目（如果有子项）。                                 |
| Minus    | 同上。                                                       |
| Right    | 展开当前项目（如果有子项）。                                 |
| Plus     | 同上。                                                       |
| Asterisk | 展开当前项目及其所有子项（如果有）。                         |
| PageUp   | 向上移动光标一页。                                           |
| PageDown | 向下移动光标一页。                                           |
| Home     | 将光标移动到模型中第一个顶层项目的同一列的第一个行项目。     |
| End      | 将光标移动到模型中最后一个顶层项目的同一列的最后一行项目。   |
| F2       | 在可编辑模型中打开当前项目进行编辑。按 Escape 键可以取消编辑过程并还原数据。 |

可以给视图提供有关其处理的数据的提示，以提高在显示大量项目时的性能。其中一种方法是为旨在显示具有相等高度的项目的视图设置 `uniformRowHeights` 属性为 `true`。

### QTreeView API

| API 签名                                                     | 说明                                                |
| ------------------------------------------------------------ | --------------------------------------------------- |
| `QTreeView(QWidget *parent = nullptr)`                       | 构造函数，创建一个 `QTreeView` 对象，父对象为可选。 |
| `virtual ~QTreeView()`                                       | 虚析构函数，释放 `QTreeView` 对象。                 |
| `bool allColumnsShowFocus() const`                           | 返回所有列是否显示焦点。                            |
| `int autoExpandDelay() const`                                | 返回自动展开的延迟时间。                            |
| `int columnAt(int x) const`                                  | 返回在指定 x 坐标处的列索引。                       |
| `int columnViewportPosition(int column) const`               | 返回指定列在视口中的位置。                          |
| `int columnWidth(int column) const`                          | 返回指定列的宽度。                                  |
| `bool expandsOnDoubleClick() const`                          | 返回双击是否会展开项目。                            |
| `QHeaderView *header() const`                                | 返回树视图的头部。                                  |
| `int indentation() const`                                    | 返回项目的缩进量。                                  |
| `QModelIndex indexAbove(const QModelIndex &index) const`     | 返回指定索引上方的索引。                            |
| `QModelIndex indexBelow(const QModelIndex &index) const`     | 返回指定索引下方的索引。                            |
| `bool isAnimated() const`                                    | 返回树视图是否以动画方式展开/收起。                 |
| `bool isColumnHidden(int column) const`                      | 返回指定列是否隐藏。                                |
| `bool isExpanded(const QModelIndex &index) const`            | 返回指定索引的项目是否展开。                        |
| `bool isFirstColumnSpanned(int row, const QModelIndex &parent) const` | 返回指定行的第一个列是否跨越。                      |
| `bool isHeaderHidden() const`                                | 返回头部是否隐藏。                                  |
| `bool isRowHidden(int row, const QModelIndex &parent) const` | 返回指定行是否隐藏。                                |
| `bool isSortingEnabled() const`                              | 返回是否启用了排序。                                |
| `bool itemsExpandable() const`                               | 返回项目是否可以展开。                              |
| `void resetIndentation()`                                    | 重置缩进量。                                        |
| `bool rootIsDecorated() const`                               | 返回根项是否被装饰。                                |
| `void setAllColumnsShowFocus(bool enable)`                   | 设置是否所有列都显示焦点。                          |
| `void setAnimated(bool enable)`                              | 设置树视图是否以动画方式展开/收起。                 |
| `void setAutoExpandDelay(int delay)`                         | 设置自动展开的延迟时间。                            |
| `void setColumnHidden(int column, bool hide)`                | 设置指定列为隐藏或显示。                            |
| `void setColumnWidth(int column, int width)`                 | 设置指定列的宽度。                                  |
| `void setExpanded(const QModelIndex &index, bool expanded)`  | 设置指定索引的项目为展开或收起。                    |
| `void setExpandsOnDoubleClick(bool enable)`                  | 设置双击是否会展开项目。                            |
| `void setFirstColumnSpanned(int row, const QModelIndex &parent, bool span)` | 设置指定行的第一个列是否跨越。                      |
| `void setHeader(QHeaderView *header)`                        | 设置树视图的头部。                                  |
| `void setHeaderHidden(bool hide)`                            | 设置头部为隐藏或显示。                              |
| `void setIndentation(int i)`                                 | 设置项目的缩进量。                                  |
| `void setItemsExpandable(bool enable)`                       | 设置项目是否可以展开。                              |
| `void setRootIsDecorated(bool show)`                         | 设置根项是否被装饰。                                |
| `void setRowHidden(int row, const QModelIndex &parent, bool hide)` | 设置指定行为隐藏或显示。                            |
| `void setSortingEnabled(bool enable)`                        | 设置是否启用排序。                                  |
| `void setTreePosition(int index)`                            | 设置树的位置。                                      |
| `void setUniformRowHeights(bool uniform)`                    | 设置是否使用统一行高。                              |
| `void setWordWrap(bool on)`                                  | 设置是否启用自动换行。                              |
| `int treePosition() const`                                   | 返回树的位置。                                      |
| `bool uniformRowHeights() const`                             | 返回是否使用统一行高。                              |
| `bool wordWrap() const`                                      | 返回是否启用自动换行。                              |

#### 重载公共函数

| API 签名                                                     | 说明                         |
| ------------------------------------------------------------ | ---------------------------- |
| `virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>()) override` | 当模型数据发生更改时被调用。 |
| `virtual QModelIndex indexAt(const QPoint &point) const override` | 返回指定点的模型索引。       |
| `virtual void keyboardSearch(const QString &search) override` | 在树中进行键盘搜索。         |
| `virtual void reset() override`                              | 重置视图。                   |
| `virtual void scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint = EnsureVisible) override` | 滚动到指定索引。             |
| `virtual void selectAll() override`                          | 选择所有项目。               |
| `virtual void setModel(QAbstractItemModel *model) override`  | 设置数据模型。               |
| `virtual void setRootIndex(const QModelIndex &index) override` | 设置根索引。                 |
| `virtual void setSelectionModel(QItemSelectionModel *selectionModel) override` | 设置选择模型。               |
| `virtual QRect visualRect(const QModelIndex &index) const override` | 返回指定索引的可视矩形。     |

#### 公共槽

| API 签名                                                     | 说明                     |
| ------------------------------------------------------------ | ------------------------ |
| `void collapse(const QModelIndex &index)`                    | 折叠指定索引的项目。     |
| `void collapseAll()`                                         | 折叠所有项目。           |
| `void expand(const QModelIndex &index)`                      | 展开指定索引的项目。     |
| `void expandAll()`                                           | 展开所有项目。           |
| `void expandRecursively(const QModelIndex &index, int depth = -1)` | 递归展开指定索引的项目。 |
| `void expandToDepth(int depth)`                              | 展开到指定深度。         |
| `void hideColumn(int column)`                                | 隐藏指定列。             |
| `void resizeColumnToContents(int column)`                    | 根据内容调整列宽。       |
| `void showColumn(int column)`                                | 显示指定列。             |
| `void sortByColumn(int column, Qt::SortOrder order)`         | 按指定列排序。           |

#### 信号

| 信号                                       | 说明                               |
| ------------------------------------------ | ---------------------------------- |
| `void collapsed(const QModelIndex &index)` | 当指定索引的项目被折叠时发出信号。 |
| `void expanded(const QModelIndex &index)`  | 当指定索引的项目被展开时发出信号。 |

## QFileSystemModel

这个类提供对本地文件系统的访问，提供重命名、删除文件和目录以及创建新目录的功能。在最简单的情况下，它可以与合适的显示控件一起使用，作为浏览器或过滤器的一部分。

`QFileSystemModel` 可以使用 `QAbstractItemModel` 提供的标准接口进行访问，但它还提供了一些特定于目录模型的便利函数。`fileInfo()`、`isDir()`、`fileName()` 和 `filePath()` 函数提供有关模型中项目相关的底层文件和目录的信息。可以使用 `mkdir()` 和 `rmdir()` 创建和删除目录。

**注意**：`QFileSystemModel` 需要 `QApplication` 的实例。

一个显示默认目录内容的目录模型通常以父对象的方式构造：

```
QFileSystemModel *model = new QFileSystemModel;
model->setRootPath(QDir::currentPath());
```

可以使用树视图显示模型的内容：

```
QTreeView *tree = new QTreeView(splitter);
tree->setModel(model);
```

并可以通过设置树视图的根索引显示特定目录的内容：

```
tree->setRootIndex(model->index(QDir::currentPath()));
```

视图的根索引可以控制显示层次模型的多少。`QFileSystemModel` 提供了一个便利函数，可以返回指向模型中目录路径的适当模型索引。

`QFileSystemModel` 在调用 `setRootPath()` 之前不会获取任何文件或目录。这将防止在此之前对文件系统进行不必要的查询，例如列出 Windows 上的驱动器。

`QFileSystemModel` 使用单独的线程来填充自身，因此在查询文件系统时不会导致主线程挂起。调用 `rowCount()` 将返回 0，直到模型填充了目录。

`QFileSystemModel` 保持一个文件信息的缓存。缓存会使用 `QFileSystemWatcher` 自动保持最新。

#### 枚举 QFileSystemModel::Option

| 常量                                            | 值           | 说明                                                         |
| ----------------------------------------------- | ------------ | ------------------------------------------------------------ |
| `QFileSystemModel::DontWatchForChanges`         | `0x00000001` | 不对路径添加文件监视器。适用于简单任务，减少开销。           |
| `QFileSystemModel::DontResolveSymlinks`         | `0x00000002` | 不解析文件系统模型中的符号链接。默认情况下，符号链接会被解析。 |
| `QFileSystemModel::DontUseCustomDirectoryIcons` | `0x00000004` | 始终使用默认目录图标。一些平台允许用户设置不同图标。自定义图标查找会对网络或可移动驱动器造成较大性能影响。 |

`Options` 类型是 `QFlags<Option>` 的别名。它存储了多个选项值的按位或组合。

#### 枚举 QFileSystemModel::Roles

| 常量                                | 值                   |
| ----------------------------------- | -------------------- |
| `QFileSystemModel::FileIconRole`    | `Qt::DecorationRole` |
| `QFileSystemModel::FilePathRole`    | `Qt::UserRole + 1`   |
| `QFileSystemModel::FileNameRole`    | `Qt::UserRole + 2`   |
| `QFileSystemModel::FilePermissions` | `Qt::UserRole + 3`   |
| `QFileSystemModel::FileInfoRole`    | `Qt::UserRole - 4`   |

### QFileSystemModel API

| API 签名                                                     | 说明                                                       |
| ------------------------------------------------------------ | ---------------------------------------------------------- |
| `QFileSystemModel(QObject *parent = nullptr)`                | 构造函数，创建一个 `QFileSystemModel` 对象，父对象为可选。 |
| `virtual ~QFileSystemModel()`                                | 虚析构函数，释放 `QFileSystemModel` 对象。                 |
| `QIcon fileIcon(const QModelIndex &index) const`             | 返回指定索引的文件图标。                                   |
| `QFileInfo fileInfo(const QModelIndex &index) const`         | 返回指定索引的文件信息。                                   |
| `QString fileName(const QModelIndex &index) const`           | 返回指定索引的文件名。                                     |
| `QString filePath(const QModelIndex &index) const`           | 返回指定索引的文件路径。                                   |
| `QDir::Filters filter() const`                               | 返回当前的过滤器设置。                                     |
| `QAbstractFileIconProvider *iconProvider() const`            | 返回图标提供者。                                           |
| `QModelIndex index(const QString &path, int column = 0) const` | 返回指定路径的模型索引。                                   |
| `bool isDir(const QModelIndex &index) const`                 | 返回指定索引是否是目录。                                   |
| `bool isReadOnly() const`                                    | 返回模型是否为只读。                                       |
| `QDateTime lastModified(const QModelIndex &index) const`     | 返回指定索引的文件的最后修改时间。                         |
| `(since 6.6) QDateTime lastModified(const QModelIndex &index, const QTimeZone &tz) const` | 返回指定索引的文件在指定时区的最后修改时间。               |
| `QModelIndex mkdir(const QModelIndex &parent, const QString &name)` | 在指定父索引下创建新目录。                                 |
| `QVariant myComputer(int role = Qt::DisplayRole) const`      | 返回“我的电脑”项。                                         |
| `bool nameFilterDisables() const`                            | 返回名称过滤器是否被禁用。                                 |
| `QStringList nameFilters() const`                            | 返回名称过滤器列表。                                       |
| `QFileSystemModel::Options options() const`                  | 返回当前的选项设置。                                       |
| `QFileDevice::Permissions permissions(const QModelIndex &index) const` | 返回指定索引的文件权限。                                   |
| `bool remove(const QModelIndex &index)`                      | 删除指定索引的文件或目录。                                 |
| `bool resolveSymlinks() const`                               | 返回是否解析符号链接。                                     |
| `bool rmdir(const QModelIndex &index)`                       | 删除指定索引的空目录。                                     |
| `QDir rootDirectory() const`                                 | 返回根目录对象。                                           |
| `QString rootPath() const`                                   | 返回根路径。                                               |
| `void setFilter(QDir::Filters filters)`                      | 设置过滤器。                                               |
| `void setIconProvider(QAbstractFileIconProvider *provider)`  | 设置图标提供者。                                           |
| `void setNameFilterDisables(bool enable)`                    | 设置名称过滤器是否禁用。                                   |
| `void setNameFilters(const QStringList &filters)`            | 设置名称过滤器列表。                                       |
| `void setOption(QFileSystemModel::Option option, bool on = true)` | 设置指定选项。                                             |
| `void setOptions(QFileSystemModel::Options options)`         | 设置多个选项。                                             |
| `void setReadOnly(bool enable)`                              | 设置模型为只读或可写。                                     |
| `void setResolveSymlinks(bool enable)`                       | 设置是否解析符号链接。                                     |
| `QModelIndex setRootPath(const QString &newPath)`            | 设置新根路径并返回相应的模型索引。                         |
| `qint64 size(const QModelIndex &index) const`                | 返回指定索引的文件大小。                                   |
| `bool testOption(QFileSystemModel::Option option) const`     | 测试指定选项是否启用。                                     |
| `QString type(const QModelIndex &index) const`               | 返回指定索引的文件类型。                                   |

| API 签名                                                     | 说明                           |
| ------------------------------------------------------------ | ------------------------------ |
| `virtual bool canFetchMore(const QModelIndex &parent) const override` | 返回是否可以获取更多数据。     |
| `virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override` | 返回指定父索引下的列数。       |
| `virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override` | 返回指定索引的数据。           |
| `virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override` | 处理拖放数据。                 |
| `virtual void fetchMore(const QModelIndex &parent) override` | 获取更多数据。                 |
| `virtual Qt::ItemFlags flags(const QModelIndex &index) const override` | 返回指定索引的项目标志。       |
| `virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override` | 返回指定父索引是否有子项。     |
| `virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override` | 返回头部数据。                 |
| `virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override` | 返回指定行和列的模型索引。     |
| `virtual QMimeData *mimeData(const QModelIndexList &indexes) const override` | 返回指定索引列表的 MIME 数据。 |
| `virtual QStringList mimeTypes() const override`             | 返回支持的 MIME 类型。         |
| `virtual QModelIndex parent(const QModelIndex &index) const override` | 返回指定索引的父索引。         |
| `virtual QHash<int, QByteArray> roleNames() const override`  | 返回角色名称的哈希表。         |
| `virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override` | 返回指定父索引下的行数。       |
| `virtual bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override` | 设置指定索引的数据。           |
| `virtual QModelIndex sibling(int row, int column, const QModelIndex &idx) const override` | 返回指定索引的兄弟索引。       |
| `virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override` | 按指定列排序。                 |
| `virtual Qt::DropActions supportedDropActions() const override` | 返回支持的拖放动作。           |

| 信号                                                         | 说明                               |
| ------------------------------------------------------------ | ---------------------------------- |
| `void directoryLoaded(const QString &path)`                  | 当指定路径的目录被加载时发出信号。 |
| `void fileRenamed(const QString &path, const QString &oldName, const QString &newName)` | 当文件重命名时发出信号。           |
| `void rootPathChanged(const QString &newPath`                | 当根路径更改时发出信号。           |

| API 签名                                               | 说明             |
| ------------------------------------------------------ | ---------------- |
| `virtual bool event(QEvent *event) override`           | 处理事件。       |
| `virtual void timerEvent(QTimerEvent *event) override` | 处理定时器事件。 |