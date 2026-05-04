#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>

#include "SettingsDialog.h"

// ============================================================================
// MainWindow: 主窗口，演示按钮盒对话框
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void onOpenSettings();

    QTextEdit *m_resultEdit = nullptr;
};

#endif // MAINWINDOW_H
