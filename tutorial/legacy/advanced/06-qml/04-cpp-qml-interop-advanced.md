---
title: "6.4 C++/QML 进阶：类型系统与 QML 模块注册"
description: "入门篇我们用 qmlRegisterType 手动注册 C++ 类型到 QML，能跑但不够现代。Qt 6 的玩法是用 QML_ELEMENT / QML_SINGLETON 宏配合 qt6_add_qml_module 在 CMake 中自动完成注册，同时 QAbstractListModel 驱动 QML ListView 也是 C++/QML 互操作的高频场景。"
---

# 现代Qt开发教程（进阶篇）6.4——C++/QML 进阶：类型系统与 QML 模块注册

## 1. 前言

入门篇我们用 `qmlRegisterType` 手动注册 C++ 类型到 QML，能跑但不够现代——每次加一个类型就要写一行注册代码，还得手动指定模块名、版本号、QML 中可用的类型名，维护起来很容易漏。Qt 6 给了一套基于宏 + CMake 的声明式注册机制：你在 C++ 类上加 `QML_ELEMENT`，在 CMake 里用 `qt6_add_qml_module` 把它打包成 QML 模块，构建系统会自动生成 `qmldir` 和类型注册代码。这套机制不光简化了注册流程，还提供了 `QML_SINGLETON`（全局单例）、`QML_UNCREATABLE`（只暴露枚举和常量，不让 QML 实例化）等精细控制手段。

另外一个绕不开的实战场景是 C++ `QAbstractListModel` 驱动 QML `ListView`。QML 的 `ListView` 本身不持有数据——它需要一个 model，而 C++ 侧实现 `QAbstractListModel` 是最灵活也最常用的方案。这篇我们把类型注册、单例、不可实例化类型、`QAbstractListModel` 完整实现、以及 `Q_INVOKABLE` 线程安全注意事项一口气拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，核心依赖 `Qt6::Qml` 和 `Qt6::Quick` 模块。`qt6_add_qml_module` 是 Qt 6.2 引入的 CMake API，如果你还在用 Qt 5.15 的老式 `qmlRegisterType`，建议迁移到新方案。CMake 最低版本要求 3.16，推荐 3.21+ 以获得更好的 QML 模块支持。

## 3. 核心概念讲解

### 3.1 QML_ELEMENT / QML_SINGLETON 与 qt6_add_qml_module

`QML_ELEMENT` 宏的用法非常简单——在 C++ 类声明中加上它，这个类就会被自动注册为 QML 类型。`QML_SINGLETON` 则把类注册为单例，QML 中直接通过类型名访问，不需要创建实例。这两个宏定义在 `<QQmlEngine>` 或 `<QtQml/qqml.h>` 中，不需要额外 include 特殊头文件，Qt 6 以后它们被统一收拢到 `<QtQml>` 模块。

我们先看一个典型的 C++ 类声明：

```cpp
#include <QObject>
#include <QtQml>

/// @brief 应用全局配置管理器，以 QML 单例方式暴露。
class AppConfig : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString theme READ theme WRITE set_theme NOTIFY themeChanged)
    Q_PROPERTY(int fontSize READ font_size WRITE set_font_size NOTIFY fontSizeChanged)

public:
    explicit AppConfig(QObject* parent = nullptr);

    QString theme() const;
    void set_theme(const QString& theme);

    int font_size() const;
    void set_font_size(int size);

signals:
    void themeChanged();
    void fontSizeChanged();

private:
    QString m_theme;
    int m_font_size;
};
```

这里的 `QML_ELEMENT` 告诉构建系统「这个类需要注册到 QML」，`QML_SINGLETON` 进一步指定它是单例。接下来关键的一步是 CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Qml Quick)

qt6_add_qml_module(myapp
    URI MyApp
    VERSION 1.0
    SOURCES
        appconfig.h
        appconfig.cpp
    QML_FILES
        Main.qml
)
```

`qt6_add_qml_module` 做了三件事：第一，它会扫描标记了 `QML_ELEMENT` 等宏的 C++ 类，自动生成类型注册代码；第二，它生成 `qmldir` 文件，让 QML 引擎能找到这个模块；第三，它把 `QML_FILES` 中列出的 `.qml` 文件打包进模块资源。`URI MyApp` 意味着在 QML 中用 `import MyApp 1.0` 就能引入这个模块里注册的所有类型。

QML 侧的使用方式如下：

```qml
import QtQuick
import MyApp 1.0

Window {
    Text {
        // 单例直接用类名访问属性
        text: AppConfig.theme
        font.pixelSize: AppConfig.fontSize
    }
}
```

不需要任何 `AppConfig { ... }` 实例化——`QML_SINGLETON` 保证了全局只有一个实例。

### 3.2 QML_UNCREATABLE——只暴露枚举和常量

有时候我们有一组枚举或常量需要在 QML 中使用，但不希望 QML 侧创建这个类的实例。比如一个定义了一堆状态码的工具类，它只有静态成员和枚举，实例化毫无意义。`QML_UNCREATABLE(reason)` 就是干这个的：

```cpp
/// @brief 全局状态码定义，QML 中只使用枚举，不实例化。
class StatusCode : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("StatusCode is an enum container only")

public:
    enum class Code {
        kOk = 0,
        kNotFound = 404,
        kServerError = 500
    };
    Q_ENUM(Code)
};
```

QML 侧可以直接引用枚举值：

```qml
import MyApp 1.0

Text {
    text: StatusCode.kOk === 0 ? "All good" : "Error"
}
```

但如果有人试图写 `StatusCode { }`，QML 引擎会报错并显示 `QML_UNCREATABLE` 中指定的原因字符串。这是一种很好的设计约束——把「不该做的事」在编译期/加载期就拦住，而不是靠文档约定。

现在有一道思考题给大家。假设你有一个 `Logger` 类标记了 `QML_SINGLETON`，另一个 `LogLevel` 类标记了 `QML_UNCREATABLE`，Logger 的 `Q_PROPERTY` 中引用了 `LogLevel::Level` 枚举。在 QML 中直接写 `Logger.logLevel === LogLevel.Info` 能正常工作吗？如果 `LogLevel` 没有注册到同一个 QML 模块会怎样？

答案是：只要 `LogLevel` 和 `Logger` 在同一个 `qt6_add_qml_module` 的 URI 下，QML 就能正确解析 `LogLevel.Info`。但如果 `LogLevel` 被注册到另一个模块或者忘记注册，QML 引擎会在加载时报 `LogLevel is not defined`——因为 QML 是按 import 的模块来查找类型的。

### 3.3 QAbstractListModel 完整实现供 QML ListView 驱动

QML 的 `ListView` 需要一个 model 提供数据。对于简单场景可以用 `ListModel`，但数据来源是 C++ 侧（网络请求、数据库、文件系统等）时，我们就需要实现 `QAbstractListModel` 的三个核心方法：`roleNames()`、`data()`、`rowCount()`。

```cpp
/// @brief 联系人列表模型，暴露给 QML ListView 使用。
class ContactModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /// @brief 自定义角色枚举，供 QML delegate 通过 model.name 等方式访问。
    enum class Role {
        kName = Qt::DisplayRole,
        kPhone = Qt::UserRole + 1,
        kAvatar
    };
    Q_ENUM(Role)

    explicit ContactModel(QObject* parent = nullptr);

    /// @brief 返回模型行数，ListView 据此知道要创建多少个 delegate。
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /// @brief 返回指定索引和角色的数据。
    /// @param index 行索引
    /// @param role 角色 ID（对应 roleNames 中的映射）
    QVariant data(const QModelIndex& index, int role) const override;

    /// @brief 将枚举角色 ID 映射为 QML 中可用的字符串名称。
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Contact {
        QString name;
        QString phone;
        QString avatar_url;
    };
    std::vector<Contact> m_contacts;
};
```

`roleNames()` 的实现是关键——它决定了 QML delegate 中怎么写 `model.xxx`：

```cpp
QHash<int, QByteArray> ContactModel::roleNames() const
{
    return {
        {static_cast<int>(Role::kName), "name"},
        {static_cast<int>(Role::kPhone), "phone"},
        {static_cast<int>(Role::kAvatar), "avatarUrl"}
    };
}
```

`data()` 根据 role 返回对应字段：

```cpp
QVariant ContactModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_contacts.size())) {
        return {};
    }

    const auto& contact = m_contacts[index.row()];
    switch (static_cast<Role>(role)) {
    case Role::kName:   return contact.name;
    case Role::kPhone:  return contact.phone;
    case Role::kAvatar: return contact.avatar_url;
    }
    return {};
}
```

QML 侧的使用：

```qml
ListView {
    model: ContactModel { id: contactModel }
    delegate: Item {
        Row {
            Text { text: model.name }
            Text { text: model.phone }
        }
    }
}
```

注意一个细节：如果模型的底层数据在运行时会动态变化（增删行），必须调用 `beginInsertRows` / `endInsertRows` 或 `beginRemoveRows` / `endRemoveRows`。直接修改 `m_contacts` 然后期望 ListView 自动刷新是行不通的——Qt 的 Model/View 机制依赖这些通知函数来驱动视图更新，跳过它们等于视图完全不知道数据变了。

### 3.4 Q_INVOKABLE 的线程安全注意事项

`Q_INVOKABLE` 宏让 QML 可以直接调用 C++ 方法。但这里有一个很容易忽略的线程安全问题：QML 引擎的所有 UI 操作都在主线程（GUI 线程）上执行，当你从 QML 中调用一个 `Q_INVOKABLE` 方法时，这个方法也在主线程上运行。如果你的 `Q_INVOKABLE` 方法内部执行了耗时操作（网络请求、文件 IO、大量计算），整个 UI 会卡住。

```cpp
class DataProvider : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    /// @brief 从网络获取数据——错误示范，会阻塞 UI 线程。
    Q_INVOKABLE QString fetch_data_sync(const QString& url);

    /// @brief 异步获取数据——正确做法，通过信号通知结果。
    Q_INVOKABLE void fetch_data_async(const QString& url);

signals:
    void dataReady(const QString& result);
};
```

`fetch_data_sync` 在 QML 调用时直接阻塞主线程等网络返回，UI 冻结。正确做法是 `fetch_data_async` 内部启动异步操作（用 `QNetworkAccessManager` 或 `QtConcurrent::run`），完成后通过 `dataReady` 信号把结果推送给 QML。信号在跨线程时会自动走 `Qt::QueuedConnection`，不会阻塞调用方。

另外，如果你的 `Q_INVOKABLE` 方法会被工作线程间接调用（比如信号槽连接到了一个工作线程的信号），你需要自己保证方法内部的线程安全——`Q_INVOKABLE` 本身不提供任何线程保护。

## 4. 踩坑预防

第一个坑是 `Q_DECLARE_METATYPE` 遗漏导致信号参数传递失败。当你的 C++ 类用 `Q_PROPERTY` 或信号传递自定义类型（非 Qt 内置类型）到 QML 时，必须用 `Q_DECLARE_METATYPE` 注册这个类型。如果你定义了一个 `struct Contact { QString name; QString phone; }` 并在信号中传递 `Contact` 参数，但没有 `Q_DECLARE_METATYPE(Contact)`，编译可能不报错，但运行时信号发射后 QML 侧的 handler 收到的参数是 `undefined`。原因在于 Qt 的元类型系统不知道如何序列化和传递这个类型。解决方案是在头文件末尾加上 `Q_DECLARE_METATYPE(Contact)`，并在 `main()` 函数中调用 `qRegisterMetaType<Contact>()`（后者主要用于跨线程的 queued connection，纯同线程场景 `Q_DECLARE_METATYPE` 就够了，但加上不会有副作用，属于防御性编程）。

第二个坑是 `qt6_add_qml_module` 的 `SOURCES` 列表遗漏头文件。这个函数需要看到所有标记了 `QML_ELEMENT` 等宏的头文件才能正确生成注册代码。如果你只在 `SOURCES` 里列了 `.cpp` 文件而漏了 `.h` 文件，构建系统扫描不到宏，类型就不会被注册，QML import 时会报 module not found 或 type not available。正确的做法是把所有参与 QML 类型注册的 `.h` 文件都列在 `SOURCES` 中。

第三个坑是 `QAbstractListModel` 的 `data()` 方法中对 `index` 参数的边界检查遗漏。很多初学者的实现只检查了 `index.row() < rowCount` 但忘了检查 `index.isValid()`。当 ListView 在模型为空时或者动画过渡期间请求越界索引，传入的 `index` 可能是无效的，不检查就直接访问底层数据容器会触发越界访问，轻则显示垃圾数据，重则直接 crash。

## 5. 练习项目

练习项目：C++ 驱动的任务清单应用。我们要做一个基于 QML ListView 的任务列表，数据由 C++ 侧的 `TaskModel`（继承 `QAbstractListModel`）提供。

需要实现的功能是：`TaskModel` 暴露 `title`（任务标题）、`done`（完成状态）、`priority`（优先级枚举）三个角色；QML 侧的 delegate 展示任务标题和优先级标签，点击可以切换 done 状态；通过 `Q_INVOKABLE` 方法从 C++ 添加新任务和删除已完成任务。额外定义一个 `Priority` 类，用 `QML_UNCREATABLE` 标记，只暴露 `kLow` / `kMedium` / `kHigh` 三个枚举值供 QML 使用。完成标准是任务增删正确反映到 ListView，切换 done 状态实时更新，枚举值在 QML 中可正确引用。

提示几个关键点：`TaskModel` 的 `setData()` 方法需要同时发出 `dataChanged` 信号；添加任务用 `beginInsertRows` / `endInsertRows` 包裹；`Priority` 枚举类需要 `Q_ENUM` 宏才能在 QML 中使用字符串名称查找。

## 6. 官方文档参考链接

[Qt 文档 · qqmlintegration.h](https://doc.qt.io/qt-6/qqmlintegration-h.html) -- QML_ELEMENT / QML_SINGLETON / QML_UNCREATABLE 宏定义

[Qt 文档 · qt_add_qml_module](https://doc.qt.io/qt-6/qt-add-qml-module.html) -- CMake QML 模块注册 API

[Qt 文档 · Singletons in QML](https://doc.qt.io/qt-6/qml-singleton.html) -- QML 单例创建指南

[Qt 文档 · QAbstractListModel](https://doc.qt.io/qt-6/qabstractlistmodel.html) -- C++ 列表模型基类

[Qt 文档 · Using C++ Models with Qt Quick Views](https://doc.qt.io/qt-6/qtquick-modelviewsdata-cppmodels.html) -- C++ 模型与 QML 视图集成

[Qt 文档 · Exposing C++ Attributes to QML](https://doc.qt.io/qt-6/qtqml-cppintegration-exposecppattributes.html) -- C++ 类型暴露到 QML 的完整参考

[Qt 文档 · Creating Custom Qt Types](https://doc.qt.io/qt-6/custom-types.html) -- 自定义类型注册与 Q_DECLARE_METATYPE

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。QML_ELEMENT + qt6_add_qml_module 的声明式注册、QML_SINGLETON 的全局单例、QML_UNCREATABLE 的枚举容器、QAbstractListModel 驱动 ListView、Q_INVOKABLE 的线程陷阱——这五样搞清楚了，C++ 和 QML 之间的桥梁就算是修通了。接下来你会发现大部分 C++/QML 互操作问题都是这些机制的组合运用。
