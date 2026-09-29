// Qt 容器入门示例
// 演示 QList、QMap、QHash、QSet 和隐式共享的基本用法

#include <QCoreApplication>
#include <QTextStream>
#include <QList>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QtGlobal>

// 用于输出到标准输出
QTextStream out(stdout);

// 辅助函数：打印 QList 内容
template<typename T>
void printList(const QString &title, const QList<T> &list) {
    out << title << ": ";
    for (const auto &item : list) {
        out << item << " ";
    }
    out << Qt::endl;
}

// 演示 QList 的基本操作
void demonstrateQList() {
    out << "=== QList 演示 ===" << Qt::endl;

    // 创建并初始化 QList
    QList<int> numbers = {1, 2, 3, 4, 5};
    printList("初始列表", numbers);

    // 添加元素
    numbers.append(6);              // 尾部追加，均摊 O(1)
    numbers.prepend(0);             // 头部插入，O(n)
    printList("追加后", numbers);

    // 插入和访问
    numbers.insert(2, 99);          // 在索引 2 位置插入 99
    printList("插入后", numbers);

    // 安全访问与不安全访问
    int safeValue = numbers.at(3);  // 越界会报错，安全
    int fastValue = numbers[3];     // 不检查越界，更快
    out << "at(3): " << safeValue << " operator[](3): " << fastValue << Qt::endl;

    // 获取底层 C 数组指针
    int* rawPtr = numbers.data();
    out << "底层指针指向的第一个元素: " << *rawPtr << Qt::endl;

    // 使用范围-based for 遍历（配合 std::as_const 避免隐式 detach）
    out << "遍历元素: ";
    for (const auto &item : std::as_const(numbers)) {
        out << item << " ";
    }
    out << Qt::endl << Qt::endl;
}

// 演示 QMap（有序映射）
void demonstrateQMap() {
    out << "=== QMap 演示 ===" << Qt::endl;

    // 创建并插入键值对
    QMap<QString, int> scores;
    scores["Alice"] = 95;
    scores["Charlie"] = 88;
    scores["Bob"] = 87;

    out << "所有成绩（按键排序）:" << Qt::endl;
    // QMap 迭代时按 key 排序
    for (auto it = scores.cbegin(); it != scores.cend(); ++it) {
        out << "  " << it.key() << ": " << it.value() << Qt::endl;
    }

    // 查找操作
    QString name = "Alice";
    if (scores.contains(name)) {
        out << name << " 的成绩是: " << scores.value(name) << Qt::endl;
    }

    // value() 方法找不到时返回默认值，不插入新元素
    int missing = scores.value("David", -1);  // 不存在则返回 -1
    out << "查找不存在的学生: " << missing << Qt::endl;
    out << Qt::endl;
}

// 演示 QHash（哈希表，无序但快速）
void demonstrateQHash() {
    out << "=== QHash 演示 ===" << Qt::endl;

    // 创建并插入键值对
    QHash<QString, int> hashScores;
    hashScores["Alice"] = 95;
    hashScores["Charlie"] = 88;
    hashScores["Bob"] = 87;

    out << "所有成绩（哈希表，无序）:" << Qt::endl;
    // QHash 迭代顺序不确定
    for (auto it = hashScores.cbegin(); it != hashScores.cend(); ++it) {
        out << "  " << it.key() << ": " << it.value() << Qt::endl;
    }

    // 演示 operator[] 的陷阱：找不到时会自动插入
    out << Qt::endl << "operator[] 陷阱演示:" << Qt::endl;
    out << "插入前 contains('David')? " << (hashScores.contains("David") ? "是" : "否") << Qt::endl;
    int temp = hashScores["David"];  // 这会插入一个默认值 0！
    out << "访问 'David' 后 contains('David')? " << (hashScores.contains("David") ? "是" : "否") << Qt::endl;
    out << "自动插入的值: " << hashScores["David"] << Qt::endl;

    // 正确的查询方式
    int safeLookup = hashScores.value("Eve", -1);  // 不插入
    out << "安全查询 'Eve': " << safeLookup << Qt::endl;
    out << Qt::endl;
}

// 演示 QSet（集合，去重）
void demonstrateQSet() {
    out << "=== QSet 演示 ===" << Qt::endl;

    // 从 QList 构造 QSet，自动去重
    QList<int> numbers = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
    QSet<int> uniqueNumbers(numbers.begin(), numbers.end());

    out << "原始列表: ";
    for (int n : numbers) out << n << " ";
    out << Qt::endl;

    out << "去重后: ";
    for (int n : uniqueNumbers.values()) out << n << " ";
    out << Qt::endl;

    // 集合运算
    QSet<int> set1 = {1, 2, 3, 4};
    QSet<int> set2 = {3, 4, 5, 6};

    QSet<int> intersection = set1.intersect(set2);  // 交集
    QSet<int> unionSet = set1.unite(set2);          // 并集

    out << Qt::endl << "集合1: ";
    for (int n : set1.values()) out << n << " ";
    out << Qt::endl;

    out << "集合2: ";
    for (int n : set2.values()) out << n << " ";
    out << Qt::endl;

    out << "交集: ";
    for (int n : intersection.values()) out << n << " ";
    out << Qt::endl;

    out << "并集: ";
    for (int n : unionSet.values()) out << n << " ";
    out << Qt::endl << Qt::endl;
}

// 演示隐式共享（写时复制）
void demonstrateImplicitSharing() {
    out << "=== 隐式共享演示 ===" << Qt::endl;

    // 创建第一个列表
    QList<int> list1 = {1, 2, 3, 4, 5};
    out << "list1 地址: " << quintptr(list1.data()) << Qt::endl;

    // 浅拷贝：共享数据，引用计数变为 2
    QList<int> list2 = list1;
    out << "list2 地址（复制后）: " << quintptr(list2.data()) << Qt::endl;
    out << "两个地址相同? " << ((list1.data() == list2.data()) ? "是" : "否") << Qt::endl;

    // 修改 list2 触发 detach（写时复制）
    list2[0] = 99;
    out << Qt::endl << "修改 list2[0] = 99 后:" << Qt::endl;
    printList("list1", list1);
    printList("list2", list2);
    out << "list1 地址: " << quintptr(list1.data()) << Qt::endl;
    out << "list2 地址（detach后）: " << quintptr(list2.data()) << Qt::endl;
    out << "两个地址相同? " << ((list1.data() == list2.data()) ? "是" : "否") << Qt::endl;
    out << Qt::endl;
}

// 演示统计字符频率（代码填空题答案）
void demonstrateCharCounting() {
    out << "=== 字符频率统计演示 ===" << Qt::endl;

    QString text = "hello world";
    QHash<QChar, int> charCount;

    for (QChar c : text) {
        if (charCount.contains(c)) {
            charCount[c]++;           // 计数加一
        } else {
            charCount[c] = 1;         // 首次出现设为 1
        }
    }

    out << "文本: " << text << Qt::endl;
    out << "字符频率:" << Qt::endl;
    for (auto it = charCount.cbegin(); it != charCount.cend(); ++it) {
        out << "  '" << it.key() << "': " << it.value() << Qt::endl;
    }
    out << Qt::endl;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    out << "Qt 容器入门示例" << Qt::endl;
    out << "==================" << Qt::endl << Qt::endl;

    demonstrateQList();
    demonstrateQMap();
    demonstrateQHash();
    demonstrateQSet();
    demonstrateImplicitSharing();
    demonstrateCharCounting();

    out << "演示完成！" << Qt::endl;
    return 0;
}
