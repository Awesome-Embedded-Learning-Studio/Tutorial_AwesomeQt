#include "mainwindow.h"
#include "triangleglwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QOpenGLWidget 三角形演示");
    resize(600, 500);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ---- OpenGL 渲染区域 ----
    m_glWidget = new TriangleGLWidget(this);
    m_glWidget->setMinimumSize(400, 350);
    mainLayout->addWidget(m_glWidget, 1);

    // ---- 信息标签 ----
    auto *infoLabel = new QLabel(
        "三角形使用 OpenGL 3.3 Core Profile 渲染，"
        "每个顶点一个颜色，GPU 自动插值渐变。",
        this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #666; font-size: 11px; padding: 4px;");
    mainLayout->addWidget(infoLabel);

    // ---- 控制按钮栏 ----
    auto *controlLayout = new QHBoxLayout;

    auto *pauseBtn = new QPushButton("暂停旋转", this);
    auto *resumeBtn = new QPushButton("继续旋转", this);
    auto *resetBtn = new QPushButton("重置角度", this);

    controlLayout->addWidget(pauseBtn);
    controlLayout->addWidget(resumeBtn);
    controlLayout->addWidget(resetBtn);
    controlLayout->addStretch();

    mainLayout->addLayout(controlLayout);

    // ---- 连接按钮信号 ----
    connect(pauseBtn, &QPushButton::clicked, this, [this]() {
        m_glWidget->setRotating(false);
    });
    connect(resumeBtn, &QPushButton::clicked, this, [this]() {
        m_glWidget->setRotating(true);
    });
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_glWidget->resetRotation();
    });
}
