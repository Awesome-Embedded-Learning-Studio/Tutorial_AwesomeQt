#pragma once

#include <QWidget>

class QLabel;
class StudentTablePanel;
class NameListPanel;

// 主窗口：组合表格面板和姓名列表面板
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void updateStatus();

    StudentTablePanel *m_tablePanel = nullptr;
    NameListPanel *m_namePanel = nullptr;
    QLabel *m_statusLabel = nullptr;
};
