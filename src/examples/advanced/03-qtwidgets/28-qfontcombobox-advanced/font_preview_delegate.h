/// @file    font_preview_delegate.h
/// @brief   自定义 QStyledItemDelegate，在下拉列表中用字体自身渲染预览文本。
///
/// 对应教程：进阶层 03-QtWidgets/28-QFontComboBox 进阶。

#pragma once

#include <QStyledItemDelegate>

/// QFontComboBox 下拉列表自定义委托。
///
/// 左侧用粗体显示字体名称（使用对应字体渲染），
/// 右侧用对应字体渲染预览文本 "AaBbCc 123"，
/// 让用户在下拉列表中直接看到字体效果。
class FontPreviewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit FontPreviewDelegate(QObject* parent = nullptr);

    /// @brief 绘制单行选项：背景 + 左侧字体名称（粗体）+ 右侧预览文本。
    /// @param[in] painter 绘图设备。
    /// @param[in] option  绘制选项（含 rect、state 等）。
    /// @param[in] index   当前项的 Model 索引。
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /// @brief 返回每行的推荐尺寸，适当加高以容纳预览文本。
    /// @param[in] option 绘制选项。
    /// @param[in] index  当前项的 Model 索引。
    /// @return 推荐的行尺寸。
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

private:
    /// @brief 预览用的固定示例文本。
    static constexpr const char* kPreviewText = "AaBbCc 123";
};
