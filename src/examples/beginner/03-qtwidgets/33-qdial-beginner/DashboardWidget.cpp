#include "DashboardWidget.h"

// ============================================================================
// DashboardWidget: 模拟汽车仪表盘面板
// ============================================================================
DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QDial 综合演示 — 汽车仪表盘");
    resize(640, 400);
    initUi();
}

/// @brief 初始化界面
void DashboardWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *title = new QLabel("汽车仪表盘模拟");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Arial", 16, QFont::Bold));
    mainLayout->addWidget(title);

    // ---- 三个仪表旋钮 ----
    auto *gaugesLayout = new QHBoxLayout();
    gaugesLayout->setSpacing(24);

    // 速度表：0-240 km/h，有边界
    auto *speedGroup = createGauge(
        "速度表", "km/h", 0, 240, 10,
        false,  /* 不需要 wrapping */
        QColor("#1976D2"));
    gaugesLayout->addWidget(speedGroup);

    // 转速表：0-8000 RPM，有边界
    auto *rpmGroup = createGauge(
        "转速表", "RPM", 0, 8000, 500,
        false,
        QColor("#F57C00"));
    gaugesLayout->addWidget(rpmGroup);

    // 温度表：40-120 C，有边界，带颜色变化
    auto *tempGroup = createTempGauge();
    gaugesLayout->addWidget(tempGroup);

    mainLayout->addLayout(gaugesLayout, 1);

    // ---- 角度演示：wrapping 旋钮 ----
    auto *wrapGroup = new QGroupBox("角度旋钮（setWrapping = true）");
    auto *wrapLayout = new QHBoxLayout(wrapGroup);

    m_angleDial = new QDial;
    m_angleDial->setRange(0, 359);
    m_angleDial->setWrapping(true);
    m_angleDial->setNotchesVisible(true);
    m_angleDial->setSingleStep(15);
    m_angleDial->setFixedSize(120, 120);

    m_angleLabel = new QLabel("0 度");
    m_angleLabel->setAlignment(Qt::AlignCenter);
    m_angleLabel->setFont(QFont("Arial", 14, QFont::Bold));
    m_angleLabel->setStyleSheet("color: #7B1FA2;");

    connect(m_angleDial, &QDial::valueChanged, this,
            [this](int value) {
        m_angleLabel->setText(QString::number(value) + " 度");
    });

    auto *wrapInfo = new QLabel(
        "范围 0-359\n"
        "拧过 359 直接回到 0\n"
        "适合角度、色相等循环值域");
    wrapInfo->setStyleSheet("color: #666; font-size: 12px;");

    wrapLayout->addWidget(m_angleDial);
    wrapLayout->addWidget(m_angleLabel);
    wrapLayout->addWidget(wrapInfo);
    wrapLayout->addStretch();

    mainLayout->addWidget(wrapGroup);

    // ---- 底部状态 ----
    m_statusLabel = new QLabel("拖拽旋钮调节数值");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "color: #888; font-size: 12px;"
        "padding: 8px;"
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    mainLayout->addWidget(m_statusLabel);
}

/// @brief 创建通用仪表旋钮组
QGroupBox *DashboardWidget::createGauge(const QString &name, const QString &unit,
                           int minVal, int maxVal, int step,
                           bool wrapping, const QColor &color)
{
    auto *group = new QGroupBox(name);
    auto *layout = new QVBoxLayout(group);
    layout->setAlignment(Qt::AlignCenter);

    auto *dial = new QDial;
    dial->setRange(minVal, maxVal);
    dial->setWrapping(wrapping);
    dial->setNotchesVisible(true);
    dial->setSingleStep(step);
    dial->setFixedSize(140, 140);

    auto *valueLabel = new QLabel(QString::number(minVal) + " " + unit);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setFont(QFont("Arial", 14, QFont::Bold));
    valueLabel->setStyleSheet(
        QString("color: %1;").arg(color.name()));

    connect(dial, &QDial::valueChanged, this,
            [this, valueLabel, unit](int value) {
        valueLabel->setText(QString::number(value) + " " + unit);
        m_statusLabel->setText(
            "当前操作: " + valueLabel->text());
    });

    layout->addWidget(dial, 0, Qt::AlignCenter);
    layout->addWidget(valueLabel);

    return group;
}

/// @brief 创建温度仪表旋钮组（带颜色变化）
QGroupBox *DashboardWidget::createTempGauge()
{
    auto *group = new QGroupBox("温度表");
    auto *layout = new QVBoxLayout(group);
    layout->setAlignment(Qt::AlignCenter);

    auto *dial = new QDial;
    dial->setRange(40, 120);
    dial->setWrapping(false);
    dial->setNotchesVisible(true);
    dial->setSingleStep(5);
    dial->setFixedSize(140, 140);
    dial->setValue(40);  // 初始温度

    m_tempLabel = new QLabel("40 C");
    m_tempLabel->setAlignment(Qt::AlignCenter);
    m_tempLabel->setFont(QFont("Arial", 14, QFont::Bold));

    // 根据温度设置颜色
    updateTempColor(40);

    connect(dial, &QDial::valueChanged, this,
            [this](int temp) {
        m_tempLabel->setText(QString::number(temp) + " C");
        updateTempColor(temp);
        QString status;
        if (temp < 80) {
            status = "冷车状态，正在预热";
        } else if (temp <= 100) {
            status = "温度正常";
        } else {
            status = "温度过高，请注意冷却系统！";
        }
        m_statusLabel->setText(
            "温度: " + QString::number(temp) + " C — " + status);
    });

    layout->addWidget(dial, 0, Qt::AlignCenter);
    layout->addWidget(m_tempLabel);

    return group;
}

/// @brief 根据温度值更新标签颜色
void DashboardWidget::updateTempColor(int temp)
{
    QString color;
    if (temp < 80) {
        color = "#1976D2";  // 蓝色：冷车
    } else if (temp <= 100) {
        color = "#388E3C";  // 绿色：正常
    } else {
        color = "#D32F2F";  // 红色：过热
    }
    m_tempLabel->setStyleSheet(QString("color: %1;").arg(color));
}
