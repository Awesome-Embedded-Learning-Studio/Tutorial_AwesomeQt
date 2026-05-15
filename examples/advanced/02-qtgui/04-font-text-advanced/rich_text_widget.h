/// @file    rich_text_widget.h
/// @brief   演示 QTextDocument 富文本编辑与格式控制。
///
/// 对应教程：进阶层 02-QtGui/04-字体与文本高级用法。
/// 展示 QTextCharFormat / QTextBlockFormat / QTextCursor 的配合使用，
/// 以及如何通过 QTextDocument 获取文档结构信息。

#pragma once

#include <QWidget>

class QHBoxLayout;
class QPushButton;
class QTextEdit;
class QLabel;

/// @brief 富文本编辑控件，演示 QTextDocument 的格式化能力。
///
/// 提供 Bold / Italic / 颜色 / 对齐等格式按钮，并通过 QTextCursor
/// 和 QTextCharFormat / QTextBlockFormat 对选中文字或段落施加格式。
/// 底部状态栏实时显示文档的 block 数和字符数。
class RichTextWidget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化 UI 布局与信号槽连接。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit RichTextWidget(QWidget* parent = nullptr);

private slots:
    /// @brief 切换选中文字的粗体状态。
    void toggleBold();

    /// @brief 切换选中文字的斜体状态。
    void toggleItalic();

    /// @brief 弹出颜色选择器，为选中文字设置前景色。
    void pickTextColor();

    /// @brief 将当前段落设为居中对齐。
    void alignCenter();

    /// @brief 将当前段落设为左对齐。
    void alignLeft();

    /// @brief 刷新底部状态栏中的文档结构信息。
    void updateDocumentInfo();

private:
    /// @brief 创建格式工具栏按钮并水平排列。
    /// @return 包含所有工具按钮的布局。
    auto createToolbarLayout() -> QHBoxLayout*;

    /// @brief 创建底部文档信息标签。
    /// @return 信息标签指针。
    auto createInfoLabel() -> QLabel*;

    QTextEdit* m_textEdit;       ///< 富文本编辑区域
    QLabel*    m_infoLabel;      ///< 文档结构信息展示
    QPushButton* m_boldButton;   ///< 粗体切换按钮
    QPushButton* m_italicButton; ///< 斜体切换按钮
};
