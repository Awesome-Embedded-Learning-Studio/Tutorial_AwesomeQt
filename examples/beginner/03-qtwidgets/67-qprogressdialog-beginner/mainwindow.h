#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QThread>

#include "worker.h"

// ============================================================================
// MainWindow: 启动后台任务并显示进度对话框
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void onStartTask();

    QPushButton *m_startButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    QThread *m_thread = nullptr;
    Worker *m_worker = nullptr;
};

#endif // MAINWINDOW_H
