/// @file    sort_filter_model.h
/// @brief   Sort/filter proxy model with QML-bindable filter string.
///
/// Wraps ContactModel and provides real-time name filtering driven from
/// a QML TextField.  dynamicSortFilter is enabled so the proxy updates
/// automatically whenever the filter string or source data changes.

#pragma once

#include <QtQmlIntegration/qqmlintegration.h>

#include <QSortFilterProxyModel>
#include <QString>

/// @brief A QSortFilterProxyModel that filters contacts by name.
///
/// Exposes a `filterString` QML property so the QML TextField can drive
/// the filter directly without calling setFilterFixedString() from C++.
class SortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    /// @brief The substring to match against contact names (case-insensitive).
    Q_PROPERTY(QString filterString READ filterString WRITE setFilterString
                   NOTIFY filterStringChanged)

public:
    /// @brief Constructs the proxy with dynamic sorting enabled.
    /// @param[in] parent Parent QObject for ownership.
    explicit SortFilterModel(QObject* parent = nullptr);

    /// @brief Returns the current filter string.
    QString filterString() const;

    /// @brief Sets the filter string and triggers re-evaluation.
    /// @param[in] filter The new substring to filter by.
    void setFilterString(const QString& filter);

    /// @brief Determines whether the row at the given source index passes
    ///        the filter.  Matches the "name" role case-insensitively.
    /// @param[in] sourceRow      Row index in the source model.
    /// @param[in] sourceParent   Parent index in the source model.
    /// @return true if the row should be visible.
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex& sourceParent) const override;

signals:
    /// @brief Emitted when filterString changes.
    void filterStringChanged();

private:
    QString m_filterString;  ///< Current filter substring
};
