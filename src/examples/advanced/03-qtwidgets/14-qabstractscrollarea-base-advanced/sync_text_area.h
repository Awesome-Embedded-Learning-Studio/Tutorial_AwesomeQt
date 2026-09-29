/// @file    sync_text_area.h
/// @brief   自定义 QAbstractScrollArea 子类，重写 scrollContentsBy 并发射同步信号。
///
/// 演示 QAbstractScrollArea 子类化、scrollContentsBy 虚函数重写、
/// 视口坐标计算以及通过信号通知外部滚动位置变化。
///
/// 对应教程：进阶层 03-QtWidgets/14-QAbstractScrollArea 基类进阶。

#pragma once

#include <QAbstractScrollArea>

/// 可同步滚动的纯文本显示区域。
///
/// 继承 QAbstractScrollArea，手动管理文本内容的绘制与滚动。
/// 重写 scrollContentsBy 在滚动时发射 scrollChanged 信号，
/// 供外部实现双区域同步滚动。
class SyncTextArea : public QAbstractScrollArea
{
    Q_OBJECT

public:
    /// @brief 构造函数，加载示例文本并初始化滚动条范围。
    /// @param[in] parent 父控件指针。
    explicit SyncTextArea(QWidget* parent = nullptr);

    /// @brief 设置要显示的文本行列表，同时更新滚动条范围。
    /// @param[in] lines 文本行列表。
    void setTextLines(const QStringList& lines);

    /// @brief 返回当前垂直滚动条的值。
    /// @return 当前滚动位置（像素值）。
    int scrollValue() const;

    /// @brief 设置垂直滚动条到指定值，不发射信号（用于同步场景）。
    /// @param[in] value 目标滚动位置。
    void setScrollValueSilently(int value);

signals:
    /// @brief 滚动位置发生变化时发射，供外部同步使用。
    /// @param[in] value 新的垂直滚动条值。
    void scrollChanged(int value);

protected:
    /// @brief 滚动内容时由 QAbstractScrollArea 内部调用。
    /// @param[in] dx 水平滚动偏移（像素）。
    /// @param[in] dy 垂直滚动偏移（像素）。
    void scrollContentsBy(int dx, int dy) override;

    /// @brief 在视口上绘制可见的文本行。
    /// @param[in] event 绘制事件，包含需要重绘的区域。
    void paintEvent(QPaintEvent* event) override;

    /// @brief 鼠标滚轮事件，转发给垂直滚动条。
    /// @param[in] event 滚轮事件。
    void wheelEvent(QWheelEvent* event) override;

private:
    /// @brief 根据文本行数和字体高度重新计算滚动条范围。
    void updateScrollBarRange();

    /// @brief 生成用于演示的示例文本。
    /// @return 包含多行文本的字符串列表。
    static QStringList generateSampleText();

    QStringList m_lines;          // 要显示的文本行
    int m_lineHeight;             // 每行文本的像素高度
    int m_totalHeight;            // 所有文本行的总像素高度
};
