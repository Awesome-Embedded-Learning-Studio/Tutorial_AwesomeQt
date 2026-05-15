/// @file    smart_pointer_demo.h
/// @brief   演示 QSharedPointer / QWeakPointer / QScopedPointer 的基本用法。
///
/// 对应教程：进阶层 01-QtBase/06-内存管理。
/// 涵盖引用计数追踪、弱引用提升、QScopedPointer 与 std::unique_ptr 对比。

#pragma once

#include <QString>

#include <QSharedPointer>
#include <QWeakPointer>
#include <QScopedPointer>

/// 追踪对象生命周期的辅助类，用于演示智能指针行为。
class TrackedObject
{
public:
    /// @brief 构造一个带名称的追踪对象。
    /// @param[in] name 用于在日志中区分不同实例。
    explicit TrackedObject(const QString& name);

    /// @brief 析构时打印日志，验证生命周期是否正确。
    ~TrackedObject();

    /// @brief 获取对象名称。
    /// @return 对象名称的 const 引用。
    QString name() const;

private:
    QString m_name;  // 对象名称，仅用于日志输出
};

// ---------------------------------------------------------------------------
// 演示函数声明
// ---------------------------------------------------------------------------

/// @brief 演示 QSharedPointer 引用计数追踪与 QWeakPointer 观察者模式。
void demoSharedPointer();

/// @brief 演示 QScopedPointer 与 std::unique_ptr 的异同及选择建议。
void demoScopedPointer();
