# 现代Qt开发教程（新手篇）6.6——QML Model/Delegate 数据驱动视图

## 1. 数据驱动 UI 的思路转变

我们前面几篇一直在手写 QML 界面——每个 `Text`、每个 `Rectangle` 都是显式声明的，数据也是直接写在属性里的。这种做法写 demo 没问题，但做实际项目就会遇到一个根本性的矛盾：你的数据是动态的。一个联系人列表可能有 5 个人，也可能有 500 个人；一个商品网格可能有 3 列，也可能根据窗口宽度自动变成 5 列。你不可能在 QML 里写 500 个 `Rectangle`，然后祈祷数据不要变。

数据驱动视图的核心思路是：你只负责描述「一条数据长什么样」（delegate），「总共有哪些数据」（model），以及「它们怎么排列」（view）。QML 引擎根据 model 的数量自动创建对应数量的 delegate 实例，数据变了就自动更新对应的 delegate。这种模式在前端框架里叫「列表渲染」——Vue 的 `v-for`、React 的 `Array.map()`，本质上干的是同一件事。

QML 里这套机制的核心是三个概念的组合：Model 提供数据、Delegate 定义单条数据的视觉呈现、View 负责排列和滚动。掌握了这个三角关系，你就掌握了 QML 中所有列表和网格场景的开发方法。

---

## 2. 环境说明

本文档基于以下环境编写和验证：

- Qt 6.9.1，使用 `qt_add_executable` + `qt_add_qml_module` 构建
- CMake 3.26+，C++17 标准
- 纯 QML 部分（ListModel、ListView）只需 `Qt6::Quick` 模块
- C++ QAbstractListModel 部分需要 C++ 源文件配合，额外链接 `Qt6::Quick`
- 本篇示例包含一个完整的 C++ 后端模型类，CMakeLists.txt 和 main.cpp 需要相应配置

---

## 3. ListModel + ListView——纯 QML 的列表方案

### 3.1 ListModel：QML 内置的数据容器

`ListModel` 是 QML 内置的数据模型类型，适合存放简单的结构化数据。你可以把它理解成一个 JavaScript 数组，每个元素是一个键值对对象（QML 里叫 `ListElement`）。

```qml
ListModel {
    id: fruitModel

    ListElement {
        name: "Apple"
        color: "#e74c3c"
        price: 5.5
        inStock: true
    }
    ListElement {
        name: "Banana"
        color: "#f39c12"
        price: 3.2
        inStock: true
    }
    ListElement {
        name: "Grape"
        color: "#9b59b6"
        price: 12.0
        inStock: false
    }
}
```

`ListElement` 里的属性名就是这条数据的字段，属性值可以是 `string`、`number`、`bool` 等基本类型。`ListModel` 也支持动态操作——`append()`、`set()`、`remove()`、`clear()`、`get()` 等方法让你可以在运行时增删改查数据：

```qml
// 动态添加一条数据
fruitModel.append({ "name": "Orange", "color": "#f39c12", "price": 4.0, "inStock": true })

// 修改第一条数据的 price
fruitModel.set(0, { "price": 6.0 })

// 删除第二条
fruitModel.remove(1)

// 清空所有数据
fruitModel.clear()
```

### 3.2 ListView：默认的列表视图

`ListView` 是 QML 中最常用的视图组件，专门用于单列垂直或水平排列的数据列表。它的核心属性只有三个：`model` 指定数据源，`delegate` 指定每条数据的渲染方式，以及 `layout.direction` 控制排列方向。

```qml
ListView {
    id: fruitList
    width: parent.width
    height: parent.height
    model: fruitModel
    spacing: 8
    clip: true

    // 基础的 listview 需要指定一个 delegate
    delegate: Rectangle {
        width: fruitList.width
        height: 60
        color: "#ffffff"
        radius: 8
        border.color: "#e0e0e0"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Rectangle {
                width: 36
                height: 36
                radius: 18
                color: model.color
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: model.name
                    font.pixelSize: 15
                    font.bold: true
                }
                Text {
                    text: "Price: $" + model.price.toFixed(2)
                    font.pixelSize: 12
                    color: "#888"
                }
            }

            Text {
                text: model.inStock ? "In Stock" : "Out of Stock"
                font.pixelSize: 12
                color: model.inStock ? "#27ae60" : "#e74c3c"
            }
        }
    }
}
```

注意 delegate 内部访问数据的方式。在 `ListView` 的 delegate 中，`model` 是一个特殊的对象，它代表当前这条数据。`model.name`、`model.color`、`model.price` 就是你在 `ListElement` 里定义的字段。QML 引擎会为 model 中的每一项数据创建一个 delegate 实例，并把对应的数据注入到该实例的 `model` 对象中。

`ListView` 自带滚动支持——当数据总量超过视图高度时，用户可以用鼠标滚轮或触摸滑动来浏览。`clip: true` 确保超出视图范围的内容不会渲染到外面。

### 3.3 delegate 的抽取和复用

当 delegate 的代码量比较大时（几十行甚至上百行），直接内联在 `ListView` 里会让文件变得很难维护。更好的做法是把 delegate 抽成一个独立的组件：

```qml
// FruitDelegate.qml
import QtQuick

Rectangle {
    // 声明对外暴露的属性，由 ListView 自动注入 model 数据
    required property string name
    required property color fruitColor
    required property real price
    required property bool inStock

    width: ListView.view.width
    height: 60
    color: "#ffffff"
    radius: 8
    border.color: "#e0e0e0"

    // ... 内部布局同上
}
```

然后在 `ListView` 中引用：

```qml
ListView {
    delegate: FruitDelegate {}
}
```

`ListView` 会自动把 model 中的字段名和 delegate 的 `required property` 匹配起来。字段名和属性名必须一致——如果 model 里叫 `color`，delegate 里也叫 `color`，它们就能自动对接。如果你在 model 里叫 `fruitColor` 而 delegate 里叫 `color`，那就需要手动映射了。

---

## 4. GridLayout + Repeater——网格布局

### 4.1 Repeater：轻量级的重复器

`Repeater` 不是一个视图组件，而是一个「重复生成器」。它根据 model 的数据量重复创建指定数量的 delegate，然后把它们塞进自己的父容器里。和 `ListView` 不同，`Repeater` 不自带滚动和虚拟化——它直接把所有元素全部实例化出来。适合数据量不大（几十条以内）且需要灵活布局的场景。

```qml
GridLayout {
    columns: 3
    rowSpacing: 12
    columnSpacing: 12

    Repeater {
        model: fruitModel

        delegate: Rectangle {
            Layout.fillWidth: true
            height: 100
            color: "#ffffff"
            radius: 8
            border.color: "#e0e0e0"

            Column {
                anchors.centerIn: parent
                spacing: 8

                Rectangle {
                    width: 40
                    height: 40
                    radius: 20
                    color: model.color
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: model.name
                    font.pixelSize: 13
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
}
```

`Repeater` 配合 `GridLayout` 可以轻松实现网格效果。`columns` 指定每行放几个元素，`Repeater` 依次创建的 delegate 会按从左到右、从上到下的顺序填入网格。

### 4.2 GridView——专用的网格视图

如果你的网格数据量比较大（几百条），需要滚动和虚拟化支持，那就应该用 `GridView` 而不是 `GridLayout + Repeater`。`GridView` 和 `ListView` 的 API 几乎一样，只是多了 `cellWidth` 和 `cellHeight` 来定义每个单元格的尺寸：

```qml
GridView {
    width: parent.width
    height: parent.height
    cellWidth: width / 3
    cellHeight: 120
    model: fruitModel
    clip: true

    delegate: Rectangle {
        width: GridView.view.cellWidth - 8
        height: GridView.view.cellHeight - 8
        color: "#ffffff"
        radius: 8

        Column {
            anchors.centerIn: parent
            spacing: 6

            Rectangle {
                width: 36
                height: 36
                radius: 18
                color: model.color
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: model.name
                font.pixelSize: 13
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
```

`GridView` 有虚拟化机制——只有当前可见区域内的 delegate 会被实例化，滚动出视野的 delegate 会被回收复用。这意味着即使 model 有一万条数据，实际创建的 delegate 实例也只有屏幕上能看到的那些。这对性能至关重要。

---

## 5. C++ QAbstractListModel——从后端驱动 QML 列表

### 5.1 为什么需要 C++ 模型

`ListModel` 够简单，但有一个致命的限制：数据只能存在 QML 里。如果你的数据来自数据库、网络请求、或者复杂的业务计算，你不可能先把这些数据全部序列化成 QML 的 `ListElement` 再显示——那太低效了，也不符合前后端分离的架构原则。

Qt 提供的解决方案是 `QAbstractListModel`。你在 C++ 端继承这个类，实现几个虚函数，QML 就能通过 `ListView` 直接消费这个模型。数据留在 C++ 端管理，QML 只负责展示。

### 5.2 实现 QAbstractListModel

`QAbstractListModel` 继承自 `QAbstractItemModel`，但针对一维列表做了简化。你最少需要实现三个虚函数：`rowCount()` 返回数据总条数，`data()` 返回指定行列的数据，`roleNames()` 返回角色名到角色 ID 的映射。

```cpp
// fruit_model.h
#ifndef FRUIT_MODEL_H
#define FRUIT_MODEL_H

#include <QAbstractListModel>
#include <QVector>

struct FruitItem
{
    QString name;
    QString color;
    double price;
    bool inStock;
};

class FruitModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // 自定义角色枚举，对应 QML 中 model.xxx 的字段名
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ColorRole,
        PriceRole,
        InStockRole
    };

    explicit FruitModel(QObject *parent = nullptr);

    // 必须实现的三个虚函数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // QML 可调用的数据操作方法
    Q_INVOKABLE void addFruit(const QString &name, const QString &color,
                              double price, bool inStock);
    Q_INVOKABLE void removeFruit(int index);
    Q_INVOKABLE void clearAll();

private:
    QVector<FruitItem> m_fruits;
};

#endif // FRUIT_MODEL_H
```

角色枚举是连接 C++ 数据和 QML 显示的关键。`roleNames()` 函数会返回一个映射，告诉 QML 「`NameRole` 这个角色对应的名字叫 `name`」。之后在 QML 的 delegate 里写 `model.name`，QML 引擎就会调用 C++ 的 `data()` 函数，传入对应的角色 ID，获取数据。

```cpp
// fruit_model.cpp
#include "fruit_model.h"

FruitModel::FruitModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // 初始测试数据
    m_fruits = {
        {"Apple", "#e74c3c", 5.5, true},
        {"Banana", "#f1c40f", 3.2, true},
        {"Grape", "#9b59b6", 12.0, false},
        {"Orange", "#e67e22", 4.0, true},
        {"Blueberry", "#3498db", 18.5, true},
        {"Watermelon", "#2ecc71", 8.0, false}
    };
}

int FruitModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_fruits.size();
}

QVariant FruitModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_fruits.size()) {
        return {};
    }

    const auto &fruit = m_fruits.at(index.row());
    switch (role) {
    case NameRole:
        return fruit.name;
    case ColorRole:
        return fruit.color;
    case PriceRole:
        return fruit.price;
    case InStockRole:
        return fruit.inStock;
    default:
        return {};
    }
}

QHash<int, QByteArray> FruitModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {ColorRole, "fruitColor"},
        {PriceRole, "price"},
        {InStockRole, "inStock"}
    };
}

void FruitModel::addFruit(const QString &name, const QString &color,
                          double price, bool inStock)
{
    beginInsertRows(QModelIndex(), m_fruits.size(), m_fruits.size());
    m_fruits.append({name, color, price, inStock});
    endInsertRows();
}

void FruitModel::removeFruit(int index)
{
    if (index < 0 || index >= m_fruits.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    m_fruits.removeAt(index);
    endRemoveRows();
}

void FruitModel::clearAll()
{
    beginResetModel();
    m_fruits.clear();
    endResetModel();
}
```

这里有几个非常关键但新手经常忽略的细节。`beginInsertRows()` 和 `endInsertRows()` 必须成对调用——它们通知视图「数据即将变化」和「数据变化完成」。如果你只修改了底层数据但没调用这两个函数，QML 视图不会收到任何通知，界面就不会更新。同理，`beginRemoveRows()` / `endRemoveRows()` 用于删除操作，`beginResetModel()` / `endResetModel()` 用于整体重置。

这些函数的调用顺序也很讲究：`beginXxx()` 必须在修改数据之前调用，`endXxx()` 在修改之后调用。如果反过来，视图可能会读到不一致的数据状态，导致崩溃。

### 5.3 在 QML 中使用 C++ 模型

在 `main.cpp` 中创建模型实例，通过 `setContextProperty` 注入：

```cpp
// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "fruit_model.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    FruitModel fruitModel;
    engine.rootContext()->setContextProperty("fruitModel", &fruitModel);

    const QUrl url(u"qrc:/QmlModelDelegateDemo/Main.qml"_qs);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
```

然后在 QML 中直接使用：

```qml
ListView {
    model: fruitModel   // 直接引用 C++ 模型对象

    delegate: Rectangle {
        width: ListView.view.width
        height: 60

        Text {
            text: model.name           // 对应 NameRole
        }
        Rectangle {
            color: model.fruitColor    // 对应 ColorRole
        }
        Text {
            text: "$" + model.price    // 对应 PriceRole
        }
    }
}
```

字段名和 `roleNames()` 返回的映射完全对应。`NameRole` 映射到 `"name"`，所以在 delegate 里用 `model.name`；`ColorRole` 映射到 `"fruitColor"`，所以用 `model.fruitColor`。这个名字你可以随便取，但要保持一致——不一致的话 delegate 里拿到的值就是 `undefined`。

---

## 6. 完整示例代码

完整的项目包含 CMakeLists.txt、main.cpp、fruit_model.h/cpp 和 Main.qml，请参考 `examples/beginner/06-qml/06-qml-model-delegate-beginner/` 目录。示例中包含了一个 TabBar，分别展示 `ListView`（竖向列表）和 `GridView`（网格）两种视图模式，以及一个添加/删除水果的操作面板，所有数据都由 C++ 端的 `FruitModel` 驱动。

---

## 7. 虚拟化与性能注意事项

`ListView` 和 `GridView` 都有内置的虚拟化机制。这意味着不管你的 model 有多少条数据，实际创建的 delegate 实例数量等于可见区域内能显示的数量加上下各一个缓冲区。滚动时，离开视野的 delegate 会被回收到复用池，新的 delegate 从池中取出并填充新数据。这个机制大大减少了内存占用和实例化开销。

但虚拟化也带来了一些需要注意的点。delegate 里的 `Component.onCompleted` 和 `Component.onDestruction` 不等价于数据的「创建」和「销毁」——它们只代表 delegate 这个 UI 组件的创建和回收。你不能在这两个信号里做数据的初始化或清理，因为同一个 delegate 可能被多条数据复用。正确的做法是使用 `onXXXChanged` 信号处理器或 `required property` 绑定来响应数据变化。

另一个性能注意点是 delegate 的复杂度。每个 delegate 内部的元素越多、绑定表达式越复杂，滚动时的帧率就越低。如果发现列表滚动有卡顿，首先检查 delegate 里是否有不必要的嵌套和复杂绑定。一个简单的优化是把 delegate 的高度固定（避免高度计算触发额外布局），以及尽量减少 delegate 内的 `Loader` 和条件渲染。

到这里就大功告成了。Model/Delegate/View 三角关系是 QML 数据驱动 UI 的基石，无论是纯 QML 的 `ListModel` 还是 C++ 的 `QAbstractListModel`，底层机制都是统一的。下一篇是入门层的最后一篇，我们来看 QML 的 Canvas 绘图和粒子系统。
