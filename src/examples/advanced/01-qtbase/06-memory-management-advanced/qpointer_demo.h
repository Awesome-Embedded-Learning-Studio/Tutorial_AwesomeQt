/// @file    qpointer_demo.h
/// @brief   演示 QPointer 自动置空机制与内存管理策略选择指南。
///
/// 对应教程：进阶层 01-QtBase/06-内存管理。
/// 涵盖 QPointer 对比裸指针、QObject 对象树用法、策略选择总结。

#pragma once

#include <QString>

#include <QObject>
#include <QPointer>

/// 被 QPointer 监视的 QObject 子类，用于演示自动置空行为。
class WatchedObject : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造一个带名称的被监视对象。
    /// @param[in] name 用于日志输出的对象名称。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit WatchedObject(const QString& name, QObject* parent = nullptr);

    /// @brief 析构时打印日志，验证生命周期。
    ~WatchedObject() override;

    /// @brief 获取对象名称。
    /// @return 对象名称。
    QString name() const;

private:
    QString m_name;  // 对象名称，仅用于日志输出
};

// ---------------------------------------------------------------------------
// 演示函数声明
// ---------------------------------------------------------------------------

/// @brief 演示 QPointer 在 QObject 被删除后自动置空的行为。
void demoQPointerAutoNull();

/// @brief 演示 QObject 对象树用法并总结各类内存管理策略的选择建议。
void demoMemoryManagementGuide();
