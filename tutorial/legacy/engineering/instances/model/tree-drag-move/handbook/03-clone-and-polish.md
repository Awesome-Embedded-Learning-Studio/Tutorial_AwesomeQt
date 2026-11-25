---
title: "Step 5-6：深拷贝子树 + 状态维护"
description: "cloneItem 递归深拷贝让带子节点的父节点整体搬家，dropEvent 末尾维护选中态与展开态（展开目标父节点、选中切到新落点），并接 demo 双树演示。"
---

# Step 5-6：深拷贝子树 + 选中/展开态维护

← [Step 3-4](./02-protocol-and-drop.md) · [手册首页](./index.md) →

Step 3-4 跑通了，但拖带子节点的父节点会发现子节点丢一半——因为 Step 3 的 clone 只复制了顶层项。这步补深拷贝，再收尾选中/展开态。

## Step 5：深拷贝 `cloneItem`（带子节点的父节点一起搬）

### 目标

写一个私有 `cloneItem(const QTreeWidgetItem* src)`，递归复制整棵子树（text/icon/toolTip/UserRole/checkState + 所有 child），替换 Step 3 里那个浅 clone。

### 提示

1. **私有方法签名**：`QTreeWidgetItem* cloneItem(const QTreeWidgetItem* src) const;`（src 只读）
2. src 为 null 直接返回 nullptr
3. `auto* clone = new QTreeWidgetItem;`
4. 遍历每列复制：`setText(c, src->text(c))` / `setIcon` / `setToolTip` / `setData(c, Qt::UserRole, src->data(c, Qt::UserRole))`
5. `clone->setCheckState(0, src->checkState(0))`
6. **递归 child**：`for (int i = 0; i < src->childCount(); ++i)` → `clone->addChild(cloneItem(src->child(i)))`
7. 回 dropMimeData，把 Step 3 的 `new QTreeWidgetItem({src->text(0)})` 换成 `cloneItem(src)`

### 关键认知

- **MoveAction 下源连同子树一起删**（Step 4 的 takeChild/takeTopLevelItem 删的是顶层源，它的 child 由 Qt 对象树跟着析构）。所以只要克隆完整，源端不需要单独处理 child。

### 检查点

拖一个有 2-3 层子节点的父节点到别处，**整棵子树完整出现在新位置、原位置整棵消失** = 深拷贝对了。

### 对照答案

- cloneItem 递归深拷贝：`src/tree-drag-move.cpp:201-222`
- dropMimeData 调 cloneItem：`src/tree-drag-move.cpp:108`

---

## Step 6：选中态 / 展开态维护 + demo 双树

### 目标

拖完两件事别忘：①落在节点上时自动**展开**目标父节点（否则新子节点看不见，用户以为没拖成功）；②把**选中态**从已删除的源切到新落点（否则选中停在幽灵项上，下次操作崩）。

### 提示（在 dropEvent 末尾，删源之后）

1. 判断 `drop_on_item`（Step 4 存的 target_parent 是否非 null）
2. `if (drop_on_item && target_parent)`：`target_parent->setExpanded(true);` + `setCurrentItem(target_parent);`
3. `else if (topLevelItemCount() > 0)`：`setCurrentItem(topLevelItem(topLevelItemCount() - 1));`（落空白，选最后一个顶级项）

### demo 双树（验证跨树）

1. demo 里建两棵 TreeDragMove，左右各一
2. 两棵都开 `setDragEnabled/setAcceptDrops/setDragDropMode(InternalMove)`
3. 种子数据：左树放「项目结构」（src/tests/docs 多层），右树放「回收区」
4. 左拖右 = 跨树转移（节点从左消失、出现在右）

### 检查点

拖完落点是折叠节点 → 节点自动展开，新子项可见 = 展开态对了。拖完后选中态停在源（已删）→ 下次键盘操作崩 = 没维护好，回头查 setCurrentItem。左树节点拖到右树、左树消失右树有 = 跨树通了。

### 对照答案

- 展开目标父节点 + 切选中：`src/tree-drag-move.cpp:190-198`
- demo 双树 + 标志位：`demo/tree-drag-move_window.cpp:31-60`
- 种子数据：`demo/tree-drag-move_window.cpp:87-110`

---

搓完基础版，想再深一层去看 [手册首页的进阶挑战](./index.md#_3-进阶挑战-可选)：跨进程序列化、限定可拖节点、拖拽撤销、或转 QTreeView+自定义模型。卡住了翻 [troubleshooting](./troubleshooting.md)。
