/// @file    custom_hash.cpp
/// @brief   自定义 QHash key 类型演示的实现。
///
/// 对应教程：进阶层 01-QtBase/04-容器（高级）。

#include "custom_hash.h"

#include <QDebug>

/// @brief 演示自定义 key 类型在 QHash 中的插入、查找、遍历与删除。
void demoCustomHashKey()
{
    qDebug() << "=== 自定义 QHash Key 类型 ===";

    QHash<PersonKey, QString> phoneBook;

    // 插入条目——PersonKey 作为 key
    phoneBook[{QStringLiteral("张"), QStringLiteral("三")}] =
        QStringLiteral("138-0000-0001");
    phoneBook[{QStringLiteral("李"), QStringLiteral("四")}] =
        QStringLiteral("139-0000-0002");
    phoneBook[{QStringLiteral("王"), QStringLiteral("五")}] =
        QStringLiteral("137-0000-0003");
    phoneBook[{QStringLiteral("张"), QStringLiteral("三丰")}] =
        QStringLiteral("136-0000-0004");

    qDebug() << "电话簿条目数:" << phoneBook.size();

    // 查找——构造临时 PersonKey 作为查找 key
    PersonKey target{QStringLiteral("张"), QStringLiteral("三")};
    if (phoneBook.contains(target)) {
        qDebug() << "查找 张三:" << phoneBook.value(target);
    }

    // 遍历所有条目
    qDebug() << "所有条目:";
    for (auto it = phoneBook.cbegin(); it != phoneBook.cend(); ++it) {
        qDebug() << "  " << it.key().firstName << it.key().lastName
                 << "->" << it.value();
    }

    // 哈希碰撞测试：相同的 key 应该找到同一个条目
    PersonKey duplicate{QStringLiteral("张"), QStringLiteral("三")};
    qDebug() << "重复 key 查找结果:" << phoneBook.value(duplicate)
             << "(与之前相同，说明 operator== 和 qHash 配合正确)";

    // 删除条目
    phoneBook.remove(target);
    qDebug() << "删除 张三 后条目数:" << phoneBook.size();
    qDebug() << "删除后 contains(张三):" << phoneBook.contains(target);

    qDebug() << "";
}
