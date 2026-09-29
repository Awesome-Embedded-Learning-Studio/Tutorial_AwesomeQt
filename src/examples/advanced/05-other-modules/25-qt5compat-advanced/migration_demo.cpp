/// @file    migration_demo.cpp
/// @brief   MigrationDemo 类的实现，五组 Qt5→Qt6 迁移模式的具体演示逻辑。

#include "migration_demo.h"

#include <QDebug>
#include <QRegularExpression>
#include <QStringView>

#include <QRegExp>  // Core5Compat 兼容头，提供 Qt5 的 QRegExp

#include <typeinfo>

// ----------------------------------------------------------------------------
// Construction
// ----------------------------------------------------------------------------

MigrationDemo::MigrationDemo(QObject* parent) : QObject(parent) {}

// ----------------------------------------------------------------------------
// Public entry
// ----------------------------------------------------------------------------

void MigrationDemo::runAllDemos()
{
    qInfo() << "=== Qt5 → Qt6 API Migration Demos ===\n";

    demoHashIterationOrder();
    demoRegExpMigration();
    demoStringRefMigration();
    demoEnumTypeChanges();
    demoVectorListUnification();

    qInfo() << "=== All migration demos completed ===";
}

// ----------------------------------------------------------------------------
// Demo 1: QHash iteration order
// ----------------------------------------------------------------------------

void MigrationDemo::demoHashIterationOrder()
{
    printSeparator("Demo 1: QHash Iteration Order");

    qInfo() << "[Qt5 behavior] QHash iteration order was arbitrary / undefined.";
    qInfo() << "[Qt6 behavior] QHash iteration is now deterministic (insertion order).\n";

    QHash<QString, int> scores;
    scores[QStringLiteral("charlie")] = 95;
    scores[QStringLiteral("alice")] = 88;
    scores[QStringLiteral("bob")] = 72;
    scores[QStringLiteral("diana")] = 100;

    qInfo() << "Iterating QHash<QString, int> (Qt6 guarantees insertion order):";
    for (auto it = scores.constBegin(); it != scores.constEnd(); ++it) {
        qInfo() << " " << it.key() << "=>" << it.value();
    }

    // 两次迭代验证稳定性
    qInfo() << "";
    qInfo() << "Second pass (should match the order above):";
    for (const auto& key : scores.keys()) {
        qInfo() << " " << key << "=>" << scores.value(key);
    }

    qInfo() << "";
    qInfo() << "[Migration note] Code that relied on QHash::keys() being sorted";
    qInfo() << "  must now use QMap or sort explicitly. QHash only guarantees";
    qInfo() << "  insertion order, NOT key-sorted order.\n";
}

// ----------------------------------------------------------------------------
// Demo 2: QRegExp → QRegularExpression migration
// ----------------------------------------------------------------------------

void MigrationDemo::demoRegExpMigration()
{
    printSeparator("Demo 2: QRegExp → QRegularExpression Migration");

    qInfo() << "[Qt5 pattern] QRegExp rx(\"(\\\\d+)\");  // removed from Qt6 Core";
    qInfo() << "[Qt6 pattern] QRegularExpression re(\"(\\\\d+)\");  // PCRE-based\n";

    // Qt5 兼容写法（需要 Core5Compat 模块）
    QRegExp qt5Rx(QStringLiteral("\\d+"));
    bool qt5Match = qt5Rx.exactMatch(QStringLiteral("12345"));
    qInfo() << "QRegExp exactMatch(\"12345\"):" << qt5Match;

    // Qt6 推荐写法
    QRegularExpression qt6Re(QStringLiteral("\\d+"));
    QRegularExpressionMatch match = qt6Re.match(QStringLiteral("12345"));
    bool qt6Match = match.hasMatch();
    qInfo() << "QRegularExpression match(\"12345\"):" << qt6Match;

    // 捕获组对比
    QRegExp qt5Capture(QStringLiteral("(\\w+)@(\\w+)"));
    qt5Capture.indexIn(QStringLiteral("user@host"));
    qInfo() << "";
    qInfo() << "[Qt5 capture] cap(0):" << qt5Capture.cap(0)
            << "cap(1):" << qt5Capture.cap(1)
            << "cap(2):" << qt5Capture.cap(2);

    QRegularExpression qt6Capture(QStringLiteral("(\\w+)@(\\w+)"));
    QRegularExpressionMatch capMatch = qt6Capture.match(QStringLiteral("user@host"));
    qInfo() << "[Qt6 capture] captured(0):" << capMatch.captured(0)
            << "captured(1):" << capMatch.captured(1)
            << "captured(2):" << capMatch.captured(2);

    qInfo() << "";
    qInfo() << "[Migration note] QRegExp is removed from Qt6 Core.";
    qInfo() << "  Core5Compat provides it for transition. New code should use";
    qInfo() << "  QRegularExpression which supports full PCRE syntax.\n";
}

// ----------------------------------------------------------------------------
// Demo 3: QStringRef -> QStringView
// ----------------------------------------------------------------------------

void MigrationDemo::demoStringRefMigration()
{
    printSeparator("Demo 3: QStringRef → QStringView Migration");

    qInfo() << "[Qt5 pattern] QStringRef ref = str.midRef(0, 5);  // deprecated in Qt6";
    qInfo() << "[Qt6 pattern] QStringView view(str);  // lightweight, non-owning view\n";

    const QString original = QStringLiteral("Hello, Qt6 Migration World!");

    // Qt6 推荐写法：QStringView 作为只读子串视图
    QStringView fullView(original);
    qInfo() << "Full view:" << fullView;

    // 左侧子串
    QStringView hello = fullView.left(5);
    qInfo() << "left(5):" << hello;

    // 中间子串
    QStringView mid = fullView.mid(7, 3);
    qInfo() << "mid(7, 3):" << mid;

    // 右侧子串
    QStringView tail = fullView.right(6);
    qInfo() << "right(6):" << tail;

    // QStringView 可以直接用 u"" 字面量构造（Qt6 特性）
    QStringView literal = u"UTF-16 literal view";
    qInfo() << "u\"\" literal:" << literal;

    // 切片操作
    qInfo() << "sliced(7, 2):" << fullView.sliced(7, 2);

    qInfo() << "";
    qInfo() << "[Migration note] Replace all QStringRef with QStringView.";
    qInfo() << "  QStringView is more efficient: no reference counting overhead.";
    qInfo() << "  Use QStringView::toString() when you need an owning QString.\n";
}

// ----------------------------------------------------------------------------
// Demo 4: Enum type changes (scoped enums)
// ----------------------------------------------------------------------------

void MigrationDemo::demoEnumTypeChanges()
{
    printSeparator("Demo 4: Enum Type Changes (Scoped Enums)");

    qInfo() << "[Qt5 behavior] Many enums were unscoped: Qt::black, Qt::AlignLeft";
    qInfo() << "[Qt6 behavior] Enums are now scoped (enum class) in many cases.";
    qInfo() << "  Values are accessed as Qt::GlobalColor::black, etc.\n";

    // Qt::GlobalColor 在 Qt6 中仍可作为 Qt::black 访问（通过 Q_ENUM 兼容），
    // 但新 API 倾向于 scoped enum 风格
    Qt::GlobalColor color = Qt::black;
    qInfo() << "Qt::GlobalColor value:" << color;

    // AlignmentFlag 的组合
    Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter;
    qInfo() << "Combined alignment flags:" << align;

    // 在 Qt6 中部分枚举已变成 enum class
    // 例如 QIODeviceBase::OpenModeFlag 是 scoped enum
    auto openMode = QIODeviceBase::ReadOnly | QIODeviceBase::Text;
    qInfo() << "QIODeviceBase::OpenMode (scoped enum):" << openMode;

    qInfo() << "";
    qInfo() << "[Migration note] switch-case on Qt enums may need explicit casts";
    qInfo() << "  in Qt6. Prefer using the scoped name directly when available.";
    qInfo() << "  Qt5Compat module provides the old unscoped access where needed.\n";
}

// ----------------------------------------------------------------------------
// Demo 5: QVector → QList unification
// ----------------------------------------------------------------------------

void MigrationDemo::demoVectorListUnification()
{
    printSeparator("Demo 5: QVector → QList Unification");

    qInfo() << "[Qt5 behavior] QList<T> and QVector<T> were distinct classes.";
    qInfo() << "[Qt6 behavior] QList<T> IS QVector<T> internally (QVector = alias).\n";

    // Qt6: QList 是主容器，QVector 是 typedef 别名
    QList<int> numbers = {10, 20, 30, 40, 50};
    qInfo() << "QList<int> contents:" << numbers;

    // 随机访问（O(1)，因为内部就是数组）
    qInfo() << "numbers[2] (random access):" << numbers[2];

    // 前后插入
    numbers.prepend(5);
    numbers.append(60);
    qInfo() << "After prepend/append:" << numbers;

    // QVector 仍然存在，但是 QList 的别名
    QVector<int> alias = numbers;  // 完全相同类型，无转换
    qInfo() << "QVector<int> alias (same type):" << alias;

    // 验证类型一致性
    qInfo() << "typeid(QList<int>) == typeid(QVector<int>):"
            << (typeid(QList<int>) == typeid(QVector<int>) ? "true" : "false");

    // QList 现在保证元素连续存储（和 Qt5 QVector 一样）
    const int* rawPtr = numbers.data();
    qInfo() << "Contiguous memory:" << (rawPtr + 1 == &numbers[1] ? "yes" : "no");

    qInfo() << "";
    qInfo() << "[Migration note] Replace QVector with QList in new code.";
    qInfo() << "  QList<T> in Qt6 always has contiguous memory for trivial types.";
    qInfo() << "  No performance difference between QList and QVector.\n";
}

// ----------------------------------------------------------------------------
// Utility
// ----------------------------------------------------------------------------

void MigrationDemo::printSeparator(const QString& title) const
{
    qInfo() << "----------------------------------------";
    qInfo() << title;
    qInfo() << "----------------------------------------";
}
