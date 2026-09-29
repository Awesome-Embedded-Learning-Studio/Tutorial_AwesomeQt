/// @file    line_number_editor.h
/// @brief   带行号侧边栏的 QPlainTextEdit 子类。
///
/// 管理 LineNumberSidebar 的生命周期和布局，
/// 连接 blockCountChanged / updateRequest / resizeEvent 三信号
/// 实现行号区域与编辑器内容的同步。
///
/// 对应教程：进阶层 03-QtWidgets/24-QPlainTextEdit 进阶。

#pragma once

#include <QPlainTextEdit>

class LineNumberSidebar;

/// 带行号侧边栏的纯文本编辑器。
///
/// 声明 LineNumberSidebar 为友元类，允许侧边栏访问
/// firstVisibleBlock / blockBoundingRect / contentOffset 等 protected 接口。
/// 这是 Qt Code Editor Example 中的标准模式。
///
/// 在左侧通过 setViewportMargins 预留空间放置 LineNumberSidebar，
/// 监听 blockCountChanged 更新侧边栏宽度，
/// 监听 updateRequest 同步绘制行号。
class LineNumberEditor : public QPlainTextEdit
{
    Q_OBJECT
    friend class LineNumberSidebar;

public:
    /// @brief 构造函数，初始化行号侧边栏和信号连接。
    /// @param[in] parent 父控件指针。
    explicit LineNumberEditor(QWidget* parent = nullptr);

protected:
    /// @brief 窗口大小变化时重新定位行号侧边栏。
    /// @param[in] event 尺寸变化事件。
    void resizeEvent(QResizeEvent* event) override;

private slots:
    /// @brief 文档 block 数量变化时更新行号区域宽度。
    /// @param[in] newCount 新的 block 数量。
    void onBlockCountChanged(int newCount);

    /// @brief 编辑器请求重绘时同步更新行号区域。
    /// @param[in] rect 需要重绘的矩形区域。
    /// @param[in] dy 垂直滚动偏移（像素）。
    void onUpdateRequest(const QRect& rect, int dy);

private:
    /// @brief 更新行号侧边栏的宽度和视口边距。
    void updateSidebarWidth();

    LineNumberSidebar* m_sidebar;   // 行号侧边栏（子控件，Qt 对象树自动管理）
};
