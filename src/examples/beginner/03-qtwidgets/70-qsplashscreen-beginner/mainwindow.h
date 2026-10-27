#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// ============================================================================
// 主窗口: 启动完成后显示
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        double elapsedSec,
        QWidget *parent = nullptr);
};

#endif // MAINWINDOW_H
