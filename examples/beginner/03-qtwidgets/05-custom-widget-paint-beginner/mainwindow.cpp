#include "mainwindow.h"
#include "circulargauge.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QTimer>

// ============================================================================
// 主窗口：两个仪表盘 + 滑块控制
// ============================================================================
MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("自定义绘制 Widget — 圆形仪表盘");
    resize(560, 400);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // 说明标签
    auto *descLabel = new QLabel(
        "拖动下方滑块查看仪表盘的动画效果。"
        "paintEvent 自定义绘制 / update() 异步重绘 / QPropertyAnimation 平滑过渡");
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #666; font-size: 12px; padding: 4px 0;");
    mainLayout->addWidget(descLabel);

    // ---- 两个仪表盘并排 ----
    auto *gaugeLayout = new QHBoxLayout;
    gaugeLayout->setSpacing(20);

    m_cpuGauge = new CircularGauge("CPU 使用率");
    m_memGauge = new CircularGauge("内存使用率");

    gaugeLayout->addWidget(m_cpuGauge, 1);
    gaugeLayout->addWidget(m_memGauge, 1);

    mainLayout->addLayout(gaugeLayout, 1);

    // ---- 控制面板 ----
    auto *controlGroup = new QGroupBox("控制面板");
    auto *controlLayout = new QGridLayout(controlGroup);
    controlLayout->setSpacing(12);

    // CPU 滑块
    auto *cpuLabel = new QLabel("CPU:");
    m_cpuSlider = new QSlider(Qt::Horizontal);
    m_cpuSlider->setRange(0, 100);
    m_cpuSlider->setValue(0);
    m_cpuValueLabel = new QLabel("0%");
    m_cpuValueLabel->setMinimumWidth(40);

    controlLayout->addWidget(cpuLabel, 0, 0);
    controlLayout->addWidget(m_cpuSlider, 0, 1);
    controlLayout->addWidget(m_cpuValueLabel, 0, 2);

    connect(m_cpuSlider, &QSlider::valueChanged, this, [this](int val) {
        m_cpuGauge->setValue(val);
        m_cpuValueLabel->setText(QString::number(val) + "%");
    });

    // 内存滑块
    auto *memLabel = new QLabel("内存:");
    m_memSlider = new QSlider(Qt::Horizontal);
    m_memSlider->setRange(0, 100);
    m_memSlider->setValue(0);
    m_memValueLabel = new QLabel("0%");
    m_memValueLabel->setMinimumWidth(40);

    controlLayout->addWidget(memLabel, 1, 0);
    controlLayout->addWidget(m_memSlider, 1, 1);
    controlLayout->addWidget(m_memValueLabel, 1, 2);

    connect(m_memSlider, &QSlider::valueChanged, this, [this](int val) {
        m_memGauge->setValue(val);
        m_memValueLabel->setText(QString::number(val) + "%");
    });

    mainLayout->addWidget(controlGroup);

    // ---- 底部按钮栏 ----
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();

    auto *randomBtn = new QPushButton("随机值");
    connect(randomBtn, &QPushButton::clicked, this, [this]() {
        int cpuVal = QRandomGenerator::global()->bounded(0, 101);
        int memVal = QRandomGenerator::global()->bounded(0, 101);
        m_cpuSlider->setValue(cpuVal);
        m_memSlider->setValue(memVal);
    });

    auto *resetBtn = new QPushButton("重置");
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_cpuSlider->setValue(0);
        m_memSlider->setValue(0);
    });

    auto *stressBtn = new QPushButton("压力测试 (快速刷新)");
    connect(stressBtn, &QPushButton::clicked, this, [this]() {
        // 快速连续调用 setValue，验证 update() 的合并机制
        for (int i = 0; i < 50; ++i) {
            QTimer::singleShot(i * 20, this, [this, i]() {
                m_cpuSlider->setValue(QRandomGenerator::global()->bounded(0, 101));
                m_memSlider->setValue(QRandomGenerator::global()->bounded(0, 101));
            });
        }
    });

    buttonLayout->addWidget(randomBtn);
    buttonLayout->addWidget(resetBtn);
    buttonLayout->addWidget(stressBtn);
    mainLayout->addLayout(buttonLayout);

    // 设置初始值
    m_cpuSlider->setValue(45);
    m_memSlider->setValue(62);
}
