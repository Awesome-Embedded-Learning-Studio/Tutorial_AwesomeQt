---
title: "Step 5-6：自定义过滤——单列/全列 + Qt6 通知范式"
description: "重写 filterAcceptsRow 做关键词子串匹配，FilterScope 切换单列/全列；用 begin/endFilterChange（不踩 invalidateFilter 弃用坑）。"
---

# Step 5-6：自定义过滤——单列/全列 + Qt6 通知范式

← [手册首页](./index.md) · 上一步 [Step 3-4 自定义排序](./02-custom-sort.md) →

## Step 5：重写 filterAcceptsRow——单列 / 全列过滤

### 目标

顶部搜索框输入 `beijing`，表格只剩城市是 Beijing 的行；切到「全列扫描」模式，输入 `e` 连 Name 列带 `e` 的行也出来。

### 提示

- 重写 `protected bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override`
- 先 `sourceModel()` 拿源模型，**null 兜底 `return true`**（构造期 / 拆卸期安全）
- 取关键词：`filterRegularExpression().pattern()`——基类把 `setFilterFixedString` 喂进来的串存这儿
- **取单元格文本别调 proxy 的 `data()`**（会递归，见 [卡住怎么办](./troubleshooting.md)），封装 `sourceText(idx)` 走 `sourceModel()->data(idx, Qt::DisplayRole).toString()`
- 加 `enum class FilterScope { kActiveColumn, kAllColumns }`：
  - `kActiveColumn`：只看 `filterKeyColumn()` 那列，`sourceModel()->index(row, filterKeyColumn(), parent)`
  - `kAllColumns`：遍历 `columnCount()`，任一列 `contains(needle, Qt::CaseInsensitive)` 即保留
- 大小写：这里显式 `Qt::CaseInsensitive`（独立于 `setFilterCaseSensitivity`，教学上把语义写死）

### 检查点

输入 `beijing`，只剩 Beijing 的行（单列模式）；切全列模式输入 `e`，Alice/Charlie/Eve/Beijing/... 凡含 `e` 的行都出来 = 过滤范围切换对了。

> filterAcceptsRow / 过滤管线不熟？[Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)。

### 对照答案

- filterAcceptsRow 主体 + null 兜底：`src/proxy-model.cpp:51-73`
- kActiveColumn 单列分支：`src/proxy-model.cpp:61-63`
- kAllColumns 全列循环：`src/proxy-model.cpp:67-72`
- sourceText 帮手（防递归）：`src/proxy-model.cpp:77-83`

---

## Step 6：切换过滤范围用 Qt 6 通知范式（不踩弃用坑）

### 目标

`setFilterScope` 切换后，视图**立刻**按新范围重新过滤（不卡住、不留旧结果），且编译零警告。

### 提示

- 切换后必须通知代理「行的可见性要重算」
- **不要用 `invalidateFilter()`**——它在 Qt 6.13 起被标记 `QT_DEPRECATED_VERSION_X_6_13`，开了 `-Wdeprecated-declarations` 会警告
- 改用 `beginFilterChange()` + `endFilterChange(QSortFilterProxyModel::Direction::Rows)`：这对调用通知视图「行的过滤状态要变」，代理重新逐行调 `filterAcceptsRow`
- （`invalidate()` 没被弃用——它重算排序+过滤；只有 `invalidateFilter`/`invalidateRowsFilter`/`invalidateColumnsFilter` 这几个细粒度的被弃了）
- 想偷懒也可以直接调 `invalidate()`（未弃用），但它比重算过滤更重——教学上选更精准的 `begin/endFilterChange`

### 检查点

切「全列扫描」瞬间表格按新范围刷新，编译器零 `deprecated` 警告 = Qt 6 通知范式用对了。

### 对照答案

- setFilterScope + begin/endFilterChange：`src/proxy-model.cpp:26-37`
- demo 控件驱动 setFilterScope：`demo/proxy-model_window.cpp:127-129`

---

全部 6 步搓完，回去对照 [成品导览](../) 的架构图和踩坑表，确认心智对齐。还有进阶玩法见 [手册首页 §3](./index.md#_3-进阶挑战-可选)。
