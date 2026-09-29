/// @file    circular_fix.cpp
/// @brief   NodeA / NodeB 类实现与循环引用演示函数。

#include "circular_fix.h"

#include <QDebug>

// ============================================================================
// NodeA 实现
// ============================================================================

NodeA::NodeA(const QString& name) : m_name(name)
{
    qDebug() << "  [构造] NodeA:" << m_name;
}

NodeA::~NodeA()
{
    qDebug() << "  [析构] NodeA:" << m_name;
}

void NodeA::setPartner(const QSharedPointer<NodeB>& partner)
{
    m_partner = partner;  // 强引用
}

QString NodeA::name() const
{
    return m_name;
}

// ============================================================================
// NodeB 实现
// ============================================================================

NodeB::NodeB(const QString& name) : m_name(name)
{
    qDebug() << "  [构造] NodeB:" << m_name;
}

NodeB::~NodeB()
{
    qDebug() << "  [析构] NodeB:" << m_name;
}

void NodeB::setPartner(const QSharedPointer<NodeA>& partner)
{
    m_weakPartner = partner;  // 弱引用，不阻止析构
}

QSharedPointer<NodeA> NodeB::getPartner() const
{
    return m_weakPartner.toStrongRef();
}

QString NodeB::name() const
{
    return m_name;
}

// ============================================================================
// demoCircularReference
// ============================================================================

void demoCircularReference()
{
    qDebug() << "\n=== 循环引用演示 ===";

    // --- 问题演示：QSharedPointer 双向强引用会导致内存泄漏 ---
    qDebug() << "\n--- 场景 1: 双向 QSharedPointer（内存泄漏）---";
    qDebug() << "(两个 QSharedPointer 互相持有，引用计数永不为 0)";
    {
        // 实际开发中如果 A 和 B 都用 QSharedPointer 互相持有就会泄漏
        // 这里只创建但不互相引用来展示正常析构
        QSharedPointer<NodeA> a(new NodeA("A-正常"));
        QSharedPointer<NodeB> b(new NodeB("B-正常"));
        qDebug() << "A 和 B 各有 1 个强引用（自身持有的 QSharedPointer）";
        // 不设置关联，离开作用域正常析构
    }
    qDebug() << "--- 两个对象正常析构 ---";

    // --- 解决方案：一方使用 QWeakPointer ---
    qDebug() << "\n--- 场景 2: QWeakPointer 打破循环引用 ---";
    {
        QSharedPointer<NodeA> a(new NodeA("A-安全"));
        QSharedPointer<NodeB> b(new NodeB("B-安全"));

        // A 持有 B 的强引用
        a->setPartner(b);
        // B 持有 A 的弱引用（不增加引用计数）
        b->setPartner(a);

        qDebug() << "A 的强引用数: 1（只有 a 本身，b 持有的是弱引用）";
        qDebug() << "B 的强引用数: 2（a 和 b 都持有强引用）";

        // 通过 B 的弱引用安全访问 A
        QSharedPointer<NodeA> partnerFromB = b->getPartner();
        if (partnerFromB) {
            qDebug() << "B 的合作伙伴:" << partnerFromB->name();
        }
    }
    qDebug() << "--- 两个对象正常析构（无泄漏）---";
}
