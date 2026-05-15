/// @file    cow_demo.cpp
/// @brief   COW 机制演示的实现。
///
/// 对应教程：进阶层 01-QtBase/04-容器（高级）。

#include "cow_demo.h"

#include <algorithm>
#include <iterator>
#include <type_traits>

#include <QDebug>
#include <QElapsedTimer>

/// @brief 演示 const 与非 const 访问对 COW detach 的影响。
///
/// 非 const 的 begin()/operator[]/data() 在共享状态下会触发 detach（深拷贝），
/// 而 const 版本的 cbegin()/constData() 不会。
void demoDetachBehavior()
{
    qDebug() << "=== COW detach 行为演示 ===";

    // 创建一个较大的列表用于观察
    QList<int> original;
    for (int i = 0; i < 100000; ++i) {
        original.append(i);
    }

    // 拷贝共享数据——只是引用计数 +1，没有深拷贝
    QList<int> shared = original;
    qDebug() << "拷贝后，两个 QList 共享同一块数据";

    // ------ 非 const 访问触发 detach ------
    {
        QElapsedTimer timer;
        timer.start();

        // 非 const 的 begin() 会触发 detach：100000 个 int 的深拷贝
        // 因为 Qt 无法确定你拿到迭代器后会不会修改数据
        auto it = shared.begin();
        Q_UNUSED(it)

        qint64 detachTime = timer.nsecsElapsed();
        qDebug() << "非 const begin() 触发 detach, 耗时:"
                 << detachTime / 1000 << "us";
    }

    // 现在 shared 和 original 已经不共享数据了，重新创建共享状态
    shared = original;
    qDebug() << "重新赋值后，再次共享数据";

    // ------ const 访问不触发 detach ------
    {
        QElapsedTimer timer;
        timer.start();

        // const 的 cbegin() 不会触发 detach
        auto cit = shared.cbegin();
        Q_UNUSED(cit)

        qint64 constTime = timer.nsecsElapsed();
        qDebug() << "const cbegin() 不触发 detach, 耗时:"
                 << constTime / 1000 << "us"
                 << "(几乎为零)";
    }

    qDebug() << "";
}

/// @brief 演示 std::as_const() 防止 range-based for 触发意外 detach。
///
/// 非 const 的 range-for 在引用计数 > 1 时会 detach，
/// 用 std::as_const() 将容器转为 const 引用即可避免。
void demoStdAsConst()
{
    qDebug() << "=== std::as_const() 防止意外深拷贝 ===";

    QList<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    QList<int> ref = data;  // 共享数据

    // 危险：range-based for 内部调用非 const 的 begin()/end()
    // 如果 ref count > 1，这里会触发 detach
    int sum1 = 0;
    for (auto& item : ref) {
        sum1 += item;
    }
    qDebug() << "非 const range-for 后 data 是否仍共享:"
             << (data.constData() == ref.constData())
             << "(不共享，已被 detach)";

    // 重新共享
    ref = data;

    // 安全：std::as_const() 返回 const 引用，range-for 调用 const begin()
    // 不会触发 detach
    int sum2 = 0;
    for (const auto& item : std::as_const(ref)) {
        sum2 += item;
    }
    qDebug() << "std::as_const() range-for 后 data 是否仍共享:"
             << (data.constData() == ref.constData())
             << "(仍然共享，没有 detach)";

    qDebug() << "两个 sum 相等:" << (sum1 == sum2);
    qDebug() << "";
}

/// @brief 演示 STL 算法（sort / find_if / transform / erase-remove）在 QList 上的使用。
void demoStlAlgorithms()
{
    qDebug() << "=== STL 算法 on QList ===";

    QList<int> data = {5, 3, 8, 1, 9, 2, 7, 4, 6, 10};

    // std::sort 排序
    std::sort(data.begin(), data.end());
    qDebug() << "std::sort 后:" << data;

    // std::find_if 查找
    auto it = std::find_if(data.cbegin(), data.cend(),
                           [](int v) { return v > 7; });
    if (it != data.cend()) {
        qDebug() << "std::find_if (>7): 第一个满足条件的元素 =" << *it;
    }

    // std::transform 变换到新容器
    QList<int> doubled;
    doubled.reserve(data.size());
    std::transform(data.cbegin(), data.cend(),
                   std::back_inserter(doubled),
                   [](int v) { return v * 2; });
    qDebug() << "std::transform (x2):" << doubled;

    // erase-remove 惯用法：删除所有偶数
    data.erase(std::remove_if(data.begin(), data.end(),
                               [](int v) { return v % 2 == 0; }),
               data.end());
    qDebug() << "erase-remove (去偶数):" << data;

    qDebug() << "";
}

/// @brief 演示 Qt6 中 QList 与 QVector 的互操作。
///
/// Qt6 中 QList 和 QVector 是同一类型（typedef 关系），toVector() 和
/// 迭代器构造可以高效互转。
void demoQListQVectorInterop()
{
    qDebug() << "=== QList/QVector 互操作 ===";

    QList<int> qlist = {1, 2, 3, 4, 5};
    qDebug() << "原始 QList:" << qlist;

    // Qt6 中 QList 和 QVector 是同一个类型
    // toVector() 返回的是数据的拷贝或移动
    QVector<int> vec = qlist.toVector();
    qDebug() << "toVector() 后:" << vec;

    // 从 QVector 构造 QList
    QVector<int> anotherVec = {10, 20, 30};
    QList<int> fromVec = QList<int>(anotherVec.begin(), anotherVec.end());
    qDebug() << "从 QVector 构造 QList:" << fromVec;

    // Qt6 中 QList 就是 QVector，类型相同
    qDebug() << "Qt6 中 QList<int> 和 QVector<int> 是同一类型:"
             << std::is_same_v<QList<int>, QVector<int>>;

    qDebug() << "";
}
