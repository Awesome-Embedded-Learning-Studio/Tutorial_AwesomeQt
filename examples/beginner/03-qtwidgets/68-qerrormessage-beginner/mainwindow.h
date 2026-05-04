#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QErrorMessage>
#include <QMainWindow>
#include <QTextEdit>

// ============================================================================
// MainWindow: 演示 QErrorMessage 各种用法
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void ensureErrorDialog();
    void onTriggerError();
    void onRepeatError();
    void onDifferentTypes();
    void onInstallHandler();
    void onTriggerWarning();
    void onTriggerCritical();
    void onResetSuppressed();

    QTextEdit *m_logEdit = nullptr;
    QErrorMessage *m_errorMsg = nullptr;
};

#endif // MAINWINDOW_H
