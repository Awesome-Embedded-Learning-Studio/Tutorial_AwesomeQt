#include "mainwindow.h"

#include "aboutpage.h"
#include "appearancepage.h"
#include "profilepage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Qt 布局系统演示");
    resize(700, 500);

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto *navWidget = new QWidget;
    navWidget->setFixedWidth(160);
    navWidget->setStyleSheet(
        "QWidget { background-color: #2C3E50; }"
        "QPushButton {"
        "  color: #ECF0F1;"
        "  background-color: transparent;"
        "  border: none;"
        "  padding: 12px 16px;"
        "  text-align: left;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #34495E; }"
        "QPushButton:checked { background-color: #1ABC9C; }"
    );

    auto *navLayout = new QVBoxLayout(navWidget);
    navLayout->setContentsMargins(0, 10, 0, 0);
    navLayout->setSpacing(0);

    auto *navTitle = new QLabel("导航菜单");
    navTitle->setAlignment(Qt::AlignCenter);
    navTitle->setStyleSheet(
        "color: #BDC3C7; font-size: 12px; padding: 8px;"
        "border-bottom: 1px solid #34495E;");
    navLayout->addWidget(navTitle);

    auto *btnProfile = new QPushButton("  个人信息");
    auto *btnAppearance = new QPushButton("  外观设置");
    auto *btnAbout = new QPushButton("  关于");

    btnProfile->setCheckable(true);
    btnAppearance->setCheckable(true);
    btnAbout->setCheckable(true);
    btnProfile->setChecked(true);

    navLayout->addWidget(btnProfile);
    navLayout->addWidget(btnAppearance);
    navLayout->addWidget(btnAbout);
    navLayout->addStretch();

    mainLayout->addWidget(navWidget);

    auto *stackedWidget = new QWidget;
    auto *contentLayout = new QVBoxLayout(stackedWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    m_stackedLayout = new QStackedLayout;

    m_stackedLayout->addWidget(new ProfilePage(this));
    m_stackedLayout->addWidget(new AppearancePage(this));
    m_stackedLayout->addWidget(new AboutPage(this));

    contentLayout->addLayout(m_stackedLayout);
    mainLayout->addWidget(stackedWidget, 1);

    connect(btnProfile, &QPushButton::clicked, this, [this, btnProfile]() {
        switchPage(0, btnProfile);
    });
    connect(btnAppearance, &QPushButton::clicked, this,
            [this, btnAppearance]() {
                switchPage(1, btnAppearance);
            });
    connect(btnAbout, &QPushButton::clicked, this, [this, btnAbout]() {
        switchPage(2, btnAbout);
    });

    m_navButtons = {btnProfile, btnAppearance, btnAbout};
}

void MainWindow::switchPage(int index, QPushButton *activeBtn)
{
    m_stackedLayout->setCurrentIndex(index);
    for (auto *btn : std::as_const(m_navButtons)) {
        btn->setChecked(btn == activeBtn);
    }
}
