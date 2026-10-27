/// @file    contact_model.cpp
/// @brief   Implementation of ContactModel — sample data and role accessors.

#include "contact_model.h"

ContactModel::ContactModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_contacts{
          // Work contacts (5)
          {QStringLiteral("Alice Wang"),   QStringLiteral("Work"),     QStringLiteral("+1-202-555-0101")},
          {QStringLiteral("Bob Zhang"),    QStringLiteral("Work"),     QStringLiteral("+1-202-555-0102")},
          {QStringLiteral("Carol Li"),     QStringLiteral("Work"),     QStringLiteral("+1-202-555-0103")},
          {QStringLiteral("David Chen"),   QStringLiteral("Work"),     QStringLiteral("+1-202-555-0104")},
          {QStringLiteral("Eve Liu"),      QStringLiteral("Work"),     QStringLiteral("+1-202-555-0105")},
          // Personal contacts (5)
          {QStringLiteral("Frank Zhao"),   QStringLiteral("Personal"), QStringLiteral("+1-303-555-0201")},
          {QStringLiteral("Grace Sun"),    QStringLiteral("Personal"), QStringLiteral("+1-303-555-0202")},
          {QStringLiteral("Henry Wu"),     QStringLiteral("Personal"), QStringLiteral("+1-303-555-0203")},
          {QStringLiteral("Ivy Huang"),    QStringLiteral("Personal"), QStringLiteral("+1-303-555-0204")},
          {QStringLiteral("Jack Xu"),      QStringLiteral("Personal"), QStringLiteral("+1-303-555-0205")},
          // Family contacts (5)
          {QStringLiteral("Karen Yang"),   QStringLiteral("Family"),   QStringLiteral("+1-404-555-0301")},
          {QStringLiteral("Leo Gao"),      QStringLiteral("Family"),   QStringLiteral("+1-404-555-0302")},
          {QStringLiteral("Mia Tang"),     QStringLiteral("Family"),   QStringLiteral("+1-404-555-0303")},
          {QStringLiteral("Noah Zhou"),    QStringLiteral("Family"),   QStringLiteral("+1-404-555-0304")},
          {QStringLiteral("Olivia He"),    QStringLiteral("Family"),   QStringLiteral("+1-404-555-0305")},
      }
{
}

int ContactModel::rowCount(const QModelIndex& parent) const
{
    // Flat list: no children under an invalid parent index
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_contacts.size());
}

QVariant ContactModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(m_contacts.size()))
    {
        return {};
    }

    const auto& contact = m_contacts.at(row);
    switch (static_cast<Role>(role))
    {
    case Role::kNameRole:
        return contact.name;
    case Role::kCategoryRole:
        return contact.category;
    case Role::kPhoneRole:
        return contact.phone;
    default:
        return {};
    }
}

QHash<int, QByteArray> ContactModel::roleNames() const
{
    return {
        {static_cast<int>(Role::kNameRole),     "name"},
        {static_cast<int>(Role::kCategoryRole), "category"},
        {static_cast<int>(Role::kPhoneRole),    "phone"},
    };
}
