/**
 * @file proxy-model.h
 * @brief SortFilterProxyModel——QSortFilterProxyModel 子类，model 栏 proxy 范式样例
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QSortFilterProxyModel>

namespace AwesomeQt {

/// @brief 代理模型的过滤匹配范围。
enum class FilterScope {
    kActiveColumn, ///< 仅在 setFilterKeyColumn 指定的列匹配
    kAllColumns    ///< 在所有列匹配（任一列命中即保留）
};

/// @brief QSortFilterProxyModel 子类：自定义排序 + 自定义过滤。
///
/// 范式要点（model 栏 proxy-model，演示 Model/View 代理层）：
/// - 代理模型**不存数据**，只把 sourceModel 的行列映射出去；源改了 proxy 自动刷新；
/// - 重写 lessThan 实现自定义排序（数值列按数值比、文本列不区分大小写）；
/// - 重写 filterAcceptsRow 实现自定义过滤（关键词子串匹配 + 可选列范围）；
/// - setFilterKeyColumn / setFilterCaseSensitivity 等 API 直接复用基类。
class SortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_ENUM(FilterScope)
    Q_PROPERTY(
        FilterScope filterScope READ filterScope WRITE setFilterScope NOTIFY filterScopeChanged)

  public:
    explicit SortFilterProxyModel(QObject* parent = nullptr);

    /// @brief 设置哪些列按数值排序（其它列走不区分大小写的字典序）。
    /// @param[in] columns 列索引集合；传入空集合表示全部按文本排序。
    void setNumericColumns(const QList<int>& columns);

    /// @brief 当前过滤匹配范围（活动列 / 全部列）。
    FilterScope filterScope() const;

    /// @brief 设置过滤匹配范围。设为 kAllColumns 时无视 filterKeyColumn，全列扫描。
    /// @param[in] scope 新的过滤范围。
    void setFilterScope(FilterScope scope);

  protected:
    /// @brief 自定义排序：数值列按 double 比较，文本列不区分大小写字典序。
    /// @param[in] left 左侧索引（须属于 sourceModel）。
    /// @param[in] right 右侧索引（须属于 sourceModel）。
    /// @return true 表示 left 应排在 right 之前。
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    /// @brief 自定义过滤：按 filterScope 决定扫描范围，子串匹配（大小写敏感由基类控制）。
    /// @param[in] source_row sourceModel 中的行号。
    /// @param[in] source_parent 父索引（平表模型恒为无效索引）。
    /// @return true 表示该行通过过滤、应被代理保留。
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

  signals:
    void filterScopeChanged();

  private:
    /// @brief 取 sourceModel 某单元格的字符串（走 source 接口，避免递归回代理）。
    QString sourceText(const QModelIndex& source_index) const;

    /// @brief 判断某列是否按数值排序。
    bool isNumericColumn(int column) const;

    QList<int> numeric_columns_;
    FilterScope filter_scope_{FilterScope::kActiveColumn};
};

} // namespace AwesomeQt
