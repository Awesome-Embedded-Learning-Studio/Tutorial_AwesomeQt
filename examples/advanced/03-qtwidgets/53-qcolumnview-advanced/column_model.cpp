/// @file    column_model.cpp
/// @brief   PreviewPanel 和 ColumnViewWindow 的实现。

#include "column_model.h"

#include <QColumnView>
#include <QHeaderView>
#include <QScrollArea>

PreviewPanel::PreviewPanel(QWidget* parent)
    : QWidget(parent)
    , m_titleLabel(new QLabel("Select an item", this))
    , m_typeLabel(new QLabel("", this))
    , m_descLabel(new QLabel("", this))
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    // 标题用大号粗体字
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setWordWrap(true);

    m_typeLabel->setStyleSheet("color: gray;");
    m_descLabel->setWordWrap(true);
    m_descLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_typeLabel);
    layout->addSpacing(10);
    layout->addWidget(m_descLabel);
    layout->addStretch();

    setMinimumWidth(200);
}

void PreviewPanel::onSelectionChanged(const QModelIndex& index)
{
    if (!index.isValid()) {
        m_titleLabel->setText("Select an item");
        m_typeLabel->setText("");
        m_descLabel->setText("");
        return;
    }

    // 从模型取出显示名称和自定义 type 角色
    QString name = index.data(Qt::DisplayRole).toString();
    QString type = index.data(Qt::UserRole).toString();

    m_titleLabel->setText(name);
    m_typeLabel->setText("Type: " + type);
    m_descLabel->setText(descriptionForType(type));
}

QString PreviewPanel::descriptionForType(const QString& type) const
{
    // 根据分类类型返回不同描述
    if (type == "category") {
        return "This is a top-level category. Click to expand and see "
               "its sub-items in the next column.";
    }
    if (type == "document") {
        return "A document file (.txt, .md, .pdf). Double-click to open "
               "in the default editor.";
    }
    if (type == "image") {
        return "An image file (.png, .jpg, .svg). Preview is available "
               "in the detail pane.";
    }
    if (type == "source") {
        return "Source code file (.cpp, .h, .py). Contains program "
               "logic and definitions.";
    }
    return "A file system item.";
}

ColumnViewWindow::ColumnViewWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_model(new QStandardItemModel(this))
    , m_preview(new PreviewPanel(this))
{
    buildModel();

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    auto* columnView = new QColumnView(this);
    columnView->setModel(m_model);

    // 设置自定义列宽：第一列宽 200，第二列 180，第三列 160
    columnView->setColumnWidths({200, 180, 160});

    // 将预览面板设为 QColumnView 的预览组件
    // 预览组件会出现在最右侧列，当用户选中叶子节点时显示
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidget(m_preview);
    scrollArea->setWidgetResizable(true);
    columnView->setPreviewWidget(scrollArea);

    // 选中项变化时更新预览
    connect(columnView->selectionModel(), &QItemSelectionModel::currentChanged,
            m_preview, &PreviewPanel::onSelectionChanged);

    layout->addWidget(columnView);
    setCentralWidget(central);

    setWindowTitle("QColumnView 自定义列宽与预览组件");
    resize(800, 500);
}

void ColumnViewWindow::buildModel()
{
    m_model->clear();
    m_model->setHorizontalHeaderLabels(QStringList{"Name"});

    // 顶层分类
    struct Category
    {
        QString name;
        std::vector<std::pair<QString, QString>> children;
    };

    const Category categories[] = {
        {"Documents", {{"readme.txt", "document"}, {"notes.md", "document"},
                       {"report.pdf", "document"}}},
        {"Images",    {{"photo.png", "image"}, {"icon.svg", "image"},
                       {"banner.jpg", "image"}}},
        {"Source",    {{"main.cpp", "source"}, {"widget.h", "source"},
                       {"utils.py", "source"}}},
        {"Misc",      {{"config.ini", "document"}, {"data.csv", "document"}}},
    };

    for (const auto& cat : categories) {
        auto* parentItem = new QStandardItem(cat.name);
        parentItem->setData("category", Qt::UserRole);

        addChildren(parentItem, cat.children);
        m_model->appendRow(parentItem);
    }
}

void ColumnViewWindow::addChildren(
    QStandardItem* parent,
    const std::vector<std::pair<QString, QString>>& children)
{
    for (const auto& child : children) {
        auto* item = new QStandardItem(child.first);
        item->setData(child.second, Qt::UserRole);
        parent->appendRow(item);
    }
}
