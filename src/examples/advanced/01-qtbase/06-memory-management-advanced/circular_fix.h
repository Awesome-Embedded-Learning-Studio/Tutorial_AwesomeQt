/// @file    circular_fix.h
/// @brief   演示 QSharedPointer 循环引用问题及 QWeakPointer 解决方案。
///
/// 对应教程：进阶层 01-QtBase/06-内存管理。
/// 通过 NodeA / NodeB 双向关联展示强引用泄漏与弱引用修复。

#pragma once

#include <QString>

#include <QSharedPointer>
#include <QWeakPointer>

// 前向声明，避免循环头文件依赖
class NodeB;

// ---------------------------------------------------------------------------
// NodeA: 持有 NodeB 的 QSharedPointer（强引用端）
// ---------------------------------------------------------------------------

/// 演示节点 A，持有对 NodeB 的强引用，用于构造循环引用场景。
class NodeA
{
public:
    /// @brief 构造一个带名称的 NodeA。
    /// @param[in] name 用于日志输出的节点名称。
    explicit NodeA(const QString& name);

    /// @brief 析构时打印日志，验证生命周期。
    ~NodeA();

    /// @brief 设置合作伙伴节点（强引用）。
    /// @param[in] partner 指向 NodeB 的共享指针。
    void setPartner(const QSharedPointer<NodeB>& partner);

    /// @brief 获取节点名称。
    /// @return 节点名称的 const 引用。
    QString name() const;

private:
    QString m_name;                  // 节点名称
    QSharedPointer<NodeB> m_partner; // 强引用，可能导致循环引用
};

// ---------------------------------------------------------------------------
// NodeB: 使用 QWeakPointer 打破循环引用（弱引用端）
// ---------------------------------------------------------------------------

/// 演示节点 B，持有对 NodeA 的弱引用，打破循环引用。
class NodeB
{
public:
    /// @brief 构造一个带名称的 NodeB。
    /// @param[in] name 用于日志输出的节点名称。
    explicit NodeB(const QString& name);

    /// @brief 析构时打印日志，验证生命周期。
    ~NodeB();

    /// @brief 设置合作伙伴节点（弱引用，不增加引用计数）。
    /// @param[in] partner 指向 NodeA 的共享指针，内部转为弱引用存储。
    void setPartner(const QSharedPointer<NodeA>& partner);

    /// @brief 将弱引用提升为共享指针以安全访问合作伙伴。
    /// @return 若合作伙伴仍存活则返回有效指针，否则返回 nullptr。
    QSharedPointer<NodeA> getPartner() const;

    /// @brief 获取节点名称。
    /// @return 节点名称的 const 引用。
    QString name() const;

private:
    QString m_name;                // 节点名称
    QWeakPointer<NodeA> m_weakPartner; // 弱引用，打破循环
};

// ---------------------------------------------------------------------------
// 演示函数声明
// ---------------------------------------------------------------------------

/// @brief 演示循环引用问题与 QWeakPointer 解决方案。
void demoCircularReference();
