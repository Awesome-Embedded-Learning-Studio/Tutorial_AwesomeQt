#include "MainWindow.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("QTableWidget 综合演示 — 学生信息管理");
    resize(720, 480);
    initUi();
    populateStudentData();
    updateStatistics();
}

void MainWindow::initUi()
{
    auto *centralWidget = new QWidget;
    auto *mainHLayout = new QHBoxLayout(centralWidget);

    // ================================================================
    // 左侧：QTableWidget（6 列学生表）
    // ================================================================
    auto *leftPanel = new QWidget;
    auto *leftLayout = new QVBoxLayout(leftPanel);

    // 操作按钮栏
    auto *btnLayout = new QHBoxLayout;
    auto *addBtn = new QPushButton("添加学生");
    auto *deleteBtn = new QPushButton("删除选中行");
    deleteBtn->setStyleSheet("QPushButton { color: #D32F2F; }");
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    leftLayout->addLayout(btnLayout);

    // 表格
    m_tableWidget = new QTableWidget;
    m_tableWidget->setColumnCount(6);
    m_tableWidget->setHorizontalHeaderLabels(
        {"学号", "姓名", "性别", "年龄", "专业", "成绩"});
    m_tableWidget->verticalHeader()->hide();
    m_tableWidget->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(
        QAbstractItemView::SingleSelection);
    m_tableWidget->setEditTriggers(
        QAbstractItemView::DoubleClicked
        | QAbstractItemView::EditKeyPressed);

    // 列宽策略
    m_tableWidget->horizontalHeader()
        ->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()
        ->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()
        ->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()
        ->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()
        ->setStretchLastSection(true);

    leftLayout->addWidget(m_tableWidget, 1);

    // 底部统计标签
    m_statsLabel = new QLabel;
    m_statsLabel->setStyleSheet(
        "padding: 4px 8px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #DDD;"
        "border-radius: 3px;");
    leftLayout->addWidget(m_statsLabel);

    mainHLayout->addWidget(leftPanel, 1);

    // ================================================================
    // 右侧：详情面板
    // ================================================================
    auto *rightPanel = new QWidget;
    rightPanel->setFixedWidth(200);
    auto *rightLayout = new QVBoxLayout(rightPanel);

    rightLayout->addWidget(new QLabel("学生详情:"));

    m_detailLabel = new QLabel("点击表格中的行查看详情");
    m_detailLabel->setWordWrap(true);
    m_detailLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_detailLabel->setStyleSheet(
        "padding: 10px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #DDD;"
        "border-radius: 4px;");
    rightLayout->addWidget(m_detailLabel, 1);

    // 修改提示
    m_editHintLabel = new QLabel;
    m_editHintLabel->setWordWrap(true);
    m_editHintLabel->setStyleSheet("color: #666;");
    rightLayout->addWidget(m_editHintLabel);

    mainHLayout->addWidget(rightPanel);

    setCentralWidget(centralWidget);

    // ================================================================
    // 信号连接
    // ================================================================
    // 点击单元格时更新详情
    connect(m_tableWidget, &QTableWidget::cellClicked,
            this, &MainWindow::onCellClicked);

    // 当前单元格变化时更新详情
    connect(m_tableWidget, &QTableWidget::currentCellChanged,
            this, [this](int curRow, int, int, int) {
        updateDetail(curRow);
    });

    // 单元格内容变化
    connect(m_tableWidget, &QTableWidget::cellChanged,
            this, [this](int row, int col) {
        m_editHintLabel->setText(
            QString("已修改: (%1, %2)").arg(row).arg(col));
        updateStatistics();
    });

    // 添加学生
    connect(addBtn, &QPushButton::clicked, this,
            &MainWindow::onAddStudent);

    // 删除选中行
    connect(deleteBtn, &QPushButton::clicked, this,
            &MainWindow::onDeleteStudent);
}

void MainWindow::populateStudentData()
{
    struct Student {
        QString id;
        QString name;
        QString gender;
        int age;
        QString major;
        double score;
    };

    const Student students[] = {
        {"2024001", "张三",   "男", 20, "计算机科学", 92.5},
        {"2024002", "李四",   "女", 21, "软件工程",   88.0},
        {"2024003", "王五",   "男", 19, "人工智能",   95.0},
        {"2024004", "赵六",   "女", 20, "数据科学",   78.5},
        {"2024005", "钱七",   "男", 22, "信息安全",   85.0},
        {"2024006", "孙八",   "女", 20, "计算机科学", 91.0},
        {"2024007", "周九",   "男", 21, "软件工程",   76.5},
    };

    m_tableWidget->setRowCount(
        static_cast<int>(std::size(students)));

    m_tableWidget->blockSignals(true);

    for (int i = 0;
         i < static_cast<int>(std::size(students));
         ++i) {
        const auto &s = students[i];
        m_tableWidget->setItem(i, 0,
            new QTableWidgetItem(s.id));
        m_tableWidget->setItem(i, 1,
            new QTableWidgetItem(s.name));
        m_tableWidget->setItem(i, 2,
            new QTableWidgetItem(s.gender));
        m_tableWidget->setItem(i, 3,
            new QTableWidgetItem(QString::number(s.age)));
        m_tableWidget->setItem(i, 4,
            new QTableWidgetItem(s.major));
        m_tableWidget->setItem(i, 5,
            new QTableWidgetItem(
                QString::number(s.score, 'f', 1)));

        // 学号列不可编辑
        QTableWidgetItem *idItem =
            m_tableWidget->item(i, 0);
        idItem->setFlags(
            idItem->flags() & ~Qt::ItemIsEditable);
    }

    m_tableWidget->blockSignals(false);
}

void MainWindow::onCellClicked(int row, int /*column*/)
{
    updateDetail(row);
}

void MainWindow::updateDetail(int row)
{
    if (row < 0 || row >= m_tableWidget->rowCount()) {
        m_detailLabel->setText(
            "点击表格中的行查看详情");
        return;
    }

    auto getCellText = [this](int r, int c) -> QString {
        QTableWidgetItem *item = m_tableWidget->item(r, c);
        return item ? item->text() : "--";
    };

    QString details;
    details += QString("学号: %1\n").arg(getCellText(row, 0));
    details += QString("姓名: %1\n").arg(getCellText(row, 1));
    details += QString("性别: %1\n").arg(getCellText(row, 2));
    details += QString("年龄: %1\n").arg(getCellText(row, 3));
    details += QString("专业: %1\n").arg(getCellText(row, 4));
    details += QString("成绩: %1\n").arg(getCellText(row, 5));

    m_detailLabel->setText(details);
}

void MainWindow::updateStatistics()
{
    int count = m_tableWidget->rowCount();
    double totalScore = 0.0;
    int validCount = 0;

    for (int i = 0; i < count; ++i) {
        QTableWidgetItem *scoreItem =
            m_tableWidget->item(i, 5);
        if (scoreItem) {
            bool ok = false;
            double score = scoreItem->text().toDouble(&ok);
            if (ok) {
                totalScore += score;
                ++validCount;
            }
        }
    }

    double avg = (validCount > 0)
                     ? (totalScore / validCount)
                     : 0.0;

    m_statsLabel->setText(
        QString("总计: %1 名学生 | 平均成绩: %2")
            .arg(count)
            .arg(avg, 0, 'f', 1));
}

void MainWindow::onAddStudent()
{
    bool ok = false;
    QString name = QInputDialog::getText(
        this, "添加学生", "姓名:",
        QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    int row = m_tableWidget->rowCount();
    m_tableWidget->insertRow(row);

    // 生成学号
    QString newId =
        QString("2024%1").arg(row + 1, 3, 10, QChar('0'));

    m_tableWidget->blockSignals(true);

    m_tableWidget->setItem(row, 0,
        new QTableWidgetItem(newId));
    m_tableWidget->setItem(row, 1,
        new QTableWidgetItem(name.trimmed()));
    m_tableWidget->setItem(row, 2,
        new QTableWidgetItem("未知"));
    m_tableWidget->setItem(row, 3,
        new QTableWidgetItem("20"));
    m_tableWidget->setItem(row, 4,
        new QTableWidgetItem("未指定"));
    m_tableWidget->setItem(row, 5,
        new QTableWidgetItem("0.0"));

    // 学号不可编辑
    QTableWidgetItem *idItem =
        m_tableWidget->item(row, 0);
    idItem->setFlags(
        idItem->flags() & ~Qt::ItemIsEditable);

    m_tableWidget->blockSignals(false);

    m_tableWidget->setCurrentCell(row, 0);
    updateStatistics();
    updateDetail(row);
    m_editHintLabel->setText(
        QString("已添加: %1").arg(name.trimmed()));
}

void MainWindow::onDeleteStudent()
{
    int row = m_tableWidget->currentRow();
    if (row < 0) {
        m_editHintLabel->setText("请先选中一行");
        return;
    }

    QTableWidgetItem *nameItem =
        m_tableWidget->item(row, 1);
    QString name = nameItem ? nameItem->text() : "";

    auto result = QMessageBox::question(
        this, "确认删除",
        QString("确定删除学生 \"%1\"?").arg(name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (result == QMessageBox::Yes) {
        m_tableWidget->blockSignals(true);
        m_tableWidget->removeRow(row);
        m_tableWidget->blockSignals(false);
        updateStatistics();
        m_detailLabel->setText(
            "点击表格中的行查看详情");
        m_editHintLabel->setText(
            QString("已删除: %1").arg(name));
    }
}
