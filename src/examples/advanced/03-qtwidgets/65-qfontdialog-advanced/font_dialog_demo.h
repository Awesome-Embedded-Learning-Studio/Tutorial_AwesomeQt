/// @file    font_dialog_demo.h
/// @brief   演示 QFontDialog 过滤字体并预览效果。
///
/// 对应教程：进阶层 03-QtWidgets/65-QFontDialog 进阶。
/// 展示 QFontDatabase 字体过滤、getFont() 静态方法与对话框对象方式对比、
/// 以及选中字体的实时预览与信息显示。

#pragma once

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QPushButton;

/// @brief 字体选择与预览演示控件。
class FontDialogDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit FontDialogDemo(QWidget* parent = nullptr);

private slots:
    /// @brief 使用静态方法 getFont() 选择字体。
    void pickFontStatic();

    /// @brief 使用 QFontDialog 对象方式选择字体（可定制）。
    void pickFontObject();

    /// @brief 切换是否只显示等宽字体。
    /// @param[in] checked 复选框是否选中。
    void toggleMonospaceOnly(bool checked);

private:
    /// @brief 构建界面。
    void setupUI();

    /// @brief 应用字体到预览区并更新信息标签。
    /// @param[in] font 选中的字体。
    void applyFont(const QFont& font);

    QPlainTextEdit* m_previewEdit;     ///< 字体预览文本编辑区
    QLabel* m_fontInfoLabel;           ///< 字体信息标签
    QPushButton* m_staticBtn;          ///< 静态方法按钮
    QPushButton* m_objectBtn;          ///< 对象方式按钮
    bool m_monospaceOnly;              ///< 是否只显示等宽字体
};
