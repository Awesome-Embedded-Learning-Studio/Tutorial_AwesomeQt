/// @file    drag_drop_list_view.h
/// @brief   支持内部拖拽排序的 QListView 子类。
///
/// 演示 QAbstractItemView 拖拽框架的四个核心属性配置：
/// setDragEnabled、setAcceptDrops、setDragDropMode(InternalMove)、
/// setDefaultDropAction(Qt::MoveAction)。
///
/// 对应教程：进阶层 03-QtWidgets/15-QAbstractItemView 基类进阶。

#pragma once

#include <QListView>

class QStandardItemModel;

/// 可拖拽排序的列表视图。
///
/// 继承 QListView，在构造函数中一次性配置好拖拽排序所需的全部属性。
/// 搭配 QStandardItemModel 使用，利用其内置的 mimeData / dropMimeData /
/// removeRows 默认实现，无需自定义模型即可完成 InternalMove 排序。
class DragDropListView : public QListView
{
    Q_OBJECT

public:
    /// @brief 构造函数，配置拖拽属性并初始化模型数据。
    /// @param[in] parent 父控件指针。
    /// @note 必须在构造函数中完成模型创建与属性配置，确保用户首次拖拽时行为正确。
    explicit DragDropListView(QWidget* parent = nullptr);

    /// @brief 获取内部使用的标准模型指针。
    /// @return QStandardItemModel 指针，生命周期由本类管理。
    QStandardItemModel* model() const;
};
