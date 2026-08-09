---
title: "TreeDragMove 手搓手册"
description: "从能拖动的 QTreeWidget 一行行搓出协议全接管：开标志位、重写 mimeTypes/mimeData/dropMimeData、dropEvent 落点判定与删源、深拷贝子树。"
---

# TreeDragMove 手搓手册

> **source**：成品答案在 `model/01-mv-pattern/tree-drag-move/`（做完对照）· **related**：[拖放系统（入门）](../../../../../beginner/02-qtgui/06-drag-drop-beginner.md)、[QTreeWidget（入门）](../../../../../beginner/03-qtwidgets/48-qtreewidget-beginner.md)

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 TreeDragMove，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **QAbstractItemView 拖放四件套标志位**：`setDragEnabled` / `setAcceptDrops` / `setDragDropMode` / `setDefaultDropAction`——拖放的总开关
- **拖放协议四个虚函数**：`mimeTypes` / `mimeData` / `supportedDropActions` / `dropMimeData`——框架按固定顺序回调，接管它们就能自定义拖拽数据
- **自定义 MIME 类型 + QDataStream 序列化**：进程内用指针编码零歧义定位源节点
- **dropEvent 落点判定**：`itemAt(pos)` 区分「成为子节点」与「同级插入」
- **InternalMove 的删源时序**：协议层只复制、dropEvent 统一删源，避开 double-free

## 1. 起点

先有个能跑、能填数据的 QTreeWidget 空壳。继承 `QTreeWidget`：

```cpp
#include <QApplication>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class MyTree : public QTreeWidget {
  public:
    MyTree(QWidget* p = nullptr) : QTreeWidget(p) {
        setHeaderLabel("拖我");
        addTopLevelItem(new QTreeWidgetItem({"A"}));
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MyTree t;
    t.show();
    return app.exec();
}
```

跑出来看到一棵树、有数据 = 环境通了，往下走。QTreeWidget 不熟先看 [QTreeWidget（入门）](../../../../../beginner/03-qtwidgets/48-qtreewidget-beginner.md)。

## 2. 任务清单

分 6 步，每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 开拖放标志位，节点能拖动 | [01](./01-flags-and-drag.md) |
| 2 | 重写 `mimeTypes` + `mimeData`（拖出端序列化源） | [01](./01-flags-and-drag.md) |
| 3 | 重写 `supportedDropActions` + `dropMimeData`（拖入端落地） | [02](./02-protocol-and-drop.md) |
| 4 | `dropEvent` 落点判定 + 删源（InternalMove 收尾） | [02](./02-protocol-and-drop.md) |
| 5 | 深拷贝 `cloneItem`（带子节点的父节点一起搬） | [03](./03-clone-and-polish.md) |
| 6 | 选中态 / 展开态维护 + demo 双树 | [03](./03-clone-and-polish.md) |

成品对照：`model/01-mv-pattern/tree-drag-move/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **跨进程拖拽**：当前用指针编码（只在进程内有效）。真要跨进程，得把整棵子树（含数据）序列化成字节流，drop 端凭字节重建——想想需要哪些字段。
- **限定可拖节点**：某些节点（如根节点）不允许拖。提示：重写 `mimeData` 时按节点的 UserRole 标记过滤，或重写 `startDrag`。
- **撤销拖拽**：把每次移动记进 QUndoStack，拖完能 Ctrl+Z 回去。提示：参考本仓库 `model/11-command-pattern/undo-redo-framework/` 的 MoveCommand（命令模式），把「删源 + 插目标」包成一个 command push 进栈。
- **下一站**：QTreeView + 自定义模型（QAbstractItemModel）——那套要重写 `mimeData`/`dropMimeData` 在 model 层，比 widget 层更底层，但能支持百万级节点。
