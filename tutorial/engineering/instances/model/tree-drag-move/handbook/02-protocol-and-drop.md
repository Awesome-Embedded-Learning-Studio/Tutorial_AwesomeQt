---
title: "Step 3-4：拖入端落地 + dropEvent 删源"
description: "重写 supportedDropActions 收窄成 MoveAction、dropMimeData 反序列化并把节点插到目标位置，再写 dropEvent 做落点判定与 InternalMove 下的删源——整件最核心的时序逻辑。"
---

# Step 3-4：拖入端落地 + dropEvent 删源（核心）

← [Step 1-2](./01-flags-and-drag.md) · [手册首页](./index.md) · 下一步 [Step 5-6 深拷贝 + 状态维护](./03-clone-and-polish.md) →

这两步是整个控件的核心——把 Step 2 拖出去的数据包**接住、落地、删源**。难点不在单个虚函数，而在**时序**：哪个方法删源、何时删，搞错就崩溃。

## Step 3：重写 `supportedDropActions` + `dropMimeData`（拖入端）

### 目标

框架松手时回调 `dropMimeData(parent, index, data, action)`，让你把 Step 2 编码的源节点**反序列化、插到 (parent, index) 处**。`supportedDropActions` 先声明本树接受什么动作。

### 提示

1. **重写 `supportedDropActions()`**：`return Qt::MoveAction;`——收窄成纯移动（默认会返回 Move|Copy，导致行为飘）
2. **重写 `dropMimeData(QTreeWidgetItem* parent, int index, const QMimeData* data, Qt::DropAction action)`**：
   - 先判 `data == nullptr || !data->hasFormat(kMimeType)` → return false
   - action 不是 Move/Copy → return false
   - `data->data(kMimeType)` 取回 payload，`QDataStream(ReadOnly)` 读
   - 决定容器：`parent == nullptr` → 顶级，用 `insertTopLevelItem`；否则 `parent->insertChild(index, clone)`
   - index 越界兜底 `if (index < 0) index = 0;`，每插一个 `++index` 顺延
   - **这步只克隆插入，不要删源**（删源放 Step 4，见下「关键认知」）
3. **暂时用一个简单 clone**（复制 text 即可，完整深拷贝留 Step 5）：`auto* clone = new QTreeWidgetItem({src->text(0)});`

### 关键认知

- **为什么 dropMimeData 不删源**：InternalMove 模式下，基类 `QTreeWidget::dropEvent` 内部可能也会删源（默认实现）。如果你在这里删，又调基类，就 double-free。所以**协议层只复制**，删源集中到 Step 4 的 dropEvent，顺序锁死。

### 检查点

拖一个节点到另一个节点上，目标位置出现新节点（克隆出来的）= dropMimeData 通了。**此时源还在（没删）**——这是预期的，Step 4 删。

> 落点 / index 语义不熟？进阶 [QTreeWidget](../../../../../advanced/03-qtwidgets/48-qtreewidget-advanced.md)、[QAbstractItemView](../../../../../advanced/03-qtwidgets/15-qabstractitemview-base-advanced.md)。

### 对照答案

- supportedDropActions 收窄：`src/tree-drag-move.cpp:31-34`
- dropMimeData 反序列化 + 插入（不删源）：`src/tree-drag-move.cpp:69-119`

---

## Step 4：`dropEvent` 落点判定 + 删源（InternalMove 收尾）

### 目标

dropMimeData 只复制了，源还在（两份）。这步在 `dropEvent` 里做三件事：**判落点、删源、维护状态**。这是整件最容易崩的地方——时序错了就 segfault。

### 提示（按顺序）

1. **重写 `dropEvent(QDropEvent* event)`**
2. **先判 data 格式**：`event->mimeData()` 没有自定义格式 → `QTreeWidget::dropEvent(event); return;`（交给基类处理非本树拖入）
3. **判落点**：`QTreeWidgetItem* target_parent = itemAt(event->position().toPoint());`——非 null = 落节点上（成子节点），null = 落空白（同级）
4. **提前备份源指针**：从 `event->mimeData()->data(kMimeType)` 反解出所有源指针存进 `QList`。**必须在调基类 dropEvent 之前取**——基类可能删源导致后面取不到
5. **调基类 `QTreeWidget::dropEvent(event)`**——这会触发你重写的 dropMimeData 完成复制
6. **MoveAction 下删源**：遍历备份的源指针，分两路：
   - `src->parent() != nullptr` → `p->takeChild(p->indexOfChild(src))` + `delete`
   - 顶级 → `takeTopLevelItem(indexOfTopLevelItem(src))` + `delete`
   - 每个 indexOf/take 前判 `i >= 0`，防 -1

### 关键认知

- **顺序铁律：备份源 → 调基类（复制）→ 删源**。调换任何一步都可能崩：
  - 先删源再调基类：基类 dropMimeData 取源时已悬空 → segfault
  - 不备份直接删：基类删过一遍你删第二遍 → double-free
- **为什么 `takeChild` + `delete` 两步**：`takeChild` 只是从树里摘下（所有权交还调用方），不释放内存；`delete` 才真正释放。MoveAction 的「移动」= 源真正消失。

### 检查点

拖一个节点到另一个节点上，源位置消失、目标位置有 = 删源时序对了。拖带子节点的父节点（这步 clone 还是浅的）——**子节点可能丢**，正常，Step 5 补深拷贝。反复拖不崩 = 时序稳。

> 时序崩溃排查见 [troubleshooting 的「删源崩溃」](./troubleshooting.md)。

### 对照答案

- dropEvent 判落点：`src/tree-drag-move.cpp:144`
- 备份源指针：`src/tree-drag-move.cpp:147-161`
- 调基类完成复制：`src/tree-drag-move.cpp:163-164`
- MoveAction 删源（take+delete）：`src/tree-drag-move.cpp:166-186`

---

下一步：[Step 5-6 深拷贝子树 + 选中/展开态维护](./03-clone-and-polish.md)。
