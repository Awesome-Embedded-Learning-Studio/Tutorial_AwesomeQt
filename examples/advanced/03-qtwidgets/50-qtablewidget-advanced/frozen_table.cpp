/// @file    frozen_table.cpp
/// @brief   FrozenTable 类的实现，单元格合并与冻结首行首列。

#include "frozen_table.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QResizeEvent>
#include <QScrollBar>
#include <QVBoxLayout>

namespace
{
    constexpr int kRowCount = 8;
    constexpr int kColCount = 8;
}

FrozenTable::FrozenTable(int rows, int columns, QWidget* parent)
    : QWidget(parent)
    , m_mainTable(new QTableWidget(rows, columns, this))
    , m_frozenTable(new QTableWidget(1, 1, this))
{
    // 主布局：直接将主表格铺满，冻结表格叠在上面
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainTable);

    setupMainTable();
    setupFrozenTable();
    updateFrozenTableGeometry();
}

void FrozenTable::setupMainTable()
{
    m_mainTable->setHorizontalHeaderLabels(
        QStringList{"项目", "Q1", "Q2", "Q3", "Q4", "合计", "备注", "评级"});
    m_mainTable->setVerticalHeaderLabels(
        QStringList{"产品A", "产品B", "产品C", "产品D",
                     "产品E", "产品F", "产品G", "产品H"});

    // 填充示例数据
    for (int row = 0; row < kRowCount; ++row) {
        for (int col = 0; col < kColCount; ++col) {
            auto* item = new QTableWidgetItem;
            if (col >= 1 && col <= 4) {
                item->setText(QString::number((row + 1) * (col + 1) * 100));
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            } else if (col == 5) {
                // 合计列：简单的求和演示
                int sum = 0;
                for (int q = 1; q <= 4; ++q) {
                    sum += (row + 1) * (q + 1) * 100;
                }
                item->setText(QString::number(sum));
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            } else if (col == 6) {
                item->setText("正常");
            } else if (col == 7) {
                item->setText(row < 3 ? "A" : (row < 6 ? "B" : "C"));
                item->setTextAlignment(Qt::AlignCenter);
            }
            m_mainTable->setItem(row, col, item);
        }
    }

    // 使用 setSpan 合并"项目"列中产品A和产品B的单元格，演示合并效果
    m_mainTable->setSpan(0, 0, 2, 1);
    // 合并备注列中产品E和产品F
    m_mainTable->setSpan(4, 6, 2, 1);
    // 被合并的首单元格内容覆盖子区域
    m_mainTable->item(4, 6)->setText("合并区域");

    m_mainTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    m_mainTable->verticalHeader()->setDefaultSectionSize(30);
    m_mainTable->setWindowTitle("QTableWidget 冻结首行首列 + 合并单元格");
}

void FrozenTable::setupFrozenTable()
{
    // 冻结表格只显示主表的第 0 行和第 0 列交汇的单元格
    m_frozenTable->setParent(m_mainTable->viewport());
    m_frozenTable->setFocusPolicy(Qt::NoFocus);
    m_frozenTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_frozenTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_frozenTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_frozenTable->setSelectionModel(m_mainTable->selectionModel());
    m_frozenTable->setHorizontalHeaderLabels(QStringList{"项目"});
    m_frozenTable->setVerticalHeaderLabels(QStringList{"产品A"});
    m_frozenTable->setFrameStyle(QFrame::NoFrame);

    // 复制左上角单元格数据
    auto* sourceItem = m_mainTable->item(0, 0);
    if (sourceItem) {
        auto* frozenItem = new QTableWidgetItem(*sourceItem);
        m_frozenTable->setItem(0, 0, frozenItem);
    }

    // 行高/列宽同步：用户拖拽主表列宽时，冻结表必须跟随
    connect(m_mainTable->horizontalHeader(), &QHeaderView::sectionResized,
            this, &FrozenTable::syncColumnWidths);
    connect(m_mainTable->verticalHeader(), &QHeaderView::sectionResized,
            this, &FrozenTable::syncRowHeight);

    // 滚动时冻结表不跟着滚，只需要同步选中高亮
    m_frozenTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeMode::Fixed);
    m_frozenTable->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeMode::Fixed);
}

void FrozenTable::updateFrozenTableGeometry()
{
    // 冻结表格定位在主表 viewport 左上角，尺寸取第 0 行行高和第 0 列列宽
    const int colWidth = m_mainTable->columnWidth(0);
    const int rowHeight = m_mainTable->rowHeight(0);
    const int headerHeight = m_mainTable->horizontalHeader()->height();
    const int headerWidth = m_mainTable->verticalHeader()->width();

    m_frozenTable->setGeometry(headerWidth, headerHeight, colWidth, rowHeight);
    m_frozenTable->setColumnWidth(0, colWidth);
    m_frozenTable->setRowHeight(0, rowHeight);
}

void FrozenTable::syncColumnWidths(int column, int /*oldWidth*/, int newWidth)
{
    // 只需同步第 0 列的宽度到冻结表格
    if (column == 0) {
        m_frozenTable->setColumnWidth(0, newWidth);
        updateFrozenTableGeometry();
    }
}

void FrozenTable::syncRowHeight(int row, int /*oldHeight*/, int newHeight)
{
    // 只需同步第 0 行的高度到冻结表格
    if (row == 0) {
        m_frozenTable->setRowHeight(0, newHeight);
        updateFrozenTableGeometry();
    }
}

void FrozenTable::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateFrozenTableGeometry();
}
