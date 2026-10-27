/// @file    progress_delegate.h
/// @brief   自定义 QStyledItemDelegate——在进度列中渲染可视化进度条。
///
/// 对应教程：进阶层 03-QtWidgets/03-Model/View 进阶。

#pragma once

#include <QStyledItemDelegate>

/// 进度条 Delegate——在 TableView 的进度列中绘制 QStyleOptionProgressBar。
///
/// 只重写 paint()，编辑功能回退到默认实现。
/// Delegate 的设计哲学是"按列定制"：只重写需要的列，其他列回退默认行为。
class ProgressDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit ProgressDelegate(QObject* parent = nullptr);

    /// @brief 自定义绘制——在进度列中画一个带文字的进度条。
    /// @param[in] painter 绘制器。
    /// @param[in] option 绘制选项（包含单元格矩形、选中状态等）。
    /// @param[in] index 当前单元格的模型索引。
    /// @note 必须先用 drawControl(CE_ItemViewItem) 画背景，否则选中行的蓝色高亮不会出现。
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};
