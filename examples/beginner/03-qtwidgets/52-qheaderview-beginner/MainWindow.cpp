#include "MainWindow.h"

#include "ColoredHeaderView.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QHeaderView 综合演示 — 商品清单");
    resize(900, 520);
    initUi();
    populateProductData();
}

/// @brief 初始化界面
void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainHLayout = new QHBoxLayout(centralWidget);

    // ================================================================
    // 左侧：QTableView + 自定义表头
    // ================================================================
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels(
        {"编号", "名称", "分类", "单价", "库存", "备注"});

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);

    m_tableView = new QTableView;
    m_tableView->setModel(m_proxyModel);
    m_tableView->setSortingEnabled(true);
    m_tableView->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    m_tableView->verticalHeader()->hide();

    // 安装自定义表头
    auto *customHeader =
        new ColoredHeaderView(Qt::Horizontal, m_tableView);
    m_tableView->setHorizontalHeader(customHeader);
    customHeader->setSortIndicator(0, Qt::AscendingOrder);

    // 默认列宽策略：混合模式
    customHeader->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    customHeader->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    customHeader->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);
    customHeader->setSectionResizeMode(
        3, QHeaderView::ResizeToContents);
    customHeader->setSectionResizeMode(
        4, QHeaderView::ResizeToContents);
    customHeader->setStretchLastSection(true);

    mainHLayout->addWidget(m_tableView, 1);

    // ================================================================
    // 右侧：控制面板
    // ================================================================
    auto *rightPanel = new QWidget;
    rightPanel->setFixedWidth(220);
    auto *rightLayout = new QVBoxLayout(rightPanel);

    // -- 列宽策略切换 --
    rightLayout->addWidget(
        new QLabel("<b>列宽策略</b>"));

    auto *fixedBtn = new QPushButton("固定模式 (Fixed)");
    auto *contentsBtn =
        new QPushButton("自适应 (ResizeToContents)");
    auto *stretchBtn = new QPushButton("拉伸模式 (Stretch)");
    auto *interactiveBtn =
        new QPushButton("交互模式 (Interactive)");

    connect(fixedBtn, &QPushButton::clicked, this, [this]() {
        applyResizeMode(QHeaderView::Fixed);
    });
    connect(contentsBtn, &QPushButton::clicked, this, [this]() {
        applyResizeMode(QHeaderView::ResizeToContents);
    });
    connect(stretchBtn, &QPushButton::clicked, this, [this]() {
        applyResizeMode(QHeaderView::Stretch);
    });
    connect(interactiveBtn, &QPushButton::clicked, this, [this]() {
        applyResizeMode(QHeaderView::Interactive);
    });

    rightLayout->addWidget(fixedBtn);
    rightLayout->addWidget(contentsBtn);
    rightLayout->addWidget(stretchBtn);
    rightLayout->addWidget(interactiveBtn);

    rightLayout->addSpacing(12);

    // -- 列显示/隐藏 --
    rightLayout->addWidget(
        new QLabel("<b>列可见性</b>"));

    const QStringList colNames =
        {"编号", "名称", "分类", "单价", "库存", "备注"};
    for (int i = 0; i < colNames.size(); ++i) {
        auto *cb = new QCheckBox(colNames[i]);
        cb->setChecked(true);
        connect(cb, &QCheckBox::toggled, this,
                [this, i](bool checked) {
            auto *header = m_tableView->horizontalHeader();
            if (checked) {
                header->showSection(i);
            } else {
                header->hideSection(i);
            }
        });
        rightLayout->addWidget(cb);
    }

    rightLayout->addSpacing(12);

    // -- 排序控制 --
    rightLayout->addWidget(
        new QLabel("<b>排序控制</b>"));

    auto *sortCombo = new QComboBox;
    sortCombo->addItems(colNames);
    rightLayout->addWidget(sortCombo);

    auto *sortAscBtn = new QPushButton("升序排列");
    auto *sortDescBtn = new QPushButton("降序排列");

    connect(sortAscBtn, &QPushButton::clicked, this,
            [this, sortCombo]() {
        int col = sortCombo->currentIndex();
        m_proxyModel->sort(col, Qt::AscendingOrder);
        m_tableView->horizontalHeader()
            ->setSortIndicator(col, Qt::AscendingOrder);
    });
    connect(sortDescBtn, &QPushButton::clicked, this,
            [this, sortCombo]() {
        int col = sortCombo->currentIndex();
        m_proxyModel->sort(col, Qt::DescendingOrder);
        m_tableView->horizontalHeader()
            ->setSortIndicator(col, Qt::DescendingOrder);
    });

    rightLayout->addWidget(sortAscBtn);
    rightLayout->addWidget(sortDescBtn);

    rightLayout->addStretch();

    mainHLayout->addWidget(rightPanel);
    setCentralWidget(centralWidget);
}

/// @brief 填充模拟的商品数据
void MainWindow::populateProductData()
{
    struct Product {
        QString id;
        QString name;
        QString category;
        double price;
        int stock;
        QString note;
    };

    const Product products[] = {
        {"P001", "机械键盘 K8 Pro",    "外设",   599.0,  42, "蓝牙双模"},
        {"P002", "4K 显示器 27 寸",     "显示器", 2699.0, 15, "IPS 面板"},
        {"P003", "无线鼠标 M720",       "外设",   349.0,  88, "多设备切换"},
        {"P004", "USB-C 扩展坞",        "配件",   459.0,  33, "12 合 1"},
        {"P005", "降噪耳机 WH-1000",    "音频",   1999.0, 27, "主动降噪"},
        {"P006", "便携 SSD 1TB",        "存储",   699.0,  56, "读速 1050MB/s"},
        {"P007", "人体工学椅 S300",     "座椅",   1899.0, 8,  "网面透气"},
        {"P008", "桌面台灯 L1",         "照明",   259.0,  64, "色温可调"},
    };

    for (const auto &p : products) {
        QList<QStandardItem *> row;
        auto *idItem = new QStandardItem(p.id);
        idItem->setFlags(
            idItem->flags() & ~Qt::ItemIsEditable);
        row << idItem
            << new QStandardItem(p.name)
            << new QStandardItem(p.category)
            << new QStandardItem(
                   QString::number(p.price, 'f', 0))
            << new QStandardItem(QString::number(p.stock))
            << new QStandardItem(p.note);
        m_model->appendRow(row);
    }
}

/// @brief 应用统一的列宽策略到所有列
void MainWindow::applyResizeMode(QHeaderView::ResizeMode mode)
{
    auto *header = m_tableView->horizontalHeader();
    // 先重置所有列为同一种模式
    header->setSectionResizeMode(mode);

    // 固定模式下给每列一个合理的默认宽度
    if (mode == QHeaderView::Fixed) {
        const int widths[] = {70, 160, 70, 70, 60, 160};
        for (int i = 0;
             i < m_model->columnCount() && i < 6;
             ++i) {
            header->resizeSection(i, widths[i]);
        }
    }

    // 拉伸模式下拉伸最后一列通常多余，这里保持简洁
    // 交互模式下使用默认宽度
}
