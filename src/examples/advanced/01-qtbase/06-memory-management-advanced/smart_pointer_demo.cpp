/// @file    smart_pointer_demo.cpp
/// @brief   TrackedObject 类实现与智能指针演示函数。

#include "smart_pointer_demo.h"

#include <memory>

#include <QDebug>

// ============================================================================
// TrackedObject 实现
// ============================================================================

TrackedObject::TrackedObject(const QString& name) : m_name(name)
{
    qDebug() << "  [构造] TrackedObject:" << m_name;
}

TrackedObject::~TrackedObject()
{
    qDebug() << "  [析构] TrackedObject:" << m_name;
}

QString TrackedObject::name() const
{
    return m_name;
}

// ============================================================================
// demoSharedPointer
// ============================================================================

void demoSharedPointer()
{
    qDebug() << "\n=== QSharedPointer 引用计数追踪 ===";

    qDebug() << "\n--- 创建第一个 QSharedPointer ---";
    QSharedPointer<TrackedObject> ptr1(new TrackedObject("对象A"));
    // QSharedPointer 内部使用引用计数，可通过 use_count() 查看强引用数
    qDebug() << "当前强引用数:" << (ptr1 != nullptr ? 1 : 0);

    {
        qDebug() << "\n--- 进入内部作用域，创建第二个引用 ---";
        QSharedPointer<TrackedObject> ptr2 = ptr1;
        qDebug() << "共享后强引用数: 2（ptr1 和 ptr2 共享所有权）";
        qDebug() << "ptr1 和 ptr2 指向同一对象:" << (ptr1.data() == ptr2.data());
        qDebug() << "--- 离开内部作用域 ---";
    }
    qDebug() << "一个引用离开作用域后，强引用数: 1";

    qDebug() << "\n--- 使用 QWeakPointer 观察 ---";
    QWeakPointer<TrackedObject> weakPtr = ptr1.toWeakRef();
    qDebug() << "weakPtr 是否过期:" << weakPtr.isNull();
    QSharedPointer<TrackedObject> locked = weakPtr.toStrongRef();
    qDebug() << "通过 weakPtr 提升获取对象:" << (locked ? locked->name() : "nullptr");

    qDebug() << "\n--- 重置 ptr1，对象应该被销毁 ---";
    ptr1.reset();
    qDebug() << "ptr1 重置后，weakPtr 是否过期:" << weakPtr.isNull();
    QSharedPointer<TrackedObject> locked2 = weakPtr.toStrongRef();
    qDebug() << "提升失败返回 nullptr:" << !locked2;
}

// ============================================================================
// demoScopedPointer
// ============================================================================

void demoScopedPointer()
{
    qDebug() << "\n=== QScopedPointer vs std::unique_ptr ===";

    // QScopedPointer: Qt 的独占指针，不可拷贝，比 std::unique_ptr 更轻量
    {
        QScopedPointer<TrackedObject> scoped(new TrackedObject("QScoped 对象"));
        qDebug() << "QScopedPointer 持有:" << scoped->name();

        // take() 释放所有权但不删除，调用者负责管理
        TrackedObject* raw = scoped.take();
        qDebug() << "take() 后 scoped 是否为空:" << scoped.isNull();
        delete raw;  // 手动删除（演示 take 用法，实际项目优先不裸 delete）
    }

    // std::unique_ptr: C++ 标准独占指针
    {
        std::unique_ptr<TrackedObject> unique(new TrackedObject("unique_ptr 对象"));
        qDebug() << "std::unique_ptr 持有:" << unique->name();

        // release() 类似于 take()，释放所有权
        TrackedObject* raw = unique.release();
        qDebug() << "release() 后 unique 是否为空:" << (unique == nullptr);
        delete raw;
    }

    // 互操作: QScopedPointer 和 std::unique_ptr 都可以显式管理所有权转移
    qDebug() << "\n--- 选择建议 ---";
    qDebug() << "  Qt API 交互时优先使用 QScopedPointer/QSharedPointer";
    qDebug() << "  现代 C++ 代码中可使用 std::unique_ptr/std::shared_ptr";
    qDebug() << "  QObject 子类优先使用对象树 (parent-child) 管理内存";
}
