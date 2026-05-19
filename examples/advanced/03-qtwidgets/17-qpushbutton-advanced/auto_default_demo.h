/// @file    auto_default_demo.h
/// @brief   演示 QPushButton 在 QDialog 中的 autoDefault 键盘拦截行为。
///
/// 对应教程：进阶层 03-QtWidgets/17-QPushButton 进阶。

#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QTextEdit;

/// QPushButton autoDefault 行为演示对话框。
///
/// 展示三个进阶知识点：
/// - autoDefault 在 QDialog 中拦截 Enter 键，阻止 QLineEdit::returnPressed
/// - setMenu() 导致 clicked() 信号被抑制
/// - flat 按钮的焦点框残留与 setFocusPolicy(Qt::NoFocus) 解决方案
class AutoDefaultDemo : public QDialog
{
    Q_OBJECT

public:
    /// @brief 构造函数，搭建演示界面。
    /// @param[in] parent 父控件指针。
    explicit AutoDefaultDemo(QWidget* parent = nullptr);

private:
    /// @brief 构建"autoDefault 拦截"演示区域。
    /// @param[in] parent 布局所属的容器控件。
    QWidget* createAutoDefaultSection(QWidget* parent);

    /// @brief 构建"setMenu 信号抑制"演示区域。
    /// @param[in] parent 布局所属的容器控件。
    QWidget* createMenuSection(QWidget* parent);

    /// @brief 构建"flat 按钮状态"演示区域。
    /// @param[in] parent 布局所属的容器控件。
    QWidget* createFlatSection(QWidget* parent);

    /// @brief 向日志区追加一条消息，附带时间戳。
    /// @param[in] message 要显示的日志文本。
    void appendLog(const QString& message);

    QTextEdit* m_logOutput;          // 日志输出区，记录信号触发情况
};
