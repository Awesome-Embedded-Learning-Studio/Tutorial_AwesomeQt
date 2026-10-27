/// @file    custom_hash.h
/// @brief   演示自定义结构体作为 QHash key 所需的 operator== 与 qHash 重载。
///
/// 对应教程：进阶层 01-QtBase/04-容器（高级）。
/// QHash 的 key 类型必须提供 operator==（const）和 qHash() 全局重载
/// （带 seed 参数、noexcept）。对比 QMap 只需要 operator<。

#pragma once

#include <QHash>
#include <QString>

/// @brief 用 firstName + lastName 组合作为哈希表 key 的自定义结构体。
struct PersonKey
{
    QString firstName;  ///< 名
    QString lastName;   ///< 姓

    /// @brief 判断两个 PersonKey 是否相等。
    /// @param[in] other 另一个 PersonKey。
    /// @return 姓名完全相同返回 true。
    /// @note 必须是 const 成员函数，QHash 用它在哈希碰撞后判断 key 真正相等。
    bool operator==(const PersonKey& other) const
    {
        return firstName == other.firstName && lastName == other.lastName;
    }

    /// @brief 判断两个 PersonKey 是否不等。
    /// @param[in] other 另一个 PersonKey。
    /// @return 姓名不同返回 true。
    bool operator!=(const PersonKey& other) const
    {
        return !(*this == other);
    }
};

/// @brief qHash 全局重载，为 PersonKey 计算哈希值。
/// @param[in] key   待哈希的 key。
/// @param[in] seed  Qt 内部传入的随机种子，必须参与计算以防止碰撞攻击。
/// @return 哈希值。
/// @note 必须标记 noexcept，QHash 内部不处理异常。
inline size_t qHash(const PersonKey& key, size_t seed) noexcept
{
    // 逐字段组合，把 seed 传递给每个 qHash 调用
    // Qt 内部会用 seed 做额外的混合运算，防止碰撞攻击
    size_t h1 = qHash(key.firstName, seed);
    size_t h2 = qHash(key.lastName, seed);
    return h1 ^ (h2 << 1);  // 简单的位运算组合
}

/// @brief 演示自定义 key 类型在 QHash 中的增删查。
void demoCustomHashKey();
