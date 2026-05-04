// fruit_model.cpp — FruitModel 实现
#include "fruit_model.h"

FruitModel::FruitModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // 初始测试数据
    m_fruits = {
        {"Apple",      "#e74c3c", 5.5,  true},
        {"Banana",     "#f1c40f", 3.2,  true},
        {"Grape",      "#9b59b6", 12.0, false},
        {"Orange",     "#e67e22", 4.0,  true},
        {"Blueberry",  "#3498db", 18.5, true},
        {"Watermelon", "#2ecc71", 8.0,  false}
    };
}

int FruitModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_fruits.size();
}

QVariant FruitModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_fruits.size()) {
        return {};
    }

    const auto &fruit = m_fruits.at(index.row());
    switch (role) {
    case NameRole:
        return fruit.name;
    case ColorRole:
        return fruit.color;
    case PriceRole:
        return fruit.price;
    case InStockRole:
        return fruit.inStock;
    default:
        return {};
    }
}

QHash<int, QByteArray> FruitModel::roleNames() const
{
    return {
        {NameRole,   "name"},
        {ColorRole,  "fruitColor"},
        {PriceRole,  "price"},
        {InStockRole, "inStock"}
    };
}

void FruitModel::addFruit(const QString &name, const QString &color,
                          double price, bool inStock)
{
    beginInsertRows(QModelIndex(), m_fruits.size(), m_fruits.size());
    m_fruits.append({name, color, price, inStock});
    endInsertRows();
}

void FruitModel::removeFruit(int index)
{
    if (index < 0 || index >= m_fruits.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    m_fruits.removeAt(index);
    endRemoveRows();
}

void FruitModel::clearAll()
{
    beginResetModel();
    m_fruits.clear();
    endResetModel();
}
