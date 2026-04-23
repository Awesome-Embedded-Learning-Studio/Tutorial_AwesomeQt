# 现代Qt开发教程（新手篇）1.4——容器

## 1. 前言

说实话，我刚从 STL 转过来的时候也有个疑问：为什么不用 std::vector 和 std::map？C++ 标准库不是挺成熟的吗？

后来实际项目跑起来才明白，Qt 容器确实有一套自己的哲学。首先它们是隐式共享的，传值不拷贝，这在信号槽这种到处都是参数传递的场景下简直是神器。其次它们和 Qt 其他类型配合得天衣无缝，QString、QDateTime 这些往容器里一塞，什么都不用管。还有 Qt6 之后 QList 统一成了连续数组模型，性能表现和 std::vector 一样优秀。

但我们也要承认，如果你的项目已经大量用了 STL，没有特别必要全部换成 Qt 容器。但至少你要了解它们，因为 Qt API 里到处都是 QList、QStringList、QMap 这种返回类型，你不可能躲得掉。

## 2. 环境说明

本文基于 Qt 6.10+，所有示例代码都经过 CMake 3.26+ 环境验证。如果你还在用 qmake 时代，建议尽快迁移，CMake 对 Qt6 的支持比 qmake 顺畅太多。

## 3. 核心概念

### 3.1 QList：万能的连续数组

先说清楚一个历史包袱：Qt5 时代的 QList 是个指针数组的混合体，中间层间接访问，性能不是最优。Qt6 把 QList 和 QVector 统一了，现在就是纯粹的连续内存数组，和 std::vector 一样是连续存储。

```cpp
#include <QList>

QList<int> numbers = {1, 2, 3, 4, 5};
numbers.append(6);              // 尾部追加，均摊 O(1)
numbers.prepend(0);             // 头部插入，O(n) 需要移动所有元素
numbers.insert(2, 99);          // 位置 2 插入，同样 O(n)
int value = numbers.at(3);      // 安全访问，越界会报错
int fast = numbers[3];          // 不检查越界，性能更好
```

你会发现我特别强调 "连续内存" 这个特性。这意味着什么？意味着你可以用指针直接遍历，意味着 CPU 缓存命中率高，意味着和 C 数组互操作很方便。

```cpp
int* rawPtr = numbers.data();                      // 获取底层 C 数组指针
const int* constRawPtr = std::as_const(numbers).data();
```

你可能会问，QList 和 std::vector 到底有什么本质区别？什么时候该优先选择 QList？答案是：在 Qt6 之后，QList 的底层实现和 std::vector 几乎一样，真正的差异在于隐式共享机制和与 Qt 类型系统的深度集成。当你需要和 Qt API 交互、需要信号槽传递容器、需要隐式共享带来的传值效率时，QList 是更自然的选择。纯算法逻辑场景下，std::vector 完全够用，不必强求替换。

### 3.2 QMap vs QHash：有序还是快速

这个问题我面试时经常问。QMap 是红黑树实现，键值对按 key 排序存储，查找是 O(log n)。QHash 是哈希表实现，查找均摊 O(1)，但迭代顺序是乱的。

```cpp
#include <QMap>
#include <QHash>

QMap<QString, int> scores;
scores["Alice"] = 95;
scores["Bob"] = 87;
// 迭代时按 key 排序：Alice, Bob
for (auto it = scores.cbegin(); it != scores.cend(); ++it) {
    qDebug() << it.key() << ":" << it.value();
}

QHash<QString, int> hashScores;
hashScores["Alice"] = 95;
hashScores["Bob"] = 87;
// 迭代顺序不确定
```

这里有个很实际的抉择：如果你需要有序遍历，或者需要范围查询（比如查找所有以 "A" 开头的 key），用 QMap。如果纯粹是 key-value 查找，QHash 是性能之王。

```cpp
// QHash 查找示例
if (hashScores.contains("Alice")) {
    int score = hashScores.value("Alice");      // 找不到返回默认构造值 0
    int score2 = hashScores["Alice"];           // 找不到会自动插入默认值！
}
```

注意上面那个坑：`operator[]` 找不到 key 时会自动插入一个默认构造的值，这可能是你想要的，也可能不是。用 `value()` 方法更安全，找不到直接返回默认值但不插入。

说到这里，我们不妨动手试一下：如果要统计一个字符串中每个字符出现的次数，你会怎么写？大致思路是用 QHash<QChar, int> 做计数器，遍历字符串时判断字符是否已经存在于哈希表中——存在的话计数加一，不存在的话首次出现设为 1。

### 3.3 QSet：去重利器

QSet 本质上是个 QHash<Key, void>，只存 key 不存 value，用来做去重和集合运算。

```cpp
#include <QSet>

QList<int> numbers = {1, 2, 2, 3, 3, 3, 4};
QSet<int> uniqueNumbers(numbers.begin(), numbers.end());  // 从 QList 构造，自动去重
// uniqueNumbers 现在是 {1, 2, 3, 4}

// 集合运算
QSet<int> set1 = {1, 2, 3};
QSet<int> set2 = {2, 3, 4};
QSet<int> intersection = set1.intersect(set2);      // 交集 {2, 3}
QSet<int> union_ = set1.unite(set2);                // 并集 {1, 2, 3, 4}
```

### 3.4 隐式共享：写时复制的魔法

这是 Qt 容器的杀手级特性，我专门放在最后说因为它确实有点反直觉。

当你复制一个 Qt 容器时，表面上看是复制了整个容器，实际上内部只是复制了一个指针和一个引用计数。真正的数据复制只有在一个容器被修改时才会发生，这叫做 "detach"（分离）。

```cpp
QList<int> list1 = {1, 2, 3};
QList<int> list2 = list1;          // 浅拷贝，共享同一块数据，引用计数变为 2
// 此时 list1 和 list2 的数据指针指向同一个内存块

list2[0] = 99;                     // 写操作触发 detach，list2 独立复制数据
// 现在 list2 = {99, 2, 3}，list1 仍然是 {1, 2, 3}
```

这个机制使得按值传递容器非常高效。比如一个函数返回 QList：

```cpp
QList<int> getNumbers() {
    QList<int> result;
    result << 1 << 2 << 3;
    return result;  // C++17 之后即使没有 RVO 也因为隐式共享很高效
}
```

但这里有个很重要的陷阱：迭代器和隐式共享的冲突问题，我们下一节专门讲。

## 4. 踩坑预警

隐式共享虽然好用，但有几个坑是一定会遇到的。我们挨个来说。

第一个坑是迭代器失效。当你持有某个容器的迭代器，然后又复制了这个容器，接着对原容器做了写操作，这时原容器会发生 detach——它复制了一份独立数据，但迭代器仍然指向旧的共享数据。如果另一个容器此时被清空，迭代器就彻底失效了，解引用是未定义行为。所以正确的做法是：先复制容器，再获取迭代器；或者干脆不要在迭代器活跃期间复制容器。一句话记住——迭代器活跃期间不要复制容器，容器复制后不要用旧的迭代器。

```cpp
// 典型的错误场景
QList<int> a, b;
a.resize(100000);
auto it = a.begin();
b = a;                          // it 现在指向共享数据
a[0] = 5;                       // a detach，it 变成指向 b 的迭代器
b.clear();                      // it 彻底失效
// int value = *it;             // 未定义行为！

// 正确做法：先复制，再获取迭代器
QList<int> a2, b2;
a2.resize(100000);
b2 = a2;                        // 先复制
auto it2 = a2.begin();          // 再获取迭代器
```

第二个坑是 QHash 的 `operator[]` 会自动插入。很多人在条件判断里顺手写了 `scores["Charlie"] > 80`，以为这只是查询，结果 "Charlie" 不存在时被自动插入了默认值 0，哈希表里多了一堆脏数据。正确做法是用 `value("Charlie", 0)` 或者先 `contains()` 再访问。查询用 `value()`，确定要插入时才用 `operator[]`，这个习惯要刻进肌肉里。

```cpp
// 错误：查询时意外插入了数据
QHash<QString, int> scores;
if (scores["Charlie"] > 80) {   // "Charlie" 不存在但被插入了！
    qDebug() << "Good score";
}

// 正确：用 value() 安全查询
if (scores.value("Charlie", 0) > 80) {  // 找不到返回 0，不插入
    qDebug() << "Good score";
}
// 或者用 contains()
if (scores.contains("Charlie") && scores["Charlie"] > 80) {
    qDebug() << "Good score";
}
```

第三个坑是 for 循环中的隐式 detach。看下面这段代码，range-based for 看起来是只读遍历，但在某些情况下会触发 detach，因为 range-based for 默认不是 const 的。大容器一 detach 就是一次完整的内存复制，性能直接拉垮。解决办法很简单：只读遍历时包一层 `std::as_const()`，养成肌肉记忆。

```cpp
// 可能触发不必要的 detach
QList<QString> list = {"a", "b", "c"};
for (const auto &item : list) {      // 等等，真的是 const 吗？
    qDebug() << item;
}

// 正确做法：用 std::as_const 避免 detach
QList<QString> list2 = {"a", "b", "c"};
for (const auto &item : std::as_const(list2)) {
    qDebug() << item;
}
```

最后留一个调试题给你：下面这段代码有什么问题？

```cpp
QList<QWidget*> widgets;
for (int i = 0; i < 10; ++i) {
    widgets.append(new QWidget());
}

for (auto w : widgets) {
    delete w;
}
widgets.clear();  // 这行有必要吗？
```

提示一下：`clear()` 之后列表里存的是已被 delete 的指针值，但 `delete` 不会把指针置空，所以 dangling pointer 的问题需要考虑。另外 `for (auto w : ...)` 这里是值拷贝，每次循环都拷贝了一个指针，用 `const auto&` 会更高效。

## 5. 随堂测验回顾

前面我们穿插了几个小测验，简单回顾一下要点。关于 QList 和 std::vector 的区别，核心在于隐式共享和 Qt 类型系统集成。关于字符计数，关键是判断 contains 后分别处理加一和初始化。关于 QWidget 指针容器的调试题，要点是理解 delete 指针后容器内的指针值不会自动清空。

## 6. 练习项目

练习项目：学生成绩管理系统。

实现一个简单的学生成绩管理程序，用 QMap 存储学生姓名和成绩，用 QList 存储班级所有学生。程序需要支持添加学生、删除学生、按成绩排序、统计平均分等功能。

完成标准是程序能够正确添加、删除、查询学生成绩，使用 QHash 或 QMap 实现姓名到成绩的映射，用 QList 维护学生名单。排序功能可以手动实现排序算法，也可以用 std::sort。程序启动时预置一些测试数据，退出前打印所有学生信息。

几个思路供参考：用 `QMap<QString, int>` 存储姓名到成绩的映射可以自动按姓名排序；如果需要按成绩排序，考虑用 `QList<QPair<QString, int>>` 配合 `std::sort`；别忘了在添加学生时检查是否已存在同名学生；统计功能可以用 range-based for 配合 `std::as_const()`。

## 7. 官方文档参考链接

[Qt 文档 - Container Classes](https://doc.qt.io/qt-6/containers.html) - 容器类总览，包含所有容器类的性能对比和用法示例

[Qt 文档 - Implicit Sharing](https://doc.qt.io/qt-6/implicit-sharing.html) - 隐式共享机制的详细解释，包括隐式共享类的完整列表
