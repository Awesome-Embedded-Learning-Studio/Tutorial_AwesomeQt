# 现代Qt开发教程（新手篇）3.53——QColumnView：多列级联视图

## 1. 前言 / 另一种浏览层级数据的方式

前面我们用了两篇来聊 QTreeView——一个用缩进和展开/折叠来展示层级数据的经典树形控件。这种"从左到右缩进"的树形展示方式在 Windows 和 Linux 的文件管理器中几乎是标配。但如果你用过 macOS 的 Finder，你会发现它的列视图（Column View）是另一种完全不同的层级浏览方式：每选中一个目录，右侧就新开一列显示它的子项，像是一层层"钻入"数据结构内部。这种浏览方式的优势在于，用户可以在视觉上清晰地看到从根到叶子节点的完整路径，同时在任意层级之间快速跳转——不需要反复展开折叠，也不需要面对一棵纵深很深、内容被挤压到右侧的树。

QColumnView 就是 Qt 提供的这种"多列级联"视图控件。它同样基于 Model/View 架构，使用和 QTreeView 完全相同的 Model 接口——任何能喂给 QTreeView 的 Model（QStandardItemModel、QFileSystemModel、自定义 QAbstractItemModel）都可以直接喂给 QColumnView。区别只在于渲染方式：QTreeView 用缩进表现层级关系，QColumnView 用并排的多列来表现每一层。

今天的内容从四个方面展开。先看 QColumnView 的基本用法和它与 macOS Finder 列视图的关系，然后通过 setModel 与层级数据的绑定方式来构建一个完整的多列浏览界面，接着研究 updatePreviewWidget 如何在右侧挂载一个预览面板，最后讨论 QColumnView 在文件浏览器和设置面板导航这两种典型场景下的应用。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QColumnView 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QColumnView、QStandardItemModel、QFileSystemModel、QLabel、QListView、QPushButton、QVBoxLayout、QHBoxLayout 和 QSplitter。

## 3. 核心概念讲解

### 3.1 macOS Finder 列视图风格

QColumnView 的视觉表现和 macOS Finder 的列视图几乎一模一样。当 View 绑定了一个层级 Model 后，最左侧的一列显示 Model 的顶层条目。用户点击其中一个条目时，如果这个条目有子节点（Model 的 hasChildren 返回 true），它的右侧就会新开一列显示它的子项。继续点击子项中的某个条目，右侧再开一列——如此逐层深入，每一层的数据独立占一列。每一列内部的表现类似一个 QListView——条目从上到下排列，当前选中的条目高亮显示，右侧有一个小三角箭头表示该条目有子节点。

这种"逐层深入"的浏览方式天然适合层级数据——文件系统目录树、组织架构图、产品分类体系、设置项的树形结构。和 QTreeView 相比，QColumnView 的优势在于每一层的所有选项都平铺显示在同一列中，不需要展开折叠操作，浏览效率更高。尤其是当用户需要在同一层级的多个兄弟节点之间频繁切换时，QColumnView 的体验远好于 QTreeView——在 QTreeView 中你需要先折叠再展开另一个节点，在 QColumnView 中只需要在同一列里点击另一个条目，右侧的子列立刻刷新。

```cpp
auto *model = new QStandardItemModel(this);
model->setHorizontalHeaderLabels({"名称"});

// 构建层级数据...
// （省略数据构建代码，下一节详细展开）

auto *columnView = new QColumnView;
columnView->setModel(model);
```

QColumnView 默认在内部为每一层创建一个 QListView 实例。这些内部的 QListView 是 QColumnView 自动管理的——你不需要手动创建或者布局它们。当用户在某一列中选中一个有子节点的条目时，QColumnView 自动在右侧创建（或更新）一个新的 QListView 来显示子节点。当用户回退到上一层的另一个条目时，右侧的列自动更新。整个多列的滚动和尺寸调整也是 QColumnView 内部处理的。

QColumnView 继承自 QAbstractItemView，因此它拥有和 QListView、QTreeView、QTableView 相同的选择模式、编辑触发、委托（delegate）机制等基础能力。你可以通过 setSelectionBehavior 设置选择行为，通过 setEditTriggers 设置编辑触发方式，通过 setItemDelegate 设置自定义的渲染委托。这些设置会应用到 QColumnView 内部创建的每一列的 QListView 上。

### 3.2 setModel 与层级数据绑定

QColumnView 的 setModel 接受任何实现了 QAbstractItemModel 接口的 Model。由于 QColumnView 需要处理层级数据，所以它依赖 Model 的 index()、parent()、rowCount()、hasChildren() 等方法来构建多列视图。一个只实现了 rowCount() 和 data() 的平铺 Model（比如 QStringListModel）在 QColumnView 中只会显示一列——因为没有层级关系，就没有"钻入"的必要。

用 QStandardItemModel 构建层级数据是最直观的方式。QStandardItemModel 的每个 QStandardItem 可以通过 appendRow 添加子节点，形成任意深度的树。QColumnView 会根据这棵树的结构自动创建对应数量的列。

```cpp
auto *model = new QStandardItemModel(this);

// 顶层分类
auto *devTools = new QStandardItem("开发工具");
auto *designTools = new QStandardItem("设计工具");
auto *officeTools = new QStandardItem("办公工具");
model->appendRow(devTools);
model->appendRow(designTools);
model->appendRow(officeTools);

// 开发工具的子分类
auto *editors = new QStandardItem("编辑器");
auto *compilers = new QStandardItem("编译器");
auto *debuggers = new QStandardItem("调试器");
devTools->appendRow(editors);
devTools->appendRow(compilers);
devTools->appendRow(debuggers);

// 编辑器下的具体工具
editors->appendRow(new QStandardItem("VS Code"));
editors->appendRow(new QStandardItem("Vim"));
editors->appendRow(new QStandardItem("Emacs"));

// 编译器下的具体工具
compilers->appendRow(new QStandardItem("GCC"));
compilers->appendRow(new QStandardItem("Clang"));
compilers->appendRow(new QStandardItem("MSVC"));

auto *columnView = new QColumnView;
columnView->setModel(model);
```

上面这段代码构建了一个三层的分类树：第一层是"开发工具/设计工具/办公工具"，第二层是每个大分类下的子分类，第三层是具体的工具名称。QColumnView 会把它渲染成三列——点击"开发工具"后第二列显示"编辑器/编译器/调试器"，点击"编辑器"后第三列显示"VS Code/Vim/Emacs"。

每一列的宽度可以通过 setColumnWidths(const QList<int> &widths) 来设置。传入的 QList 中每个元素对应一列的宽度（像素）。如果列表长度小于当前显示的列数，多余的列使用默认宽度。

```cpp
columnView->setColumnWidths({150, 150, 200});
```

QColumnView 的 clicked(const QModelIndex &index) 和 doubleClicked(const QModelIndex &index) 信号在用户点击或双击某个条目时发射，参数是被点击条目的 QModelIndex。通过 index 你可以获取 Model 中的数据——model->data(index, Qt::DisplayRole) 获取显示文本，model->hasChildren(index) 判断是否有子节点。

QColumnView 还有一个 currentChanged 信号（继承自 QAbstractItemView），在当前选中条目变化时发射。这在需要"选中一个条目就在预览面板中显示详细信息"的场景下非常有用——下一节会详细讲。

如果你已经有了一个用于 QTreeView 的 Model，把它直接喂给 QColumnView 就行了，不需要任何修改。这是因为 QColumnView 和 QTreeView 使用完全相同的 Model 接口——index()、parent()、rowCount()、columnCount()、data()、hasChildren()。你的 Model 甚至不需要知道 View 是 QTreeView 还是 QColumnView。这种"一个 Model 多种 View"的能力正是 Model/View 架构的核心价值。

### 3.3 updatePreviewWidget 预览面板

QColumnView 有一个独特的能力：在所有列的右侧挂一个预览面板（preview widget）。当用户在最后一列选中一个条目时，预览面板会显示该条目的详细信息。这个功能在 macOS Finder 的列视图中对应的是最右侧的文件预览区域——选中一个文件后右侧显示文件图标、文件名、大小、修改日期等信息。

setPreviewWidget(QWidget *widget) 设置预览面板。传入的 QWidget 会被 QColumnView 放在所有列的右侧，占据一个独立的区域。你需要自己管理这个 QWidget 的内容——QColumnView 只负责布局和显示，不负责填充数据。

```cpp
// 创建预览面板
auto *previewWidget = new QWidget;
auto *previewLayout = new QVBoxLayout(previewWidget);
auto *previewLabel = new QLabel("选择一个条目查看详情");
previewLabel->setWordWrap(true);
previewLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
previewLayout->addWidget(previewLabel);

columnView->setPreviewWidget(previewWidget);
```

但仅仅设置一个静态的 QWidget 不够——你需要在用户选中不同的条目时更新预览内容。这就是 updatePreviewWidget(const QModelIndex &index) 虚函数的用途。QColumnView 在当前选中条目变化时会调用这个函数，传入新选中条目的 QModelIndex。你需要继承 QColumnView 并重写这个方法，在里面更新预览面板的内容。

```cpp
class PreviewColumnView : public QColumnView
{
    Q_OBJECT

public:
    explicit PreviewColumnView(QWidget *parent = nullptr)
        : QColumnView(parent)
    {
        m_previewLabel = new QLabel("选择一个条目查看详情");
        m_previewLabel->setWordWrap(true);
        m_previewLabel->setAlignment(
            Qt::AlignTop | Qt::AlignLeft);
        m_previewLabel->setMinimumWidth(200);

        auto *container = new QWidget;
        auto *layout = new QVBoxLayout(container);
        layout->addWidget(m_previewLabel);
        setPreviewWidget(container);
    }

protected:
    void updatePreviewWidget(
        const QModelIndex &index) override
    {
        if (!index.isValid()) {
            m_previewLabel->setText("选择一个条目查看详情");
            return;
        }

        QString name = model()->data(
            index, Qt::DisplayRole).toString();
        bool hasChildren = model()->hasChildren(index);

        QString info;
        info += QString("名称: %1\n").arg(name);
        info += QString("层级: %1\n")
                    .arg(getDepth(index));
        info += QString("是否有子项: %1\n")
                    .arg(hasChildren ? "是" : "否");

        // 读取自定义数据
        QVariant userData = model()->data(index, Qt::UserRole);
        if (userData.isValid()) {
            info += QString("附加信息: %1\n")
                        .arg(userData.toString());
        }

        m_previewLabel->setText(info);
    }

private:
    /// @brief 计算条目在树中的深度
    int getDepth(const QModelIndex &index) const
    {
        int depth = 0;
        QModelIndex parent = index.parent();
        while (parent.isValid()) {
            ++depth;
            parent = parent.parent();
        }
        return depth;
    }

    QLabel *m_previewLabel = nullptr;
};
```

上面这个 PreviewColumnView 在用户选中条目时自动更新预览面板，显示条目的名称、在树中的深度、是否有子项等信息。你可以在 updatePreviewWidget 中做任何你需要的操作——从 Model 中读取详细数据、查询数据库、加载文件信息，然后把结果更新到预览面板上。

预览面板的 QWidget 的生命周期由 QColumnView 管理——你在 setPreviewWidget 中传入的 QWidget 会在 QColumnView 析构时被自动 delete。不要在 QColumnView 析构后继续引用这个 QWidget。

预览面板只在用户选中的条目没有子节点（hasChildren 返回 false）时才会显示。如果选中的条目有子节点，QColumnView 会在右侧新开一列显示子节点，此时预览面板被"挤"到更右边的位置。这是 QColumnView 的设计逻辑——有子节点就展开子列，没子节点就显示预览。如果你的所有条目都没有子节点，预览面板会在选中任何条目时都可见。

### 3.4 典型应用场景：文件浏览器与设置面板导航

QColumnView 最典型的应用场景是文件浏览器。配合 QFileSystemModel，你可以用不到十行代码搭建一个 macOS Finder 风格的文件浏览器。QFileSystemModel 把文件系统的目录结构映射成层级 Model，QColumnView 把每一层目录渲染成一列——点击一个目录就进入下一层，最终到达文件时在预览面板中显示文件信息。

```cpp
auto *fsModel = new QFileSystemModel(this);
fsModel->setRootPath(QDir::homePath());
fsModel->setFilter(QDir::AllDirs | QDir::Files
                   | QDir::NoDotAndDotDot);

auto *columnView = new QColumnView;
columnView->setModel(fsModel);

// 只显示文件名列
for (int i = 1; i < fsModel->columnCount(); ++i) {
    columnView->hideColumn(i);
}
```

文件浏览器的核心交互是：用户点击一个目录，右侧新列显示目录内容；用户点击一个文件，预览面板显示文件详情。用 QColumnView 的 updatePreviewWidget 配合 QFileSystemModel 的 filePath(index)、fileName(index)、fileInfo(index) 方法，可以轻松实现这个交互。fileInfo 返回的 QFileInfo 对象提供了文件大小、后缀名、最后修改时间等所有你需要的信息。

另一个典型场景是设置面板导航。很多桌面应用的"设置"或"首选项"对话框采用左侧分类树 + 右侧设置页面的布局。用 QColumnView 可以更进一步——把分类层级用多列方式展示，最终在预览面板中显示对应分类的设置控件。比如系统设置的层级是"硬件 > 显示 > 分辨率"，用 QColumnView 的话用户依次在第一列点"硬件"，第二列点"显示"，右侧预览面板就展示分辨率设置页面。这种浏览方式比传统的 QTreeWidget + QStackedWidget 组合更直观，因为每一层的选择都是可见的，用户不需要在脑子里维护"我从哪来到哪去"的路径信息。

```cpp
// 设置面板场景：用 QStandardItemModel 构建设置层级
auto *settingsModel = new QStandardItemModel(this);

auto *general = new QStandardItem("通用");
auto *appearance = new QStandardItem("外观");
auto *network = new QStandardItem("网络");
settingsModel->appendRow({general, appearance, network});

// 通用设置子项
general->appendRow(new QStandardItem("语言"));
general->appendRow(new QStandardItem("启动"));
general->appendRow(new QStandardItem("更新"));

// 外观设置子项
auto *theme = new QStandardItem("主题");
auto *font = new QStandardItem("字体");
auto *icon = new QStandardItem("图标");
appearance->appendRow({theme, font, icon});

// 绑定到 QColumnView
auto *settingsView = new QColumnView;
settingsView->setModel(settingsModel);
settingsView->setPreviewWidget(createSettingsPanel());
```

在这两种场景中，QColumnView 的核心优势是相同的：把层级结构的每一层平铺展示，让用户对当前浏览路径一目了然。当你需要在有限的界面空间内展示一个深度不确定的层级结构，并且希望用户的浏览操作尽可能高效时，QColumnView 是一个值得考虑的选择。

当然 QColumnView 也有它的局限性。和 QTreeView 相比，当层级非常深（超过 5-6 层）时，QColumnView 需要的横向空间会变得很大——每一层占一列，6 层就是 6 列。在窄屏设备或者嵌入式界面上，这可能导致内容被挤到看不见。另外，QColumnView 不支持 QTreeView 的分支线（branch lines）、展开/折叠动画等视觉元素。如果你的数据层级不深（2-4 层）且横向空间充足，QColumnView 的体验通常优于 QTreeView。

## 4. 踩坑预防

第一个坑是 QColumnView 只适用于有层级关系的 Model。如果你给它绑定一个平铺的 QStringListModel 或者单层的 QStandardItemModel，它只会显示一列——因为没有 hasChildren 为 true 的条目，不会触发"钻入"行为。在开始使用 QColumnView 之前，先确认你的 Model 确实有层级结构。

第二个坑是 QColumnView 内部的每一列都是一个独立的 QAbstractItemView，但它们不直接暴露给你。如果你需要对某一列做特殊定制（比如给某一列设一个不同的 delegate），只能通过 QColumnView 的 setItemDelegate 统一设置，或者通过 createColumn 虚函数来拦截内部列的创建过程。createColumn(const QModelIndex &index) 在 QColumnView 需要为新层级创建一列时被调用，返回一个 QAbstractItemView *。你可以重写这个方法，在返回之前对列进行自定义配置。

第三个坑是 setPreviewWidget 的 QWidget 被放在所有列的右侧，而不是紧跟在最后一列之后。这意味着如果你有很多列，预览面板可能被挤到视口之外。在设计界面布局时需要考虑预览面板的最小宽度，必要时用 QSplitter 来手动分配列和预览面板的空间比例。

第四个坑是 updatePreviewWidget 在 Model 的数据变化时也会被调用。如果你在 updatePreviewWidget 中做了重量级操作（比如加载文件内容生成缩略图），需要考虑性能问题。建议在 updatePreviewWidget 中只更新元信息，延迟加载详细内容——或者用 QTimer::singleShot 做一个短暂的延迟，避免用户快速连续点击时触发大量加载操作。

第五个坑是 QColumnView 的 setColumnWidths 设置的宽度在用户交互后可能被覆盖。当用户在某一列中选中一个新的条目时，QColumnView 可能会自动调整列宽。如果你希望列宽保持固定，需要在适当的时候重新调用 setColumnWidths。

## 5. 练习项目

我们来做一个综合练习：创建一个"产品分类浏览器"窗口。中央是一个自定义的 PreviewColumnView（继承 QColumnView，重写 updatePreviewWidget），使用 QStandardItemModel 构建一个三层产品分类树：第一层是产品大类（电子产品、服装、食品），第二层是每个大类下的子分类，第三层是具体的产品。用户逐层点击分类时右侧逐列展示子项，最终选中一个产品时预览面板显示产品的详细信息（名称、分类路径、价格、描述）。预览面板使用 QLabel 显示格式化的产品信息卡片。窗口底部有一个 QLabel 显示当前选中条目的完整路径（类似面包屑导航），通过 clicked 信号更新。用 setColumnWidths 设置每列宽度为 150 像素，预览面板最小宽度 250 像素。

提示：在构建 QStandardItemModel 时，对叶子节点（产品）通过 setData(Qt::UserRole, QVariantMap) 存储产品的完整信息。updatePreviewWidget 中通过 model()->data(index, Qt::UserRole) 取出 QVariantMap，然后格式化显示。

## 6. 官方文档参考链接

[Qt 文档 -- QColumnView](https://doc.qt.io/qt-6/qcolumnview.html) -- 多列级联视图

[Qt 文档 -- QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- 抽象 View 基类

[Qt 文档 -- QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 标准 Model 实现

[Qt 文档 -- QFileSystemModel](https://doc.qt.io/qt-6/qfilesystemmodel.html) -- 文件系统 Model

[Qt 文档 -- Model/View Programming](https://doc.qt.io/qt-6/model-view-programming.html) -- Model/View 编程指南

---

到这里，QColumnView 的核心用法就全部讲完了。它用 macOS Finder 列视图的多列级联方式来展示层级数据，配合任何支持树形结构的 Model 就能跑起来。setModel 和层级数据绑定与 QTreeView 完全一致，updatePreviewWidget 让你在最右侧挂一个动态更新的预览面板，文件浏览器和设置面板导航是它最典型的应用场景。当你的数据层级不深、横向空间充足时，QColumnView 提供的浏览体验是 QTreeView 无法替代的。
