/// @file    main.cpp
/// @brief   Entry point for the smooth-scrolling QScrollArea demo.
///
/// Creates a window with a SmoothScrollArea containing a tall widget
/// with many colored sections, plus buttons to smoothly scroll to
/// top, middle, and bottom positions.
///
/// 对应教程：进阶层 03-QtWidgets/44-QScrollArea 平滑滚动动画。

#include "smooth_scroll_area.h"

#include <QApplication>
#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

/// @brief Creates a tall content widget with alternating colored sections.
/// @param[in] sectionCount Number of colored sections to generate.
/// @param[in] sectionHeight Height of each section in pixels.
/// @param[in] parent Parent widget for Qt ownership.
/// @return Pointer to the tall content widget.
/// @note Each section has a distinct background color and a section label
///       to make the scrolling effect clearly visible.
static QWidget* createTallContent(int sectionCount, int sectionHeight,
                                  QWidget* parent = nullptr)
{
    auto* content = new QWidget(parent);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Generate a range of hue values for distinct section colors
    for (int i = 0; i < sectionCount; ++i) {
        auto* section = new QWidget(content);
        section->setFixedHeight(sectionHeight);
        section->setAutoFillBackground(true);

        // Use HSV to generate evenly distributed colors
        int hue = static_cast<int>((i * 360.0 / sectionCount)) % 360;
        QColor sectionColor = QColor::fromHsv(hue, 120, 230);

        QPalette pal;
        pal.setColor(QPalette::Window, sectionColor);
        section->setPalette(pal);

        auto* sectionLayout = new QVBoxLayout(section);
        auto* label = new QLabel(
            QString("Section %1 of %2").arg(i + 1).arg(sectionCount), section);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");
        sectionLayout->addWidget(label);

        // Show the Y position for reference when testing scroll-to-position
        auto* posLabel = new QLabel(
            QString("Y position: %1 px").arg(i * sectionHeight), section);
        posLabel->setAlignment(Qt::AlignCenter);
        posLabel->setStyleSheet("font-size: 11px; color: rgba(255,255,255,180);");
        sectionLayout->addWidget(posLabel);

        layout->addWidget(section);
    }

    return content;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    window->setWindowTitle("Smooth Scrolling QScrollArea Demo");
    window->resize(500, 500);

    auto* mainLayout = new QVBoxLayout(window);

    // Create the smooth scroll area with a tall content widget
    auto* scrollArea = new SmoothScrollArea(window);

    const int sectionCount = 15;
    const int sectionHeight = 120;
    auto* content = createTallContent(sectionCount, sectionHeight, scrollArea);
    scrollArea->setWidget(content);

    mainLayout->addWidget(scrollArea, 1);

    // Navigation buttons at the bottom
    auto* btnLayout = new QHBoxLayout();

    auto* topBtn = new QPushButton("Scroll to Top", window);
    auto* midBtn = new QPushButton("Scroll to Middle", window);
    auto* bottomBtn = new QPushButton("Scroll to Bottom", window);

    btnLayout->addWidget(topBtn);
    btnLayout->addWidget(midBtn);
    btnLayout->addWidget(bottomBtn);

    mainLayout->addLayout(btnLayout);

    // Connect buttons to smooth scroll actions
    QObject::connect(topBtn, &QPushButton::clicked, scrollArea,
                     &SmoothScrollArea::scrollToTop);

    QObject::connect(midBtn, &QPushButton::clicked, scrollArea,
                     [scrollArea, sectionCount, sectionHeight]() {
                         // Scroll to the approximate vertical center
                         int midY = (sectionCount * sectionHeight) / 2
                                    - scrollArea->viewport()->height() / 2;
                         scrollArea->smoothScrollTo(std::max(0, midY));
                     });

    QObject::connect(bottomBtn, &QPushButton::clicked, scrollArea,
                     &SmoothScrollArea::scrollToBottom);

    window->show();
    return app.exec();
}
