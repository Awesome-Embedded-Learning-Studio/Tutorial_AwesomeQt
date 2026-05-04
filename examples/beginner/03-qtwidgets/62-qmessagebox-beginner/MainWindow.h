#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// QtWidgets 入门示例 62: QMessageBox 消息对话框
// 演示：information / warning / critical / question 四种静态方法
//       自定义按钮文字
//       setDetailedText 折叠显示技术细节
//       工作线程通过 QMetaObject::invokeMethod 线程安全触发

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 演示四种 QMessageBox + 线程安全触发
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // information: 纯信息提示
    // ====================================================================
    void onInformation();

    // ====================================================================
    // warning: 警告 + 用户选择
    // ====================================================================
    void onWarning();

    // ====================================================================
    // critical: 严重错误 + setDetailedText
    // ====================================================================
    void onCritical();

    // ====================================================================
    // question: 确认操作 + 自定义按钮文字
    // ====================================================================
    void onQuestion();

    // ====================================================================
    // 模拟后台任务: QTimer + QMetaObject::invokeMethod
    // ====================================================================
    void onBackgroundTask();

    void bgBtnSetEnabled(bool enabled);

    QTextEdit *m_resultEdit = nullptr;
};

#endif // MAINWINDOW_H
