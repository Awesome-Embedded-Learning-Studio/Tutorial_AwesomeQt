/**
 * @file custom-model.h
 * @brief CustomTableModel——QAbstractTableModel 子类化自管数据源（QList<Task>），model 栏 MV
 * 范式样例
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QString>

namespace AwesomeQt {

/// @brief 单条任务记录——模型自管的行数据（POD 结构体）。
///
/// 把行数据放普通结构体而非 QObject，是为了凸显「自定义模型的魂是自管数据源」：
/// 模型持有 QList<Task>，所有 rowCount/data/setData/insertRows 都围绕这个容器打转。
struct Task {
    QString title;     ///< 任务标题（可编辑）
    QString assignee;  ///< 负责人（可编辑）
    int priority = 0;  ///< 优先级 0..3，越大越紧急（可编辑）
    bool done = false; ///< 是否完成（可编辑）
};

/// @brief 自定义表格模型：子类化 QAbstractTableModel 自管 QList<Task>。
///
/// 范式要点（model 栏 reference，后续自定义模型照此复刻）：
/// - **自管数据源**：内部持有 `QList<Task>`，模型就是这份列表的 Qt 门面；
/// - **五个必须重写的虚函数**：rowCount / columnCount / data / flags / headerData；
/// - **可编辑**：data 处理 Qt::EditRole，setData 回写结构体 + 发 dataChanged；
/// - **增删行**：insertRows / removeRows 用 beginInsertRows/endInsertRows 等模型索引信号包夹
///   容器改动，否则视图不知道行数变了、会越界访问或显示错乱；
/// - **多角色**：data 里按 Qt::ItemDataRole 分发 DisplayRole / EditRole / TextAlignmentRole /
///   BackgroundRole / ToolTipRole，一个单元格能同时给文本、对齐、底色、悬停提示。
class CustomTableModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    /// @brief 列定义：列索引与语义一一对应，改列数只动这里 + columnCount
    enum Column { kTitle = 0, kAssignee = 1, kPriority = 2, kDone = 3, kColumnCount = 4 };
    Q_ENUM(Column)

    /// @brief 优先级阈值：>= 此值的行底色泛红，用于演示 BackgroundRole
    static constexpr int kHighPriorityThreshold = 2;

    explicit CustomTableModel(QObject* parent = nullptr);

    // —— 必须重写：模型几何 + 取数 ——
    /// @brief 行数 = 容器当前元素数（parent 非空时返回 0，树模型才用）
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    /// @brief 列数 = 固定 kColumnCount（parent 非空时返回 0）
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    /// @brief 取单元格数据：按 role 分发（Display/Edit/TextAlignment/Background/ToolTip）
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // —— 可编辑：setData + flags ——
    /// @brief 回写单元格：把新值落回 Task 结构体，发 dataChanged 通知视图刷新
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    /// @brief 单元格能力位：可选中 + 可编辑（ItemIsEditable），否则双击进不了编辑态
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // —— 增删行：配 begin/end 模型索引信号 ——
    /// @brief 在 row 处插入 count 个空行（count 个默认 Task）
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    /// @brief 从 row 起删除 count 行
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    // —— 表头 ——
    /// @brief 行列表头：水平方向给列标题，垂直方向给行号
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // —— 便捷 API（演示用，非 QAbstractItemModel 协议） ——
    /// @brief 末尾追加一条已有 Task（等价于 insertRows + setData 的便捷封装）
    /// @param task 要追加的任务记录
    void appendTask(const Task& task);
    /// @brief 取某行原始 Task（越界返回默认 Task）
    Task taskAt(int row) const;
    /// @brief 当前任务总数
    int taskCount() const;

  private:
    /// @brief 某列的显示文本（DisplayRole）集中处理，避免 data 里一长串 switch
    QString displayText(const Task& task, int column) const;
    /// @brief 某列的可编辑值（EditRole），与 displayText 对称
    QVariant editValue(const Task& task, int column) const;
    /// @brief 把 EditRole 的 QVariant 落回 Task 的指定列（越界/类型不符返回 false）
    bool applyEdit(Task& task, int column, const QVariant& value);

    QList<Task> tasks_; ///< 自管数据源：所有行数据都在这
};

} // namespace AwesomeQt
