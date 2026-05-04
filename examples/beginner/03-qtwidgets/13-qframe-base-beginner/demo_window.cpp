// QtWidgets 入门示例 13: QFrame 可视框架基类
// 演示：QFrame::Shape（Box / Panel / StyledPanel / HLine / VLine）
//       QFrame::Shadow（Raised / Sunken / Plain）
//       lineWidth / midLineWidth 边框宽度控制

#include "demo_window.h"

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>

// ============================================================================
// DemoWindow: QFrame 边框效果展示窗口
// ============================================================================

DemoWindow::DemoWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QFrame 可视框架基类演示");
    resize(700, 620);
    initUi();
}

void DemoWindow::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("QFrame 边框效果对比矩阵");
    titleLabel->setFont(QFont("Arial", 14, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ---- 水平分隔线 ----
    auto *hSeparator = new QFrame();
    hSeparator->setFrameShape(QFrame::HLine);
    hSeparator->setFrameShadow(QFrame::Sunken);
    hSeparator->setFixedHeight(2);
    mainLayout->addWidget(hSeparator);

    // ---- 3x3 对比矩阵 ----
    auto *gridGroup = new QGroupBox("Shape x Shadow 组合效果");
    auto *gridLayout = new QGridLayout(gridGroup);
    gridLayout->setSpacing(8);

    // 三种 Shape
    struct ShapeInfo {
        QFrame::Shape shape;
        QString name;
    };
    ShapeInfo shapes[] = {
        {QFrame::Box,         "Box"},
        {QFrame::Panel,       "Panel"},
        {QFrame::StyledPanel, "StyledPanel"},
    };

    // 三种 Shadow
    struct ShadowInfo {
        QFrame::Shadow shadow;
        QString name;
    };
    ShadowInfo shadows[] = {
        {QFrame::Raised, "Raised (凸起)"},
        {QFrame::Sunken, "Sunken (凹入)"},
        {QFrame::Plain,  "Plain (平面)"},
    };

    // 列头
    for (int col = 0; col < 3; ++col) {
        auto *header = new QLabel(shadows[col].name);
        header->setAlignment(Qt::AlignCenter);
        header->setFont(QFont("Arial", 10, QFont::Bold));
        gridLayout->addWidget(header, 0, col + 1);
    }

    // 填充 3x3 矩阵
    for (int row = 0; row < 3; ++row) {
        // 行头
        auto *rowHeader = new QLabel(shapes[row].name);
        rowHeader->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rowHeader->setFont(QFont("Arial", 10, QFont::Bold));
        gridLayout->addWidget(rowHeader, row + 1, 0);

        for (int col = 0; col < 3; ++col) {
            auto *frame = new QFrame();
            frame->setFrameShape(shapes[row].shape);
            frame->setFrameShadow(shadows[col].shadow);
            frame->setLineWidth(2);
            frame->setMidLineWidth(0);
            frame->setMinimumSize(180, 80);

            auto *label = new QLabel(
                QString("%1\n%2").arg(shapes[row].name, shadows[col].name));
            label->setAlignment(Qt::AlignCenter);
            label->setStyleSheet("font-size: 11px; color: #555;");

            // 用 QVBoxLayout 让 label 在 frame 中居中
            auto *innerLayout = new QVBoxLayout(frame);
            innerLayout->setContentsMargins(8, 8, 8, 8);
            innerLayout->addWidget(label);

            m_demoFrames.append(frame);
            gridLayout->addWidget(frame, row + 1, col + 1);
        }
    }

    gridLayout->setColumnStretch(1, 1);
    gridLayout->setColumnStretch(2, 1);
    gridLayout->setColumnStretch(3, 1);
    mainLayout->addWidget(gridGroup);

    // ---- 分隔线演示区 ----
    auto *sepGroup = new QGroupBox("分隔线演示（HLine / VLine）");
    auto *sepLayout = new QHBoxLayout(sepGroup);

    // 左侧：带水平分隔线的垂直布局
    auto *leftPanel = new QWidget();
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->addWidget(new QLabel("上方区域"));
    auto *hLine = new QFrame();
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);
    hLine->setFixedHeight(2);
    leftLayout->addWidget(hLine);
    leftLayout->addWidget(new QLabel("下方区域"));
    leftLayout->addStretch();

    // 右侧：带垂直分隔线的水平布局
    auto *rightPanel = new QWidget();
    auto *rightLayout = new QHBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addWidget(new QLabel("左侧"));
    auto *vLine = new QFrame();
    vLine->setFrameShape(QFrame::VLine);
    vLine->setFrameShadow(QFrame::Sunken);
    vLine->setFixedWidth(2);
    rightLayout->addWidget(vLine);
    rightLayout->addWidget(new QLabel("右侧"));

    sepLayout->addWidget(leftPanel, 1);
    sepLayout->addWidget(rightPanel, 1);
    mainLayout->addWidget(sepGroup);

    // ---- 控制面板 ----
    auto *controlGroup = new QGroupBox("动态控制");
    auto *controlLayout = new QHBoxLayout(controlGroup);

    // lineWidth 控制
    controlLayout->addWidget(new QLabel("lineWidth:"));
    m_lineWidthSpin = new QSpinBox();
    m_lineWidthSpin->setRange(0, 10);
    m_lineWidthSpin->setValue(2);
    connect(m_lineWidthSpin, &QSpinBox::valueChanged, this, &DemoWindow::updateFrameWidths);

    // midLineWidth 控制
    controlLayout->addWidget(new QLabel("midLineWidth:"));
    m_midLineWidthSpin = new QSpinBox();
    m_midLineWidthSpin->setRange(0, 5);
    m_midLineWidthSpin->setValue(0);
    connect(m_midLineWidthSpin, &QSpinBox::valueChanged, this, &DemoWindow::updateFrameWidths);

    controlLayout->addStretch();
    mainLayout->addWidget(controlGroup);

    // ---- 底部提示 ----
    auto *hint = new QLabel(
        "提示: 调整 lineWidth / midLineWidth 观察边框粗细变化 | "
        "QSS 的 border 属性会覆盖 QFrame 原生边框");
    hint->setStyleSheet("color: #999; font-size: 11px;");
    mainLayout->addWidget(hint);
}

void DemoWindow::updateFrameWidths()
{
    int lw = m_lineWidthSpin->value();
    int mlw = m_midLineWidthSpin->value();
    for (QFrame *frame : m_demoFrames) {
        frame->setLineWidth(lw);
        frame->setMidLineWidth(mlw);
        frame->update();
    }
}
