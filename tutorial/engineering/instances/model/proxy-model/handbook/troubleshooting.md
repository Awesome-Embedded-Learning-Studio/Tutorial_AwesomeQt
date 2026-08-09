---
title: "卡住怎么办"
description: "按症状查：重写后栈溢出、过滤不刷新、字典序乱排、大小写错、视图空白、特殊字符过滤怪——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `model/01-mv-pattern/proxy-model/`，对照着看。

## 重写 filterAcceptsRow / lessThan 后栈溢出或崩

- 你在 `filterAcceptsRow` / `lessThan` 里**是不是调了 `this->data()`**（proxy 自己的 data）？proxy 的 data 内部要做 source 映射、又会回调这俩函数 → 无限递归。封装 `sourceText()` 强制读 `sourceModel()->data()`。→ `src/proxy-model.cpp:77-83`、[成品导览踩坑①](../#_5-踩坑)
- 进阶排查：[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)

## 切了过滤范围，视图不刷新

- 用了 **Qt 6.13 弃用的 `invalidateFilter()`** 吗？改 `beginFilterChange()` + `endFilterChange(Direction::Rows)`。→ `src/proxy-model.cpp:35-36`、[成品导览踩坑②](../#_5-踩坑)
- 或者压根**没通知代理重算**就改了成员变量？成员变了代理不会自动重算，必须显式通知。
- 进阶排查：[QSortFilterProxyModel 官方文档](https://doc.qt.io/qt-6/qsortfilterproxymodel.html)

## 年龄列排成 `100 < 35 < 9`

- 数值列**是不是走了默认 lessThan**（字符串字典序）？`"100"` 首字符 `'1'` 最小所以排最前。用 `toDouble()` 比数值，且 `setNumericColumns` 标记该列。→ `src/proxy-model.cpp:40-43`、[成品导览踩坑③](../#_5-踩坑)
- 检查 `setNumericColumns({1})` **是不是真的调了**，demo 接线在 `demo/proxy-model_window.cpp:63`。

## `"bob"` 排在 `"Charlie"` 后面

- 文本列**是不是用了默认 lessThan**（区分大小写，大写 ASCII 小）？非数值列显式 `Qt::CaseInsensitive`。→ `src/proxy-model.cpp:48`、[成品导览踩坑④](../#_5-踩坑)

## 挂了代理，视图空白 / 看到的是未过滤的原数据

- 代理**有没有 `setSourceModel(source)`**？没挂源，代理没数据可映射。→ `demo/proxy-model_window.cpp:61`
- 视图 `setModel` **是不是挂了 proxy 而非 source**？挂 source 就绕过了代理。→ `demo/proxy-model_window.cpp:101`
- 进阶排查：[Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)

## 搜索框输入 `.` `*` `?` 时过滤行为怪 / 报错

- 你**是不是把关键词当正则喂**了（`setFilterRegExp` 或手动塞 `filterRegularExpression`）？`.` 匹配任意字符、`*` 触发量词。用 `setFilterFixedString` 走字面子串匹配。→ `demo/proxy-model_window.cpp:117`、[成品导览踩坑⑥](../#_5-踩坑)

## moc 报错（Q_PROPERTY / Q_ENUM 不认识）

- 头文件**有没有 `Q_OBJECT`**？→ `include/proxy-model.h:26`
- CMake **有没有开 AUTOMOC**？→ `model/01-mv-pattern/proxy-model/CMakeLists.txt`
- `Q_ENUM(FilterScope)` 的枚举**是不是在类外、类里 Q_ENUM 紧跟**？本类把 `FilterScope` 放命名空间、Q_ENUM 放类里，moc 认得。→ `include/proxy-model.h:13,27`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)

## 编译过不了：找不到 proxy-model.h

- `target_include_directories(awesomeqt_proxy-model PUBLIC include)` 写了吗？demo link 了 `awesomeqt_proxy-model` 后靠这个 PUBLIC 传递 include 路径。→ `model/01-mv-pattern/proxy-model/CMakeLists.txt`
