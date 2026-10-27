/**
 * @file custom-model.cpp
 * @brief CustomTableModel 实现——QAbstractTableModel 五大虚函数 + 增删行 + 多角色 data
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "custom-model.h"

#include <QBrush>
#include <QColor>
#include <QFont>

namespace AwesomeQt {

// ============================================================================
// 构造：填几条种子数据，跑起来 demo 不是空表
// ============================================================================
CustomTableModel::CustomTableModel(QObject* parent) : QAbstractTableModel(parent) {
    // 种子数据：让 demo 一启动就有内容可看、可编辑、可增删
    tasks_ = {
        {QStringLiteral("设计登录页"), QStringLiteral("Alice"), 3, false},
        {QStringLiteral("对接支付接口"), QStringLiteral("Bob"), 2, false},
        {QStringLiteral("写单元测试"), QStringLiteral("Carol"), 1, true},
        {QStringLiteral("修启动崩溃"), QStringLiteral("Dave"), 3, false},
    };
}

// ============================================================================
// 模型几何：rowCount / columnCount
// 关键：parent 有效时返回 0（这是表格模型，不是树；表格没有父子层级）
// ============================================================================
int CustomTableModel::rowCount(const QModelIndex& parent) const {
    // 表格模型没有子项：parent 有效即表示「问子项数」，恒为 0
    if (parent.isValid()) {
        return 0;
    }
    return tasks_.size();
}

int CustomTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return kColumnCount;
}

// ============================================================================
// 取数：data，按 role 分发
// 教学：一个单元格能同时回答「显示什么文本 / 编辑时的值 / 怎么对齐 / 底色 / 悬停提示」
// ============================================================================
QVariant CustomTableModel::data(const QModelIndex& index, int role) const {
    // 边界保护：越界 index 给空 QVariant，视图拿到空值就留白，不崩
    if (!index.isValid()) {
        return {};
    }
    if (index.row() < 0 || index.row() >= tasks_.size() || index.column() < 0 ||
        index.column() >= kColumnCount) {
        return {};
    }

    const Task& task = tasks_.at(index.row());

    switch (role) {
        case Qt::DisplayRole:
            // 显示文本：布尔列显示「是/否」更可读，而非 1/0
            return displayText(task, index.column());

        case Qt::EditRole:
            // 编辑值：双击进入编辑器时，编辑器拿到/回写的原始值
            return editValue(task, index.column());

        case Qt::TextAlignmentRole:
            // 对齐：优先级 + 完成态居中（短字段好看），其余靠左
            if (index.column() == kPriority || index.column() == kDone) {
                return QVariant(Qt::AlignCenter);
            }
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

        case Qt::BackgroundRole:
            // 底色：高优先级行整行泛浅红，扫一眼就能定位紧急任务
            if (task.priority >= kHighPriorityThreshold) {
                return QVariant(QBrush(QColor(255, 235, 235)));
            }
            return {};

        case Qt::ForegroundRole:
            // 前景色：已完成行字色变灰，降低视觉权重（演示 ForegroundRole）
            if (task.done) {
                return QVariant(QBrush(QColor(150, 150, 150)));
            }
            return {};

        case Qt::ToolTipRole:
            // 悬停提示：给整行一个摘要，演示 ToolTipRole
            return QVariant(QStringLiteral("任务：%1　负责人：%2　优先级：%3")
                                .arg(task.title, task.assignee)
                                .arg(task.priority));

        case Qt::FontRole:
            // 字体：已完成行加删除线效果（用 strikeout 近似），演示 FontRole
            if (task.done) {
                QFont font;
                font.setStrikeOut(true);
                return QVariant(font);
            }
            return {};

        default:
            // 其余 role（DecorationRole / SizeHintRole 等）不关心，交还基类语义
            return {};
    }
}

// ============================================================================
// 回写：setData，编辑后把新值落回 Task 并发 dataChanged
// 关键：发 dataChanged 时列范围用全部列，因为改优先级可能连带改变底色/对齐
// ============================================================================
bool CustomTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role != Qt::EditRole) {
        // 只响应编辑角色；CheckState 等其它角色本例不处理
        return false;
    }
    if (!index.isValid()) {
        return false;
    }
    if (index.row() < 0 || index.row() >= tasks_.size() || index.column() < 0 ||
        index.column() >= kColumnCount) {
        return false;
    }

    Task& task = tasks_[index.row()];
    if (!applyEdit(task, index.column(), value)) {
        return false; // 类型不符 / 列非法，不改
    }

    // 通知视图：本行所有列都可能受影响（如优先级变了底色也变），所以列范围给全部
    const QModelIndex top_left = index.siblingAtRow(index.row());
    const QModelIndex bottom_right = this->index(index.row(), kColumnCount - 1);
    emit dataChanged(top_left, bottom_right,
                     {Qt::DisplayRole, Qt::EditRole, Qt::BackgroundRole, Qt::ForegroundRole,
                      Qt::FontRole, Qt::ToolTipRole});
    return true;
}

// ============================================================================
// 能力位：flags
// 关键：要可编辑就必须返回 Qt::ItemIsEditable，否则双击不进入编辑器
// ============================================================================
Qt::ItemFlags CustomTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QAbstractTableModel::flags(index);
    }
    // 可选中 + 可编辑 + 可启用（启用是默认，但显式列出更清楚）
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

// ============================================================================
// 增行：insertRows
// 教学：容器改动必须用 beginInsertRows / endInsertRows 包夹，否则视图不知道行数变了
// ============================================================================
bool CustomTableModel::insertRows(int row, int count, const QModelIndex& parent) {
    if (parent.isValid()) {
        return false; // 表格模型无父子层级
    }
    if (count <= 0) {
        return false;
    }
    // row 允许的范围：[0, tasks_.size()]（末尾追加也算合法插入点）
    if (row < 0 || row > tasks_.size()) {
        return false;
    }

    // ① 先通知视图「我要在 [row, row+count-1] 插入了」
    beginInsertRows(parent, row, row + count - 1);
    // ② 真正改容器：从 row 起插入 count 个默认 Task
    for (int i = 0; i < count; ++i) {
        tasks_.insert(row, Task{});
    }
    // ③ 通知视图「插入完成」，视图据此刷新行数
    endInsertRows();
    return true;
}

// ============================================================================
// 删行：removeRows
// 同样必须用 beginRemoveRows / endRemoveRows 包夹容器改动
// ============================================================================
bool CustomTableModel::removeRows(int row, int count, const QModelIndex& parent) {
    if (parent.isValid()) {
        return false;
    }
    if (count <= 0) {
        return false;
    }
    if (row < 0 || row + count > tasks_.size()) {
        return false; // 越界：不允许删超出已有范围的行
    }

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        tasks_.removeAt(row); // 逐个删；removeAt(row) 后后续元素自动前移
    }
    endRemoveRows();
    return true;
}

// ============================================================================
// 表头：水平给列标题，垂直给行号
// ============================================================================
QVariant CustomTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        // 水平表头：列名
        switch (section) {
            case kTitle:
                return QVariant(QStringLiteral("任务标题"));
            case kAssignee:
                return QVariant(QStringLiteral("负责人"));
            case kPriority:
                return QVariant(QStringLiteral("优先级"));
            case kDone:
                return QVariant(QStringLiteral("完成"));
            default:
                return {};
        }
    }

    // 垂直表头：行号（从 1 开始更直观）
    if (orientation == Qt::Vertical) {
        if (section >= 0 && section < tasks_.size()) {
            return QVariant(section + 1);
        }
    }
    return {};
}

// ============================================================================
// 便捷 API
// ============================================================================
void CustomTableModel::appendTask(const Task& task) {
    // 末尾插入一行，再把字段逐列回写——等价于 insertRows + setData，但一次事务更清晰
    const int row = tasks_.size();
    if (!insertRow(row)) {
        return; // insertRow 是 insertRows(row,1) 的便捷封装
    }
    // 回写四列（setData 会发 dataChanged）
    setData(index(row, kTitle), task.title);
    setData(index(row, kAssignee), task.assignee);
    setData(index(row, kPriority), task.priority);
    setData(index(row, kDone), task.done);
}

Task CustomTableModel::taskAt(int row) const {
    if (row < 0 || row >= tasks_.size()) {
        return Task{}; // 越界返回默认 Task，调用方拿到空标题即可判别
    }
    return tasks_.at(row);
}

int CustomTableModel::taskCount() const {
    return tasks_.size();
}

// ============================================================================
// 私有辅助：displayText / editValue / applyEdit——把列相关逻辑集中，避免 data/setData 重复
// ============================================================================
QString CustomTableModel::displayText(const Task& task, int column) const {
    switch (column) {
        case kTitle:
            return task.title;
        case kAssignee:
            return task.assignee;
        case kPriority:
            return QString::number(task.priority);
        case kDone:
            // 布尔显示「是/否」比 1/0 可读
            return task.done ? QStringLiteral("是") : QStringLiteral("否");
        default:
            return {};
    }
}

QVariant CustomTableModel::editValue(const Task& task, int column) const {
    switch (column) {
        case kTitle:
            return QVariant(task.title);
        case kAssignee:
            return QVariant(task.assignee);
        case kPriority:
            return QVariant(task.priority);
        case kDone:
            return QVariant(task.done);
        default:
            return {};
    }
}

bool CustomTableModel::applyEdit(Task& task, int column, const QVariant& value) {
    switch (column) {
        case kTitle:
            task.title = value.toString();
            return true;
        case kAssignee:
            task.assignee = value.toString();
            return true;
        case kPriority: {
            bool ok = false;
            const int v = value.toInt(&ok);
            if (!ok) {
                return false;
            }
            // 夹到 0..3，防止视图编辑器塞进越界值（虽然 QSpinBox 会拦，这里再兜一道）
            task.priority = (v < 0) ? 0 : (v > 3 ? 3 : v);
            return true;
        }
        case kDone: {
            // 布尔列：接受 bool 或可转 bool 的值（如 QCheckBox 的 Qt::Checked）
            if (value.canConvert<bool>()) {
                task.done = value.toBool();
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}

} // namespace AwesomeQt
