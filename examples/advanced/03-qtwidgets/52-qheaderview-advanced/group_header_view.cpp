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
    : QHeaderView(orientation, parent) {
    // 必须可点击才能触发排序
    setSectionsClickable(true);
    setSortIndicatorShown(true);
    setSectionsMovable(false);

    connect(this, &QHeaderView::sectionResized, this, &GroupHeaderView::onSectionResized);
}

void GroupHeaderView::setHeaderGroups(const std::vector<HeaderGroup>& groups) {
    m_groups = groups;
    // 分组行需要额外高度，触发 sizeHint 重算
    updateGeometry();
    viewport()->update();
}

QSize GroupHeaderView::sizeHint() const {
    // 在默认高度基础上加上分组行的高度
    QSize base = QHeaderView::sizeHint();
    base.setHeight(base.height() + kGroupHeaderHeight);
    return base;
}

void GroupHeaderView::paintEvent(QPaintEvent* event) {
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
    // 标准列标题由基类 paintEvent 画在 y=0 起的原位；
    // 本视图通过 sizeHint 抬高表头、分组行画在顶部实现双层布局，
    // 这里不额外覆盖背景。
}

void GroupHeaderView::onSectionResized(int /*logicalIndex*/) {
    // 列宽变化后需要完整重绘，因为分组跨度取决于底层列宽
    viewport()->update();
}

GroupHeaderWindow::GroupHeaderWindow(QWidget* parent)
    : QMainWindow(parent), m_model(new QStandardItemModel(this)) {
    // 设置模型列标题
    m_model->setHorizontalHeaderLabels(QStringList{"Name", "Age", "Email", "Phone"});
    m_model->setRowCount(6);

    populateModel();

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    auto* tableView = new QTableView(this);

    // 使用排序代理模型；注意本示例未调 setSortingEnabled，
    // 点击表头只会翻转排序指示箭头，不会真正触发排序
    // （接线补法见教程正文结语实验）
    auto* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_model);
    tableView->setModel(proxyModel);

    // 安装自定义分组表头
    auto* header = new GroupHeaderView(Qt::Horizontal, tableView);
    tableView->setHorizontalHeader(header);

    // 定义分组："Personal Info" 跨 Name+Age，"Contact" 跨 Email+Phone
    header->setHeaderGroups({
        {"Personal Info", 0, 2},
        {"Contact", 2, 2},
    });

    // 行高与交替色只是外观设置；表头绘制下移由视图自身的几何与 paintEvent 安排，
    // 与这里的两行无关
    tableView->verticalHeader()->setDefaultSectionSize(28);
    tableView->setAlternatingRowColors(true);

    layout->addWidget(tableView);
    setCentralWidget(central);

    setWindowTitle("QHeaderView 双级表头与自定义排序");
    resize(700, 400);
}

void GroupHeaderWindow::populateModel() {
    // 填充几行示例数据
    struct Person {
        const char* name;
        int age;
        const char* email;
        const char* phone;
    };

    const Person people[] = {
        {"Alice", 28, "alice@example.com", "123-4567"},
        {"Bob", 35, "bob@example.com", "234-5678"},
        {"Charlie", 22, "charlie@example.com", "345-6789"},
        {"Diana", 30, "diana@example.com", "456-7890"},
        {"Eve", 27, "eve@example.com", "567-8901"},
        {"Frank", 40, "frank@example.com", "678-9012"},
    };

    for (int i = 0; i < 6; ++i) {
        m_model->setItem(i, 0, new QStandardItem(people[i].name));
        m_model->setItem(i, 1, new QStandardItem(QString::number(people[i].age)));
        m_model->setItem(i, 2, new QStandardItem(people[i].email));
        m_model->setItem(i, 3, new QStandardItem(people[i].phone));
    }
}
