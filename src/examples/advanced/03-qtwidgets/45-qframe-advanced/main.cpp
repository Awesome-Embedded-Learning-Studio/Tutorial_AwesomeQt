/// @file    main.cpp
/// @brief   Entry point for the QFrame rounded shadow demo.
///
/// Displays multiple ShadowFrame instances with different configurations
/// (border radius, shadow offset, blur, colors) to demonstrate the
/// customizability of the painting approach.
///
/// 对应教程：进阶层 03-QtWidgets/45-QFrame 自定义圆角阴影边框绘制。

#include "shadow_frame.h"

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

/// @brief Creates a ShadowFrame with a centered info label.
/// @param[in] title       Title shown inside the frame.
/// @param[in] description Configuration description text.
/// @param[in] parent      Parent widget for Qt ownership.
/// @return Pointer to the configured ShadowFrame.
static ShadowFrame* createDemoFrame(const QString& title,
                                    const QString& description,
                                    QWidget* parent = nullptr)
{
    auto* frame = new ShadowFrame(parent);
    frame->setFixedSize(200, 160);

    auto* layout = new QVBoxLayout(frame);

    auto* titleLabel = new QLabel(title, frame);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
    layout->addWidget(titleLabel);

    auto* descLabel = new QLabel(description, frame);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 11px; color: #666;");
    layout->addWidget(descLabel);

    layout->addStretch();
    return frame;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    window->setWindowTitle("QFrame Custom Rounded Shadow Demo");
    window->resize(700, 450);

    // Set a light gray window background so shadows are visible
    window->setStyleSheet("background-color: #E0E0E0;");

    auto* mainLayout = new QVBoxLayout(window);

    auto* infoLabel = new QLabel(
        "Each card below uses ShadowFrame with different configurations:\n"
        "border radius, shadow offset, blur radius, and colors.", window);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("padding: 8px; color: #444; font-size: 13px;");
    mainLayout->addWidget(infoLabel);

    // Grid of ShadowFrame demos with different configurations
    auto* grid = new QGridLayout();
    grid->setSpacing(30);

    // Frame 1: Default configuration
    auto* frame1 = createDemoFrame("Default",
        "Radius: 12px\nBlur: 15px\nOffset: (4, 4)", window);
    grid->addWidget(frame1, 0, 0);

    // Frame 2: Larger border radius, more blur
    auto* frame2 = createDemoFrame("Large Radius",
        "Radius: 24px\nBlur: 20px\nOffset: (6, 6)", window);
    frame2->setBorderRadius(24);
    frame2->setShadowBlur(20);
    frame2->setShadowOffsetX(6);
    frame2->setShadowOffsetY(6);
    grid->addWidget(frame2, 0, 1);

    // Frame 3: Colored shadow
    auto* frame3 = createDemoFrame("Colored Shadow",
        "Radius: 16px\nBlur: 18px\nBlue shadow", window);
    frame3->setBorderRadius(16);
    frame3->setShadowBlur(18);
    frame3->setShadowColor(QColor(33, 150, 243, 80));
    grid->addWidget(frame3, 0, 2);

    // Frame 4: Tight shadow, small radius
    auto* frame4 = createDemoFrame("Tight Shadow",
        "Radius: 6px\nBlur: 8px\nOffset: (2, 2)", window);
    frame4->setBorderRadius(6);
    frame4->setShadowBlur(8);
    frame4->setShadowOffsetX(2);
    frame4->setShadowOffsetY(2);
    grid->addWidget(frame4, 1, 0);

    // Frame 5: Dark background frame
    auto* frame5 = createDemoFrame("Dark Theme",
        "Radius: 14px\nBlur: 16px\nDark background", window);
    frame5->setBorderRadius(14);
    frame5->setShadowBlur(16);
    frame5->setBackgroundColor(QColor(55, 55, 55));
    frame5->setBorderColor(QColor(80, 80, 80));
    // Override the label colors for dark background
    frame5->setStyleSheet(
        "QLabel { color: #EEE; }");
    grid->addWidget(frame5, 1, 1);

    // Frame 6: High offset, very soft
    auto* frame6 = createDemoFrame("Soft & Offset",
        "Radius: 18px\nBlur: 25px\nOffset: (8, 8)", window);
    frame6->setBorderRadius(18);
    frame6->setShadowBlur(25);
    frame6->setShadowOffsetX(8);
    frame6->setShadowOffsetY(8);
    frame6->setShadowColor(QColor(0, 0, 0, 40));
    grid->addWidget(frame6, 1, 2);

    mainLayout->addLayout(grid, 1);
    mainLayout->addStretch();

    window->show();
    return app.exec();
}
