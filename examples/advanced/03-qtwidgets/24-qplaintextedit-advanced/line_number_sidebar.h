/// @file    line_number_sidebar.h
/// @brief   行号侧边栏控件，作为 QPlainTextEdit 的同伴绘制行号。
///
/// 根据 blockCount 动态计算宽度，在 paintEvent 中遍历可见 block
/// 绘制对应的行号文字。
///
/// 对应教程：进阶层 03-QtWidgets/24-QPlainTextEdit 进阶。

#pragma once

#include <QWidget>

class LineNumberEditor;

/// 行号侧边栏控件。
///
/// 固定在 LineNumberEditor 左侧，根据编辑器的 block 布局
/// 绘制对应的行号。宽度随行数动态变化（例如从 99 行变成 100 行时
/// 行号区域从两位数变为三位数宽度）。
class LineNumberSidebar : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] editor 关联的行号编辑器指针。
    explicit LineNumberSidebar(LineNumberEditor* editor);

    /// @brief 根据当前 blockCount 计算行号区域的推荐宽度。
    /// @return 推荐宽度（像素）。
    int calculateWidth() const;

    /// @brief 返回关联的编辑器指针。
    /// @return LineNumberEditor 指针。
    LineNumberEditor* editor() const;

protected:
    /// @brief 绘制行号。
    /// @param[in] event 绘制事件。
    void paintEvent(QPaintEvent* event) override;

private:
    LineNumberEditor* m_editor;    // 关联的编辑器（非拥有）
};
