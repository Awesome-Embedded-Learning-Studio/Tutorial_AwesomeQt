/// @file    tree_model.cpp
/// @brief   自定义树形模型的实现。
///
/// 对应教程：进阶层 03-QtWidgets/49-qtreeview-advanced。

#include "tree_model.h"

// ---------------------------------------------------------------------------
// TreeNode
// ---------------------------------------------------------------------------

TreeNode::TreeNode(const QString& name, const QString& value, TreeNode* parent)
    : m_name(name), m_value(value), m_parent(parent)
{
}

TreeNode::~TreeNode() = default;

void TreeNode::appendChild(std::unique_ptr<TreeNode> child)
{
    m_children.push_back(std::move(child));
}

TreeNode* TreeNode::child(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_children.size())) {
        return nullptr;
    }
    return m_children.at(static_cast<std::size_t>(index)).get();
}

int TreeNode::childCount() const
{
    return static_cast<int>(m_children.size());
}

int TreeNode::row() const
{
    // @note 顶层节点的 m_parent 为 nullptr，默认返回 0
    if (m_parent) {
        for (int i = 0; i < m_parent->childCount(); ++i) {
            if (m_parent->child(i) == this) {
                return i;
            }
        }
    }
    return 0;
}

TreeNode* TreeNode::parent() const
{
    return m_parent;
}

QVariant TreeNode::data(int column) const
{
    if (column == 0) {
        return m_name;
    }
    if (column == 1) {
        return m_value;
    }
    return QVariant();
}

int TreeNode::columnCount() const
{
    return 2;
}

// ---------------------------------------------------------------------------
// TreeModel
// ---------------------------------------------------------------------------

TreeModel::TreeModel(QObject* parent)
    : QAbstractItemModel(parent), m_rootNode(std::make_unique<TreeNode>("Root"))
{
    setupSampleData();
}

TreeModel::~TreeModel() = default;

int TreeModel::rowCount(const QModelIndex& parent) const
{
    TreeNode* parentNode = nullptr;
    if (!parent.isValid()) {
        parentNode = m_rootNode.get();
    } else {
        parentNode = static_cast<TreeNode*>(parent.internalPointer());
    }
    return parentNode ? parentNode->childCount() : 0;
}

int TreeModel::columnCount(const QModelIndex& /*parent*/) const
{
    // @note 所有节点列数固定为 2
    return 2;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    auto* node = static_cast<TreeNode*>(index.internalPointer());
    return node ? node->data(index.column()) : QVariant();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    // @note 两列表头：名称 + 值
    if (section == 0) {
        return QStringLiteral("名称");
    }
    if (section == 1) {
        return QStringLiteral("值");
    }
    return QVariant();
}

QModelIndex TreeModel::index(int row, int column,
                             const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeNode* parentNode = nullptr;
    if (!parent.isValid()) {
        parentNode = m_rootNode.get();
    } else {
        parentNode = static_cast<TreeNode*>(parent.internalPointer());
    }

    TreeNode* childNode = parentNode ? parentNode->child(row) : nullptr;
    // @note createIndex 的第三个参数（internalPointer）存储节点指针，
    // 用于后续 parent() 调用时找回父节点
    if (childNode) {
        return createIndex(row, column, childNode);
    }
    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    auto* childNode = static_cast<TreeNode*>(child.internalPointer());
    TreeNode* parentNode = childNode ? childNode->parent() : nullptr;

    // @note 根节点的子节点 → 返回无效 QModelIndex（顶层）
    if (!parentNode || parentNode == m_rootNode.get()) {
        return QModelIndex();
    }

    return createIndex(parentNode->row(), 0, parentNode);
}

QModelIndex TreeModel::getIndex(TreeNode* node) const
{
    if (!node || node == m_rootNode.get()) {
        return QModelIndex();
    }
    return createIndex(node->row(), 0, node);
}

void TreeModel::setupSampleData()
{
    // -- 构建三层示例数据 --

    // 第一层：主分类
    auto* systems = new TreeNode(QStringLiteral("系统"), QString(), m_rootNode.get());
    auto* network = new TreeNode(QStringLiteral("网络"), QString(), m_rootNode.get());
    auto* storage = new TreeNode(QStringLiteral("存储"), QString(), m_rootNode.get());

    m_rootNode->appendChild(std::unique_ptr<TreeNode>(systems));
    m_rootNode->appendChild(std::unique_ptr<TreeNode>(network));
    m_rootNode->appendChild(std::unique_ptr<TreeNode>(storage));

    // 第二层：系统子项
    auto* cpuItem = new TreeNode(QStringLiteral("CPU"), QStringLiteral("4 核"), systems);
    auto* memItem = new TreeNode(QStringLiteral("内存"), QStringLiteral("16 GB"), systems);
    auto* osItem = new TreeNode(QStringLiteral("操作系统"), QStringLiteral("Linux"), systems);
    systems->appendChild(std::unique_ptr<TreeNode>(cpuItem));
    systems->appendChild(std::unique_ptr<TreeNode>(memItem));
    systems->appendChild(std::unique_ptr<TreeNode>(osItem));

    // 第三层：CPU 子项
    auto* core0 = new TreeNode(QStringLiteral("核心 0"), QStringLiteral("1.2 GHz"), cpuItem);
    auto* core1 = new TreeNode(QStringLiteral("核心 1"), QStringLiteral("1.2 GHz"), cpuItem);
    auto* core2 = new TreeNode(QStringLiteral("核心 2"), QStringLiteral("1.2 GHz"), cpuItem);
    auto* core3 = new TreeNode(QStringLiteral("核心 3"), QStringLiteral("1.2 GHz"), cpuItem);
    cpuItem->appendChild(std::unique_ptr<TreeNode>(core0));
    cpuItem->appendChild(std::unique_ptr<TreeNode>(core1));
    cpuItem->appendChild(std::unique_ptr<TreeNode>(core2));
    cpuItem->appendChild(std::unique_ptr<TreeNode>(core3));

    // 第二层：网络子项
    auto* eth0 = new TreeNode(QStringLiteral("eth0"), QStringLiteral("192.168.1.10"), network);
    auto* wlan0 = new TreeNode(QStringLiteral("wlan0"), QStringLiteral("10.0.0.5"), network);
    auto* dnsItem = new TreeNode(QStringLiteral("DNS"), QStringLiteral("8.8.8.8"), network);
    network->appendChild(std::unique_ptr<TreeNode>(eth0));
    network->appendChild(std::unique_ptr<TreeNode>(wlan0));
    network->appendChild(std::unique_ptr<TreeNode>(dnsItem));

    // 第二层：存储子项
    auto* diskA = new TreeNode(QStringLiteral("磁盘 A"), QStringLiteral("256 GB"), storage);
    auto* diskB = new TreeNode(QStringLiteral("磁盘 B"), QStringLiteral("1 TB"), storage);
    storage->appendChild(std::unique_ptr<TreeNode>(diskA));
    storage->appendChild(std::unique_ptr<TreeNode>(diskB));

    // 第三层：磁盘分区
    auto* part1 = new TreeNode(QStringLiteral("分区 1"), QStringLiteral("/ (50 GB)"), diskA);
    auto* part2 = new TreeNode(QStringLiteral("分区 2"), QStringLiteral("/home (200 GB)"), diskA);
    diskA->appendChild(std::unique_ptr<TreeNode>(part1));
    diskA->appendChild(std::unique_ptr<TreeNode>(part2));
}
