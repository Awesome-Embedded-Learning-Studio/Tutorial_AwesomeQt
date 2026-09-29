/**
 * @file proxy-model.cpp
 * @brief SortFilterProxyModel 实现——自定义 lessThan + filterAcceptsRow
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "proxy-model.h"

#include <QAbstractItemModel>
#include <QString>

namespace AwesomeQt {

SortFilterProxyModel::SortFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {}

void SortFilterProxyModel::setNumericColumns(const QList<int>& columns) {
    numeric_columns_ = columns;
    // 列的排序语义变了，强制重新评估排序 / 过滤，视图才会刷新
    invalidate();
}

FilterScope SortFilterProxyModel::filterScope() const {
    return filter_scope_;
}

void SortFilterProxyModel::setFilterScope(FilterScope scope) {
    if (filter_scope_ == scope) {
        return;
    }
    filter_scope_ = scope;
    emit filterScopeChanged();
    // 匹配范围变了，已有行的去留要重新判定：失效当前过滤，代理重算
    // filterAcceptsRow 并通知视图刷新。invalidateFilter() 自 Qt 6.9 即可用；
    // Qt 6.13 起另有更细粒度的 begin/endFilterChange(Direction) 变体，留待届时迁移。
    invalidateFilter();
}

bool SortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    if (isNumericColumn(left.column())) {
        // 数值列：按 double 比，避免 "10" < "2" 的字典序陷阱
        const double left_value = sourceText(left).toDouble();
        const double right_value = sourceText(right).toDouble();
        return left_value < right_value;
    }

    // 文本列：不区分大小写的字典序（基类默认区分大小写，这里覆盖）
    return QString::compare(sourceText(left), sourceText(right), Qt::CaseInsensitive) < 0;
}

bool SortFilterProxyModel::filterAcceptsRow(int source_row,
                                            const QModelIndex& source_parent) const {
    const auto* model = sourceModel();
    if (model == nullptr) {
        return true; // 没挂源模型，全放行（构造期 / 拆卸期的安全兜底）
    }

    const QString needle = filterRegularExpression().pattern();

    // kActiveColumn：只看 filterKeyColumn 那一列
    if (filter_scope_ == FilterScope::kActiveColumn) {
        const QModelIndex idx = model->index(source_row, filterKeyColumn(), source_parent);
        return sourceText(idx).contains(needle, Qt::CaseInsensitive);
    }

    // kAllColumns：任一列命中即保留
    const int column_count = model->columnCount(source_parent);
    for (int column = 0; column < column_count; ++column) {
        const QModelIndex idx = model->index(source_row, column, source_parent);
        if (sourceText(idx).contains(needle, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

QString SortFilterProxyModel::sourceText(const QModelIndex& source_index) const {
    const auto* model = sourceModel();
    if (model == nullptr || !source_index.isValid()) {
        return {};
    }
    // 直接读 sourceModel 的 display 数据，绝不能走 proxy 自己的 data()，否则递归
    return model->data(source_index, Qt::DisplayRole).toString();
}

bool SortFilterProxyModel::isNumericColumn(int column) const {
    for (int numeric_column : numeric_columns_) {
        if (numeric_column == column) {
            return true;
        }
    }
    return false;
}

} // namespace AwesomeQt
