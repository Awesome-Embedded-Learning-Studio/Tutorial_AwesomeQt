/**
 * @file checkbox_tree.h
 * @brief 勾选树控件 CheckboxTree——封装 QTreeWidget，三态勾选 + 父子自动联动
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */
#pragma once

#include <QList>
#include <QString>
#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;

namespace AwesomeQt {

/// @brief 勾选树：每项带复选框，父子联动——勾父全选子、子部分勾则父三态。
///
/// 设计要点：
/// - 内含一个 `QTreeWidget*`（构造期 new，parent=this 由对象树托管），本控件不自绘，
///   只负责「数据模型组织 + 勾选联动逻辑」；
/// - 父子联动靠 `QTreeWidget::itemChanged` 驱动：用户点击某项后，向下传播状态给子孙，
///   再从该项目的父起逐层向上重算 tri-state；
/// - 程序化 `setCheckState` 会再次触发 `itemChanged`，递归改子孙时必须 `blockSignals`
///   守卫，否则信号雪崩（栈溢出 / 性能塌陷）。
class CheckboxTree : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：联动开关 / 缩进，可被 Designer / 动画驱动 ——
    Q_PROPERTY(bool propagationEnabled READ isPropagationEnabled WRITE setPropagationEnabled NOTIFY
                   propagationEnabledChanged)
    Q_PROPERTY(int indentation READ indentation WRITE setIndentation NOTIFY indentationChanged)

  public:
    explicit CheckboxTree(QWidget* parent = nullptr);

    /// @brief 追加一项到指定父下（parent=nullptr 表示顶层）。
    /// @param parent 父节点，传 nullptr 则加到树根。
    /// @param text 该项显示文本（第 0 列）。
    /// @param state 初始勾选状态，默认未勾选。
    /// @return 新建并已挂入树的节点指针。
    QTreeWidgetItem* addItem(QTreeWidgetItem* parent, const QString& text,
                             Qt::CheckState state = Qt::Unchecked);

    /// @brief 收集所有处于 Checked 状态的项（不含 PartiallyChecked）。
    /// @return 已勾选项列表，空树返回空列表。
    QList<QTreeWidgetItem*> checkedItems() const;

    /// @brief 以联动逻辑设置某项的勾选状态（非裸 setCheckState）。
    /// @param item 目标节点，nullptr 安全返回。
    /// @param state 目标状态。
    void setItemChecked(QTreeWidgetItem* item, Qt::CheckState state);

    /// @brief 全部勾选（顶层逐项置 Checked 并触发联动）。
    void checkAll();

    /// @brief 全部取消勾选（顶层逐项置 Unchecked 并触发联动）。
    void uncheckAll();

    /// @brief 暴露内部 view，供外部读选中行 / 设置表头等只读访问。
    /// @return 内部 QTreeWidget 指针（所有权仍归本控件）。
    QTreeWidget* treeWidget() const;

    bool isPropagationEnabled() const;
    void setPropagationEnabled(bool enabled);

    int indentation() const;
    void setIndentation(int pixels);

    QSize sizeHint() const override;

  signals:
    /// @brief 任意项勾选状态变化（联动完成后发，已对自身递归修改去重）。
    void checkStateChanged(QTreeWidgetItem* item);
    void propagationEnabledChanged(bool enabled);
    void indentationChanged(int pixels);

  private:
    /// @brief itemChanged 槽：用户点击 / 程序改写的统一入口。
    void onItemChanged(QTreeWidgetItem* item, int column);

    /// @brief 把非 PartiallyChecked 状态递归传播给 item 的全部子孙。
    void propagateDown(QTreeWidgetItem* item, Qt::CheckState state);

    /// @brief 从 item 起逐层向上重算 tri-state（按子项混合度推导父态）。
    void recalcUp(QTreeWidgetItem* item);

    /// @brief 统计某节点直接子项的勾选分布，推导它的聚合态。
    Qt::CheckState aggregateState(const QTreeWidgetItem* item) const;

    /// @brief 是否正在程序化批量改写（用于 onItemChanged 区分用户触发 vs 自身触发）。
    bool is_propagating_{false};

    QTreeWidget* tree_{nullptr};
    bool propagation_enabled_{true};
};

} // namespace AwesomeQt
