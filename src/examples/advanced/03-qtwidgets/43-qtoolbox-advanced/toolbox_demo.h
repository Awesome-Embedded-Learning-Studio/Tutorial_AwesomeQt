/// @file    toolbox_demo.h
/// @brief   Demo widget showcasing QToolBox with custom styling and dynamic items.
///
/// Demonstrates QSS-based title tab customization, item icons, and
/// runtime add/remove operations on a QToolBox.
///
/// 对应教程：进阶层 03-QtWidgets/43-QToolBox 自定义标题栏样式。

#pragma once

#include <QIcon>
#include <QWidget>

class QToolBox;
class QPushButton;
class QLabel;

/// @brief A demo window containing a styled QToolBox and buttons for
///        dynamically adding and removing pages at runtime.
class ToolboxDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief Constructs the demo widget.
    /// @param[in] parent Parent widget for Qt ownership.
    explicit ToolboxDemo(QWidget* parent = nullptr);

private:
    /// @brief Sets up the QToolBox with initial pages and QSS styling.
    /// @note QSS is applied globally to the toolbox to customize the tab
    ///       title bar appearance, including custom backgrounds, borders,
    ///       and font styling for selected/unselected states.
    void setupToolBox();

    /// @brief Creates a content widget for a toolbox page.
    /// @param[in] text  Descriptive text shown in the page body.
    /// @param[in] parent Parent for ownership.
    /// @return A widget suitable for QToolBox::addItem().
    QWidget* createPageContent(const QString& text, QWidget* parent = nullptr);

    /// @brief Adds a new page to the toolbox at runtime.
    /// @note Demonstrates dynamic item insertion with addItem().
    void addPage();

    /// @brief Removes the last page from the toolbox at runtime.
    /// @note Demonstrates dynamic item removal with removeItem().
    ///       Guarded to keep at least one page in the toolbox.
    void removePage();

    QToolBox* m_toolBox{nullptr};       ///< The styled QToolBox widget
    QPushButton* m_addBtn{nullptr};     ///< Button to add a new page
    QPushButton* m_removeBtn{nullptr};  ///< Button to remove the last page
    QLabel* m_statusLabel{nullptr};     ///< Shows current page count
    int m_dynamicCount{0};              ///< Counter for dynamically added pages
};
