/// @file    sort_filter_model.cpp
/// @brief   Implementation of SortFilterModel — filter logic and property bridge.

#include "sort_filter_model.h"

#include "contact_model.h"

#include <QString>
#include <QtGlobal>

SortFilterModel::SortFilterModel(QObject* parent) : QSortFilterProxyModel(parent) {
    // Re-sort and re-filter whenever source data or filter parameters change
    setDynamicSortFilter(true);
}

QString SortFilterModel::filterString() const {
    return m_filterString;
}

void SortFilterModel::setFilterString(const QString& filter) {
    if (m_filterString == filter) {
        return;
    }
    m_filterString = filter;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    endFilterChange();
#else
    invalidateFilter();
#endif
    emit filterStringChanged();
}

bool SortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    // An empty filter string means "show everything"
    if (m_filterString.isEmpty()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString name = index.data(static_cast<int>(ContactModel::Role::kNameRole)).toString();

    // Case-insensitive substring match on the contact name
    return name.contains(m_filterString, Qt::CaseInsensitive);
}
