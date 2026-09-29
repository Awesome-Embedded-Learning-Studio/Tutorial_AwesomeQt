#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>

#include "UserConfigDialog.h"
#include "FindDialog.h"

// ============================================================================
// MainWindow: 主窗口，演示模态和非模态对话框
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void onNewUser();
    void onFind();
    void onWindowModal();

    QTextEdit *m_resultEdit = nullptr;
};

#endif // MAINWINDOW_H
