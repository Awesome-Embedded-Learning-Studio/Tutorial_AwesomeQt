/// @file    big_table_model.h
/// @brief   演示 QTableView 配合自定义 QAbstractTableModel 实现百万行虚拟滚动。
///
/// 对应教程：进阶层 03-QtWidgets/51-QTableView 百万行数据虚拟滚动。
/// 本示例的核心思想是：模型不在内存中预生成所有数据，而是在 data() 被调用时
/// 即时生成对应单元格的字符串，QTableView 只会请求当前可见区域的数据。

#pragma once

#include <QAbstractTableModel>
#include <QMainWindow>
#include <QStatusBar>

/// @brief 百万行数据的懒加载表格模型。
///
/// 模型声明支持 1,000,000 行，但数据完全按需生成。
/// QTableView 在滚动时只对可见单元格调用 data()，因此即使行数巨大，
/// 内存占用也几乎为零——这就是 Qt Model/View 架构的虚拟化能力。
class BigTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /// @brief 构造函数，指定行数和列数。
    /// @param[in] rowCount    总行数（默认一百万行）
    /// @param[in] columnCount 总列数
    /// @param[in] parent      父对象指针
    explicit BigTableModel(int rowCount = 1000000, int columnCount = 10,
                           QObject* parent = nullptr);

    /// @brief 返回总行数。
    /// @param[in] parent 父索引（顶层无效）
    /// @return 行数
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 返回总列数。
    /// @param[in] parent 父索引（顶层无效）
    /// @return 列数
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 按需生成指定单元格的显示数据。
    /// @param[in] index  单元格索引
    /// @param[in] role   数据角色
    /// @return 对应数据
    /// @note QTableView 只对可见区域调用此方法，所以百万行不会拖慢性能。
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /// @brief 返回列表头数据。
    /// @param[in] section     列号
    /// @param[in] orientation 方向
    /// @param[in] role        数据角色
    /// @return 列标题文本
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    int m_rowCount;    ///< 总行数
    int m_columnCount; ///< 总列数
};

/// @brief 主窗口，包含 QTableView 和状态栏信息。
class BigTableWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建模型/视图并组装界面。
    /// @param[in] parent 父控件指针
    explicit BigTableWindow(QWidget* parent = nullptr);

private:
    BigTableModel* m_model; ///< 百万行数据模型
};
