/// @file    qpointer_demo.cpp
/// @brief   WatchedObject 类实现、QPointer 演示与内存管理策略选择指南。

#include "qpointer_demo.h"

#include <QDebug>

// ============================================================================
// WatchedObject 实现
// ============================================================================

WatchedObject::WatchedObject(const QString& name, QObject* parent)
    : QObject(parent), m_name(name)
{
    qDebug() << "  [构造] WatchedObject:" << m_name;
}

WatchedObject::~WatchedObject()
{
    qDebug() << "  [析构] WatchedObject:" << m_name;
}

QString WatchedObject::name() const
{
    return m_name;
}

// ============================================================================
// demoQPointerAutoNull
// ============================================================================

void demoQPointerAutoNull()
{
    qDebug() << "\n=== QPointer 自动置空演示 ===";

    // QPointer 只能用于 QObject 子类
    // 当被监视的 QObject 被删除时，QPointer 自动变为 nullptr
    QPointer<WatchedObject> watcher;

    {
        WatchedObject* obj = new WatchedObject("被监视对象");
        watcher = obj;  // QPointer 开始监视

        qDebug() << "设置 QPointer 后是否为空:" << watcher.isNull();
        qDebug() << "通过 QPointer 访问名称:" << watcher->name();

        qDebug() << "\n--- 删除被监视的对象 ---";
        delete obj;  // 对象被删除
    }

    // 关键特性: QPointer 自动置为 nullptr
    qDebug() << "对象删除后 QPointer 是否为空:" << watcher.isNull();
    qDebug() << "安全检查: watcher == nullptr =>" << (watcher == nullptr);

    // 与裸指针对比
    qDebug() << "\n--- 裸指针 vs QPointer 对比 ---";
    WatchedObject* rawPtr = nullptr;
    QPointer<WatchedObject> qPtr;

    {
        WatchedObject* obj = new WatchedObject("对比对象");
        rawPtr = obj;
        qPtr = obj;
        delete obj;
    }

    // 裸指针成为悬空指针（危险！）
    qDebug() << "裸指针不为空但已悬空:" << (rawPtr != nullptr) << "（危险！）";
    // QPointer 安全地置为 nullptr
    qDebug() << "QPointer 已安全置空:" << qPtr.isNull() << "（安全）";
}

// ============================================================================
// demoMemoryManagementGuide
// ============================================================================

void demoMemoryManagementGuide()
{
    qDebug() << "\n=== 内存管理策略选择指南 ===";

    // 方式 1: QObject 对象树（推荐用于有明确父子关系的 QObject）
    {
        QObject root;
        root.setObjectName("根对象");
        // 子对象会自动被父对象管理
        QObject* child = new QObject(&root);
        child->setObjectName("子对象");
        qDebug() << "对象树方式: 根对象有" << root.children().size() << "个子对象";
        // root 离开作用域时自动删除 child
    }
    qDebug() << "根对象销毁后，子对象自动清理";

    qDebug() << "\n--- 选择建议总结 ---";
    qDebug() << "  1. QObject 子类之间有父子关系 -> 使用对象树 (parent-child)";
    qDebug() << "  2. 需要共享所有权的 QObject -> QSharedPointer + QWeakPointer";
    qDebug() << "  3. 需要监视 QObject 但不拥有 -> QPointer";
    qDebug() << "  4. 非 QObject 的独占资源 -> QScopedPointer 或 std::unique_ptr";
    qDebug() << "  5. 非 QObject 的共享资源 -> QSharedPointer 或 std::shared_ptr";
    qDebug() << "  6. 循环引用时，一方使用 QWeakPointer 打破循环";
}
