---
title: "Step 2-3：解析校验 + 树递归填充"
description: "QJsonDocument::fromJson + QJsonParseError 拿错误偏移报红字，成功后建虚拟根、fillValue 递归 switch 七种 QJsonValue::Type 填树。"
---

# Step 2-3：解析校验 + 树递归填充

← [手册首页](./index.md) · 上一步 [Step 1 分栏与装配](./01-layout-and-actions.md) · 下一步 [Step 4-5 格式化与读写](./03-format-and-fileio.md) →

## Step 2：Validate——解析 + 报错偏移 + 状态栏

### 目标

按 Ctrl+Enter 或点 Validate，编辑区文本被 `QJsonDocument::fromJson` 解析。**成功**：状态栏绿字 `OK`、右侧树填充（下一步）；**失败**：状态栏红字报 `错误类型 @byte offset 字节偏移`、树清空。

### 提示

- 抽一个 `parse(const QByteArray& bytes, QJsonDocument* out)` 函数：`QJsonParseError err; QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);`
- `err.error != QJsonParseError::NoError` 时：`showStatusError(QString("%1 @byte offset %2").arg(err.errorString()).arg(err.offset))`——offset 是从 bytes 起始算的**字节偏移**（含中文时与编辑器字符列不等，标 `@byte offset` 区分），定位语法错误就靠它，别只报 errorString
- 成功：`*out = doc; return true;`
- `onValidate`：`editor_->toPlainText().toUtf8()` 拿字节，parse 失败就 `tree_->clear(); return;`，成功就调 `populateTree(doc)`（Step 3 实现）+ `showStatusOk()`
- 状态栏两个函数：`showStatusOk()` 设 `setStyleSheet("color: green;")` + 文本 `"OK"`；`showStatusError(msg)` 红字 + `"Error: " + msg`
- **别挂 textChanged 实时校验**——大文件每次按键都解析会卡，Validate 是显式动作

### 检查点

编辑区故意写个非法 JSON（比如 `{"a": }`），按 Ctrl+Enter，状态栏红字给出错误类型 + offset；写合法 JSON，状态栏绿字 `OK` = 解析链通了。

> JSON 解析基础看 [JSON 与 XML 解析](../../../../../beginner/01-qtbase/16-json-xml-beginner.md)，进阶看 [JSON 进阶](../../../../../advanced/01-qtbase/16-json-xml-advanced.md)。状态栏看 [状态栏](../../../../../beginner/03-qtwidgets/58-qstatusbar-beginner.md)。

### 对照答案

- parse 函数（fromJson + QJsonParseError + offset 报错）：`demo/json_editor_window.cpp:146-157`
- onValidate（拿字节 → parse → 清树/填充 → 状态栏）：`demo/json_editor_window.cpp:162-171`
- showStatusOk / showStatusError：`demo/json_editor_window.cpp:346-354`

---

## Step 3：树递归填充——虚拟根 + fillValue

### 目标

解析成功后，右侧树把整个 JSON 铺开：顶层一个 `(root)` 虚拟节点（type 标 object/array/null），object 的键、array 的 `[i]` 索引、scalar 值都递归挂上去，`(root)` 默认展开。

### 提示

- `populateTree(doc)` 先 `tree_->clear()`
- doc.isObject()：建 `new QTreeWidgetItem({"(root)", "", "object"})` 当 `addTopLevelItem`，遍历 `doc.object()` 的 `begin()/end()` 迭代器，每个 `{key, value}` 调 `fillValue(root, key, value)`
- doc.isArray()：同理，根标 `"array"`，遍历 `doc.array()`，元素用 `QString("[%1]").arg(idx)` 当 key 列
- 既不是 object 也不是 array（空文档/单 scalar）：根标 `"null"`，value 列写 `"(empty)"`
- 根 `setExpanded(true)`
- `fillValue(parent, key, value)` 是核心递归——`switch (value.type())` 七种：
  - **Object**：建子节点（type `"object"`）、`addChild`、再遍历它的 keys 递归 `fillValue`
  - **Array**：同理，子节点 type `"array"`，元素用 `[i]` 递归
  - **Bool**：`new QTreeWidgetItem(parent, {key, value.toBool()?"true":"false", "bool"})`
  - **Double**：**重点**——Qt JSON 数字统一是 double，整数判定用精确比较 `std::isfinite(d) && d == std::floor(d)`（`qFuzzyCompare` 对 \|d\|>=1e12 量级会失效，带小数的大数被误判整数），是整数转 `qint64` 再 `QString::number`（否则整数会显示 `1.0`），否则 `QString::number(d)`
  - **String**：`{key, value.toString(), "string"}`
  - **Null**：`{key, "null", "null"}`
  - **default**：`{key, "(unknown)", "unknown"}`
- 注意 Object/Array 分支里要 `break`，别穿透；new QTreeWidgetItem 时**Object/Array 要先建 node 再递归**（node 当 parent），scalar 直接 `new QTreeWidgetItem(parent, ...)` 挂到当前 parent

### 检查点

编辑区贴一段含嵌套 object、array、中文、布尔、null 的 JSON，Validate 后树铺开成多层、`(root)` 展开、array 元素 key 列显示 `[0]/[1]`、整数显示 `1` 不带 `.0`、`6.11` 显示 `6.11` = 递归链通了。

> QJsonValue 七种类型看官方 [QJsonValue::Type](https://doc.qt.io/qt-6/qjsonvalue.html#Type-enum)，QTreeWidget 递归填充看 [QTreeWidget](../../../../../beginner/03-qtwidgets/48-qtreewidget-beginner.md)。

### 对照答案

- populateTree（虚拟根 + object/array/null 三分支）：`demo/json_editor_window.cpp:176-203`
- fillValue（switch 七种类型 + Double 整数判定）：`demo/json_editor_window.cpp:206-261`
- array 元素用 `[i]` 当 key：`demo/json_editor_window.cpp:194`、`:224`
- Double 整数分支（isfinite + d==floor(d) + qint64 范围检查）：`demo/json_editor_window.cpp:233-242`

---

下一步：[Step 4-5 格式化/紧凑 + 文件读写](./03-format-and-fileio.md)。
