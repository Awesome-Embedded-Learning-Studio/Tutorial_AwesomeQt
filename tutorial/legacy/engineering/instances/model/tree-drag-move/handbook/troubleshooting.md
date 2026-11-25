---
title: "卡住怎么办"
description: "按症状查：拖不动、节点重复、删源崩溃、子节点丢失、选中幽灵项、落点判定错——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `model/01-mv-pattern/tree-drag-move/`，对照着看。

## 拖不动 / 没反应

- **四个标志位都开了吗**？`setDragEnabled(true)`（拖出）+ `setAcceptDrops(true)`（拖入）+ `setDragDropMode(InternalMove)` + `setDefaultDropAction(MoveAction)`，缺一个拖动就不完整。→ `demo/tree-drag-move_window.cpp:40-43`
- `mimeTypes()` 有没有返回自定义类型？没返回框架不知道拖出去装什么。→ `src/tree-drag-move.cpp:36`
- 进阶排查：[拖放系统（入门）](../../../../../beginner/02-qtgui/06-drag-drop-beginner.md)

## 拖完节点出现两份（源没删）

- InternalMove 模式下，**dropMimeData 只复制没删源**，得在 `dropEvent` 的 MoveAction 分支手动删。→ `src/tree-drag-move.cpp:166-186`
- 删源时要分两路：`parent != nullptr` 用 `takeChild`，顶级用 `takeTopLevelItem`。→ `src/tree-drag-move.cpp:172-186`

## 删源时崩溃（segfault / double-free）

- **顺序对了吗**？铁律：备份源指针 → 调基类 `QTreeWidget::dropEvent`（触发 dropMimeData 复制）→ 再删源。先删源再调基类，基类取源时悬空崩。→ `src/tree-drag-move.cpp:147-186`
- **源指针是提前备份的吗**？不能在调基类 dropEvent 之后才取——基类可能删过一遍，取到的是悬空/无效。→ `src/tree-drag-move.cpp:147-161`
- `indexOfTopLevelItem` / `indexOfChild` 返回 -1 有没有判？删 -1 位置行为未定义。→ `src/tree-drag-move.cpp:175,182`
- 进阶排查：[QObject 与对象树](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)

## 拖带子节点的父节点，子节点丢一半

- clone 是浅的吗？只复制了顶层项的 text，没递归 child。→ 改用递归 `cloneItem`。→ `src/tree-drag-move.cpp:201-222`
- cloneItem 里 child 复制要 `clone->addChild(cloneItem(src->child(i)))`，不是只 new。→ `src/tree-drag-move.cpp:213-219`

## 拖完选中态停在已删除的源上（下次操作崩）

- QTreeWidget dropEvent 后默认选中没刷新，停在幽灵项。dropEvent 末尾手动 `setCurrentItem` 切到新落点。→ `src/tree-drag-move.cpp:190-198`
- 进阶排查：[QTreeWidget（进阶）](../../../../../advanced/03-qtwidgets/48-qtreewidget-advanced.md)

## 落在某节点上新子节点看不见

- 目标父节点是折叠的，新子项在折叠状态下不显示。落点是节点就 `target_parent->setExpanded(true)`。→ `src/tree-drag-move.cpp:194`

## 落点判定不准（该成子节点却成同级）

- 别纠结 `index` 的精确语义（容易踩坑）。用 `itemAt(event->position().toPoint())` 判：返回非 null = 落节点上（成子节点），null = 落空白（同级）。→ `src/tree-drag-move.cpp:141-142`

## 编译报错：重写的虚函数签名不对

- `mimeData` 是 `const` 方法：`QMimeData* mimeData(const QList<QTreeWidgetItem*>& items) const override;`，漏 const 不算 override。
- `dropMimeData` 参数顺序：`(QTreeWidgetItem* parent, int index, const QMimeData* data, Qt::DropAction action)`。
- 都加了 `override` 关键字吗？没 override 编译器不查签名对错，静默变成新虚函数。→ `include/tree-drag-move.h:44-65`

## moc 报错（Q_OBJECT / 自定义类型）

- 头文件有 `Q_OBJECT` 吗？继承 QTreeWidget 的子类要自己信号槽就得加。→ `include/tree-drag-move.h:34`
- CMake 开了 `AUTOMOC` 吗？自包含 CMake 里 `set(CMAKE_AUTOMOC ON)`。→ `model/01-mv-pattern/tree-drag-move/CMakeLists.txt:9`
