/// @file    widget.cpp
/// @brief   演示 QListWidget 拖放排序与自定义 ItemWidget 的实现。
///
/// 对应教程：进阶层 03-QtWidgets/46-qlistwidget-advanced。

#include "widget.h"

#include <QHBoxLayout>

// ---------------------------------------------------------------------------
// CardWidget
// ---------------------------------------------------------------------------

CardWidget::CardWidget(const QString& icon, const QString& title,
                       QListWidget* listWidget, QWidget* parent)
    : QWidget(parent), m_listWidget(listWidget)
{
    auto* layout = new QHBoxLayout(this);
    // 紧凑布局，减少内边距让卡片看起来更紧凑
    layout->setContentsMargins(4, 2, 4, 2);

    auto* iconLabel = new QLabel(icon, this);
    iconLabel->setFixedWidth(30);
    // 设置字体大小以显示 emoji 图标
    QFont iconFont = iconLabel->font();
    iconFont.setPointSize(16);
    iconLabel->setFont(iconFont);

    m_titleLabel = new QLabel(title, this);
    // 让标题占据剩余空间
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* removeBtn = new QPushButton(QStringLiteral("X"), this);
    removeBtn->setFixedSize(28, 28);
    // @note 连接按钮的 clicked 信号到本类的移除槽
    connect(removeBtn, &QPushButton::clicked, this, &CardWidget::onRemoveClicked);

    layout->addWidget(iconLabel);
    layout->addWidget(m_titleLabel, 1);
    layout->addWidget(removeBtn);
}

void CardWidget::onRemoveClicked()
{
    // @note 通过 parent() 获取 CardWidget 拥有的 QListWidgetItem 的 index widget，
    // 然后用 itemAt 定位到具体的 QListWidgetItem 并删除
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        if (m_listWidget->itemWidget(item) == this) {
            // takeItem 移除项，Qt 自动回收 itemWidget（它是 item 的子对象）
            delete m_listWidget->takeItem(i);
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Widget
// ---------------------------------------------------------------------------

Widget::Widget(QWidget* parent)
    : QWidget(parent), m_listWidget(new QListWidget(this)),
      m_statusLabel(new QLabel(QStringLiteral("选中行: 无"), this)),
      m_toggleBtn(new QPushButton(QStringLiteral("切换为 Snap 模式"), this)),
      m_freeMovement(true)
{
    auto* mainLayout = new QVBoxLayout(this);

    // -- 配置 QListWidget 的拖放行为 --
    // @note InternalMove 允许用户在列表内部拖动项来重新排序
    m_listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    // @note 默认使用 Free 移动模式：项可以放到任意像素位置
    m_listWidget->setMovement(QListView::Free);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    // 允许在网格中自由拖放
    m_listWidget->setGridSize(QSize(0, 40));
    // @note setResizeMode(Adjust) 在窗口大小变化时重新排列项
    m_listWidget->setResizeMode(QListView::Adjust);

    // 添加示例卡片数据
    addCard(QStringLiteral("\xF0\x9F\x93\x8C"), QStringLiteral("项目 A - 设计稿"));   // 📌
    addCard(QStringLiteral("\xF0\x9F\x93\x8B"), QStringLiteral("项目 B - 待审核"));   // 📋
    addCard(QStringLiteral("\xF0\x9F\x9A\x80"), QStringLiteral("项目 C - 进行中"));   // 🚀
    addCard(QStringLiteral("\xE2\x9C\x85"), QStringLiteral("项目 D - 已完成"));       // ✅
    addCard(QStringLiteral("\xF0\x9F\x94\xA7"), QStringLiteral("项目 E - 维护中"));   // 🔧

    // -- 连接 currentRowChanged 以展示选中反馈 --
    connect(m_listWidget, &QListWidget::currentRowChanged, this,
            &Widget::onCurrentRowChanged);

    // -- 切换 Movement 模式的按钮 --
    connect(m_toggleBtn, &QPushButton::clicked, this, &Widget::toggleMovementMode);

    // -- 组装布局 --
    mainLayout->addWidget(m_toggleBtn);
    mainLayout->addWidget(m_listWidget, 1);
    mainLayout->addWidget(m_statusLabel);

    setWindowTitle(QStringLiteral("QListWidget 拖放排序与自定义 ItemWidget"));
    resize(420, 400);
}

void Widget::addCard(const QString& icon, const QString& title)
{
    // @note 先创建 QListWidgetItem，设置 sizeHint 以容纳自定义控件
    auto* item = new QListWidgetItem(m_listWidget);
    // itemWidget 接管尺寸，但 sizeHint 仍需要合理初始值
    item->setSizeHint(QSize(0, 40));

    auto* card = new CardWidget(icon, title, m_listWidget, this);
    // @note setItemWidget 将自定义 QWidget 挂载到列表项上
    m_listWidget->setItemWidget(item, card);
}

void Widget::toggleMovementMode()
{
    if (m_freeMovement) {
        // @note Snap 模式下，项只能吸附到网格位置，行为更规整
        m_listWidget->setMovement(QListView::Snap);
        m_toggleBtn->setText(QStringLiteral("切换为 Free 模式"));
        m_freeMovement = false;
    } else {
        // @note Free 模式下，项可以放到任意像素位置
        m_listWidget->setMovement(QListView::Free);
        m_toggleBtn->setText(QStringLiteral("切换为 Snap 模式"));
        m_freeMovement = true;
    }
}

void Widget::onCurrentRowChanged(int row)
{
    if (row < 0) {
        m_statusLabel->setText(QStringLiteral("选中行: 无"));
    } else {
        QListWidgetItem* item = m_listWidget->item(row);
        if (item) {
            auto* card = qobject_cast<CardWidget*>(m_listWidget->itemWidget(item));
            if (card) {
                m_statusLabel->setText(
                    QStringLiteral("选中行: %1").arg(row));
            }
        }
    }
}
