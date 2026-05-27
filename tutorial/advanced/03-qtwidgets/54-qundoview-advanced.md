---
title: "3.54 QUndoView 进阶"
description: "入门篇我们用 QUndoView 配合 QUndoStack 展示了撤销/重做历史列表，掌握了 setStack/cleanChanged 信号以及基本的历史记录显示。"
---

# 现代Qt开发教程（进阶篇）3.54——QUndoView 进阶

## 1. 前言 / 当撤销重做不只是 Ctrl+Z

入门篇我们用 QUndoView 配合 QUndoStack 展示了撤销/重做历史列表，掌握了 setStack / cleanChanged 信号以及基本的历史记录显示。对于一个简单的"用户做了什么操作"的展示，入门篇够用了。但真正有价值的不是 QUndoView 的视觉效果——而是 QUndoStack + QUndoCommand 这套撤销重做框架的设计模式。QUndoView 只是框架的可视化窗口，核心是 QUndoStack 如何管理命令栈、QUndoCommand 如何实现命令的 do/undo、以及如何处理宏命令（多个操作合并为一个撤销单元）。

今天我们把 QUndoStack 完整的撤销重做系统拆透。核心内容是三个方面：QUndoCommand 的实现与嵌套、QUndoStack 的宏命令与命令合并、以及 QUndoGroup 管理多个 QUndoStack。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QUndoStack、QUndoCommand、QUndoView、QUndoGroup 都属于 QtGui 模块（Qt 6 中从 QtWidgets 移到了 QtGui），链接 Qt6::Gui 或 Qt6::Widgets 即可。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 QUndoCommand 的实现与嵌套

QUndoCommand 是撤销重做框架的核心抽象。每个用户操作封装为一个 QUndoCommand 子类，实现 redo()（执行操作）和 undo()（撤销操作）。redo() 在命令被推入 QUndoStack 时自动调用，undo() 在用户按 Ctrl+Z 时调用。

```cpp
class AddItemCommand : public QUndoCommand
{
public:
    AddItemCommand(QListWidget *list, const QString &text,
                   QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
        , m_list(list)
        , m_text(text)
        , m_row(-1)
    {
        setText(QString("添加项目: %1").arg(text));
    }

    void redo() override
    {
        m_row = m_list->count();
        m_list->addItem(m_text);
    }

    void undo() override
    {
        if (m_row >= 0 && m_row < m_list->count()) {
            delete m_list->takeItem(m_row);
        }
    }

private:
    QListWidget *m_list;
    QString m_text;
    int m_row;
};
```

setText() 设置命令的描述文字——这段文字会显示在 QUndoView 的历史列表中，也会出现在编辑菜单的"撤销 [操作描述]"中。好的命令描述应该简洁明确，比如"添加项目: 张三"而不是"修改"。

QUndoCommand 支持嵌套——构造函数接受 parent 参数。如果一个命令有子命令，redo() 和 undo() 会递归调用子命令的 redo() 和 undo()。子命令的执行顺序是：redo 时从前到后，undo 时从后到前。这在"同时修改多个属性"的场景下很有用——把每个属性的修改封装为子命令，父命令负责编排。

```cpp
class MoveItemCommand : public QUndoCommand
{
public:
    MoveItemCommand(int from, int to,
                   QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
    {
        setText(QString("移动项目 %1 → %2").arg(from).arg(to));
    }
    void redo() override { /* 执行移动 */ }
    void undo() override { /* 反向移动 */ }
};

class BatchMoveCommand : public QUndoCommand
{
public:
    BatchMoveCommand(QUndoCommand *parent = nullptr)
        : QUndoCommand("批量移动", parent)
    {
        // 添加子命令
        new MoveItemCommand(0, 3, this);
        new MoveItemCommand(1, 5, this);
        new MoveItemCommand(2, 1, this);
    }
    // 不需要重写 redo/undo——默认实现会遍历子命令
};
```

### 3.2 宏命令与命令合并

QUndoStack::beginMacro(const QString &text) 和 endMacro() 把 begin 和 end 之间的所有 push 操作合并为一个原子撤销单元。用户 Ctrl+Z 时一次撤销整个宏，而不是逐条撤销。

```cpp
undoStack->beginMacro("设置样式");
undoStack->push(new SetColorCommand(target, Qt::red));
undoStack->push(new SetFontCommand(target, QFont("Arial", 14)));
undoStack->push(new SetSizeCommand(target, 200, 100));
undoStack->endMacro();
```

用户按 Ctrl+Z 时，三个命令作为一个整体被撤销——字体、颜色、大小同时恢复。在 QUndoView 中这三个命令显示为一项"设置样式"。

宏命令在内部是通过创建一个父 QUndoCommand，然后把每个 push 的命令作为子命令来实现的。所以宏命令和手动嵌套 QUndoCommand 的效果一样，只是语法更方便。

命令合并是另一个强大的特性——QUndoCommand::id() 和 QUndoCommand::mergeWith(const QUndoCommand *other) 允许连续的相同类型命令自动合并。最经典的场景是拖动滑块——用户连续拖动产生了几十个"设值为 N"的命令，合并后只有一个"设值为最终值"。

```cpp
class SetValueCommand : public QUndoCommand
{
public:
    int id() const override { return kSetValueCommandId; }

    bool mergeWith(const QUndoCommand *other) override
    {
        if (other->id() != id()) return false;
        auto *cmd = static_cast<const SetValueCommand*>(other);
        if (cmd->m_target != m_target) return false;
        // 合并：用新命令的值替换当前值
        m_new_value = cmd->m_new_value;
        setText(QString("设值为 %1").arg(m_new_value));
        return true;
    }

    // ... redo/undo 使用 m_old_value 和 m_new_value
};
```

mergeWith 返回 true 表示 other 命令被合并到 this 中，other 不会被单独压入栈。QUndoStack 在 push 时检查栈顶命令是否可以与新命令合并——如果 canMerge 为 true 就调用 mergeWith，否则作为新命令压入。id() 返回非 -1 的值表示这个命令支持合并，返回 -1 表示不支持。

### 3.3 QUndoGroup 管理多个 QUndoStack

如果你的应用有多个独立的文档（比如多标签编辑器），每个文档有自己的撤销重做栈。QUndoGroup 把多个 QUndoStack 统一管理——当前活跃的栈决定 Ctrl+Z / Ctrl+Y 作用于哪个文档。

```cpp
auto *group = new QUndoGroup(this);

// 每个文档一个 QUndoStack
auto *doc1_stack = new QUndoStack(group);
auto *doc2_stack = new QUndoStack(group);
group->addStack(doc1_stack);
group->addStack(doc2_stack);

// 切换活跃栈
group->setActiveStack(doc1_stack);

// Ctrl+Z 撤销的是当前活跃栈
connect(undoAction, &QAction::triggered,
        group, &QUndoGroup::undo);
connect(redoAction, &QAction::triggered,
        group, &QUndoGroup::redo);
```

QUndoGroup 的 createUndoAction 和 createRedoAction 方法可以直接创建带快捷键和动态文字的 QAction——文字会自动更新为"撤销 [操作描述]"和"重做 [操作描述]"。

```cpp
auto *undo_action = group->createUndoAction(this, "撤销");
undo_action->setShortcut(QKeySequence::Undo);
menu->addAction(undo_action);

auto *redo_action = group->createRedoAction(this, "重做");
redo_action->setShortcut(QKeySequence::Redo);
menu->addAction(redo_action);
```

当活跃栈切换时，这些 QAction 的 enabled 状态和文字会自动更新——你不需要手动管理。这是 QUndoGroup 最方便的地方。

## 4. 踩坑预防

第一个坑是 undo/redo 中的操作不对称。redo() 中创建了一个 widget 但 undo() 中忘了 delete，或者 redo() 中修改了数据但 undo() 中恢复的值不对。后果是多次撤销重做后状态偏移——看起来"撤销了"但数据不对。建议在 redo 和 undo 中成对编写操作，写完一个立刻写另一个，确保每一步都有对称的反操作。

第二个坑是 mergeWith 中忘记检查目标是否一致。如果你的 SetValueCommand 可以作用于不同的目标（多个滑块），mergeWith 必须检查 other 的目标是否和 this 的目标相同。如果只检查 id 不检查 target，拖动滑块 A 的最终值可能被合并到滑块 B 的命令中。后果是撤销时滑块 B 的值被错误修改。

第三个坑是宏命令中混入不需要撤销的操作。beginMacro/endMacro 之间的所有 push 都会被合并。如果你在宏命令中间弹了一个 QMessageBox 或者做了 UI 更新（不是数据修改），这些操作也会被"包含"在宏中——撤销时不会恢复 UI 状态。解决方案是宏命令中只 push 数据修改命令，UI 更新在命令的 redo/undo 中通过信号触发。

## 5. 练习项目

练习项目：绘图工具的撤销重做系统。我们要为一个简单的绘图面板实现完整的撤销重做。

完成标准是：自定义 PaintWidget 用 QPainter 在 QPixmap 上绘图。支持三种操作：画直线（记录起点终点和颜色）、画矩形（记录左上角、宽高和颜色）、清除画布。每种操作封装为 QUndoCommand，redo() 执行绘制，undo() 恢复到操作前的 QPixmap 状态。QUndoStack 管理命令栈。QUndoView 显示在窗口右侧展示操作历史。连续画线操作使用 mergeWith 合并（一次拖动只算一步撤销）。Ctrl+Z 撤销、Ctrl+Y 重做，使用 createUndoAction/createRedoAction。

提示几个关键点：每个命令保存操作前的 QPixmap 副本用于 undo 恢复；mergeWith 只合并连续的相同类型命令；QUndoView::setStack 绑定栈即可显示历史。

## 6. 官方文档参考链接

[Qt 文档 · QUndoStack](https://doc.qt.io/qt-6/qundostack.html) -- 撤销栈，包含 push/beginMacro/endMacro/createUndoAction 等接口

[Qt 文档 · QUndoCommand](https://doc.qt.io/qt-6/qundocommand.html) -- 撤销命令基类，redo/undo/id/mergeWith

[Qt 文档 · QUndoView](https://doc.qt.io/qt-6/qundoview.html) -- 撤销历史视图

[Qt 文档 · QUndoGroup](https://doc.qt.io/qt-6/qundogroup.html) -- 撤销栈组，管理多个活跃栈

---

到这里，QUndoStack 的完整撤销重做系统就拆完了。QUndoCommand 是最小粒度的操作单元——redo 和 undo 必须严格对称。宏命令把多个操作合并为一个撤销单元，命令合并让连续同类操作不膨胀。QUndoGroup 统管多个栈，Ctrl+Z 自动作用于当前文档。这套框架设计得很优雅——一旦你把业务操作都封装为 QUndoCommand，撤销重做就几乎是免费的。
