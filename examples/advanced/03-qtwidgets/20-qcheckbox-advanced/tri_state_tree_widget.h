/// @file    tri_state_tree_widget.h
/// @brief   支持三态复选框父子联动传播的 QTreeWidget 子类。
///
/// 演示 checkState 的向下传播（parent → children）和向上冒泡（children → parent），
/// 以及 blockSignals 防递归的正确用法。
///
/// 对应教程：进阶层 03-QtWidgets/20-QCheckBox 进阶。

#pragma once

#include <QTreeWidget>

/// 三态复选框树形控件。
///
/// 核心机制：
/// - 用户勾选/取消一个节点时，自动向下传播到所有子孙节点
/// - 子孙节点变化后，自动向上冒泡计算每个祖先节点的状态
/// - 祖先节点状态规则：全 Checked → Checked，全 Unchecked → Unchecked，混合 → PartiallyChecked
/// - 使用 blockSignals 防止程序化设值时触发 itemChanged 导致无限递归
class TriStateTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，构建示例权限树并连接信号。
    /// @param[in] parent 父控件指针。
    /// @note itemChanged 信号在构造函数末尾才连接，避免构建过程中触发传播。
    explicit TriStateTreeWidget(QWidget* parent = nullptr);

private:
    /// @brief 构建"角色 → 模块 → 权限"三层权限树。
    void buildPermissionTree();

    /// @brief itemChanged 槽函数，处理三态复选框的父子联动。
    /// @param[in] item 发生变化的树节点。
    /// @param[in] column 变化所在的列。
    /// @note 使用 m_updating 标志位防止递归进入，内部通过 blockSignals
    ///       阻止程序化设值时再次触发 itemChanged。
    void onItemChanged(QTreeWidgetItem* item, int column);

    /// @brief 将 checkState 从 parent 向下传播到所有子孙节点。
    /// @param[in] parent 起始父节点。
    /// @param[in] state 要设置的复选状态。
    /// @note 传播期间 blockSignals(true)，完成后立即 blockSignals(false)，
    ///       确保最小作用域——不阻塞其他信号。
    void propagateToChildren(QTreeWidgetItem* parent, Qt::CheckState state);

    /// @brief 从给定节点向上冒泡，逐层更新祖先节点的 checkState。
    /// @param[in] item 起始节点（通常是发生变化的叶子或中间节点）。
    /// @note 逐层向上，每层根据子节点的综合状态计算当前节点的状态。
    ///       使用 blockSignals 的最小作用域模式防止信号递归。
    void bubbleToParents(QTreeWidgetItem* item);

    /// @brief 根据所有子节点的 checkState 计算父节点应有的状态。
    /// @param[in] parent 父节点。
    /// @return 计算得到的 Qt::CheckState。
    /// @note 规则：全 Checked → Checked，全 Unchecked → Unchecked，其余 → PartiallyChecked。
    ///       递归考虑子节点的子节点，保证语义一致性。
    Qt::CheckState computeParentState(QTreeWidgetItem* parent) const;

    bool m_updating;  // 防递归标志位，true 表示正在传播中
};
