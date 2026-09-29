/// @file    main.cpp
/// @brief   Entry point demonstrating custom QSplitter handle and min-size constraints.
///
/// Creates a horizontal CustomSplitter with three colored panels, each
/// having a different minimum width to constrain how far the splitter
/// handles can be dragged.
///
/// 对应教程：进阶层 03-QtWidgets/42-QSplitter 自定义拖动手柄外观与最小宽度约束。

#include "custom_splitter.h"

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

/// @brief Creates a colored panel widget with centered text and a minimum size.
/// @param[in] color     Background color for the panel.
/// @param[in] title     Text displayed in the panel center.
/// @param[in] minWidth  Minimum width constraint for the panel.
/// @param[in] minHeight Minimum height constraint for the panel.
/// @param[in] parent    Parent widget for Qt ownership.
/// @return Pointer to the created panel widget.
static QWidget* createPanel(const QColor& color, const QString& title,
                            int minWidth, int minHeight,
                            QWidget* parent = nullptr)
{
    auto* panel = new QWidget(parent);
    panel->setAutoFillBackground(true);
    panel->setMinimumWidth(minWidth);
    panel->setMinimumHeight(minHeight);

    QPalette pal;
    pal.setColor(QPalette::Window, color);
    panel->setPalette(pal);

    auto* label = new QLabel(title, panel);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 18px; font-weight: bold; color: white;");

    auto* layout = new QVBoxLayout(panel);
    layout->addWidget(label);

    // Show the minimum width as info text
    auto* infoLabel = new QLabel(
        QString("Min width: %1px").arg(minWidth), panel);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("font-size: 12px; color: rgba(255,255,255,180);");
    layout->addWidget(infoLabel);

    return panel;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    window->setWindowTitle("Custom QSplitter Handle Demo");
    window->resize(700, 400);

    auto* mainLayout = new QVBoxLayout(window);

    // Info label at the top
    auto* infoLabel = new QLabel(
        "Drag the splitter handles. Each panel has a different minimum width "
        "that constrains the drag range.", window);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("padding: 8px; color: #555;");
    mainLayout->addWidget(infoLabel);

    // Create the custom splitter with three panels
    auto* splitter = new CustomSplitter(Qt::Horizontal, window);

    // Each panel has a different minimum width to demonstrate size constraints
    splitter->addWidget(createPanel(QColor("#E53935"), "Left Panel", 100, 150));
    splitter->addWidget(createPanel(QColor("#43A047"), "Center Panel", 150, 150));
    splitter->addWidget(createPanel(QColor("#1E88E5"), "Right Panel", 80, 150));

    // Set initial size distribution: roughly equal
    splitter->setSizes(QList<int>{230, 230, 240});

    mainLayout->addWidget(splitter, 1);

    window->show();
    return app.exec();
}
