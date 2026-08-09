---
title: "Step 3-4：自定义排序——数值列按值、文本列不区分大小写"
description: "重写 lessThan：数值列用 toDouble 比避免字典序陷阱，文本列显式 CaseInsensitive 覆盖基类默认。"
---

# Step 3-4：自定义排序——数值列按值、文本列不区分大小写

← [手册首页](./index.md) · 上一步 [Step 1-2 三层骨架](./01-three-layer-skeleton.md) · 下一步 [Step 5-6 自定义过滤](./03-custom-filter.md) →

## Step 3：重写 lessThan——数值列按值排

### 目标

点 Age 列表头排序时，按数值（`7 < 9 < 18 < 29 < 100`）排，而不是字典序（`"100" < "35" < "9"`，错得离谱）。

### 提示

- 重写 `protected bool lessThan(const QModelIndex& left, const QModelIndex& right) const override`
- 注意：`left`/`right` **已经是 source 索引**（基类翻译过了），直接拿来源用
- 加一个 `setNumericColumns(const QList<int>&)` 记录哪些列是数值列，存成员变量 `numeric_columns_`
- `lessThan` 里：若该列在数值集合，`left`/`right` 的文本 `toDouble()` 后比 `double`
- 要不要重新触发排序？`setNumericColumns` 改了集合后调 `invalidate()`（整个代理重算，包括排序和过滤）——注意 `invalidate()` 没被弃用，被弃用的只是 `invalidateFilter()`

### 检查点

按 Age 列升序，行序是 `7(Grace) → 9(david) → 18(Henry) → ... → 100(Frank)` = 数值排序对了。如果看到 `100` 排最前，说明还走字典序，没生效。

> QSortFilterProxyModel 的 lessThan 不熟？[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。

### 对照答案

- 数值分支 toDouble：`src/proxy-model.cpp:40-43`
- setNumericColumns + invalidate：`src/proxy-model.cpp:16-20`
- isNumericColumn 判定：`src/proxy-model.cpp:86-92`

---

## Step 4：文本列不区分大小写

### 目标

按 Name 列排序时，`"bob"` 排在 `"Charlie"` **之前**（按字母不管大小写），而不是基类默认的区分大小写（大写 ASCII 小，`"Alice" < "Charlie" < ... < "bob"`，`bob` 垫底）。

### 提示

- `lessThan` 的非数值分支：用 `QString::compare(a, b, Qt::CaseInsensitive) < 0`
- 基类的默认 `lessThan` 是 `QVariant::compare`，对字符串走区分大小写字典序——必须显式覆盖
- 也可以调 `setSortCaseSensitivity(Qt::CaseInsensitive)`，但本类选择在 `lessThan` 里写死，把「文本列一律不区分大小写」作为类的固定语义（教学上更清楚）

### 检查点

按 Name 列升序，`bob` 排在 `Charlie` 前面（`b` 和 `c` 比，不管大小写）= 大小写不敏感生效。

### 对照答案

- 文本分支 CaseInsensitive：`src/proxy-model.cpp:48`

---

下一步：[Step 5-6 重写 filterAcceptsRow 做自定义过滤](./03-custom-filter.md)。
