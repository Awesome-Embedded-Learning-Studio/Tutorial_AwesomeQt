---
title: "1.6 内存管理进阶：智能指针与循环引用"
description: "入门篇我们聊了对象树、QSharedPointer 基本用法、QWeakPointer 和 QPointer 的概念，也提到了循环引用的存在。说实话，那些内容只是让你「知道有这些东西」。"
---

# 现代Qt开发教程（进阶篇）1.6——内存管理进阶：智能指针与循环引用

## 1. 前言 / 为什么入门知识远远不够

入门篇我们聊了对象树、QSharedPointer 基本用法、QWeakPointer 和 QPointer 的概念，也提到了循环引用的存在。说实话，那些内容只是让你「知道有这些东西」。到了真正的工程里，当你面对一个插件系统里十几个模块互相持有对方引用的时候，当你试图用 QSharedPointer 管理 QObject 却发现对象树和引用计数打架的时候，当你用 Valgrind 跑出一堆 "definitely lost" 却完全不知道从哪里开始修的时候——那些入门知识就远远不够了。

我之前在一个工业可视化项目里踩过一个让我血压拉满的坑：两个 QObject 通过信号槽互相关联，一个持有另一个的 QSharedPointer，另一个又反过来持有对方的 QSharedPointer，引用计数死活不归零。更绝的是，这个循环引用不是在开发阶段暴露的，而是在跑了三天之后的压力测试里，内存从 200MB 慢慢涨到 4GB 然后 OOM 崩溃。那一次调试让我把 Qt 智能指针的底层实现全部翻了一遍，也让我对「什么时候该用什么指针」有了刻骨铭心的认识。这篇我们把这块知识彻底吃透，不留死角。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QScopedPointer 在 Qt 6 中虽然仍然可用但官方推荐迁移到 std::unique_ptr，我们会详细讨论两者的互操作问题。内存检测工具部分涉及 Valgrind（Linux）和 AddressSanitizer（跨平台），需要 GCC 12+ 或 Clang 15+ 编译器支持。代码只依赖 QtCore 模块。

## 3. 核心概念讲解

### 3.1 QSharedPointer 与 QWeakPointer 打破循环引用——完整模式

入门篇我们知道了 QWeakPointer 不增加引用计数，可以用来「观察」一个共享对象。但知道概念和会写代码之间差着十万八千里。这里我们把三种最常见的循环引用模式彻底拆开。

第一种模式是双向关联。假设我们有一个 Document 类和一个 View 类，Document 持有 View 列表，View 也需要回指 Document。如果两边都用 QSharedPointer，引用计数永远不归零。正确做法是「主导方」用 QSharedPointer，「从属方」用 QWeakPointer。Document 作为资源拥有者，持有 View 的 QSharedPointer；View 回指 Document 时用 QWeakPointer。这样当外部释放对 Document 的最后一个 QSharedPointer 时，Document 析构，引用计数链条干净利落地断裂。

```cpp
class View;  // 前向声明

class Document
{
public:
    void addView(const QSharedPointer<View>& v);
    QList<QSharedPointer<View>> m_views;  // 强引用，Document 拥有 View
};

class View
{
public:
    QWeakPointer<Document> m_document;  // 弱引用，不增加引用计数

    void onDocumentChanged()
    {
        // 每次使用前提升为 QSharedPointer
        QSharedPointer<Document> doc = m_document.toStrongRef();
        if (doc) {
            // 安全使用 doc
        }
        // 如果 doc 已被销毁，toStrongRef 返回 nullptr
    }
};
```

注意 View::onDocumentChanged 里的 toStrongRef() 调用。这是使用 QWeakPointer 的黄金法则：永远不要直接存储 toStrongRef 的结果作为成员变量，只在需要使用时临时提升。如果你把提升后的 QSharedPointer 存为成员变量，那又回到了循环引用的老路上。

第二种模式是观察者/监听器模式。被观察者持有观察者的列表，观察者也需要访问被观察者。这和双向关联本质上是一样的解法——被观察者用 QSharedPointer 持有观察者（或者用裸指针，如果观察者的生命周期由外部管理），观察者用 QWeakPointer 回指被观察者。

第三种模式是信号槽中的隐式循环引用，这个更隐蔽。两个 QObject 通过信号槽互相关联，Lambda 槽捕获了对方的 QSharedPointer。这种情况下 Lambda 本身持有 QSharedPointer，而 Lambda 又被 QObject 的连接机制持有——只要连接不断开，QSharedPointer 就不会释放。解决方案是使用四参数 connect，让 context 对象控制 Lambda 的生命周期，或者在适当时机手动 disconnect。

### 3.2 QSharedPointer 与 std::shared_ptr——该用哪个

Qt 6 时代，C++17 标准库已经非常成熟，std::shared_ptr 和 std::weak_ptr 在功能上基本可以替代 QSharedPointer 和 QWeakPointer。但 Qt 的智能指针有几个 std 版本不具备的能力：

QSharedPointer 支持 `QSharedPointer<T>::objectCast<Target>()`，这是 Qt 特有的跨类型转换方法，内部调用 qobject_cast，比 std::static_pointer_cast 更安全（能做运行时类型检查）。如果你的对象继承自 QObject，QSharedPointer 的 objectCast 可以正确处理多继承场景。

QSharedPointer 还支持自定义 deleter，可以传入一个函数在引用计数归零时调用。这在管理非 new 分配的资源时很有用——比如一个通过 C 库 API 创建的对象需要用特定的销毁函数释放。

```cpp
// 自定义 deleter：用 C 库的销毁函数而不是 delete
QSharedPointer<sqlite3> openDb(const QString& path)
{
    sqlite3* db = nullptr;
    sqlite3_open(path.toUtf8().constData(), &db);
    // 引用计数归零时调用 sqlite3_close 而不是 delete
    return QSharedPointer<sqlite3>(db, [](sqlite3* p) {
        if (p) sqlite3_close(p);
    });
}
```

但 std::shared_ptr 也有自己的优势：标准库不依赖 Qt，可以被非 Qt 代码使用；性能在某些场景下更优（比如 make_shared 的单次分配优化）；与 std::unique_ptr 的互操作更自然。工程项目中的实践建议是：如果对象是 QObject，优先用 Qt 的对象树管理或 QSharedPointer；如果对象不是 QObject，优先用 std::unique_ptr 或 std::shared_ptr。混合使用时要特别注意不要让两种智能指针同时管理同一个对象。

### 3.3 QPointer——QObject 专属的弱观察指针

QPointer 是一个经常被忽视但极其有用的工具。它是一个模板类，持有 QObject* 但不会阻止对象被删除。当对象被 delete 后，QPointer 自动置为 nullptr——这个行为和 QWeakPointer 类似，但 QPointer 的使用更简单，不需要 QSharedPointer 配合。

```cpp
QPointer<QWidget> widget = someWidget;
// ... 稍后 ...
if (widget) {
    widget->update();  // 安全：对象仍在
}
// 如果 someWidget 被 delete，widget 自动变为 nullptr
```

QPointer 的实现原理是：QObject 析构时通知全局的 QPointer 注册表，把所有指向它的 QPointer 置 nullptr。这个机制比 QWeakPointer 更轻量——QWeakPointer 需要配合 QSharedPointer 的引用计数器，而 QPointer 只依赖 QObject 的析构通知。

QPointer 的最佳使用场景是信号槽 Lambda 捕获和事件回调。如果你在 Lambda 里捕获了一个 QObject 的裸指针，对象被 delete 后就是野指针。用 QPointer 包裹后，Lambda 里判空就能安全跳过。这是比 QWeakPointer 更简洁的方案，但前提是被观察对象必须是 QObject。

### 3.4 QScopedPointer 迁移到 std::unique_ptr

Qt 6 官方推荐用 std::unique_ptr 替代 QScopedPointer。两者功能几乎等价，但 std::unique_ptr 有几个优势：标准库不依赖 Qt 编译、移动语义更完善、自定义 deleter 的语法更灵活。

```cpp
// Qt 5 风格
QScopedPointer<MyData> data(new MyData());

// Qt 6 推荐风格
auto data = std::make_unique<MyData>();
```

迁移时有一个坑需要注意：QScopedPointer 的 `data()` 方法返回原始指针但不释放所有权，`take()` 方法返回原始指针并释放所有权。std::unique_ptr 的 `get()` 等价于 `data()`，`release()` 等价于 `take()`。名字变了，行为一样，但如果你在代码里批量替换 `data()` 为 `get()` 要小心——`data()` 这个名字在其他 Qt 类里也有，别误改了。

## 4. 踩坑预防

第一个坑是 QSharedPointer 管理 QObject 时和对象树冲突。如果你把一个 QObject 的指针交给 QSharedPointer 管理，同时又把它挂到了对象树（指定了 parent），那么当 parent 析构时对象树会 delete 这个对象，QSharedPointer 的引用计数可能还没归零——结果就是 double free。后果是程序崩溃，而且崩溃点通常在析构链的深处，栈回溯看起来莫名其妙。解决方案是二选一：要么用对象树管理（不用 QSharedPointer），要么用 QSharedPointer 管理（不挂 parent）。绝对不要两种机制同时管理同一个对象。

第二个坑是 QWeakPointer::toStrongRef() 的竞态条件。toStrongRef 返回一个 QSharedPointer，但在多线程环境下，在你调用 toStrongRef 和实际使用返回值之间，对象可能已经被另一个线程释放了。后果是拿到了一个有效的 QSharedPointer，但使用时对象已经在另一个线程被析构了。这在实际工程中表现为偶发的访问已释放内存。解决方案是确保最后一个 QSharedPointer 的释放和 QWeakPointer 的提升发生在同一个线程，或者用互斥锁保护引用计数的检查和提升操作。

第三个坑是 Lambda 捕获 QSharedPointer 形成隐式循环引用。前面讲过了，这里再强调一下后果。如果你在一个 QObject 的成员函数里用三参数 connect 捕获了 this 指针对应的 QSharedPointer（比如通过 enableSharedFromThis），那这条连接的存在会阻止对象被销毁——而连接又是对象自己建立的，永远不会断开。后果是内存泄漏，而且泄漏的对象还连着一堆信号槽，占用的资源会越来越多。解决方案是用四参数 connect 把 Lambda 的生命周期绑定到 context 对象上，或者在确定不再需要时手动 disconnect。

## 5. 练习项目

练习项目：引用计数可视化调试器。我们要实现一个工具类，能够追踪 QSharedPointer 的创建、拷贝和销毁事件，打印引用计数变化日志。

具体要求是：TrackedShared<T> 模板类包装 QSharedPointer<T>，在构造、拷贝构造、移动构造、赋值和析构时打印日志（包含对象地址、当前引用计数、操作类型）。需要追踪的对象类型继承一个 TrackedObject 基类，基类在构造和析构时也打印日志。完成标准是能清晰看到引用计数从 1 涨到 N 再归零的完整过程、循环引用场景下引用计数永远不归零能被日志直接发现、强引用提升弱引用的过程能在日志中体现。

提示几个关键点：QSharedPointer 有 `strongRef()` 的相关内部机制可以利用，模板类需要完美转发构造参数，用 `qDebug` 输出时带上 `Q_FUNC_INFO` 标识调用位置。

## 6. 官方文档参考链接

[Qt 文档 · QSharedPointer](https://doc.qt.io/qt-6/qsharedpointer.html) -- Qt 共享指针完整参考

[Qt 文档 · QWeakPointer](https://doc.qt.io/qt-6/qweakpointer.html) -- Qt 弱引用指针

[Qt 文档 · QPointer](https://doc.qt.io/qt-6/qpointer.html) -- QObject 弱观察指针

[Qt 文档 · QScopedPointer](https://doc.qt.io/qt-6/qscopedpointer.html) -- Qt 作用域指针（迁移参考）

---

到这里，Qt 内存管理的进阶知识就拆完了。三种智能指针的适用场景、循环引用的三种模式和破解方法、QObject 对象树和智能指针的冲突边界——这些在工程中会天天遇到。下一篇我们来看事件系统进阶：自定义事件、事件过滤器和跨线程事件投递。
