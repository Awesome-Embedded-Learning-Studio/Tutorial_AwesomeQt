/// @file    toolbox_demo.cpp
/// @brief   Implementation of the ToolboxDemo widget.
///
/// Creates a QToolBox with custom QSS-styled tabs, icons on each item,
/// and buttons for dynamically adding/removing pages.
///
/// 对应教程：进阶层 03-QtWidgets/43-QToolBox 自定义标题栏样式。

#include "toolbox_demo.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QToolBox>
#include <QVBoxLayout>

ToolboxDemo::ToolboxDemo(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    setupToolBox();
    mainLayout->addWidget(m_toolBox, 1);

    // Status label showing current page count
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    // Buttons for dynamic add/remove
    auto* btnLayout = new QHBoxLayout();

    m_addBtn = new QPushButton("Add Page", this);
    m_removeBtn = new QPushButton("Remove Last Page", this);

    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_removeBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_addBtn, &QPushButton::clicked, this, &ToolboxDemo::addPage);
    connect(m_removeBtn, &QPushButton::clicked, this, &ToolboxDemo::removePage);

    // Update status when the current page changes
    connect(m_toolBox, &QToolBox::currentChanged, this,
            [this]() { m_statusLabel->setText(
                QString("Total pages: %1 | Current: %2")
                    .arg(m_toolBox->count())
                    .arg(m_toolBox->currentIndex() + 1));
            });

    // Initial status update
    m_statusLabel->setText(
        QString("Total pages: %1 | Current: %2")
            .arg(m_toolBox->count())
            .arg(m_toolBox->currentIndex() + 1));
}

void ToolboxDemo::setupToolBox()
{
    m_toolBox = new QToolBox(this);

    // Apply QSS to customize the toolbox tab title bars
    // The QToolBox::tab selector targets the internal QToolBoxButton (QPushButton subclass).
    // @note 关键：min-height 显式撑开标题栏，避免 padding 后文字被裁剪。
    //       icon-size 缩小图标避免挤压文字。
    m_toolBox->setStyleSheet(
        "QToolBox::tab {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #78909C, stop:1 #546E7A);"
        "   color: white;"
        "   border: 1px solid #455A64;"
        "   border-radius: 4px;"
        "   min-height: 32px;"
        "   padding: 4px 12px;"
        "   font-weight: bold;"
        "   font-size: 13px;"
        "   icon-size: 16px;"
        "}"
        "QToolBox::tab:selected {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #26A69A, stop:1 #00897B);"
        "   color: white;"
        "   border: 1px solid #00796B;"
        "}"
        "QToolBox::tab:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #90A4AE, stop:1 #607D8B);"
        "}"
        "QToolBox::tab:!selected {"
        "   margin-top: 2px;"
        "}"
    );

    // Use standard pixmaps as icons to demonstrate icon support
    // without requiring external resource files
    QIcon folderIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    QIcon fileIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    QIcon desktopIcon = QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);

    // Page 1: Application settings
    QWidget* page1 = createPageContent(
        "General application settings.\n\n"
        "This page demonstrates basic QToolBox usage with a folder icon.",
        m_toolBox);
    m_toolBox->addItem(page1, folderIcon, "Application Settings");

    // Page 2: Network configuration
    QWidget* page2 = createPageContent(
        "Network and proxy configuration.\n\n"
        "This page uses a file icon to differentiate from the first tab.",
        m_toolBox);
    m_toolBox->addItem(page2, fileIcon, "Network Configuration");

    // Page 3: About information
    QWidget* page3 = createPageContent(
        "Version and license information.\n\n"
        "This page uses a desktop icon and demonstrates a third tab style.",
        m_toolBox);
    m_toolBox->addItem(page3, desktopIcon, "About");
}

QWidget* ToolboxDemo::createPageContent(const QString& text, QWidget* parent)
{
    auto* page = new QWidget(parent);
    auto* layout = new QVBoxLayout(page);

    auto* label = new QLabel(text, page);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    label->setStyleSheet("padding: 12px; font-size: 13px; color: #333;");
    layout->addWidget(label);

    layout->addStretch();
    return page;
}

void ToolboxDemo::addPage()
{
    m_dynamicCount++;
    QString title = QString("Dynamic Page %1").arg(m_dynamicCount);

    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView);

    QString content = QString(
        "This page was added dynamically at runtime.\n\n"
        "Dynamic page number: %1").arg(m_dynamicCount);

    QWidget* page = createPageContent(content, m_toolBox);
    m_toolBox->addItem(page, icon, title);

    // Navigate to the newly added page
    m_toolBox->setCurrentIndex(m_toolBox->count() - 1);

    m_statusLabel->setText(
        QString("Total pages: %1 | Current: %2")
            .arg(m_toolBox->count())
            .arg(m_toolBox->currentIndex() + 1));
}

void ToolboxDemo::removePage()
{
    // Always keep at least one page
    if (m_toolBox->count() <= 1) {
        m_statusLabel->setText("Cannot remove: at least one page required");
        return;
    }

    int lastIndex = m_toolBox->count() - 1;
    QWidget* pageWidget = m_toolBox->widget(lastIndex);
    m_toolBox->removeItem(lastIndex);
    // Qt parent ownership: deleting the widget after removal is safe
    delete pageWidget;

    m_statusLabel->setText(
        QString("Total pages: %1 | Current: %2")
            .arg(m_toolBox->count())
            .arg(m_toolBox->currentIndex() + 1));
}
