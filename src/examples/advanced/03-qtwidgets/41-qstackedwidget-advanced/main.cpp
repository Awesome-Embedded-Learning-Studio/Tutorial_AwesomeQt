/// @file    main.cpp
/// @brief   Entry point for the animated QStackedWidget demo.
///
/// Builds a window with a SlideStackedWidget containing three colored pages,
/// plus forward/backward navigation buttons and a label showing the current
/// page index.
///
/// 对应教程：进阶层 03-QtWidgets/41-QStackedWidget 滑动切换动画实现。

#include "slide_stacked_widget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

/// @brief Creates a colored page widget with a centered label.
/// @param[in] color  Background color for the page.
/// @param[in] title  Text displayed in the center of the page.
/// @param[in] parent Parent widget for Qt ownership.
/// @return Pointer to the newly created page widget.
/// @note The widget is created on the heap with Qt parent ownership.
static QWidget* createPage(const QColor& color, const QString& title,
                           QWidget* parent = nullptr)
{
    auto* page = new QWidget(parent);
    page->setAutoFillBackground(true);

    QPalette palette;
    palette.setColor(QPalette::Window, color);
    page->setPalette(palette);

    auto* label = new QLabel(title, page);
    label->setAlignment(Qt::AlignCenter);

    // Use a stylesheet on the label for readable text on colored backgrounds
    label->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");

    auto* layout = new QVBoxLayout(page);
    layout->addWidget(label);

    return page;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    window->setWindowTitle("QStackedWidget Slide Animation");
    window->resize(500, 350);

    auto* mainLayout = new QVBoxLayout(window);

    // Create the custom stacked widget with animated transitions
    auto* stacked = new SlideStackedWidget(window);
    stacked->addWidget(createPage(QColor("#2196F3"), "Page 1 - Blue"));
    stacked->addWidget(createPage(QColor("#4CAF50"), "Page 2 - Green"));
    stacked->addWidget(createPage(QColor("#FF9800"), "Page 3 - Orange"));
    stacked->setCurrentIndex(0);

    mainLayout->addWidget(stacked, 1);

    // Navigation controls at the bottom
    auto* navLayout = new QHBoxLayout();

    auto* prevBtn = new QPushButton("<< Previous", window);
    auto* indexLabel = new QLabel("Page: 1 / 3", window);
    indexLabel->setAlignment(Qt::AlignCenter);
    auto* nextBtn = new QPushButton("Next >>", window);

    navLayout->addWidget(prevBtn);
    navLayout->addStretch();
    navLayout->addWidget(indexLabel);
    navLayout->addStretch();
    navLayout->addWidget(nextBtn);

    mainLayout->addLayout(navLayout);

    // Update label text when the stacked widget's current page changes
    auto updateLabel = [stacked, indexLabel]() {
        int idx = stacked->currentIndex();
        int total = stacked->count();
        indexLabel->setText(
            QString("Page: %1 / %2").arg(idx + 1).arg(total));
    };

    // Connect navigation buttons to slide animation
    QObject::connect(nextBtn, &QPushButton::clicked, stacked,
                     [stacked]() {
                         int next = stacked->currentIndex() + 1;
                         if (next < stacked->count()) {
                             stacked->setCurrentIndex(next);
                         }
                     });

    QObject::connect(prevBtn, &QPushButton::clicked, stacked,
                     [stacked]() {
                         int prev = stacked->currentIndex() - 1;
                         if (prev >= 0) {
                             stacked->setCurrentIndex(prev);
                         }
                     });

    // Update the label when the logical current index changes
    QObject::connect(stacked, &QStackedWidget::currentChanged, stacked,
                     updateLabel);

    window->show();
    return app.exec();
}
