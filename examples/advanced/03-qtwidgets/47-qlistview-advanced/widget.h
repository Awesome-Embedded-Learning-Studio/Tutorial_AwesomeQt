/// @file    widget.h
/// @brief   演示 QListView 大数据虚拟列表与增量加载。
///
/// 本示例展示：
/// - 自定义 QAbstractListModel 管理超大数据集（100,000+ 项）
/// - QListView 虚拟滚动：只为可见行创建 delegate，内存占用恒定
/// - canFetchMore/fetchMore 实现增量加载，模拟分批读取
/// - 底部标签实时显示总项数和当前可见范围
///
/// 对应教程：进阶层 03-QtWidgets/47-qlistview-advanced。

#pragma once

#include <QAbstractListModel>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>

/// @brief 自定义列表模型，模拟 100,000+ 条数据的增量加载。
///
/// 通过 canFetchMore / fetchMore 机制，初始只加载一小批数据，
/// 用户滚动到列表底部时自动追加下一批，避免一次性构造全部字符串。
class LargeDataModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /// @brief 构造函数，设置总数据量和每批加载大小。
    /// @param[in] totalItems 模拟的总数据条数。
    /// @param[in] batchSize  每次 fetchMore 加载的条数。
    /// @param[in] parent     父对象指针。
    /// @note 初始不加载任何数据，等 QListView 调用 fetchMore 时才填充。
    explicit LargeDataModel(int totalItems = 100000, int batchSize = 1000,
                            QObject* parent = nullptr);

    /// @brief 返回当前已加载的行数。
    /// @param[in] parent 父索引（列表模型中无效）。
    /// @note rowCount 只返回已加载的项数，不是 totalItems。
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 返回指定索引的显示数据。
    /// @param[in] index 数据索引。
    /// @param[in] role  数据角色。
    /// @note 数据在 fetchMore 时按需生成，非预先全部构造。
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /// @brief 判断是否还有更多数据可加载。
    /// @param[in] parent 父索引。
    /// @note QListView 滚动到底部时会调用此函数判断是否触发 fetchMore。
    bool canFetchMore(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 加载下一批数据并通知视图。
    /// @param[in] parent 父索引。
    /// @note 使用 beginInsertRows / endInsertRows 通知视图增量更新。
    void fetchMore(const QModelIndex& parent = QModelIndex()) override;

    /// @brief 获取总数据量（含未加载部分）。
    /// @return 总条数。
    int totalItemCount() const;

    /// @brief 获取当前已加载的条数。
    /// @return 已加载条数。
    int loadedItemCount() const;

private:
    int m_totalItems;                  ///< 模拟的总数据条数
    int m_batchSize;                   ///< 每批加载条数
    int m_loadedCount;                 ///< 当前已加载条数
    QVector<QString> m_data;           ///< 已加载的数据缓存
};

/// @brief 主窗口，包含一个 QListView 和状态信息标签。
///
/// 通过自定义 LargeDataModel 演示虚拟滚动和增量加载机制，
/// 窗口底部实时显示总项数和当前可见行范围。
class Widget : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化模型、视图和状态标签。
    /// @param[in] parent 父控件指针。
    explicit Widget(QWidget* parent = nullptr);

private:
    /// @brief 更新底部状态标签，显示当前项数和可见范围。
    /// @note 通过 listView 的 visibleIndexes 或定时刷新获取可见范围。
    void updateStatusLabel();

    /// @brief 响应模型数据变化，刷新状态标签。
    void onDataChanged();

    /// @brief 响应"加载全部数据"按钮点击。
    /// @note 一次性加载全部数据以对比性能表现。
    void onLoadAll();

    LargeDataModel* m_model;         ///< 自定义大数据模型
    QListView* m_listView;           ///< 列表视图
    QLabel* m_statusLabel;           ///< 底部状态标签
    QPushButton* m_loadAllBtn;       ///< 加载全部数据的按钮
};
