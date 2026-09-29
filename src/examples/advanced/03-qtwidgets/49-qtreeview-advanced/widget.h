/// @file    widget.h
/// @brief   演示 QTreeView 自定义展开图标与整行选中。
///
/// 本文件包含：
/// - FullRowDelegate：自定义 QStyledItemDelegate，绘制三角形展开/折叠箭头
///   并实现整行选中高亮
/// - Widget：主窗口，组装 TreeModel + QTreeView + FullRowDelegate
///
/// 对应教程：进阶层 03-QtWidgets/49-qtreeview-advanced。

#pragma once

#include <QLabel>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

#include "tree_model.h"

/// @brief 自定义 ItemDelegate，实现三角形展开图标和整行选中高亮。
///
/// 核心功能：
/// - 在第一列左侧绘制向右/向下的三角形作为展开/折叠指示器
/// - 选中时整行绘制高亮背景（跨越所有列），而非仅第一列
/// - 通过 QStyleOptionViewItem 的 state 判断展开/选中状态
class FullRowDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] treeView 关联的 QTreeView 指针，用于查询展开状态。
    /// @param[in] parent   父对象指针。
    /// @note 需要保存 treeView 指针以查询节点是否展开。
    explicit FullRowDelegate(QTreeView* treeView, QObject* parent = nullptr);

    /// @brief 自定义绘制函数。
    /// @param[in] painter 画笔。
    /// @param[in] option  绘制选项（含状态、矩形区域等）。
    /// @param[in] index   模型索引。
    /// @note 核心绘制逻辑：整行背景 + 三角形展开图标 + 文本。
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /// @brief 返回项的尺寸提示。
    /// @param[in] option 绘制选项。
    /// @param[in] index  模型索引。
    /// @note 增加行高以留出展开图标的空间。
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

private:
    /// @brief 绘制三角形展开/折叠图标。
    /// @param[in] painter 画笔。
    /// @param[in] rect    图标绘制区域。
    /// @param[in] expanded 是否展开。
    /// @note 展开时绘制向下的实心三角形，折叠时绘制向右的三角形。
    void drawExpandTriangle(QPainter* painter, const QRect& rect,
                            bool expanded) const;

    QTreeView* m_treeView;  ///< 关联的树视图，用于查询展开状态
};

/// @brief 主窗口，包含 QTreeView 和状态信息。
///
/// 演示 TreeModel + FullRowDelegate 的组合使用，
/// 底部标签显示当前选中节点的路径信息。
class Widget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化模型、视图和委托。
    /// @param[in] parent 父控件指针。
    explicit Widget(QWidget* parent = nullptr);

private:
    /// @brief 响应选中项变化，更新状态标签。
    /// @param[in] current 当前选中索引。
    /// @note 递归向上遍历父索引构建节点路径字符串。
    void onSelectionChanged(const QModelIndex& current);

    TreeModel* m_model;          ///< 自定义树形模型
    QTreeView* m_treeView;       ///< 树视图
    FullRowDelegate* m_delegate; ///< 自定义委托
    QLabel* m_statusLabel;       ///< 底部状态标签
};
