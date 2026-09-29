/// @file    contact_model.h
/// @brief   Contact list model for QML ListView demonstration.
///
/// Provides a custom QAbstractListModel exposing contact data (name,
/// category, phone) to QML.  Contains 15 sample contacts split across
/// three categories: Work, Personal, Family.

#pragma once

#include <QtQmlIntegration/qqmlintegration.h>

#include <QAbstractListModel>
#include <QString>
#include <QVector>

/// @brief A single contact entry with name, category, and phone number.
struct Contact
{
    QString name;
    QString category;
    QString phone;
};

/// @brief Read-only list model that exposes Contact data to QML.
///
/// Roles provided:
///   - kNameRole     ("name")     — contact display name
///   - kCategoryRole ("category") — grouping category for section headers
///   - kPhoneRole    ("phone")    — phone number string
class ContactModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /// @brief Custom role IDs exported to QML via roleNames().
    enum class Role : int
    {
        kNameRole = Qt::DisplayRole + 1,
        kCategoryRole,
        kPhoneRole
    };
    Q_ENUM(Role)

    /// @brief Constructs the model with sample data.
    /// @param[in] parent Parent QObject for ownership.
    explicit ContactModel(QObject* parent = nullptr);

    /// @brief Returns the number of contacts in the model.
    /// @param[in] parent Unused (flat list model).
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief Returns data for the given role and index.
    /// @param[in] index  Model index to query.
    /// @param[in] role   The data role to retrieve.
    /// @return The requested data, or an invalid QVariant.
    QVariant data(const QModelIndex& index, int role) const override;

    /// @brief Returns the mapping from role IDs to QML property names.
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<Contact> m_contacts;  ///< In-memory contact list
};
