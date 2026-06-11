/// @file    widget.cpp
/// @brief   演示 QTreeView 自定义展开图标与整行选中的实现。
///
/// 对应教程：进阶层 03-QtWidgets/49-qtreeview-advanced。

#include "widget.h"

#include <QPainterPath>
#include <QScrollBar>
#include <QStyleOptionViewItem>

// ---------------------------------------------------------------------------
// FullRowDelegate
// ---------------------------------------------------------------------------

FullRowDelegate::FullRowDelegate(QTreeView* treeView, QObject* parent)
    : QStyledItemDelegate(parent), m_treeView(treeView)
{
}

void FullRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    painter->save();

    // -- 1. 整行选中高亮 --
    // @note 默认行为只高亮第一列；此处对非首列也绘制选中背景
    if (option.state & QStyle::State_Selected) {
        // @note 使用整行宽度（viewport 宽度）绘制高亮背景
        QRect fullRowRect(0, option.rect.y(),
                          m_treeView->viewport()->width(), option.rect.height());
        painter->fillRect(fullRowRect, option.palette.highlight());
    } else if (option.state & QStyle::State_MouseOver) {
        // 鼠标悬停时绘制浅色背景
        QRect hoverRect(0, option.rect.y(),
                        m_treeView->viewport()->width(), option.rect.height());
        QColor hoverColor = option.palette.highlight().color();
        hoverColor.setAlpha(30);
        painter->fillRect(hoverRect, hoverColor);
    }

    // -- 2. 绘制展开三角形（仅第一列且有子节点时） --
    bool hasChildren = index.model() ? index.model()->hasChildren(index) : false;
    if (index.column() == 0 && hasChildren) {
        // 计算缩进：根据层级深度
        int indent = 0;
        QModelIndex walkParent = index.parent();
        while (walkParent.isValid()) {
            indent += m_treeView->indentation();
            walkParent = walkParent.parent();
        }
        indent += m_treeView->indentation();

        // @note 三角形绘制在缩进区域中
        int triangleSize = 10;
        int triangleX = indent - triangleSize - 4;
        int triangleY = option.rect.y() + (option.rect.height() - triangleSize) / 2;

        bool expanded = m_treeView->isExpanded(index);
        QRect triangleRect(triangleX, triangleY, triangleSize, triangleSize);
        drawExpandTriangle(painter, triangleRect, expanded);
    }

    // -- 3. 绘制文本 --
    QRect textRect = option.rect.adjusted(
        index.column() == 0 ? 4 : 6, 0, -4, 0);

    QColor textColor = (option.state & QStyle::State_Selected)
                           ? option.palette.highlightedText().color()
                           : option.palette.text().color();
    painter->setPen(textColor);
    painter->setFont(option.font);

    QString displayText = index.data(Qt::DisplayRole).toString();
    // @note 使用 ElidedRight 防止文本超出列宽
    QString elidedText = option.fontMetrics.elidedText(
        displayText, Qt::ElideRight, textRect.width());
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elidedText);

    painter->restore();
}

QSize FullRowDelegate::sizeHint(const QStyleOptionViewItem& option,
                                const QModelIndex& index) const
{
    // @note 在默认 sizeHint 基础上增加行高，让三角形图标更清晰
    QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
    baseSize.setHeight(qMax(baseSize.height(), 28));
    return baseSize;
}

void FullRowDelegate::drawExpandTriangle(QPainter* painter, const QRect& rect,
                                         bool expanded) const
{
    // @note 使用 QPainterPath 绘制实心三角形
    QPainterPath path;
    int margin = 2;
    int x = rect.x() + margin;
    int y = rect.y() + margin;
    int w = rect.width() - 2 * margin;
    int h = rect.height() - 2 * margin;

    if (expanded) {
        // 展开状态：向下的三角形 ▼
        path.moveTo(x, y);
        path.lineTo(x + w, y);
        path.lineTo(x + w / 2, y + h);
        path.closeSubpath();
    } else {
        // 折叠状态：向右的三角形 ▶
        path.moveTo(x, y);
        path.lineTo(x + w, y + h / 2);
        path.lineTo(x, y + h);
        path.closeSubpath();
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(80, 80, 80));
    painter->drawPath(path);
}

// ---------------------------------------------------------------------------
// Widget
// ---------------------------------------------------------------------------

Widget::Widget(QWidget* parent)
    : QWidget(parent),
      m_model(new TreeModel(this)),
      m_treeView(new QTreeView(this)),
      m_delegate(nullptr),
      m_statusLabel(new QLabel(QStringLiteral("点击节点查看信息"), this))
{
    auto* mainLayout = new QVBoxLayout(this);

    // -- 配置 QTreeView --
    m_treeView->setModel(m_model);

    // @note 安装自定义委托，替换默认的展开图标和选中行为
    m_delegate = new FullRowDelegate(m_treeView, this);
    m_treeView->setItemDelegate(m_delegate);

    // @note 关闭默认的分支绘制，避免与自定义三角形冲突
    m_treeView->setRootIsDecorated(false);

    // 启用鼠标追踪以支持 hover 效果
    m_treeView->setMouseTracking(true);

    // 默认展开前两层
    m_treeView->expandToDepth(1);

    // -- 连接选中变化信号 --
    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &Widget::onSelectionChanged);

    // -- 组装布局 --
    mainLayout->addWidget(m_treeView, 1);
    mainLayout->addWidget(m_statusLabel);

    setWindowTitle(QStringLiteral("QTreeView 自定义展开图标与整行选中"));
    resize(500, 450);
}

void Widget::onSelectionChanged(const QModelIndex& current)
{
    if (!current.isValid()) {
        m_statusLabel->setText(QStringLiteral("无选中节点"));
        return;
    }

    // @note 递归向上构建节点路径
    QStringList pathParts;
    QModelIndex idx = current;
    while (idx.isValid()) {
        pathParts.prepend(idx.data(Qt::DisplayRole).toString());
        idx = idx.parent();
    }

    m_statusLabel->setText(
        QStringLiteral("选中: %1").arg(pathParts.join(" > ")));
}
