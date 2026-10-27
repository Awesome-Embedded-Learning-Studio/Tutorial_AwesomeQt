/// @file    file_dialog_demo.h
/// @brief   演示 QFileDialog 自定义侧边栏 URL、名称过滤与文件模式选项。
///
/// 对应教程：进阶层 03-QtWidgets/66-QFileDialog 进阶。
/// 展示不同的 QFileDialog 配置：打开文件、保存文件、选择目录，
/// 以及 DontUseNativeDialog 选项获取 Qt 内置对话框以便自定义。

#pragma once

#include <QWidget>

class QLabel;

/// @brief 文件对话框高级用法演示控件。
class FileDialogDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit FileDialogDemo(QWidget* parent = nullptr);

private slots:
    /// @brief 打开文件（带名称过滤器）。
    void openFile();

    /// @brief 保存文件对话框。
    void saveFile();

    /// @brief 选择目录。
    void selectDirectory();

    /// @brief 使用 Qt 内置对话框（非原生）打开文件。
    void openFileQtDialog();

private:
    /// @brief 构建界面。
    void setupUI();

    /// @brief 更新结果标签。
    /// @param[in] text 要显示的文件路径。
    void updateResult(const QString& text);

    QLabel* m_resultLabel;          ///< 显示选择结果的标签
};
