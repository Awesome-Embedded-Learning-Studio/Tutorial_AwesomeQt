// QtWidgets 入门示例 15: QAbstractItemView 视图基类
// DemoWindow: 主演示窗口

#include "demo_window.h"
#include "date_delegate.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QDate>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QListView>
#include <QModelIndex>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>

DemoWindow::DemoWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QAbstractItemView 视图基类演示");
    resize(860, 600);
    initModel();
    initUi();
}

void DemoWindow::initModel()
{
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({"姓名", "年龄", "城市", "入职日期"});

    struct Employee {
        const char *name;
        int age;
        const char *city;
        const char *date;
    };

    Employee employees[] = {
        {"张三", 25, "北京", "2023-03-15"},
        {"李四", 30, "上海", "2021-07-01"},
        {"王五", 28, "深圳", "2022-11-20"},
        {"赵六", 35, "杭州", "2020-01-10"},
        {"孙七", 26, "广州", "2023-08-05"},
        {"周八", 32, "成都", "2019-06-18"},
        {"吴九", 29, "南京", "2022-04-22"},
        {"郑十", 27, "武汉", "2024-02-14"},
    };

    for (const auto &emp : employees) {
        auto *nameItem = new QStandardItem(emp.name);
        auto *ageItem = new QStandardItem(QString::number(emp.age));
        auto *cityItem = new QStandardItem(emp.city);
        auto *dateItem = new QStandardItem(
            QDate::fromString(emp.date, "yyyy-MM-dd").toString("yyyy-MM-dd"));
        // 存储实际日期对象以供委托使用
        dateItem->setData(QDate::fromString(emp.date, "yyyy-MM-dd"), Qt::EditRole);
        m_model->appendRow({nameItem, ageItem, cityItem, dateItem});
    }
}

void DemoWindow::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("QAbstractItemView 视图基类综合演示");
    titleLabel->setFont(QFont("Arial", 14, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ---- 表格 + 列表 分割器 ----
    auto *splitter = new QSplitter(Qt::Horizontal);

    // 表格视图
    m_tableView = new QTableView();
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    // 第 3 列使用日期委托
    m_tableView->setItemDelegateForColumn(3, new DateDelegate(this));
    m_tableView->setAlternatingRowColors(true);
    m_tableView->resizeColumnsToContents();
    splitter->addWidget(m_tableView);

    // 列表视图（默认隐藏，勾选共享后显示）
    m_listView = new QListView();
    m_listView->setModel(m_model);
    m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_listView->hide();
    splitter->addWidget(m_listView);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter, 1);

    // ---- 焦点项信息 ----
    m_currentLabel = new QLabel("焦点项: 无");
    m_currentLabel->setStyleSheet("font-size: 12px; color: #555;");
    mainLayout->addWidget(m_currentLabel);

    // 监听焦点项变化
    connect(m_tableView->selectionModel(), &QItemSelectionModel::currentChanged,
        this, [this](const QModelIndex &current, const QModelIndex &) {
            if (current.isValid()) {
                QString name = m_model->index(current.row(), 0).data().toString();
                QString city = m_model->index(current.row(), 2).data().toString();
                m_currentLabel->setText(
                    QString("焦点项: 第 %1 行 — %2 (%3)").arg(current.row()).arg(name, city));
            }
        }
    );

    // ---- 控制面板 ----
    auto *controlGroup = new QGroupBox("控制面板");
    auto *controlLayout = new QHBoxLayout(controlGroup);

    // 选择模式按钮组
    auto *modeLayout = new QVBoxLayout();
    auto *modeLabel = new QLabel("选择模式:");
    modeLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    modeLayout->addWidget(modeLabel);

    m_modeInfoLabel = new QLabel("当前: ExtendedSelection");
    m_modeInfoLabel->setStyleSheet("color: #1565C0; font-size: 11px;");
    modeLayout->addWidget(m_modeInfoLabel);

    auto *singleBtn = new QPushButton("SingleSelection");
    auto *multiBtn = new QPushButton("MultiSelection");
    auto *extendedBtn = new QPushButton("ExtendedSelection");

    connect(singleBtn, &QPushButton::clicked, this, [this]() {
        setSelectionModeForViews(QAbstractItemView::SingleSelection);
        m_modeInfoLabel->setText("当前: SingleSelection");
    });
    connect(multiBtn, &QPushButton::clicked, this, [this]() {
        setSelectionModeForViews(QAbstractItemView::MultiSelection);
        m_modeInfoLabel->setText("当前: MultiSelection");
    });
    connect(extendedBtn, &QPushButton::clicked, this, [this]() {
        setSelectionModeForViews(QAbstractItemView::ExtendedSelection);
        m_modeInfoLabel->setText("当前: ExtendedSelection");
    });

    modeLayout->addWidget(singleBtn);
    modeLayout->addWidget(multiBtn);
    modeLayout->addWidget(extendedBtn);
    modeLayout->addStretch();
    controlLayout->addLayout(modeLayout);

    // 选中项展示区域
    auto *displayLayout = new QVBoxLayout();
    auto *displayLabel = new QLabel("选中项信息:");
    displayLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    displayLayout->addWidget(displayLabel);

    auto *showBtn = new QPushButton("显示选中项");
    displayLayout->addWidget(showBtn);

    m_selectedInfo = new QTextEdit();
    m_selectedInfo->setReadOnly(true);
    m_selectedInfo->setMaximumHeight(120);
    m_selectedInfo->setStyleSheet("font-size: 11px;");
    displayLayout->addWidget(m_selectedInfo);
    controlLayout->addLayout(displayLayout, 1);

    connect(showBtn, &QPushButton::clicked, this, [this]() {
        m_selectedInfo->clear();
        QModelIndexList rows = m_tableView->selectionModel()->selectedRows(0);
        if (rows.isEmpty()) {
            m_selectedInfo->setText("没有选中任何行");
            return;
        }
        for (const auto &index : rows) {
            QString name = index.siblingAtColumn(0).data().toString();
            QString age = index.siblingAtColumn(1).data().toString();
            QString city = index.siblingAtColumn(2).data().toString();
            QString date = index.siblingAtColumn(3).data().toString();
            m_selectedInfo->append(
                QString("行 %0: %1 | %2岁 | %3 | 入职 %4")
                    .arg(index.row())
                    .arg(name, age, city, date));
        }
        m_selectedInfo->append(
            QString("\n共选中 %1 行").arg(rows.size()));
    });

    // 共享选择复选框
    auto *optionLayout = new QVBoxLayout();
    auto *optionLabel = new QLabel("选项:");
    optionLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    optionLayout->addWidget(optionLabel);

    auto *sharedCheck = new QCheckBox("共享选择模型\n(显示同步列表)");
    optionLayout->addWidget(sharedCheck);
    optionLayout->addStretch();

    connect(sharedCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (checked) {
            m_listView->setSelectionModel(m_tableView->selectionModel());
            m_listView->show();
        } else {
            // 恢复独立选择模型
            m_listView->setSelectionModel(new QItemSelectionModel(m_model, this));
            m_listView->hide();
        }
    });

    controlLayout->addLayout(optionLayout);
    mainLayout->addWidget(controlGroup);

    // ---- 底部提示 ----
    auto *hint = new QLabel(
        "提示: 双击「入职日期」列弹出日历选择器 | "
        "勾选共享后表格和列表同步选择");
    hint->setStyleSheet("color: #999; font-size: 11px;");
    mainLayout->addWidget(hint);
}

void DemoWindow::setSelectionModeForViews(QAbstractItemView::SelectionMode mode)
{
    m_tableView->setSelectionMode(mode);
    m_listView->setSelectionMode(mode);
}
