// QtWidgets 入门示例 12: QAbstractButton 按钮基类机制
// 演示：setCheckable / setChecked / setAutoRepeat 核心属性
//       clicked / toggled / pressed / released 四个信号
//       QButtonGroup 单选互斥管理
//       继承 QAbstractButton 自定义圆形按钮

#include "DemoWindow.h"

#include "CircleButton.h"

#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFont>

DemoWindow::DemoWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QAbstractButton 基类机制演示");
    resize(640, 560);
    initUi();
}

void DemoWindow::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ---- 格式按钮组（checkable QPushButton）----
    auto *formatGroup = new QGroupBox("格式按钮（checkable QPushButton）");
    auto *formatLayout = new QHBoxLayout(formatGroup);

    m_boldBtn = new QPushButton("加粗");
    m_boldBtn->setCheckable(true);
    m_boldBtn->setFixedSize(80, 36);

    m_italicBtn = new QPushButton("斜体");
    m_italicBtn->setCheckable(true);
    m_italicBtn->setFixedSize(80, 36);

    m_underlineBtn = new QPushButton("下划线");
    m_underlineBtn->setCheckable(true);
    m_underlineBtn->setFixedSize(80, 36);

    // 连接 toggled 信号来改变预览标签的字体样式
    connect(m_boldBtn, &QPushButton::toggled, this, &DemoWindow::updatePreviewFont);
    connect(m_italicBtn, &QPushButton::toggled, this, &DemoWindow::updatePreviewFont);
    connect(m_underlineBtn, &QPushButton::toggled, this, &DemoWindow::updatePreviewFont);

    formatLayout->addWidget(m_boldBtn);
    formatLayout->addWidget(m_italicBtn);
    formatLayout->addWidget(m_underlineBtn);
    formatLayout->addStretch();
    mainLayout->addWidget(formatGroup);

    // ---- 预览标签 ----
    m_previewLabel = new QLabel("Hello QAbstractButton! 你好，按钮基类！");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumHeight(50);
    m_previewLabel->setStyleSheet(
        "background-color: #FAFAFA;"
        "border: 1px solid #DDD;"
        "border-radius: 4px;"
        "padding: 8px;"
        "font-size: 14px;");
    mainLayout->addWidget(m_previewLabel);

    // ---- 字号单选组（QRadioButton + QButtonGroup）----
    auto *sizeGroup = new QGroupBox("字号选择（QRadioButton + QButtonGroup）");
    auto *sizeLayout = new QHBoxLayout(sizeGroup);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    auto *small = new QRadioButton("小 (12px)");
    auto *medium = new QRadioButton("中 (16px)");
    auto *large = new QRadioButton("大 (22px)");
    auto *xlarge = new QRadioButton("特大 (30px)");

    m_buttonGroup->addButton(small, 12);
    m_buttonGroup->addButton(medium, 16);
    m_buttonGroup->addButton(large, 22);
    m_buttonGroup->addButton(xlarge, 30);

    medium->setChecked(true);

    connect(m_buttonGroup, &QButtonGroup::idToggled, this, [this](int id, bool checked) {
        if (checked) {
            m_currentFontSize = id;
            updatePreviewFont();
        }
    });

    sizeLayout->addWidget(small);
    sizeLayout->addWidget(medium);
    sizeLayout->addWidget(large);
    sizeLayout->addWidget(xlarge);
    mainLayout->addWidget(sizeGroup);

    // ---- 计数器组（autoRepeat QPushButton）----
    auto *counterGroup = new QGroupBox("计数器（autoRepeat QPushButton）");
    auto *counterLayout = new QHBoxLayout(counterGroup);

    // 减少按钮（圆形自定义按钮）
    auto *minusBtn = new CircleButton("-");
    minusBtn->setAutoRepeat(true);
    minusBtn->setAutoRepeatDelay(500);
    minusBtn->setAutoRepeatInterval(80);
    minusBtn->setFont(QFont("Arial", 20, QFont::Bold));
    connect(minusBtn, &CircleButton::clicked, this, [this]() {
        m_counter--;
        updateCounterLabel();
    });

    // 计数器显示
    m_counterLabel = new QLabel("0");
    m_counterLabel->setAlignment(Qt::AlignCenter);
    m_counterLabel->setFixedSize(100, 60);
    m_counterLabel->setStyleSheet(
        "font-size: 28px; font-weight: bold;"
        "color: #333; background-color: #FAFAFA;"
        "border: 1px solid #DDD; border-radius: 4px;");

    // 增加按钮（圆形自定义按钮）
    auto *plusBtn = new CircleButton("+");
    plusBtn->setAutoRepeat(true);
    plusBtn->setAutoRepeatDelay(500);
    plusBtn->setAutoRepeatInterval(80);
    plusBtn->setFont(QFont("Arial", 20, QFont::Bold));
    connect(plusBtn, &CircleButton::clicked, this, [this]() {
        m_counter++;
        updateCounterLabel();
    });

    counterLayout->addStretch();
    counterLayout->addWidget(minusBtn);
    counterLayout->addWidget(m_counterLabel);
    counterLayout->addWidget(plusBtn);
    counterLayout->addStretch();
    mainLayout->addWidget(counterGroup);

    // ---- 信号演示组 ----
    auto *signalGroup = new QGroupBox("信号触发顺序演示");
    auto *signalLayout = new QVBoxLayout(signalGroup);

    auto *signalBtn = new QPushButton("按住我观察信号顺序");
    signalBtn->setCheckable(true);
    signalBtn->setFixedHeight(36);

    m_signalLog = new QLabel("等待操作...");
    m_signalLog->setMinimumHeight(60);
    m_signalLog->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_signalLog->setWordWrap(true);
    m_signalLog->setStyleSheet(
        "background-color: #263238; color: #EEFFFF;"
        "font-family: monospace; font-size: 12px;"
        "border-radius: 4px; padding: 6px;");

    connect(signalBtn, &QPushButton::pressed, this, [this]() {
        m_signalLog->setText(m_signalLog->text() + "\n[pressed] 鼠标按下");
    });
    connect(signalBtn, &QPushButton::released, this, [this]() {
        m_signalLog->setText(m_signalLog->text() + "\n[released] 鼠标释放");
    });
    connect(signalBtn, &QPushButton::clicked, this, [this](bool checked) {
        m_signalLog->setText(m_signalLog->text() +
            QString("\n[clicked] checked = %1").arg(checked ? "true" : "false"));
    });
    connect(signalBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_signalLog->setText(m_signalLog->text() +
            QString("\n[toggled] checked = %1").arg(checked ? "true" : "false"));
    });

    signalLayout->addWidget(signalBtn);
    signalLayout->addWidget(m_signalLog);
    mainLayout->addWidget(signalGroup);

    // ---- 底部提示 ----
    auto *hint = new QLabel(
        "提示: 格式按钮使用 toggled 信号 | "
        "字号组使用 QButtonGroup::idToggled | "
        "圆形 +/- 按钮开启了 autoRepeat");
    hint->setStyleSheet("color: #999; font-size: 11px;");
    mainLayout->addWidget(hint);

    // 初始状态
    updatePreviewFont();
}

void DemoWindow::updatePreviewFont()
{
    QFont font;
    font.setBold(m_boldBtn->isChecked());
    font.setItalic(m_italicBtn->isChecked());
    font.setUnderline(m_underlineBtn->isChecked());
    font.setPointSize(m_currentFontSize);
    m_previewLabel->setFont(font);
}

void DemoWindow::updateCounterLabel()
{
    m_counterLabel->setText(QString::number(m_counter));
    // 正数绿色，负数红色，零灰色
    QString color = m_counter > 0 ? "#2E7D32" :
                    m_counter < 0 ? "#C62828" : "#333";
    m_counterLabel->setStyleSheet(
        QString("font-size: 28px; font-weight: bold; color: %1;"
                "background-color: #FAFAFA; border: 1px solid #DDD;"
                "border-radius: 4px;").arg(color));
}
