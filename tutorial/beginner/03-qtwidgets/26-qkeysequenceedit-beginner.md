# 现代Qt开发教程（新手篇）3.26——QKeySequenceEdit：快捷键录入控件

## 1. 前言 / 快捷键配置不能让用户去背按键码

我们在做桌面应用的时候，快捷键几乎是标配功能——Ctrl+S 保存、Ctrl+Z 撤销、Ctrl+C 复制，这些是所有桌面用户闭着眼都能按出来的肌肉记忆。但问题是，当你的应用功能越来越多、快捷键越来越密集的时候，不可避免地会出现快捷键冲突——两个功能绑了同一个组合键，或者某个快捷键和系统全局热键撞车了。解决这个问题的标准做法是允许用户自定义快捷键，让用户根据自己的使用习惯重新绑定。

现在问题来了：怎么让用户录入一个快捷键？你可能会想到用一个 QLineEdit 加上 `keyPressEvent`——但这条路走下去你会发现事情远没有想象的那么简单。用户按下的可能不是单个按键，而是一个组合键（Ctrl+Shift+A），你需要正确解析修饰键（Ctrl、Shift、Alt、Meta）和主键的组合；你需要处理按键释放事件来判断"录入完成"；你需要处理特殊情况比如单独按下 Ctrl 不算一个有效的快捷键。如果你自己从头实现这些逻辑，代码量不会少，而且容易出 bug。

Qt 提供了一个现成的控件来解决这个问题：QKeySequenceEdit。QKeySequenceEdit 是一个专门用于录入快捷键的小控件，它的外观就是一个普通的文本输入框——当它获得焦点时，用户按下任何按键组合，控件会自动识别并显示对应的快捷键文本（比如 "Ctrl+Shift+A"）。你不需要处理任何键盘事件，只需要监听它的 `keySequenceChanged` 信号就能拿到用户录入的 QKeySequence 对象。我们今天来把 QKeySequenceEdit 的四个核心用法讲清楚：快捷键录入的基本场景、keySequenceChanged 信号获取录入结果、setKeySequence 设置默认快捷键、以及和 QAction::setShortcut 结合的完整热键配置流程。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QKeySequenceEdit 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QKeySequenceEdit 操作的核心数据类型是 QKeySequence，QKeySequence 属于 QtGui 模块——但因为 Widgets 依赖 Gui，所以不需要额外链接。QKeySequence 可以表示最多包含 4 个按键组合的序列（比如 "Ctrl+X, Ctrl+C" 表示先按 Ctrl+X 再按 Ctrl+C），但 QKeySequenceEdit 在默认模式下只录入单个组合键（一个 QKeySequence 中只有一个按键组合）。如果你需要支持多步快捷键序列，需要自己组合多个 QKeySequenceEdit 或者进行额外的逻辑处理。

## 3. 核心概念讲解

### 3.1 用户自定义快捷键的录入场景

我们先来看 QKeySequenceEdit 的基本用法。创建一个 QKeySequenceEdit 实例，把它放到你的布局中，当控件获得焦点后用户按下任何按键组合，控件会自动显示对应的快捷键文本。它看起来就是一个带提示文字的输入框——提示文字是"Press shortcut"（或系统语言的对应翻译），用户按下组合键后文本会替换为按键序列的可读表示。

```cpp
auto *shortcutEdit = new QKeySequenceEdit();

// 用户在控件中按下 Ctrl+S，控件显示 "Ctrl+S"
// 用户按下 Ctrl+Shift+A，控件显示 "Ctrl+Shift+A"
```

QKeySequenceEdit 内部处理的键盘逻辑比你想象的要复杂。它会自动识别修饰键（Qt::ControlModifier、Qt::ShiftModifier、Qt::AltModifier、Qt::MetaModifier）和主键的组合，排除无效的单独修饰键输入。也就是说，如果用户只按下了 Ctrl 键而不按其他键，QKeySequenceEdit 不会认为这是一个有效的快捷键录入——它必须等到一个完整的主键按下才会确认录入。所谓"主键"就是字母键（A-Z）、数字键（0-9）、功能键（F1-F12）、特殊键（Space、Return、Tab 等），而 Ctrl、Shift、Alt、Meta 是"修饰键"，它们必须配合主键才能构成有效的快捷键。

QKeySequenceEdit 还会自动处理平台差异。在 Windows 和 Linux 上，Ctrl 键显示为 "Ctrl"；在 macOS 上，Command 键（Meta 键）显示为 Cmd 符号。这是通过 QKeySequence 的内部转换实现的——QKeySequence 的 `toString()` 方法会根据当前平台自动选择合适的修饰键显示名称。

在布局上，QKeySequenceEdit 通常和一个 QLabel 配对使用——Label 说明"这个快捷键是干什么的"，后面跟着 QKeySequenceEdit 用于录入。在设置页面中，你可能会看到一排这样的组合：

```
保存:     [Ctrl+S      ]
撤销:     [Ctrl+Z      ]
全选:     [Ctrl+A      ]
自定义操作: [            ]  ← 等待用户录入
```

这种布局的典型实现方式是用 QFormLayout——每个快捷键配置项是一个"标签 + 控件"的行：

```cpp
auto *formLayout = new QFormLayout();
formLayout->addRow("保存:", new QKeySequenceEdit());
formLayout->addRow("撤销:", new QKeySequenceEdit());
formLayout->addRow("全选:", new QKeySequenceEdit());
```

### 3.2 keySequenceChanged 信号获取录入结果

QKeySequenceEdit 最核心的信号是 `keySequenceChanged(const QKeySequence &)`。当用户在控件中完成一次快捷键录入后，这个信号会被触发，参数就是用户录入的 QKeySequence 对象。

```cpp
auto *shortcutEdit = new QKeySequenceEdit();

connect(shortcutEdit, &QKeySequenceEdit::keySequenceChanged,
        this, [](const QKeySequence &sequence) {
    if (sequence.isEmpty()) {
        qDebug() << "快捷键已清除";
    } else {
        qDebug() << "用户录入了快捷键:" << sequence.toString();
    }
});
```

`QKeySequence::isEmpty()` 可以判断快捷键是否为空——当用户按下了 Escape 键或 Backspace 键时，QKeySequenceEdit 会清除当前录入，把快捷键设为空序列。这其实是 QKeySequenceEdit 的一个内置行为：Escape 键用于取消当前录入，Backspace 键用于清除已录入的快捷键。用户可以通过这种方式"删除"一个已经设置的快捷键。

`QKeySequence::toString()` 返回快捷键的可读文本表示。默认情况下它使用 QKeySequence::NativeText 格式——这个格式会在 macOS 上显示 Cmd 符号，在 Windows/Linux 上显示 Ctrl。如果你需要一个跨平台一致的格式（比如存储到配置文件中），可以使用 QKeySequence::PortableText：

```cpp
QKeySequence seq("Ctrl+S");

// NativeText: "Ctrl+S" (Windows/Linux) 或 "Cmd+S" (macOS)
qDebug() << seq.toString(QKeySequence::NativeText);

// PortableText: 总是 "Ctrl+S"
qDebug() << seq.toString(QKeySequence::PortableText);
```

在存储快捷键配置时，建议使用 PortableText 格式——这样配置文件在不同平台之间是通用的。在显示给用户看时，使用 NativeText 或默认的 toString()（它等同于 NativeText）。

还有一个实用的方法是 `QKeySequence::count()`——它返回快捷键序列中包含的按键组合数量。单个快捷键（如 Ctrl+S）的 count() 返回 1，多步快捷键序列（如 Ctrl+K, Ctrl+C）的 count() 返回 2。在 QKeySequenceEdit 的默认模式下，count() 通常总是 1。

如果你需要判断用户录入的快捷键是否与某个已知的快捷键相同，直接用 `==` 比较即可：

```cpp
QKeySequence expected("Ctrl+S");
connect(shortcutEdit, &QKeySequenceEdit::keySequenceChanged,
        this, [expected](const QKeySequence &actual) {
    if (actual == expected) {
        qDebug() << "用户录入了预期的快捷键";
    }
});
```

### 3.3 setKeySequence() 设置默认快捷键

除了让用户从空白状态开始录入，你还经常需要给 QKeySequenceEdit 设置一个默认的快捷键——比如应用有推荐的默认快捷键方案，用户打开设置页面时应该看到当前的快捷键值。`setKeySequence(const QKeySequence &)` 就是做这件事的。

```cpp
auto *shortcutEdit = new QKeySequenceEdit();

// 设置默认快捷键为 Ctrl+S
shortcutEdit->setKeySequence(QKeySequence("Ctrl+S"));

// 或者用 Qt 内置的标准快捷键
shortcutEdit->setKeySequence(QKeySequence::Save);
// QKeySequence::Save 在不同平台上可能是 Ctrl+S 或 Cmd+S
```

QKeySequence 的构造函数接受多种参数形式。你可以用字符串构造：`QKeySequence("Ctrl+Shift+A")`，Qt 会自动解析字符串中的修饰键和主键。你也可以用 Qt::Key 枚举值加修饰键构造：`QKeySequence(Qt::CTRL | Qt::Key_S)`。第一种方式（字符串构造）可读性更好，第二种方式（枚举构造）类型安全性更强。

Qt 还提供了一组标准快捷键常量，定义在 QKeySequence::StandardKey 枚举中。这些常量会自动适配当前平台的快捷键约定：

```cpp
QKeySequence::Copy       // Ctrl+C (Win/Linux) 或 Cmd+C (macOS)
QKeySequence::Paste      // Ctrl+V 或 Cmd+V
QKeySequence::Cut        // Ctrl+X 或 Cmd+X
QKeySequence::Save       // Ctrl+S 或 Cmd+S
QKeySequence::Open       // Ctrl+O 或 Cmd+O
QKeySequence::Close      // Ctrl+W 或 Cmd+W
QKeySequence::Quit       // Ctrl+Q 或 Cmd+Q
QKeySequence::Undo       // Ctrl+Z 或 Cmd+Z
QKeySequence::Redo       // Ctrl+Y 或 Cmd+Shift+Z
QKeySequence::SelectAll  // Ctrl+A 或 Cmd+A
QKeySequence::Find       // Ctrl+F 或 Cmd+F
QKeySequence::Print      // Ctrl+P 或 Cmd+P
```

`keySequence()` 是对应的 getter 方法，返回当前 QKeySequenceEdit 中的快捷键。你可以用这个方法在设置页面关闭时收集所有快捷键配置。

有一个需要注意的行为：`setKeySequence()` 会触发 `keySequenceChanged` 信号。这意味着如果你在初始化界面时通过代码设置了默认快捷键，你的信号处理代码也会被执行。如果你不希望这种情况发生（通常你不需要在初始化时响应对默认值的"修改"），可以在初始化完成之前先不连接信号，或者用一个布尔标志位来区分"程序设置"和"用户操作"。

```cpp
// 方式 1: 初始化后再连接信号
shortcutEdit->setKeySequence(QKeySequence("Ctrl+S"));
// 初始化完成后...
connect(shortcutEdit, &QKeySequenceEdit::keySequenceChanged,
        this, &MyClass::onShortcutChanged);

// 方式 2: 用标志位过滤初始化设置
m_initializing = true;
shortcutEdit->setKeySequence(QKeySequence("Ctrl+S"));
m_initializing = false;

// 在槽函数中:
void onShortcutChanged(const QKeySequence &seq)
{
    if (m_initializing) return;
    // 处理用户操作
}
```

`clear()` 方法会清除当前的快捷键，效果等同于 `setKeySequence(QKeySequence())`。用户也可以通过在控件中按下 Escape 或 Backspace 键来清除快捷键。

### 3.4 与 QAction::setShortcut() 结合的完整热键配置流程

QKeySequenceEdit 的最终目的通常是为 QAction 设置快捷键。QAction 是 Qt 中"动作"的抽象——菜单项、工具栏按钮、快捷键都可以关联到同一个 QAction 上。`QAction::setShortcut(const QKeySequence &)` 为一个动作设置快捷键，当用户按下对应的按键组合时，QAction 的 `triggered()` 信号会被触发。

一个完整的"用户自定义快捷键并应用到 QAction"的流程是这样的：

首先，创建 QAction 并设置初始快捷键。然后在设置页面中为每个需要自定义快捷键的 QAction 创建一个 QKeySequenceEdit，用 `setKeySequence()` 设置为当前快捷键值。当用户在 QKeySequenceEdit 中录入新的快捷键后，通过 `keySequenceChanged` 信号获取新值，调用对应的 `QAction::setShortcut()` 应用更改。

```cpp
class ShortcutConfigPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ShortcutConfigPanel(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        // 创建动作
        m_saveAction = new QAction("保存", this);
        m_saveAction->setShortcut(QKeySequence::Save);
        connect(m_saveAction, &QAction::triggered,
                this, []() { qDebug() << "保存操作被触发"; });

        m_undoAction = new QAction("撤销", this);
        m_undoAction->setShortcut(QKeySequence::Undo);
        connect(m_undoAction, &QAction::triggered,
                this, []() { qDebug() << "撤销操作被触发"; });

        // 创建快捷键配置面板
        auto *layout = new QFormLayout(this);

        auto *saveEdit = new QKeySequenceEdit();
        saveEdit->setKeySequence(m_saveAction->shortcut());
        layout->addRow("保存:", saveEdit);

        auto *undoEdit = new QKeySequenceEdit();
        undoEdit->setKeySequence(m_undoAction->shortcut());
        layout->addRow("撤销:", undoEdit);

        // 连接信号: 快捷键变化时更新 QAction
        connect(saveEdit, &QKeySequenceEdit::keySequenceChanged,
                this, [this](const QKeySequence &seq) {
            m_saveAction->setShortcut(seq);
            qDebug() << "保存快捷键已更新为:" << seq.toString();
        });

        connect(undoEdit, &QKeySequenceEdit::keySequenceChanged,
                this, [this](const QKeySequence &seq) {
            m_undoAction->setShortcut(seq);
            qDebug() << "撤销快捷键已更新为:" << seq.toString();
        });
    }

private:
    QAction *m_saveAction = nullptr;
    QAction *m_undoAction = nullptr;
};
```

这里有一个非常重要的问题需要处理：快捷键冲突检测。如果用户把"保存"和"撤销"都设置为 Ctrl+S，那按下 Ctrl+S 时两个动作都会被触发——这显然不是用户期望的行为。所以在应用新的快捷键之前，你应该检查它是否与其他动作的快捷键冲突。

```cpp
/// @brief 检查快捷键是否与其他动作冲突
bool isShortcutConflict(const QKeySequence &seq,
                        const QList<QAction *> &actions,
                        QAction *exclude)
{
    if (seq.isEmpty()) return false;

    for (auto *action : actions) {
        if (action == exclude) continue;
        if (action->shortcut() == seq) {
            return true;
        }
    }
    return false;
}

// 在应用新快捷键之前检查冲突
connect(saveEdit, &QKeySequenceEdit::keySequenceChanged,
        this, [this](const QKeySequence &seq) {
    if (isShortcutConflict(seq, m_allActions, m_saveAction)) {
        QMessageBox::warning(this, "快捷键冲突",
            QString("快捷键 %1 已被其他操作使用，请选择其他快捷键。")
                .arg(seq.toString()));
        // 恢复原来的快捷键
        saveEdit->setKeySequence(m_saveAction->shortcut());
        return;
    }
    m_saveAction->setShortcut(seq);
});
```

`QAction::shortcuts()` 方法返回一个 `QList<QKeySequence>`——QAction 支持为一个动作设置多个快捷键。当你调用 `setShortcut()` 时会替换掉所有快捷键，只保留一个。如果你想添加额外的快捷键而不替换原有的，可以用 `setShortcuts(const QList<QKeySequence> &)` 传入一个列表，或者用 `addAction()` 的重载版本。在快捷键配置面板的场景中，通常只需要一个主快捷键，所以 `setShortcut()` 就够了。

最后提一下持久化的问题。用户自定义的快捷键配置需要保存到磁盘上，下次启动时恢复。保存时用 `QKeySequence::toString(QKeySequence::PortableText)` 转成字符串存入 QSettings 或 JSON 文件；恢复时用 `QKeySequence(storedString)` 从字符串构造 QKeySequence，然后调用 `setKeySequence()` 设置到 QKeySequenceEdit，同时调用 `setShortcut()` 应用到 QAction。

```cpp
// 保存
QSettings settings;
settings.setValue("shortcuts/save",
    m_saveAction->shortcut().toString(QKeySequence::PortableText));

// 恢复
QSettings settings;
QString saved = settings.value("shortcuts/save").toString();
if (!saved.isEmpty()) {
    m_saveAction->setShortcut(QKeySequence(saved));
    saveEdit->setKeySequence(QKeySequence(saved));
}
```

## 4. 踩坑预防

第一个坑是 `setKeySequence()` 会触发 `keySequenceChanged` 信号。如果你在初始化界面时通过代码设置了默认快捷键，你的信号处理代码也会被执行。可以在初始化完成后再连接信号，或者用标志位过滤。

第二个坑是单独按下修饰键不算有效快捷键。用户必须按下修饰键 + 主键的组合才算一次有效的录入。如果用户只按了 Ctrl 键然后松开，QKeySequenceEdit 不会发出 `keySequenceChanged` 信号。

第三个坑是快捷键冲突不会自动检测。多个 QAction 绑定同一个快捷键时，按键按下后所有匹配的动作都会被触发。你需要在应用新快捷键之前手动检查冲突。

第四个坑是 `QAction::shortcut()` 只返回主快捷键。如果你通过 `setShortcuts()` 设置了多个快捷键，`shortcut()` 只返回第一个。获取全部快捷键应该用 `shortcuts()`。

第五个坑是跨平台快捷键差异。Ctrl 键在 macOS 上通常对应 Cmd 键（Meta 键）。使用 QKeySequence::StandardKey 枚举（如 QKeySequence::Save）可以自动适配平台差异；直接使用字符串 "Ctrl+S" 在 macOS 上可能不符合用户的操作习惯。

## 5. 练习项目

我们来做一个综合练习：创建一个"快捷键配置面板"窗口，展示 QKeySequenceEdit 和 QAction 的完整协作流程。窗口包含一个菜单栏，菜单栏有一个"操作"菜单，里面有三个菜单项：新建（默认 Ctrl+N）、保存（默认 Ctrl+S）、关闭（默认 Ctrl+W）。窗口中央是一个 QFormLayout 构建的配置面板，每一行是一个快捷键配置项——左边是操作名称，右边是 QKeySequenceEdit。初始时每个 QKeySequenceEdit 显示对应 QAction 的当前快捷键。当用户修改某个 QKeySequenceEdit 的值时，实时更新对应 QAction 的快捷键。如果新快捷键与其他操作冲突，弹出 QMessageBox 警告并恢复原值。窗口底部有一个文本区域（QPlainTextEdit，只读），显示最近触发的操作名称和快捷键。每次用户通过快捷键或菜单触发某个 QAction 时，在文本区域追加一条记录。

几个提示：QAction 的 `triggered()` 信号连接到一个统一的日志槽函数，在槽函数中用 `sender()` 获取发送者并追加日志；冲突检测用一个辅助函数遍历所有 QAction 检查 shortcut 是否重复；QKeySequenceEdit 的 `keySequenceChanged` 信号中先检测冲突再决定是否应用。

## 6. 官方文档参考链接

[Qt 文档 · QKeySequenceEdit](https://doc.qt.io/qt-6/qkeysequenceedit.html) -- 快捷键录入控件

[Qt 文档 · QKeySequence](https://doc.qt.io/qt-6/qkeysequence.html) -- 快捷键数据类型

[Qt 文档 · QAction](https://doc.qt.io/qt-6/qaction.html) -- 动作抽象（菜单/工具栏/快捷键）

[Qt 文档 · QKeySequence::StandardKey](https://doc.qt.io/qt-6/qkeysequence.html#StandardKey-enum) -- 标准快捷键枚举

---

到这里，QKeySequenceEdit 的四个核心维度我们就全部讲完了。它是一个专门用于快捷键录入的控件，省去了手动处理键盘事件的麻烦。`keySequenceChanged` 信号在用户完成录入后触发，参数就是可以直接使用的 QKeySequence 对象。`setKeySequence()` 用于设置默认值或恢复之前的配置。和 QAction::setShortcut() 结合时，完整的流程是：从 QAction 获取当前快捷键设置到 QKeySequenceEdit，用户修改后通过信号获取新值，检测冲突后应用到 QAction。加上 QSettings 持久化，就构成了一个完整的用户自定义快捷键功能。
