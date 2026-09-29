// 05-variant-type-beginner 示例代码
// 演示 QVariant 的基本用法：存储、提取、类型检查、自定义类型

#include <QCoreApplication>
#include <QVariant>
#include <QDebug>
#include <QString>
#include <QColor>
#include <QList>
#include <QMap>

// ================================
// 自定义类型演示
// ================================

// 定义一个简单的自定义类型
struct Person {
    QString name;
    int age;

    // 方便调试输出
    QString toString() const {
        return QString("Person(name=%1, age=%2)").arg(name).arg(age);
    }
};

// 注册类型到 Qt 元对象系统（这是使用 QVariant 存储自定义类型的关键）
Q_DECLARE_METATYPE(Person)

// ================================
// 演示 1：QVariant 基础用法
// ================================

void demo1_BasicUsage() {
    qDebug() << "=== 演示 1: QVariant 基础用法 ===";

    // 存储不同类型的值
    QVariant v1 = 42;                           // 存储 int
    QVariant v2 = 3.14;                         // 存储 double
    QVariant v3 = true;                         // 存储 bool
    QVariant v4 = QString("Hello Qt");          // 存储 QString
    QVariant v5 = QColor(255, 0, 0);            // 存储 QColor

    qDebug() << "int:" << v1.toInt();
    qDebug() << "double:" << v2.toDouble();
    qDebug() << "bool:" << v3.toBool();
    qDebug() << "QString:" << v4.toString();
    qDebug() << "QColor:" << v5.value<QColor>();

    // 检查 QVariant 是否有效
    QVariant invalid;
    qDebug() << "Invalid QVariant:" << invalid.isValid();

    qDebug() << "";
}

// ================================
// 演示 2：类型检查的重要性
// ================================

void demo2_TypeChecking() {
    qDebug() << "=== 演示 2: 类型检查的重要性 ===";

    // 创建一个字符串类型的 QVariant
    QVariant v = "123";

    // 不安全的做法：直接转换（可能得到意外结果）
    int unsafeNum = v.toInt();
    qDebug() << "不安全转换结果:" << unsafeNum;  // 输出 123，看起来正确

    // 换个字符串试试
    QVariant v2 = "hello";
    int unsafeNum2 = v2.toInt();
    qDebug() << "\"hello\" 转换为 int:" << unsafeNum2;  // 输出 0，这是错误的！

    // 安全的做法：先检查类型
    if (v2.typeId() == QMetaType::QString) {
        QString s = v2.toString();
        qDebug() << "正确做法：先检查类型，得到字符串:" << s;
    }

    // 使用 canConvert 检查是否可转换
    QVariant v3 = "456";
    if (v3.canConvert<int>()) {
        bool ok;
        int num = v3.toString().toInt(&ok);  // 转换并检查是否成功
        if (ok) {
            qDebug() << "\"456\" 成功转换为 int:" << num;
        }
    }

    // Qt 6 推荐使用 typeId()
    QVariant v4 = 789;
    if (v4.typeId() == QMetaType::Int) {
        qDebug() << "使用 typeId() 检查类型: 这是一个 int，值为" << v4.toInt();
    }

    qDebug() << "";
}

// ================================
// 演示 3：容器类型与 QVariant
// ================================

void demo3_Containers() {
    qDebug() << "=== 演示 3: 容器类型与 QVariant ===";

    // QVariantList 实际上是 QList<QVariant>
    QVariantList list;
    list << 1 << "two" << 3.14 << true;
    qDebug() << "QVariantList:" << list;

    // 遍历并处理不同类型
    qDebug() << "遍历列表:";
    for (const QVariant &item : list) {
        if (item.typeId() == QMetaType::Int) {
            qDebug() << "  Int:" << item.toInt();
        } else if (item.typeId() == QMetaType::QString) {
            qDebug() << "  String:" << item.toString();
        } else if (item.typeId() == QMetaType::Double) {
            qDebug() << "  Double:" << item.toDouble();
        } else if (item.typeId() == QMetaType::Bool) {
            qDebug() << "  Bool:" << item.toBool();
        }
    }

    // QVariantMap 实际上是 QMap<QString, QVariant>
    QVariantMap map;
    map["name"] = "Qt";
    map["version"] = 6.5;
    map["stable"] = true;
    map["supportedPlatforms"] = QStringList({"Windows", "Linux", "macOS"});

    qDebug() << "QVariantMap:";
    qDebug() << "  name:" << map["name"].toString();
    qDebug() << "  version:" << map["version"].toDouble();
    qDebug() << "  stable:" << map["stable"].toBool();
    qDebug() << "  platforms:" << map["supportedPlatforms"].toStringList();

    qDebug() << "";
}

// ================================
// 演示 4：自定义类型与 QVariant
// ================================

void demo4_CustomType() {
    qDebug() << "=== 演示 4: 自定义类型与 QVariant ===";

    // 创建 Person 对象
    Person alice{"Alice", 30};
    Person bob{"Bob", 25};

    // 使用 fromValue 存储自定义类型
    QVariant v1 = QVariant::fromValue(alice);
    QVariant v2 = QVariant::fromValue(bob);

    qDebug() << "存储的 Person 类型信息:" << v1.typeName();
    qDebug() << "是否有效:" << v1.isValid();

    // 使用 value<T>() 提取自定义类型
    Person extracted1 = v1.value<Person>();
    Person extracted2 = v2.value<Person>();

    qDebug() << "提取的 Person 1:" << extracted1.toString();
    qDebug() << "提取的 Person 2:" << extracted2.toString();

    // 可以把 Person 存入容器
    QList<QVariant> people;
    people << QVariant::fromValue(Person{"Charlie", 35});
    people << QVariant::fromValue(Person{"David", 28});

    qDebug() << "遍历存储 Person 的列表:";
    for (const QVariant &v : people) {
        Person p = v.value<Person>();
        qDebug() << "  " << p.toString();
    }

    qDebug() << "";
}

// ================================
// 演示 5：配置系统示例（实战应用）
// ================================

void demo5_ConfigSystem() {
    qDebug() << "=== 演示 5: 配置系统示例（实战应用） ===";

    // 模拟一个配置系统，用 QVariantMap 存储各种类型的配置
    QVariantMap config;

    // 存储不同类型的配置项
    config["window.width"] = 800;
    config["window.height"] = 600;
    config["window.title"] = QString("My Application");
    config["window.fullscreen"] = false;
    config["theme.backgroundColor"] = QColor(240, 240, 240);
    config["theme.fontSize"] = 12;
    config["network.timeout"] = 30.5;
    config["network.retryCount"] = 3;
    config["user.name"] = QString("Guest");
    config["user.rememberMe"] = true;

    // 读取配置的辅助函数（使用模板）
    auto getConfig = [&config](const QString &key, const QVariant &defaultValue = QVariant()) {
        if (config.contains(key)) {
            return config[key];
        }
        qDebug() << "配置项" << key << "不存在，使用默认值:" << defaultValue;
        return defaultValue;
    };

    // 读取各种配置
    int width = getConfig("window.width", 1024).toInt();
    int height = getConfig("window.height", 768).toInt();
    QString title = getConfig("window.title", "Untitled").toString();
    bool fullscreen = getConfig("window.fullscreen", false).toBool();
    QColor bgColor = getConfig("theme.backgroundColor", QColor(Qt::white)).value<QColor>();
    int fontSize = getConfig("theme.fontSize", 10).toInt();
    double timeout = getConfig("network.timeout", 10.0).toDouble();
    QString userName = getConfig("user.name", "Anonymous").toString();

    qDebug() << "=== 配置读取结果 ===";
    qDebug() << "窗口尺寸:" << width << "x" << height;
    qDebug() << "窗口标题:" << title;
    qDebug() << "全屏模式:" << fullscreen;
    qDebug() << "背景颜色:" << bgColor;
    qDebug() << "字体大小:" << fontSize;
    qDebug() << "网络超时:" << timeout << "秒";
    qDebug() << "用户名:" << userName;

    // 修改配置
    config["user.name"] = QString("Alice");
    config["window.width"] = 1920;
    qDebug() << "\n修改后的用户名:" << config["user.name"].toString();
    qDebug() << "修改后的窗口宽度:" << config["window.width"].toInt();

    qDebug() << "";
}

// ================================
// 演示 6：类型转换陷阱
// ================================

void demo6_ConversionPitfalls() {
    qDebug() << "=== 演示 6: 类型转换陷阱 ===";

    // 陷阱 1：字符串转数字失败
    qDebug() << "--- 陷阱 1: 字符串转数字 ---";
    QVariant v1 = "abc";
    int num1 = v1.toInt();  // 返回 0，但没有错误提示
    qDebug() << "\"abc\" 转为 int:" << num1 << "(可能是错误的！)";

    // 正确做法
    bool ok;
    num1 = v1.toString().toInt(&ok);
    if (!ok) {
        qDebug() << "转换失败！这不是一个有效的数字";
    }

    // 陷阱 2：浮点数转整数精度丢失
    qDebug() << "\n--- 陷阱 2: 浮点数转整数 ---";
    QVariant v2 = 3.99;
    int num2 = v2.toInt();  // 返回 3，不是 4！
    qDebug() << "3.99 转为 int:" << num2 << "(精度丢失！)";

    // 陷阱 3：bool 和 int 的混淆
    qDebug() << "\n--- 陷阱 3: bool 和 int 的混淆 ---";
    QVariant v3 = 2;  // 非 0 非 1 的整数
    bool b = v3.toBool();  // 任何非零值都是 true
    qDebug() << "int 2 转为 bool:" << b;

    QVariant v4 = true;
    int i = v4.toInt();  // true 转为 1
    qDebug() << "bool true 转为 int:" << i;

    qDebug() << "";
}

// ================================
// main 函数
// ================================

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 运行所有演示
    demo1_BasicUsage();
    demo2_TypeChecking();
    demo3_Containers();
    demo4_CustomType();
    demo5_ConfigSystem();
    demo6_ConversionPitfalls();

    qDebug() << "=== 所有演示完成 ===";
    return 0;
}
