---
title: "6.6 QML 模型视图进阶：DelegateModel 与 section"
description: "入门篇我们用 ListModel + ListView + delegate 三件套搭建了基本的列表界面。但真实应用远不止于此——我们需要分组过滤、按首字母分节标题、搜索筛选、以及处理上千条数据时的滚动性能。这篇聚焦 DelegateModel 分组过滤、ListView section 分节、QSortFilterProxyModel 的 QML 集成、以及大数据列表的性能优化。"
---

# 现代Qt开发教程（进阶篇）6.6——QML 模型视图进阶：DelegateModel 与 section

## 1. 前言

入门篇我们用 `ListModel` + `ListView` + delegate 三件套搭建了基本的列表界面，能展示数据、能响应点击，跑起来没问题。但真实应用的需求远不止于此——你需要把联系人按首字母分组展示、在搜索框输入时实时过滤列表、从数据库查出来的上千条记录要丝滑滚动不卡顿。这些场景光靠 `ListModel` 的基础 API 是不够的。

这篇我们聚焦四个进阶能力：`DelegateModel` 的分组与过滤（让同一份模型数据在不同视图中展现不同子集）、`ListView::section` 分节标题（按字段值分组显示标题头）、`QSortFilterProxyModel` 在 QML 中的注册和使用（C++ 侧的过滤排序代理模型），以及大数据列表的性能优化手段。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写。`DelegateModel` 需要 `import QtQml.Models`，`ListView` 和 section 功能在 `import QtQuick` 中。`QSortFilterProxyModel` 需要 C++ 侧 `#include <QSortFilterProxyModel>` 并链接 `Qt6::Core`。CMake 中确保 `find_package(Qt6 REQUIRED COMPONENTS Qml Quick Core)` 包含了所需模块。

## 3. 核心概念讲解

### 3.1 DelegateModel 分组与过滤

`DelegateModel` 是一个功能比直接用 `ListView.model` + `ListView.delegate` 更强大的模型封装器。它最核心的能力是 **groups（分组）**——你可以把模型中的每一项分配到不同的分组中，然后让视图只显示某个分组的内容。

先看一个基本的 DelegateModel 使用：

```qml
import QtQml.Models

ListView {
    id: view
    model: DelegateModel {
        id: delegateModel

        model: contactModel  // 底层数据模型（ListModel 或 C++ model）

        // 定义自定义分组
        groups: [
            DelegateModelGroup {
                name: "visible"
                includeByDefault: true  // 新项默认加入此分组
            },
            DelegateModelGroup {
                name: "hidden"
                includeByDefault: false
            }
        ]

        // 只显示 visible 分组中的项
        filterOnGroup: "visible"

        delegate: Item {
            Text { text: model.name }
            MouseArea {
                onClicked: {
                    // 点击后把此项移到 hidden 分组
                    delegateModel.items.get(index).inHidden = true
                }
            }
        }
    }
}
```

`groups` 定义了两个分组：`visible` 和 `hidden`。`filterOnGroup: "visible"` 让 ListView 只渲染 `visible` 分组中的 delegate。当你把某项的 `inHidden` 设为 `true` 时，DelegateModel 会把它从 `visible` 分组移到 `hidden` 分组，ListView 中这一项就会消失——无需修改底层数据模型。

这种机制非常适合实现搜索过滤：在搜索框的 `onTextChanged` 回调中遍历所有项，匹配搜索条件的放入 `visible` 分组，不匹配的放入 `hidden` 分组。底层数据不变，只是视图展示的子集不同。

DelegateModel 还支持 `items` 属性中的排序操作。通过 `DelegateModelGroup` 的 `sort` 方法，你可以按某个角色值对分组内的项进行排序，不过这种 QML 侧的排序在大数据量时性能不如 C++ 侧的 `QSortFilterProxyModel`。

### 3.2 ListView section——分节标题

`ListView` 内置了按模型中某个字段的值进行分组并显示节标题的能力。通过 `section.property` 指定按哪个角色分组，`section.delegate` 指定节标题的外观，`section.criteria` 控制分组判定方式。

```qml
ListView {
    model: contactModel
    delegate: Text { text: model.name; height: 40 }

    // 按 role 分组——连续相同 section.property 值的项分为一组
    section.property: "group"
    section.criteria: ViewSection.FullString
    section.delegate: Rectangle {
        width: ListView.view.width
        height: 30
        color: "#e0e0e0"
        Text {
            text: section  // "section" 是当前节的值
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
```

在 section.delegate 内部，你可以用 `section` 这个只读属性拿到当前节的值。比如所有 `model.group === "A"` 的连续项会归为一节，section.delegate 中 `section` 的值就是 `"A"`。

`section.criteria` 有两个值：`ViewSection.FullString`（精确匹配整个字符串）和 `ViewSection.FirstCharacter`（只取第一个字符来分组）。后者特别适合按首字母分组展示联系人列表的场景——你不需要在模型中额外加一个「首字母」字段，直接用 `section.criteria: ViewSection.FirstCharacter` 就行。

需要注意的是 section 是基于**连续相同值**来分组的，不是全局去重。如果你的模型数据是 `[A, A, B, A]`，那会出现两个 A 节（中间夹了一个 B）。所以如果你的数据需要全局按首字母分组，必须先排好序。

### 3.3 QSortFilterProxyModel 在 QML 中注册和使用

`DelegateModel` 的过滤是在 QML 侧做的，对于小数据量没问题，但数据量大时遍历所有项的 JS 开销不低。`QSortFilterProxyModel` 是 C++ 侧的代理模型，过滤和排序逻辑在 C++ 中执行，性能好得多。

使用方式是先在 C++ 中创建源模型和代理模型，然后把代理模型注册到 QML：

```cpp
/// @brief 联系人过滤代理模型，支持按名字搜索和按分组排序。
class ContactFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString filterText READ filter_text WRITE set_filter_text
               NOTIFY filterTextChanged)

public:
    explicit ContactFilterProxy(QObject* parent = nullptr);

    QString filter_text() const;
    void set_filter_text(const QString& text);

protected:
    /// @brief 过滤逻辑——名字包含搜索文本则保留。
    bool filterAcceptsRow(int source_row,
                          const QModelIndex& source_parent) const override;

    /// @brief 排序逻辑——按分组字段排序。
    bool lessThan(const QModelIndex& left,
                  const QModelIndex& right) const override;

signals:
    void filterTextChanged();

private:
    QString m_filter_text;
};
```

QML 中的使用非常自然：

```qml
ListView {
    model: ContactFilterProxy {
        id: proxyModel
        // 源模型可以是任何 QAbstractListModel 子类
        // 注意：QSortFilterProxyModel 的 sourceModel 需要在 C++ 中设置
        // 或通过 setSourceModel 绑定
    }

    delegate: Text { text: model.name }

    section.property: "group"
    section.criteria: ViewSection.FirstCharacter
    section.delegate: SectionHeader { text: section }
}
```

当搜索框文本变化时，调用 `proxyModel.filterText = searchText`，代理模型会重新执行 `filterAcceptsRow`，只保留匹配的行。这个过滤操作在 C++ 中完成，比 QML 遍历快得多。

### 3.4 大数据列表性能优化

当列表数据量超过几百条时，ListView 默认的配置可能导致滚动不流畅。Qt Quick 提供了几个关键的性能调优属性。

`cacheBuffer` 控制 ListView 在可见区域之外预加载多少像素的 delegate。默认值是 0（不预加载），这意味着滚动到新区域时才创建新的 delegate，如果 delegate 的创建开销比较大（包含图片、复杂布局），滚动时就会出现短暂的空白闪烁。增大 `cacheBuffer` 可以提前创建屏幕外的 delegate，代价是更多的内存占用：

```qml
ListView {
    cacheBuffer: 500  // 在可见区域上下各预加载 500 像素
    // ...
}
```

`displayMarginBeginning` 和 `displayMarginEnd` 是另一个相关的属性——它们扩展 ListView 的可见区域范围，使得更多的 delegate 保持存活而不被回收。和 `cacheBuffer` 不同的是，`displayMargin` 内的 delegate 处于「可见但超出窗口」的状态，它们不会被销毁：

```qml
ListView {
    displayMarginBeginning: 200
    displayMarginEnd: 200
    // ...
}
```

对于 delegate 本身的性能，关键原则是**让 delegate 尽量简单**。每个 delegate 就是一个轻量级的展示模板——避免在 delegate 内部创建复杂的组件层级、避免在 delegate 中使用 `Loader` 动态加载（除非你确实需要延迟加载）、避免在 delegate 中使用 `Canvas` 或 `ShaderEffect` 等高开销渲染组件。

现在有一道调试题给大家。你的 ListView 有 1000 条数据，`cacheBuffer` 设为 2000，每个 delegate 高度 50px，屏幕高度 800px。请问同时存活的 delegate 大约有多少个？

答案是：屏幕可见区域约 800/50 = 16 个，`cacheBuffer` 2000px 对应上下各 40 个预加载 delegate，总共约 16 + 40 * 2 = 96 个。如果你把 `cacheBuffer` 设得更大，比如 10000，同时存活的 delegate 就会变成 416 个左右——内存占用跟着涨。所以 `cacheBuffer` 不是越大越好，要根据 delegate 的创建开销和内存预算来权衡。

## 4. 踩坑预防

第一个坑是 DelegateModel 的 `filterGroup` 在动态更新时可能不同步。当底层数据模型发生变化（添加、删除、修改项）时，DelegateModel 的分组状态需要手动重新同步。具体来说，如果你通过 `model.items.get(index).inVisible = false` 把某项从 `visible` 分组移走了，但之后底层模型插入了新行，新行的分组状态取决于 `includeByDefault` 设置。如果 `includeByDefault` 为 `true`，新行会自动进入 `visible` 分组——这可能是你想要的，也可能不是。更麻烦的是，如果你在遍历分组的同时修改了分组归属，DelegateModel 的内部索引可能不一致，导致 QML 报 "Index out of range" 错误。解决方案是批量修改分组时先收集所有需要移动的索引，然后逆序遍历（从大到小）逐个修改，避免索引偏移。

第二个坑是 `ListView::section` 的 `section.property` 引用的角色名必须和模型中实际存在的角色名完全一致（大小写敏感）。如果你在 `QAbstractListModel::roleNames()` 中注册的是 `"groupName"`，但 QML 中写的是 `section.property: "groupname"`（少了个大写 N），section 功能直接失效——ListView 会认为所有项都没有 section 属性，不显示任何节标题。调试时可以在 delegate 中打印 `model.groupName` 确认角色名是否正确。

第三个坑是 `QSortFilterProxyModel` 的 `filterAcceptsRow` 在高频调用场景下（比如搜索框每输入一个字符就触发过滤）可能导致短暂的 UI 卡顿。原因是每次过滤都会重建代理模型的内部映射表，如果源模型有几千行数据，重建操作虽然是在 C++ 中完成但仍然有可感知的延迟。解决方案是给搜索加一个 debounce（防抖）——用 `Timer` 延迟 200-300ms 后才真正更新过滤文本，避免每次按键都触发全量过滤。

## 5. 练习项目

练习项目：联系人搜索列表。C++ 侧实现 `ContactModel`（QAbstractListModel，包含 name/phone/group 三个角色）和 `ContactFilterProxy`（QSortFilterProxyModel，支持按 name 过滤）。QML 侧一个搜索框加一个 ListView，ListView 按 group 字段分节显示节标题（用 `section.property: "group"` + `section.criteria: ViewSection.FirstCharacter`），输入搜索文本时代理模型实时过滤。同时用 DelegateModel 实现一个「已收藏」分组——点击某个联系人后把它加入收藏分组，在另一个小 ListView 中单独显示收藏列表。

完成标准是搜索过滤响应流畅（使用 Timer debounce），分节标题正确显示首字母，收藏列表和主列表的数据不冲突，500 条联系人的滚动无明显卡顿。提示：`ContactFilterProxy` 的 `filterAcceptsRow` 用 `QString::contains` 做模糊匹配即可；DelegateModel 的分组切换用 `inVisible` / `inFavorite` 布尔属性控制。

## 6. 官方文档参考链接

[Qt 文档 · DelegateModel](https://doc.qt.io/qt-6/qml-qtqml-models-delegatemodel.html) -- DelegateModel 分组与过滤

[Qt 文档 · DelegateModelGroup](https://doc.qt.io/qt-6/qml-qtqml-models-delegatemodelgroup.html) -- 分组操作 API

[Qt 文档 · ListView](https://doc.qt.io/qt-6/qml-qtquick-listview.html) -- ListView section / cacheBuffer / displayMargin 属性

[Qt 文档 · QSortFilterProxyModel](https://doc.qt.io/qt-6/qsortfilterproxymodel.html) -- C++ 过滤排序代理模型

[Qt 文档 · Models and Views in Qt Quick](https://doc.qt.io/qt-6/qtquick-modelviewsdata-modelview.html) -- 模型视图架构总览

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。DelegateModel 让你在 QML 侧做灵活的分组和过滤，section 让 ListView 自动显示分节标题，QSortFilterProxyModel 把过滤排序的重活交给 C++，cacheBuffer 和 displayMargin 解决大数据滚动的流畅性问题——这四样组合起来，你就能应对大部分真实应用中的列表场景了。记住一个原则：小数据用 DelegateModel（灵活），大数据用 QSortFilterProxyModel（性能），section 加上就是锦上添花。
