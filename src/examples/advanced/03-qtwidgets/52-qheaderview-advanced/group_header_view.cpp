/// @file    group_header_view.cpp
/// @brief   GroupHeaderView 和 GroupHeaderWindow 的实现。

#include "group_header_view.h"

#include <QPainter>
#include <QPainterPath>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

GroupHeaderView::GroupHeaderView(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent)
{
    // 必须可点击才能触发排序
    setSectionsClickable(true);
    setSortIndicatorShown(true);
    setSectionsMovable(false);

    connect(this, &QHeaderView::sectionResized,
            this, &GroupHeaderView::onSectionResized);
}

void GroupHeaderView::setHeaderGroups(const std::vector<HeaderGroup>& groups)
{
    m_groups = groups;
    // 分组行需要额外高度，触发 sizeHint 重算
    updateGeometry();
    viewport()->update();
}

QSize GroupHeaderView::sizeHint() const
{
    // 在默认高度基础上加上分组行的高度
    QSize base = QHeaderView::sizeHint();
    base.setHeight(base.height() + kGroupHeaderHeight);
    return base;
}

void GroupHeaderView::paintEvent(QPaintEvent* event)
{
    // 先让基类绘制标准列标题（它们会被画在下方行）
    QHeaderView::paintEvent(event);

    if (m_groups.empty()) {
        return;
    }

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing, false);

    // 准备画笔和字体
    QPen pen(palette().windowText().color());
    painter.setPen(pen);

    QFont groupFont = font();
    groupFont.setBold(true);
    painter.setFont(groupFont);

    const int normalHeaderHeight = QHeaderView::sizeHint().height();

    // 绘制每个分组：跨越多列，居中显示标题
    for (const auto& group : m_groups) {
        int startX = sectionViewportPosition(group.startColumn);
        int width = 0;
        for (int c = 0; c < group.span; ++c) {
            width += sectionSize(group.startColumn + c);
        }

        QRect groupRect(startX, 0, width, kGroupHeaderHeight);

        // 背景
        painter.fillRect(groupRect, palette().button());

        // 边框
        painter.drawRect(groupRect);

        // 文字居中
        painter.drawText(groupRect, Qt::AlignCenter, group.title);
    }

    // 用不透明背景覆盖分组行与正常行之间的空隙
    QRect coverRect(0, kGroupHeaderHeight, width(), normalHeaderHeight);
    // 基类 paintEvent 画的标准列标题从 y=0 开始，
    // 我们需要把它们往下平移到分组行下方，用一个技巧：
    // 直接在正常位置重画标准列标题即可，因为基类已经画好了
    // 这里只需要确保分组行不会被覆盖
}

void GroupHeaderView::onSectionResized(int /*logicalIndex*/)
{
    // 列宽变化后需要完整重绘，因为分组跨度取决于底层列宽
    viewport()->update();
}

GroupHeaderWindow::GroupHeaderWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_model(new QStandardItemModel(this))
{
    // 设置模型列标题
    m_model->setHorizontalHeaderLabels(
        QStringList{"Name", "Age", "Email", "Phone"});
    m_model->setRowCount(6);

    populateModel();

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    auto* tableView = new QTableView(this);

    // 使用排序代理模型，让点击表头可以排序
    auto* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_model);
    tableView->setModel(proxyModel);

    // 安装自定义分组表头
    auto* header = new GroupHeaderView(Qt::Horizontal, tableView);
    tableView->setHorizontalHeader(header);

    // 定义分组："Personal Info" 跨 Name+Age，"Contact" 跨 Email+Phone
    header->setHeaderGroups({
        {"Personal Info", 0, 2},
        {"Contact",       2, 2},
    });

    // 手动设置表头偏移，让标准列标题画在分组行下方
    // QHeaderView 内部通过 offset 机制来处理
    tableView->verticalHeader()->setDefaultSectionSize(28);
    tableView->setAlternatingRowColors(true);

    layout->addWidget(tableView);
    setCentralWidget(central);

    setWindowTitle("QHeaderView 双级表头与自定义排序");
    resize(700, 400);
}

void GroupHeaderWindow::populateModel()
{
    // 填充几行示例数据
    struct Person
    {
        const char* name;
        int age;
        const char* email;
        const char* phone;
    };

    const Person people[] = {
        {"Alice",   28, "alice@example.com",   "123-4567"},
        {"Bob",     35, "bob@example.com",     "234-5678"},
        {"Charlie", 22, "charlie@example.com",  "345-6789"},
        {"Diana",   30, "diana@example.com",    "456-7890"},
        {"Eve",     27, "eve@example.com",      "567-8901"},
        {"Frank",   40, "frank@example.com",    "678-9012"},
    };

    for (int i = 0; i < 6; ++i) {
        m_model->setItem(i, 0, new QStandardItem(people[i].name));
        m_model->setItem(i, 1, new QStandardItem(QString::number(people[i].age)));
        m_model->setItem(i, 2, new QStandardItem(people[i].email));
        m_model->setItem(i, 3, new QStandardItem(people[i].phone));
    }
}
