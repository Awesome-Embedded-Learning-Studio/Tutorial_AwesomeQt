// fruit_model.h — 自定义 QAbstractListModel，暴露给 QML 使用
#ifndef FRUIT_MODEL_H
#define FRUIT_MODEL_H

#include <QAbstractListModel>
#include <QVector>

struct FruitItem
{
    QString name;
    QString color;
    double price;
    bool inStock;
};

class FruitModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // 自定义角色枚举，对应 QML 中 model.xxx 的字段名
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ColorRole,
        PriceRole,
        InStockRole
    };

    explicit FruitModel(QObject *parent = nullptr);

    // 必须实现的三个虚函数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // QML 可调用的数据操作方法
    Q_INVOKABLE void addFruit(const QString &name, const QString &color,
                              double price, bool inStock);
    Q_INVOKABLE void removeFruit(int index);
    Q_INVOKABLE void clearAll();

private:
    QVector<FruitItem> m_fruits;
};

#endif // FRUIT_MODEL_H
