// QtWidgets 入门示例 16: QAbstractSpinBox 数字输入基类
// DemoWindow: 主演示窗口

#include "demo_window.h"
#include "hex_spin_box.h"

#include <QApplication>
#include <QDoubleSpinBox>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

DemoWindow::DemoWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QAbstractSpinBox 数字输入基类演示");
    resize(580, 620);
    initUi();
}

/// @brief 初始化界面
void DemoWindow::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("QAbstractSpinBox 综合演示");
    titleLabel->setFont(QFont("Arial", 14, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ================================================================
    // 区域 1: HexSpinBox — 16 进制输入框
    // ================================================================
    auto *hexGroup = new QGroupBox("16 进制输入框 (HexSpinBox)");
    auto *hexLayout = new QVBoxLayout(hexGroup);

    auto *hexSpin = new HexSpinBox();
    hexSpin->setValue(0x2A);
    hexLayout->addWidget(hexSpin);

    m_hexInfoLabel = new QLabel();
    m_hexInfoLabel->setStyleSheet("font-size: 12px; color: #333; padding: 4px 0;");
    updateHexInfo(0x2A);
    hexLayout->addWidget(m_hexInfoLabel);

    connect(hexSpin, &QSpinBox::valueChanged, this, [this](int value) {
        updateHexInfo(value);
    });
    connect(hexSpin, &QSpinBox::editingFinished, this, [hexSpin, this]() {
        m_hexInfoLabel->setText(
            m_hexInfoLabel->text() + "  [editingFinished 触发]");
    });

    mainLayout->addWidget(hexGroup);

    // ================================================================
    // 区域 2: 锁定步进 — setReadOnly + stepBy
    // ================================================================
    auto *stepGroup = new QGroupBox("锁定步进 (ReadOnly + stepBy)");
    auto *stepLayout = new QHBoxLayout(stepGroup);

    m_lockedSpin = new QDoubleSpinBox();
    m_lockedSpin->setRange(0.0, 10.0);
    m_lockedSpin->setSingleStep(0.5);
    m_lockedSpin->setDecimals(1);
    m_lockedSpin->setValue(5.0);
    m_lockedSpin->setReadOnly(true);
    m_lockedSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_lockedSpin->setAlignment(Qt::AlignCenter);
    m_lockedSpin->setStyleSheet("font-size: 18px; font-weight: bold;");

    auto *minusBtn = new QPushButton("- 0.5");
    auto *plusBtn = new QPushButton("+ 0.5");

    minusBtn->setFixedWidth(70);
    plusBtn->setFixedWidth(70);

    connect(minusBtn, &QPushButton::clicked, this, [this]() {
        m_lockedSpin->stepBy(-1);
    });
    connect(plusBtn, &QPushButton::clicked, this, [this]() {
        m_lockedSpin->stepBy(1);
    });

    stepLayout->addWidget(minusBtn);
    stepLayout->addWidget(m_lockedSpin, 1);
    stepLayout->addWidget(plusBtn);

    auto *stepHint = new QLabel(
        "setReadOnly(true) + NoButtons: 禁止键盘输入和箭头按钮，"
        "只能通过外部按钮调用 stepBy()");
    stepHint->setStyleSheet("color: #999; font-size: 11px;");
    auto *stepHintLayout = new QVBoxLayout();
    stepHintLayout->addWidget(stepHint);
    stepGroup->layout()->addItem(stepHintLayout);

    mainLayout->addWidget(stepGroup);

    // ================================================================
    // 区域 3: 信号对比 — valueChanged vs editingFinished
    // ================================================================
    auto *signalGroup = new QGroupBox("信号触发对比");
    auto *signalLayout = new QVBoxLayout(signalGroup);

    auto *spinForSignal = new QSpinBox();
    spinForSignal->setRange(0, 1000);
    spinForSignal->setValue(100);
    spinForSignal->setSingleStep(10);
    signalLayout->addWidget(spinForSignal);

    // 计数器标签
    auto *counterLayout = new QHBoxLayout();
    m_valueChangedCount = 0;
    m_editingFinishedCount = 0;

    m_vcLabel = new QLabel("valueChanged: 0 次");
    m_vcLabel->setStyleSheet(
        "font-size: 12px; color: #1565C0; padding: 4px 8px;"
        "background-color: #E3F2FD; border-radius: 3px;");
    m_efLabel = new QLabel("editingFinished: 0 次");
    m_efLabel->setStyleSheet(
        "font-size: 12px; color: #2E7D32; padding: 4px 8px;"
        "background-color: #E8F5E9; border-radius: 3px;");

    counterLayout->addWidget(m_vcLabel, 1);
    counterLayout->addWidget(m_efLabel, 1);
    signalLayout->addLayout(counterLayout);

    auto *resetBtn = new QPushButton("重置计数器");
    signalLayout->addWidget(resetBtn);

    // 连接信号
    connect(spinForSignal, &QSpinBox::valueChanged, this, [this]() {
        m_valueChangedCount++;
        m_vcLabel->setText(QString("valueChanged: %1 次").arg(m_valueChangedCount));
    });
    connect(spinForSignal, &QSpinBox::editingFinished, this, [this]() {
        m_editingFinishedCount++;
        m_efLabel->setText(QString("editingFinished: %1 次").arg(m_editingFinishedCount));
    });
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_valueChangedCount = 0;
        m_editingFinishedCount = 0;
        m_vcLabel->setText("valueChanged: 0 次");
        m_efLabel->setText("editingFinished: 0 次");
    });

    auto *signalHint = new QLabel(
        "提示: 连续输入多个数字然后按 Enter，"
        "valueChanged 会触发多次，editingFinished 只触发一次");
    signalHint->setStyleSheet("color: #999; font-size: 11px;");
    signalLayout->addWidget(signalHint);

    mainLayout->addWidget(signalGroup);

    // ---- 底部说明 ----
    auto *footer = new QLabel(
        "提示: 16 进制框直接输入 hex 值或点击箭头 | "
        "锁定步进只允许外部按钮调值 | 信号对比观察触发频率差异");
    footer->setStyleSheet("color: #999; font-size: 11px;");
    mainLayout->addWidget(footer);
}

/// @brief 更新 16 进制信息标签
void DemoWindow::updateHexInfo(int value)
{
    m_hexInfoLabel->setText(
        QString("十进制: %1  |  十六进制: %2  |  二进制: %3")
            .arg(value)
            .arg(QString("0x%1").arg(value, 2, 16, QChar('0')).toUpper())
            .arg(QString("%1").arg(value, 8, 2, QChar('0'))));
}
