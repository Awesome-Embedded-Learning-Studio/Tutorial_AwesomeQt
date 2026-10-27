/// @file    migration_demo.h
/// @brief   演示 Qt5 到 Qt6 的 API 迁移模式，涵盖容器、字符串视图、枚举等关键变化。
///
/// 对应教程：进阶层 05-其他模块 / 25-Qt5Compat 模块。
/// 本类通过五个独立 Demo 分别展示 Qt5→Qt6 中最常见的破坏性变更及其应对方式。

#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

/// @brief 封装五组 Qt5→Qt6 迁移模式的演示类。
///
/// 每个 demo 方法打印 "Qt5 风格（已废弃）" vs "Qt6 推荐写法" 的对比输出，
/// 帮助开发者快速识别迁移中需要关注的 API 变更点。
class MigrationDemo : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit MigrationDemo(QObject* parent = nullptr);

    /// @brief 依次运行所有迁移演示，输出对比信息到控制台。
    void runAllDemos();

private:
    /// @brief Demo 1：QHash 迭代顺序变化。
    /// Qt5 中 QHash 迭代顺序不确定，Qt6 改为按插入顺序 deterministic 迭代。
    void demoHashIterationOrder();

    /// @brief Demo 2：QRegularExpression 替代 QRegExp。
    /// Qt6 完全移除 QRegExp，Core5Compat 提供 QRegExp 兼容层。
    void demoRegExpMigration();

    /// @brief Demo 3：QStringRef 替换为 QStringView。
    /// Qt6 废弃 QStringRef，统一使用 QStringView 作为轻量字符串视图。
    void demoStringRefMigration();

    /// @brief Demo 4：枚举类型变为 scoped enum。
    /// Qt5 中许多枚举是 unscoped 的，Qt6 改为 enum class，需要 Q_ENUM 或显式转换。
    void demoEnumTypeChanges();

    /// @brief Demo 5：QVector 与 QList 合并。
    /// Qt6 中 QList 就是原先的 QVector，QVector 成为 QList 的别名。
    void demoVectorListUnification();

    /// @brief 打印分隔线，用于区分不同 Demo 的输出。
    /// @param[in] title 分隔线标题文本。
    void printSeparator(const QString& title) const;
};
