// QtWidgets 入门示例 65: QFontDialog 字体选择对话框
// 演示：getFont 模态选择
//       setCurrentFont 初始预选
//       currentFontChanged 实时预览
//       QFontDatabase 过滤等宽字体

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFontDialog>
#include <QMainWindow>
#include <QTextEdit>

#include "monofontdialog.h"

// ============================================================================
// MainWindow: 演示 QFontDialog 三种用法 + 自定义等宽字体对话框
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // 模态字体选择
    // ====================================================================
    void onPickFont();

    // ====================================================================
    // 非模态实时预览
    // ====================================================================
    void onLivePreview();

    // ====================================================================
    // 自定义等宽字体选择对话框
    // ====================================================================
    void onMonoFont();

    QTextEdit *m_textEdit = nullptr;
    QFontDialog *m_fontDialog = nullptr;
};

#endif // MAINWINDOW_H
