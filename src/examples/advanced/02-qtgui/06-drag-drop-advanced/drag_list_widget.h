/// @file    drag_list_widget.h
/// @brief   支持自定义 MIME 拖拽的源列表控件。
///
/// 对应教程：进阶层 02-QtGui/06-拖放系统高级用法。
/// 演示如何将列表项编码为自定义 MIME 类型（application/x-custom-item）
/// 并通过 QDrag 发起拖拽操作。

#pragma once

#include <QListWidget>

/// @brief 拖拽源列表控件，支持将选中项以自定义 MIME 格式拖出。
///
/// 重写 startDrag() 以构建包含自定义 MIME 数据的 QMimeData，
/// 编码格式为 "item_text|row_index"，MIME 类型为
/// "application/x-custom-item"。
class DragListWidget : public QListWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，配置拖拽行为。
    /// @param[in] parent 父控件指针。
    explicit DragListWidget(QWidget* parent = nullptr);

protected:
    /// @brief 重写拖拽发起逻辑，构建自定义 MIME 数据。
    /// @param[in] supportedActions 支持的拖放动作（Copy/Move 等）。
    void startDrag(Qt::DropActions supportedActions) override;
};
