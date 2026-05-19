/// @file    task_table_model.h
/// @brief   自定义 QAbstractTableModel——管理 Task 结构体的可编辑表格模型。
///
/// 对应教程：进阶层 03-QtWidgets/03-Model/View 进阶。

#pragma once

#include <QAbstractTableModel>
#include <QVector>

/// 任务数据结构体——Model 的底层数据载体。
struct Task
{
    QString name;       ///< 任务名称
    int priority;       ///< 优先级（1-10）
    int progress;       ///< 进度百分比（0-100）
};

/// 自定义表格模型，管理 Task 列表并提供完整的增删改查能力。
///
/// 重写六个虚函数：rowCount、columnCount、data、headerData、flags、setData。
/// 数据变更通过 dataChanged 信号和 beginInsertRows/endInsertRows 函数对通知 View。
class TaskTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit TaskTableModel(QObject* parent = nullptr);

    // ── QAbstractTableModel 必须重写的虚函数 ──

    /// @brief 返回行数。
    int rowCount(const QModelIndex& parent = {}) const override;

    /// @brief 返回列数（固定 3 列：名称、优先级、进度）。
    int columnCount(const QModelIndex& parent = {}) const override;

    /// @brief 根据 index 和 role 返回单元格数据。
    /// @param[in] index 单元格索引。
    /// @param[in] role  数据角色（DisplayRole、EditRole、TextAlignmentRole 等）。
    /// @return 对应角色的数据，无效索引或未处理角色返回空 QVariant。
    /// @note data() 是 column x role 的二维分发器，每个 role 必须显式处理。
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /// @brief 返回表头数据。
    /// @param[in] section 列号（Qt::Horizontal）或行号（Qt::Vertical）。
    /// @param[in] orientation 水平或垂直方向。
    /// @param[in] role 数据角色。
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    /// @brief 返回单元格的交互标志（可编辑、可选择等）。
    /// @note 不重写 flags 则 setData 永远不会被触发。
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /// @brief 编辑回写通道——用户修改单元格后由 View 调用。
    /// @param[in] index 被编辑的单元格索引。
    /// @param[in] value 用户输入的新值。
    /// @param[in] role 编辑角色。
    /// @return 是否成功写入。修改后必须 emit dataChanged，否则 View 不更新。
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    // ── 业务接口 ──

    /// @brief 添加一条任务。使用 beginInsertRows/endInsertRows 函数对通知 View。
    /// @param[in] task 要添加的任务。
    /// @note beginInsertRows 的 first/last 是新行在插入后模型中的索引范围。
    void addTask(const Task& task);

    /// @brief 移除指定行的任务。使用 beginRemoveRows/endRemoveRows 函数对通知 View。
    /// @param[in] row 要移除的行号。
    void removeTask(int row);

private:
    QVector<Task> m_tasks;  ///< 底层任务数据
};
