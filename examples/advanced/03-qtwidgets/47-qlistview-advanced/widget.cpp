/// @file    widget.cpp
/// @brief   演示 QListView 大数据虚拟列表与增量加载的实现。
///
/// 对应教程：进阶层 03-QtWidgets/47-qlistview-advanced。

#include "widget.h"

#include <QScrollBar>
#include <QTimer>

// ---------------------------------------------------------------------------
// LargeDataModel
// ---------------------------------------------------------------------------

LargeDataModel::LargeDataModel(int totalItems, int batchSize, QObject* parent)
    : QAbstractListModel(parent), m_totalItems(totalItems), m_batchSize(batchSize),
      m_loadedCount(0)
{
    // @note 预分配空间以提高首次加载性能，但不填数据
    m_data.reserve(m_batchSize);
}

int LargeDataModel::rowCount(const QModelIndex& parent) const
{
    // 列表模型的 parent 应始终无效
    if (parent.isValid()) {
        return 0;
    }
    return m_loadedCount;
}

QVariant LargeDataModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_loadedCount) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        // @note 从缓存中取数据；由于 fetchMore 时已生成，此处 O(1) 访问
        return m_data.at(index.row());
    }

    if (role == Qt::ToolTipRole) {
        return QStringLiteral("行 #%1").arg(index.row());
    }

    return QVariant();
}

bool LargeDataModel::canFetchMore(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return false;
    }
    // @note 只要已加载数量小于总量，就还有更多数据可加载
    return m_loadedCount < m_totalItems;
}

void LargeDataModel::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid()) {
        return;
    }

    // 计算本批要加载的数量，不超过总量
    int remaining = m_totalItems - m_loadedCount;
    int itemsToFetch = qMin(m_batchSize, remaining);

    if (itemsToFetch <= 0) {
        return;
    }

    // @note 必须使用 beginInsertRows / endInsertRows 通知视图增量更新
    beginInsertRows(QModelIndex(), m_loadedCount, m_loadedCount + itemsToFetch - 1);

    for (int i = 0; i < itemsToFetch; ++i) {
        int idx = m_loadedCount + i;
        m_data.append(QStringLiteral("数据项 #%1").arg(idx));
    }
    m_loadedCount += itemsToFetch;

    endInsertRows();
}

int LargeDataModel::totalItemCount() const
{
    return m_totalItems;
}

int LargeDataModel::loadedItemCount() const
{
    return m_loadedCount;
}

// ---------------------------------------------------------------------------
// Widget
// ---------------------------------------------------------------------------

Widget::Widget(QWidget* parent)
    : QWidget(parent),
      m_model(new LargeDataModel(100000, 1000, this)),
      m_listView(new QListView(this)),
      m_statusLabel(new QLabel(this)),
      m_loadAllBtn(new QPushButton(QStringLiteral("一次性加载全部数据"), this))
{
    auto* mainLayout = new QVBoxLayout(this);

    // -- 配置 QListView --
    // @note QListView 默认就是虚拟滚动：只为可见行创建 delegate widget
    m_listView->setModel(m_model);
    // UniformItemSizes 优化：告知视图所有项尺寸一致，可跳过布局计算
    m_listView->setUniformItemSizes(true);
    // 开启批量加载——视图滚动到底部时自动调用 canFetchMore/fetchMore
    m_listView->setBatchSize(50);

    // -- 连接信号 --
    connect(m_model, &QAbstractItemModel::rowsInserted, this, &Widget::onDataChanged);
    connect(m_loadAllBtn, &QPushButton::clicked, this, &Widget::onLoadAll);

    // @note 使用定时器定期刷新状态标签，以显示可见范围变化
    auto* statusTimer = new QTimer(this);
    statusTimer->setInterval(200);
    connect(statusTimer, &QTimer::timeout, this, &Widget::updateStatusLabel);
    statusTimer->start();

    // -- 组装布局 --
    auto* topInfo = new QLabel(
        QStringLiteral("模型包含 100,000 条数据，初始加载 1,000 条，"
                        "滚动到底部自动增量加载"), this);
    topInfo->setWordWrap(true);

    mainLayout->addWidget(topInfo);
    mainLayout->addWidget(m_loadAllBtn);
    mainLayout->addWidget(m_listView, 1);
    mainLayout->addWidget(m_statusLabel);

    setWindowTitle(QStringLiteral("QListView 大数据虚拟列表优化"));
    resize(400, 500);
}

void Widget::updateStatusLabel()
{
    int total = m_model->totalItemCount();
    int loaded = m_model->loadedItemCount();

    // @note 通过 QListView::indexAt 获取视口顶部和底部的模型索引
    QModelIndex firstIdx = m_listView->indexAt(QPoint(0, 0));
    QModelIndex lastIdx = m_listView->indexAt(QPoint(0, m_listView->viewport()->height() - 1));

    int firstVisible = firstIdx.isValid() ? firstIdx.row() : 0;
    int lastVisible = lastIdx.isValid() ? lastIdx.row() : (loaded > 0 ? loaded - 1 : 0);

    m_statusLabel->setText(
        QStringLiteral("已加载: %1 / %2  |  可见范围: [%3..%4]")
            .arg(loaded)
            .arg(total)
            .arg(firstVisible)
            .arg(lastVisible));
}

void Widget::onDataChanged()
{
    updateStatusLabel();
}

void Widget::onLoadAll()
{
    // @note 一次性加载全部数据，用于对比虚拟滚动的内存优势
    while (m_model->canFetchMore()) {
        m_model->fetchMore();
    }
    m_loadAllBtn->setEnabled(false);
    m_loadAllBtn->setText(QStringLiteral("已全部加载"));
}
