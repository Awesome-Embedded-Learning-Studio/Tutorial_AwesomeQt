#pragma once

// QtWidgets 入门示例 20: QCheckBox 复选框
// 演示：三态复选框：setTristate(true) 与 Qt::PartiallyChecked
//       checkStateChanged(Qt::CheckState) vs toggled(bool) 信号区别
//       复选框组实现"全选/全不选"逻辑
//       与 QTreeWidget 结合的层级复选

#include <QCheckBox>
#include <QLabel>
#include <QTreeWidget>
#include <QWidget>

#include <vector>

// ============================================================================
// CheckBoxDemo: QCheckBox 综合演示
// ============================================================================
class CheckBoxDemo : public QWidget
{
    Q_OBJECT

public:
    explicit CheckBoxDemo(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 构建示例树形结构
    void buildTree();

    /// @brief 根据 children 选中情况更新"全选"状态
    void updateSelectAllState();

    /// @brief 向下传播: 递归设置 item 及其子孙节点的 checkState
    void updateTreeChildren(QTreeWidgetItem *item, Qt::CheckState state);

    /// @brief 向上冒泡: 根据 children 的选中情况更新 parent 状态
    void updateTreeParent(QTreeWidgetItem *parent);

    /// @brief 统计选中数量并更新状态标签
    void updateStatus();

private:
    QCheckBox *m_selectAllCheck = nullptr;
    std::vector<QCheckBox *> m_childChecks;
    QTreeWidget *m_treeWidget = nullptr;
    QLabel *m_statusLabel = nullptr;
    bool m_updating = false;
};
