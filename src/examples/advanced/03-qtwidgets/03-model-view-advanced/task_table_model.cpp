/// @file    task_table_model.cpp
/// @brief   TaskTableModel 类实现——自定义 QAbstractTableModel 的六个虚函数与增删接口。
///
/// 对应教程：进阶层 03-QtWidgets/03-Model/View 进阶。

#include "task_table_model.h"

#include <QColor>

// ─────────────────────────────────────────────────────────────────────────────
// 常量
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    constexpr int kColumnCount = 3;  // 固定 3 列：名称、优先级、进度

    const QStringList kHeaders = {
        QStringLiteral("任务名称"),
        QStringLiteral("优先级"),
        QStringLiteral("进度"),
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

TaskTableModel::TaskTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    // 初始示例数据
    m_tasks = {
        {QStringLiteral("界面重构"), 8, 65},
        {QStringLiteral("性能优化"), 9, 30},
        {QStringLiteral("单元测试补充"), 5, 90},
        {QStringLiteral("文档撰写"), 3, 10},
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// QAbstractTableModel 必须重写的虚函数
// ─────────────────────────────────────────────────────────────────────────────

int TaskTableModel::rowCount(const QModelIndex& parent) const
{
    // 表格模型的 parent 应始终为无效索引
    if (parent.isValid()) {
        return 0;
    }
    return m_tasks.size();
}

int TaskTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return kColumnCount;
}

QVariant TaskTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= m_tasks.size()) {
        return {};
    }

    const auto& task = m_tasks.at(row);

    // DisplayRole：单元格显示文本
    if (role == Qt::DisplayRole) {
        switch (col) {
        case 0: return task.name;
        case 1: return QString::number(task.priority);
        case 2: return QStringLiteral("%1%").arg(task.progress);
        default: return {};
        }
    }

    // EditRole：编辑器初始值（只有名称列可编辑）
    if (role == Qt::EditRole) {
        switch (col) {
        case 0: return task.name;
        case 1: return task.priority;
        case 2: return task.progress;
        default: return {};
        }
    }

    // TextAlignmentRole：数字列居中对齐
    if (role == Qt::TextAlignmentRole) {
        if (col == 1 || col == 2) {
            return Qt::AlignCenter;
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    // BackgroundRole：高优先级行背景色提示
    if (role == Qt::BackgroundRole) {
        if (task.priority >= 8) {
            return QColor(0xFD, 0xF0, 0xF0);  // 浅红背景表示高优先级
        }
    }

    // ToolTipRole：鼠标悬停提示
    if (role == Qt::ToolTipRole) {
        return QStringLiteral("任务: %1 | 优先级: %2 | 进度: %3%")
            .arg(task.name)
            .arg(task.priority)
            .arg(task.progress);
    }

    return {};
}

QVariant TaskTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // 只处理水平表头的 DisplayRole
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    if (section >= 0 && section < kHeaders.size()) {
        return kHeaders.at(section);
    }
    return {};
}

Qt::ItemFlags TaskTableModel::flags(const QModelIndex& index) const
{
    auto f = QAbstractTableModel::flags(index);

    // 名称列（col 0）允许编辑
    if (index.column() == 0) {
        f |= Qt::ItemIsEditable;
    }

    return f;
}

bool TaskTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= m_tasks.size()) {
        return false;
    }

    // EditRole：用户编辑后的回写
    if (role == Qt::EditRole && col == 0) {
        m_tasks[row].name = value.toString();
        // 修改完数据后必须 emit dataChanged，否则 View 不知道数据变了
        emit dataChanged(index, index, {role});
        return true;
    }

    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// 业务接口
// ─────────────────────────────────────────────────────────────────────────────

void TaskTableModel::addTask(const Task& task)
{
    const int row = m_tasks.size();

    // beginInsertRows 的 first/last 是新行在插入后模型中的索引范围
    // 插一行则 first = last = 目标位置
    beginInsertRows(QModelIndex(), row, row);
    m_tasks.append(task);
    endInsertRows();
}

void TaskTableModel::removeTask(int row)
{
    if (row < 0 || row >= m_tasks.size()) {
        return;
    }

    // first/last 是要删除的行在删除前的索引
    beginRemoveRows(QModelIndex(), row, row);
    m_tasks.removeAt(row);
    endRemoveRows();
}
