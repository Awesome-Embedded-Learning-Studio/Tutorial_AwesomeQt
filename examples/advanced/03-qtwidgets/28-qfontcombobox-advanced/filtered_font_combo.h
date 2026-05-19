/// @file    filtered_font_combo.h
/// @brief   带 Writing System 过滤与字体预览的 QFontComboBox 演示面板。
///
/// 对应教程：进阶层 03-QtWidgets/28-QFontComboBox 进阶。

#pragma once

#include <QWidget>

class QComboBox;
class QFontComboBox;
class QLabel;

/// 字体选择面板，结合 Writing System 过滤与自定义 Delegate 字体预览。
///
/// 上方有一个"书写系统"QComboBox，控制 QFontComboBox 通过
/// setWritingSystem 过滤只显示支持该书写系统的字体。
/// QFontComboBox 安装了 FontPreviewDelegate，在下拉列表中
/// 用每个字体自身渲染名称 + 预览文本。
/// 下方 QLabel 实时显示当前选中字体的预览。
class FilteredFontCombo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit FilteredFontCombo(QWidget* parent = nullptr);

private:
    /// @brief 填充书写系统下拉框，从 QFontDatabase::writingSystems() 获取。
    void populateWritingSystems();

    /// @brief 根据选中的书写系统更新 QFontComboBox 的过滤。
    void applyWritingSystemFilter();

    /// @brief 更新底部预览标签的字体和示例文本。
    void updatePreview();

    /// @brief 根据当前书写系统返回对应的示例预览文本。
    /// @return 包含示例文字的字符串。
    QString sampleTextForCurrentWritingSystem() const;

    // --- UI 成员 ---
    QComboBox* m_writingSystemCombo;     // 书写系统选择下拉框
    QFontComboBox* m_fontCombo;          // 字体选择下拉框（带自定义 delegate）
    QLabel* m_previewLabel;              // 底部字体预览标签
    QLabel* m_infoLabel;                 // 显示当前字体信息的标签
};
