---
title: "Step 1-2：开标志位 + 拖出端序列化"
description: "开 QAbstractItemView 拖放四件套标志位让节点动起来，再重写 mimeTypes/mimeData 把被拖源节点用自定义 MIME + 指针编码序列化进 QMimeData。"
---

# Step 1-2：开标志位 + 拖出端序列化

← [手册首页](./index.md) · 下一步 [Step 3-4 协议落地 + dropEvent](./02-protocol-and-drop.md) →

## Step 1：开拖放标志位，节点能拖动

### 目标

鼠标按住一个节点能拖起来、光标变成拖拽态。**这步还不管落地**——能拖出来就行。

### 提示

在构造函数里开四个标志位（缺一个拖动就不完整）：

- `setDragEnabled(true)`——拖出总开关
- `setAcceptDrops(true)`——拖入总开关
- `setDragDropMode(QAbstractItemView::InternalMove)`——模式：内部移动
- `setDefaultDropAction(Qt::MoveAction)`——默认动作

另建议 `setSelectionMode(SingleSelection)`，拖拽时清晰追踪当前被拖节点。

### 检查点

按住节点拖，光标变成「拖拽中」形状 = 标志位通了。松手可能没反应（还没写 dropMimeData），正常，下一步处理。

> QAbstractItemView 拖放总开关不熟？先读 [拖放系统（入门）](../../../../../beginner/02-qtgui/06-drag-drop-beginner.md)。

### 对照答案

- 四件套标志位：`demo/tree-drag-move_window.cpp:40-43`
- 单选模式：`src/tree-drag-move.cpp:27`

---

## Step 2：重写 `mimeTypes` + `mimeData`（拖出端序列化）

### 目标

拖动开始时，框架回调你的 `mimeData(items)`，让你把被拖的源节点变成一个 `QMimeData`。这步定义「拖出去的数据包里装什么」。先 `mimeTypes()` 声明类型，再 `mimeData()` 编码内容。

### 提示

1. **定义自定义 MIME 类型**：声明一个静态 `const char* kMimeType = "application/x-awesomeqt-treedragmove"`（放在类里 `static const char* const kMimeType`）
2. **重写 `mimeTypes()`**：返回 `{QString::fromLatin1(kMimeType), QStringLiteral("text/plain")}`——自定义类型 + text/plain（后者给跨进程/日志留可读通道）
3. **重写 `mimeData(const QList<QTreeWidgetItem*>& items)`**：
   - items 为空直接返回 nullptr
   - 新建 `QByteArray payload` + `QDataStream stream(&payload, WriteOnly)`，`setVersion(Qt_6_0)`
   - 遍历 items，把每个指针 `reinterpret_cast<quintptr>` 写进流（进程内拖拽，指针零歧义定位源）
   - `new QMimeData`，`setData(kMimeType, payload)` + `setText(预览文本)`
   - 返回这个 QMimeData（调用方接管所有权）

### 关键认知

- **为什么用指针编码不用文本**：同名节点（两个「main.cpp」）靠文本分不清谁是谁。指针在进程内唯一，drop 端 `reinterpret_cast` 回来就是原节点。
- **为什么加 text/plain**：纯自定义类型跨进程/跨应用时对方不认，text/plain 至少能看到节点名。这是「双通道」思路。

### 检查点

编译过（重写虚函数签名要对：`QMimeData* mimeData(const QList<QTreeWidgetItem*>& items) const override`）。拖动时如果开了调试，能在 mimeData 里打断点看到 items 非空 = 拖出端通了。

> QMimeData / 序列化不熟？[拖放系统（入门）](../../../../../beginner/02-qtgui/06-drag-drop-beginner.md)、进阶 [拖放系统](../../../../../advanced/02-qtgui/06-drag-drop-advanced.md)。

### 对照答案

- kMimeType 常量定义：`src/tree-drag-move.cpp:24`
- mimeTypes 重写：`src/tree-drag-move.cpp:36-39`
- mimeData 编码（QDataStream + 指针）：`src/tree-drag-move.cpp:42-66`

---

下一步：[Step 3-4 把数据包落地 + dropEvent 删源](./02-protocol-and-drop.md)。
