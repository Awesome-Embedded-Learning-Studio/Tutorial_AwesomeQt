/// @file    tree_model.h
/// @brief   自定义树形模型，为 QTreeView 提供层级数据。
///
/// 本文件实现一个简单的 QAbstractItemModel 子类，
/// 支持多层级树形数据结构，用于配合自定义 delegate 演示展开图标和整行选中。
///
/// 对应教程：进阶层 03-QtWidgets/49-qtreeview-advanced。

#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <memory>
#include <vector>

/// @brief 树节点的内部数据结构。
///
/// 每个节点保存名称、值、子节点列表和父节点指针，
/// 用于构建任意深度的树形结构。
class TreeNode
{
public:
    /// @brief 构造树节点。
    /// @param[in] name   节点名称（显示在第一列）。
    /// @param[in] value  节点值（显示在第二列）。
    /// @param[in] parent 父节点指针，顶层节点传 nullptr。
    explicit TreeNode(const QString& name, const QString& value = QString(),
                      TreeNode* parent = nullptr);

    /// @brief 析构时自动删除所有子节点。
    ~TreeNode();

    // 禁止拷贝
    TreeNode(const TreeNode&) = delete;
    TreeNode& operator=(const TreeNode&) = delete;

    /// @brief 添加子节点。
    /// @param[in] child 要添加的子节点指针（所有权归本节点）。
    void appendChild(std::unique_ptr<TreeNode> child);

    /// @brief 获取指定索引的子节点。
    /// @param[in] index 子节点索引。
    /// @return 子节点指针，越界返回 nullptr。
    TreeNode* child(int index) const;

    /// @brief 获取子节点数量。
    /// @return 子节点个数。
    int childCount() const;

    /// @brief 获取本节点在父节点子列表中的行号。
    /// @return 行号，顶层节点返回 0。
    int row() const;

    /// @brief 获取父节点。
    /// @return 父节点指针，顶层节点返回 nullptr。
    TreeNode* parent() const;

    /// @brief 获取节点数据。
    /// @param[in] column 列号（0=名称, 1=值）。
    /// @return 对应列的数据。
    QVariant data(int column) const;

    /// @brief 获取列数。
    /// @return 固定返回 2（名称列 + 值列）。
    int columnCount() const;

private:
    QString m_name;                             ///< 节点名称（第一列）
    QString m_value;                            ///< 节点值（第二列）
    TreeNode* m_parent;                         ///< 父节点指针（非拥有）
    std::vector<std::unique_ptr<TreeNode>> m_children;  ///< 子节点列表（拥有）
};

/// @brief 自定义树形模型，封装 TreeNode 为 QAbstractItemModel 接口。
///
/// 为 QTreeView 提供两列（名称、值）的层级数据，
/// 配合自定义 delegate 展示自定义展开图标和整行选中效果。
class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建并初始化示例数据。
    /// @param[in] parent 父对象指针。
    /// @note 在构造时即构建完整示例树，本示例不涉及动态增删。
    explicit TreeModel(QObject* parent = nullptr);

    /// @brief 析构函数，释放根节点。
    ~TreeModel() override;

    // -- QAbstractItemModel 必须实现的接口 --

    /// @brief 返回指定父节点下的行数。
    /// @param[in] parent 父索引。
    /// @note 顶层节点用无效 QModelIndex 访问。
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 返回列数，固定为 2。
    /// @param[in] parent 父索引（本模型所有节点列数相同）。
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 返回指定索引的数据。
    /// @param[in] index 数据索引。
    /// @param[in] role  数据角色。
    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;

    /// @brief 返回表头数据。
    /// @param[in] section  列号。
    /// @param[in] orientation 方向。
    /// @param[in] role     数据角色。
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    /// @brief 返回指定行列的模型索引。
    /// @param[in] row    行号。
    /// @param[in] column 列号。
    /// @param[in] parent 父索引。
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 返回指定索引的父索引。
    /// @param[in] child 子节点索引。
    QModelIndex parent(const QModelIndex& child) const override;

private:
    /// @brief 从 TreeNode 指针获取 QModelIndex。
    /// @param[in] node 树节点指针。
    /// @return 对应的模型索引。
    /// @note 内部辅助函数，用于在 index() 和 parent() 中转换。
    QModelIndex getIndex(TreeNode* node) const;

    /// @brief 构建示例数据树。
    /// @note 创建 3 层深度的示例目录结构。
    void setupSampleData();

    std::unique_ptr<TreeNode> m_rootNode;  ///< 根节点（不可见）
};
