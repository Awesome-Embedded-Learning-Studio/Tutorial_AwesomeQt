#pragma once

#include <QWidget>

class QStackedLayout;
class QPushButton;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void switchPage(int index, QPushButton *activeBtn);

    QStackedLayout *m_stackedLayout = nullptr;
    QList<QPushButton *> m_navButtons;
};
