/// @file    frozen_table.h
/// @brief   演示 QTableWidget 单元格合并与冻结左上角交汇格。
///
/// 对应教程：进阶层 03-QtWidgets/50-QTableWidget 高级用法。
/// 本示例展示 setSpan() 合并单元格，以及通过叠加第二个 QTableWidget
/// 冻结左上角 1x1 交汇格（注意：未做滚动同步，滚动时冻结格不跟随，
/// 完整的滚动同步实现留给教程正文的练手环节）。

#pragma once

#include <QTableWidget>
#include <QWidget>

/// @brief 冻结表控件，在同一个窗口内同时展示主表格和冻结的行/列表头。
///
/// 内部维护两个 QTableWidget：一个主表格和一个冻结表格。
/// 冻结表格覆盖在主表格上方，仅显示第 0 行第 0 列的 1x1 交汇格，
/// 未做滚动同步（滚动时冻结格不跟随），完整冻结窗格效果见正文练手环节。
class FrozenTable : public QWidget {
    Q_OBJECT

  public:
    /// @brief 构造函数，初始化主表格和冻结表格。
    /// @param[in] rows    主表格行数
    /// @param[in] columns 主表格列数
    /// @param[in] parent  父控件指针，Qt 对象树管理生命周期
    explicit FrozenTable(int rows, int columns, QWidget* parent = nullptr);

  protected:
    /// @brief 窗口大小变化时重新定位冻结表格。
    /// @param[in] event resize 事件
    void resizeEvent(QResizeEvent* event) override;

  private:
    /// @brief 初始化主表格的示例数据和合并单元格。
    void setupMainTable();

    /// @brief 初始化冻结表格，使其与主表第 0 行/列保持同步。
    void setupFrozenTable();

    /// @brief 将冻结表格精确定位到主表格的左上角。
    void updateFrozenTableGeometry();

    /// @brief 同步冻结表格的列宽到主表格。
    /// @note 冻结表格的列宽必须与主表格完全一致，否则内容错位。
    void syncColumnWidths(int column, int oldWidth, int newWidth);

    /// @brief 同步冻结表格的行高到主表格。
    /// @note 冻结表格的行高必须与主表格完全一致，否则内容错位。
    void syncRowHeight(int row, int oldHeight, int newHeight);

    QTableWidget* m_mainTable;   ///< 主数据表格
    QTableWidget* m_frozenTable; ///< 冻结在左上角的表格（仅第 0 行和第 0 列）
};
