/// @file    drop_list_widget.h
/// @brief   支持自定义 MIME 放下的目标列表控件。
///
/// 对应教程：进阶层 02-QtGui/06-拖放系统高级用法。
/// 演示如何检测自定义 MIME 类型、解码拖入数据并显示。

#pragma once

#include <QListWidget>

/// @brief 放下目标列表控件，接受自定义 MIME 类型的拖入数据。
///
/// 重写 dragEnterEvent / dragMoveEvent / dropEvent 三大事件处理器，
/// 仅接受 "application/x-custom-item" MIME 类型的数据。
/// dropEvent 中通过 QDataStream 反序列化原始字段。
class DropListWidget : public QListWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，配置放下行为。
    /// @param[in] parent 父控件指针。
    explicit DropListWidget(QWidget* parent = nullptr);

signals:
    /// @brief 当新项被放下时发射，携带解码后的文本。
    /// @param text 被放下项的文本内容。
    void itemDropped(const QString& text);

protected:
    /// @brief 拖拽进入控件时触发，判断是否接受该拖拽。
    /// @param[in] event 拖拽进入事件。
    void dragEnterEvent(QDragEnterEvent* event) override;

    /// @brief 拖拽在控件内移动时触发，持续判断是否接受。
    /// @param[in] event 拖拽移动事件。
    void dragMoveEvent(QDragMoveEvent* event) override;

    /// @brief 放下时触发，解码自定义 MIME 数据并添加到列表。
    /// @param[in] event 放下事件。
    void dropEvent(QDropEvent* event) override;
};
