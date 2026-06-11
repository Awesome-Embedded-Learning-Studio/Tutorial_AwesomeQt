/// @file    group_header_view.h
/// @brief   演示 QHeaderView 双级（分组）表头与自定义排序逻辑。
///
/// 对应教程：进阶层 03-QtWidgets/52-QHeaderView 双级表头。
/// 本示例通过子类化 QHeaderView，在 paintEvent 中绘制跨列的分组表头，
/// 第二行才是实际的列名，实现类似 Excel 多级表头的效果。

#pragma once

#include <QHeaderView>
#include <QMainWindow>
#include <QStandardItemModel>

/// @brief 分组表头结构体，描述一个跨列的组标题。
struct HeaderGroup
{
    QString title;    ///< 分组标题文本
    int startColumn;  ///< 起始列索引
    int span;         ///< 跨越的列数
};

/// @brief 自定义 QHeaderView，在标准列头之上绘制分组表头。
///
/// 重写 paintEvent 在第一行画出分组标题（跨列居中），
/// 第二行是正常的列标题。通过 sectionResized 信号同步分组宽度。
class GroupHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] orientation 表头方向（仅支持 Qt::Horizontal）
    /// @param[in] parent      父控件
    explicit GroupHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);

    /// @brief 设置分组信息。
    /// @param[in] groups 分组列表
    void setHeaderGroups(const std::vector<HeaderGroup>& groups);

    /// @brief 返回分组表头占用的高度。
    /// @return 像素值
    /// @note 需要重写 sizeHint 让 QTableView 为分组表头预留空间。
    QSize sizeHint() const override;

protected:
    /// @brief 绘制分组表头 + 标准列标题。
    /// @param[in] event 绘制事件
    void paintEvent(QPaintEvent* event) override;

private:
    /// @brief 当用户拖拽列宽时，更新视口以重绘分组。
    /// @param[in] logicalIndex 拖拽的列索引
    /// @note 分组表头宽度取决于底层列宽之和，列宽变化必须触发重绘。
    void onSectionResized(int logicalIndex);

    std::vector<HeaderGroup> m_groups; ///< 分组信息列表

    static constexpr int kGroupHeaderHeight = 30; ///< 分组行高度（像素）
};

/// @brief 主窗口，组装 QTableView + GroupHeaderView + 示例模型。
class GroupHeaderWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建并组装所有组件。
    /// @param[in] parent 父控件
    explicit GroupHeaderWindow(QWidget* parent = nullptr);

private:
    /// @brief 填充示例数据到模型中。
    void populateModel();

    QStandardItemModel* m_model; ///< 数据模型
};
