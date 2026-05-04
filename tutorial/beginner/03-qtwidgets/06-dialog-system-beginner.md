# 现代Qt开发教程（新手篇）3.6——对话框体系基础

## 1. 前言 / 为什么对话框值得单独拿出来说

任何一个正经的桌面应用都离不开对话框——点"文件"弹出另存为对话框、点"设置"弹出配置面板、点"关于"弹出版权信息窗口。你可能觉得对话框不就是弹出一个窗口嘛，有什么好讲的？说实话我一开始也这么想，直到被模态对话框的事件循环阻塞问题坑了一整天，才意识到 Qt 的对话框体系远比它看起来要复杂。

对话框的特殊之处在于它有自己的"生命周期语义"。一个普通 QWidget 用 `show()` 弹出来，它和主窗口是平等的，用户可以随意在两者之间切换。但对话框不一样——模态对话框会"锁住"主窗口，用户必须先处理完对话框才能回到主窗口。这种行为不是靠 `setWindowModality` 一个属性就能搞定的，它的底层涉及到一个嵌套的事件循环，也就是 `QDialog::exec()` 所做的事情。理解了这个机制，你才能正确处理对话框中的数据传递、按钮响应、关闭逻辑，以及在复杂场景下避免死锁和重入问题。

这篇文章我们聚焦四个核心议题：`QDialog::exec()` 和 `QDialog::show()` 的模态与非模态行为差异、`QDialogButtonBox` 如何用标准按钮配置对话框底部的操作区域、自定义对话框如何组织布局并向调用方返回数据、以及 `accept()` / `reject()` / `done()` 三种关闭方式的语义区别。把这四件事搞清楚，Qt 对话框的基础你就掌握了。

## 2. 环境说明

本篇代码基于 Qt 6.5+，CMake 3.26+，C++17 标准。对话框体系属于 QtWidgets 模块，所以 CMake 里链接 Qt6::Widgets 就够了。所有代码在任何支持 Qt6 的桌面平台上都能正常运行，`QDialog::exec()` 在不同平台上的行为差异主要体现在窗口装饰和模态视觉反馈上（比如 macOS 会把主窗口灰化），但语义层面完全一致。

## 3. 核心概念讲解

### 3.1 模态对话框 exec() 与非模态 show() 的本质区别

先说模态对话框。`QDialog::exec()` 是 Qt 对话框体系中最核心的方法，它做了一件非常"暴力"的事情——在当前事件循环的内部又启动了一个新的事件循环。这意味着当 `exec()` 被调用后，调用方的代码会停在那里等待，直到对话框被关闭。在这段等待时间里，主窗口的事件处理是冻结的吗？不完全是。主窗口的绘制事件和定时器事件仍然会被处理（前提是模态范围是 `Qt::ApplicationModal` 以内），但用户的鼠标和键盘输入会被对话框拦截，无法传递到主窗口。这就是"模态"的含义——用户被"困"在对话框里，必须先完成对话框的操作才能继续。

```cpp
void MainWindow::onSettingsClicked()
{
    SettingsDialog dialog(this);
    dialog.setWindowTitle("设置");

    // exec() 会阻塞在这里，直到对话框关闭
    if (dialog.exec() == QDialog::Accepted) {
        // 用户点了确定，读取对话框中的数据
        QString name = dialog.getUserName();
        int level = dialog.getDifficultyLevel();
        applySettings(name, level);
    } else {
        // 用户点了取消或关闭了对话框
        qDebug() << "用户取消了设置";
    }
}
```

你会发现 `exec()` 的返回值是一个 `int`，对应 `QDialog::DialogCode` 枚举。`QDialog::Accepted`（值为 1）表示用户确认了操作，通常由 `accept()` 触发；`QDialog::Rejected`（值为 0）表示用户取消了操作，通常由 `reject()` 触发或者直接关闭了窗口。这种返回值机制让对话框的使用方式非常清晰——像调用一个函数一样，传参进去，拿返回值出来。

而非模态对话框用 `show()` 打开，行为和普通 QWidget 完全一样——对话框弹出来后，`show()` 立刻返回，调用方代码继续往下执行。用户可以在对话框和主窗口之间自由切换。非模态对话框适合那些不需要用户立刻处理的场景，比如一个"查找替换"面板、一个"输出日志"窗口、或者一个"实时监控"仪表板。

```cpp
void MainWindow::onFindClicked()
{
    if (m_findDialog == nullptr) {
        m_findDialog = new FindDialog(this);
        m_findDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_findDialog, &FindDialog::findNext,
                this, &MainWindow::findNextInEditor);
    }

    // show() 不会阻塞，立刻返回
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}
```

这里有一个容易踩的坑：非模态对话框通常需要在类里持有指针（比如上面的 `m_findDialog`），因为 `show()` 立刻返回了，你不能在栈上创建对话框对象——否则函数一结束对话框就被销毁了，用户连影子都看不到。另一个处理方式是设置 `WA_DeleteOnClose` 属性让对话框在关闭时自动销毁，但这时候你需要在每次访问指针前检查它是否为 `nullptr`，因为对话框可能已经被用户关掉了。

另外还有一个"窗口级模态"的概念，通过 `setWindowModality(Qt::WindowModal)` 设置。窗口级模态只阻塞对话框的父窗口及其祖先窗口，而不阻塞其他独立的顶级窗口。这在多窗口应用中非常有用——比如你有一个主窗口和一个独立的工具窗口，对话框只需要锁住主窗口而不影响工具窗口，那就用 `Qt::WindowModal`。默认的 `Qt::ApplicationModal` 会锁住整个应用的所有窗口。

### 3.2 QDialogButtonBox 标准按钮配置

如果你用过 Windows 或 macOS 的标准对话框，你会发现"确定""取消""应用""帮助"这些按钮的位置在不同平台上是固定的。macOS 上"确定"在右边、"取消"在左边；Windows 上"确定"在左边、"取消"在右边。如果你自己手动摆 QPushButton，就得自己处理平台差异，非常麻烦。`QDialogButtonBox` 就是 Qt 用来解决这个问题的一站式方案——你只需要告诉它"我需要哪些按钮"，它会自动按照当前平台的惯例排列这些按钮。

```cpp
auto *buttonBox = new QDialogButtonBox(
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel
);

// 将按钮盒的 accepted/rejected 信号连接到对话框的 accept/reject 槽
connect(buttonBox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
connect(buttonBox, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
```

这两行代码做的事情是：创建一个包含"确定"和"取消"按钮的按钮盒，然后把"确定"按钮的点击事件映射到对话框的 `accept()`，把"取消"按钮的点击事件映射到 `reject()`。`QDialogButtonBox` 会根据当前运行平台自动决定按钮的文字（比如 macOS 上"确定"显示为"OK"、某些 Linux 桌面上可能显示为"确认"）、按钮的排列顺序、以及按钮之间的间距。

`QDialogButtonBox::StandardButton` 枚举提供了大量标准按钮，常用的有 `Ok`、`Cancel`、`Apply`、`Close`、`Help`、`Reset`、`Save`、`Discard`、`Yes`、`No` 等。你可以用位或操作符组合多个按钮。

```cpp
// 一个包含"保存""不保存""取消"三按钮的确认对话框
auto *buttonBox = new QDialogButtonBox(
    QDialogButtonBox::Save |
    QDialogButtonBox::Discard |
    QDialogButtonBox::Cancel
);

connect(buttonBox->button(QDialogButtonBox::Save), &QPushButton::clicked,
        this, [this]() { doSave(); accept(); });
connect(buttonBox->button(QDialogButtonBox::Discard), &QPushButton::clicked,
        this, [this]() { reject(); });
connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
        this, [this]() { reject(); });
```

`button()` 方法可以通过标准按钮枚举值获取对应的 QPushButton 指针，这样你就能对单个按钮设置自定义行为。但大多数情况下你不需要这么做——`accepted` 和 `rejected` 两个聚合信号已经覆盖了最常见的使用场景。

如果你需要自定义按钮文字（比如"导入配置"而不是"打开"），可以用 `addButton(const QString &text, ButtonRole role)` 方法手动添加按钮。`ButtonRole` 决定了按钮在按钮盒中的位置和视觉权重——`AcceptRole` 类型的按钮会被放在确认区域（通常是右侧，macOS 上更靠右），`RejectRole` 放在取消区域，`ActionRole` 放在中间，`HelpRole` 放在最左边或最右边（取决于平台）。

### 3.3 自定义对话框布局与数据返回

实际开发中，你不可能只靠 Qt 内置的几种标准对话框就搞定所有需求。绝大多数场景需要你自己设计对话框的内部布局——一个用户登录对话框需要用户名输入框和密码输入框，一个新建项目对话框需要项目名称、路径选择和模板选择。自定义对话框的核心工作就是：设计内部布局、收集用户输入数据、在对话框关闭时把数据交还给调用方。

我们先看一个完整的自定义对话框示例——一个"新建用户"对话框，包含用户名、邮箱和角色选择。

```cpp
class NewUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewUserDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("新建用户");
        setMinimumWidth(360);

        auto *formLayout = new QFormLayout(this);

        m_nameEdit = new QLineEdit;
        m_nameEdit->setPlaceholderText("输入用户名");
        formLayout->addRow("用户名:", m_nameEdit);

        m_emailEdit = new QLineEdit;
        m_emailEdit->setPlaceholderText("user@example.com");
        formLayout->addRow("邮箱:", m_emailEdit);

        m_roleCombo = new QComboBox;
        m_roleCombo->addItems({"普通用户", "编辑者", "管理员"});
        formLayout->addRow("角色:", m_roleCombo);

        // 标准按钮盒
        auto *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel
        );
        formLayout->addRow(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted,
                this, &NewUserDialog::validateAndAccept);
        connect(buttonBox, &QDialogButtonBox::rejected,
                this, &QDialog::reject);
    }

    // 调用方通过这些 getter 获取数据
    QString userName() const { return m_nameEdit->text().trimmed(); }
    QString userEmail() const { return m_emailEdit->text().trimmed(); }
    int userRole() const { return m_roleCombo->currentIndex(); }

private slots:
    void validateAndAccept()
    {
        if (userName().isEmpty()) {
            m_nameEdit->setFocus();
            return;  // 输入不合法，不关闭对话框
        }
        accept();  // 输入合法，关闭对话框并返回 Accepted
    }

private:
    QLineEdit *m_nameEdit = nullptr;
    QLineEdit *m_emailEdit = nullptr;
    QComboBox *m_roleCombo = nullptr;
};
```

你会发现这个设计模式非常统一：构造函数里用布局管理器排好控件，用 getter 方法暴露数据，在 `accept()` 之前做输入验证。调用方的使用方式也很清晰——先用 `exec()` 弹出对话框，根据返回值判断用户是确认还是取消，然后通过 getter 方法读取数据。整个过程和调用一个普通函数没有任何区别，只不过这个"函数"有一个 GUI 界面。

这里有一个很实用的技巧：不要在"确定"按钮的 clicked 信号里直接调 `accept()`。你应该先把按钮盒的 `accepted` 信号连到一个自定义的槽函数（比如上面的 `validateAndAccept`），在槽函数里先做输入验证，只有验证通过才调 `accept()`。这样当用户输入了不合法的数据时，对话框不会被关掉，而是停在那里等用户修正。`reject()` 就不需要验证了——用户点"取消"就是想放弃，直接关掉就行。

关于数据返回，除了 getter 方法之外，还有一种做法是在对话框类中定义一个结构体，在 `accept()` 的时候把数据打包好，然后通过一个公共方法一次性返回。这种做法在对话框收集的字段比较多的时候更清晰，避免调用方需要调用一堆 getter。

```cpp
struct UserInfo
{
    QString name;
    QString email;
    int role;
};

class NewUserDialog : public QDialog
{
    // ...

public:
    std::optional<UserInfo> getResult() const
    {
        if (result() == QDialog::Accepted) {
            return UserInfo{userName(), userEmail(), userRole()};
        }
        return std::nullopt;
    }
};
```

### 3.4 accept() / reject() / done() 三种关闭方式

`accept()` 和 `reject()` 是 `QDialog` 提供的两个关闭方法，语义分别是"用户确认了操作"和"用户取消了操作"。它们内部都会调用 `done(int)` 方法——`accept()` 等价于 `done(QDialog::Accepted)`，`reject()` 等价于 `done(QDialog::Rejected)`。`done(int)` 做三件事：设置对话框的结果码（`setResult(code)`）、隐藏对话框（`hide()`）、退出嵌套事件循环（让 `exec()` 返回 `code`）。

绝大多数情况下你只需要用 `accept()` 和 `reject()`。但如果你需要返回自定义的结果码（比如"保存并退出"返回 2、"不保存退出"返回 3），可以直接调 `done(customCode)`。调用方在 `exec()` 返回后通过 `result()` 方法读取这个自定义码。

```cpp
// 自定义返回码
enum class SaveResult
{
    kSave = 10,
    kDiscard = 11,
    kCancel = 12
};

// 在按钮槽中
void onSaveClicked() { done(static_cast<int>(SaveResult::kSave)); }
void onDiscardClicked() { done(static_cast<int>(SaveResult::kDiscard)); }
void onCancelClicked() { done(static_cast<int>(SaveResult::kCancel)); }
```

有一个细节需要注意：当用户点击对话框右上角的关闭按钮（X）或者按 Escape 键时，`QDialog` 默认会调用 `reject()`。这意味着如果你重写了 `reject()` 或者 `closeEvent()`，这些操作都会被触发。如果你想在用户关闭对话框时做一些清理工作（比如停止正在进行的网络请求），在 `reject()` 或者 `closeEvent()` 中做就可以了。但如果你改了 Escape 键的默认行为（比如调了 `setEscapeButton`），就要确保逻辑上没有漏洞。

还有一个和 `exec()` 相关的坑：如果你的应用有多个顶级窗口，而且某个非模态对话框也在运行，当你在模态对话框里调用 `exec()` 再弹出一个二级模态对话框时，事件循环的嵌套层数会增加。Qt 支持任意深度的嵌套模态对话框，但每一层都会增加代码的复杂度。如果你的对话框层次超过两层，建议重新审视一下你的 UI 设计——也许你需要的是一个向导（`QWizard`）而不是多层嵌套的对话框。

## 4. 踩坑预防

第一个坑是在栈上创建非模态对话框。前面已经说过了，`show()` 是非阻塞的，函数返回后栈上的局部对象就会被销毁，对话框闪现一下就消失了。非模态对话框要么用 `new` 在堆上创建，要么作为类的成员变量。如果用 `new` 创建并设置了 `WA_DeleteOnClose`，记得在访问指针前检查 `nullptr`。

第二个坑是在模态对话框的事件循环中触发信号导致重入。假设你在对话框的某个槽函数中发出了一个信号，而这个信号连接到了主窗口的某个槽，主窗口的槽又尝试打开同一个对话框或操作同一个对话框的状态——这时候你可能遇到重入问题，因为 `exec()` 内部的事件循环会处理这些信号。避免这种问题的方法很简单：在模态对话框打开期间，不要让外部代码操作对话框的状态。

第三个坑是忘记连接 `QDialogButtonBox` 的信号。如果你创建了按钮盒但没有连接 `accepted` / `rejected` 信号到对话框的 `accept()` / `reject()` 槽，点按钮什么都不会发生——按钮盒只是负责展示按钮，关闭对话框的逻辑需要你自己接。

第四个坑是对话框的父窗口设置。如果你在创建对话框时没有传 `parent`（传了 `nullptr`），对话框会成为一个独立的顶级窗口，不会出现在父窗口的上方，也不会在父窗口关闭时自动关闭。对于模态对话框来说，你不传 parent 它仍然是模态的（会锁住整个应用），但视觉上它可能出现在屏幕的任意位置而不是居中在父窗口上方。所以创建对话框时一定要传 `this` 作为 parent。

## 5. 练习项目

我们来做一个综合练习：实现一个"个人信息编辑"对话框，包含姓名、年龄、邮箱、个人简介四个字段，以及一个确认/取消按钮盒。年龄用 `QSpinBox`，范围 1 到 150。邮箱需要基本的格式验证（包含 `@` 且 `@` 后面有至少一个 `.`）。个人简介用 `QTextEdit`，限制最多 200 个字符。

主窗口上放一个"编辑信息"按钮和一个 QLabel 用来展示当前信息。点击按钮弹出模态对话框，用户填写完成后点"确定"，主窗口的 QLabel 更新为对话框中收集到的数据。如果用户点"取消"或关闭对话框，什么都不做。对话框每次打开时应该预填当前已有的数据，而不是空白。

几个提示：对话框类提供一个 `setData(const QString &name, int age, const QString &email, const QString &bio)` 方法用于预填数据，以及一个 `getData()` 方法返回结构体；邮箱验证用 `QString::contains('@')` 加上对 `@` 后面内容的检查就行，不需要上正则；`QTextEdit` 的字符计数可以通过 `toPlainText().length()` 获取，在 `validateAndAccept()` 中判断长度是否超限；对话框的布局用 `QFormLayout`，按钮盒放在表单底部。

## 6. 官方文档参考链接

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- 对话框基类，包含 exec/show/accept/reject 的完整文档

[Qt 文档 · QDialogButtonBox](https://doc.qt.io/qt-6/qdialogbuttonbox.html) -- 标准按钮盒，包含标准按钮枚举和平台自适应排列

[Qt 文档 · QDialog::DialogCode](https://doc.qt.io/qt-6/qdialog.html#DialogCode-enum) -- Accepted/Rejected 返回码枚举

[Qt 文档 · QWidget::setWindowModality](https://doc.qt.io/qt-6/qt.html#WindowModality-enum) -- 窗口模态类型枚举（ApplicationModal / WindowModal）

[Qt 文档 · QFormLayout](https://doc.qt.io/qt-6/qformlayout.html) -- 表单布局，对话框中最常用的布局管理器

---

到这里，对话框体系的基础你就掌握了。`exec()` 阻塞等待返回值、`show()` 非阻塞自由交互、`QDialogButtonBox` 标准按钮自适应平台、getter 方法返回数据——这四件事组成了 Qt 对话框的核心骨架。下一篇我们进入 QMainWindow 主窗口体系，那是一套完全不同的能力：菜单栏、工具栏、状态栏、停靠面板，把一个"窗口"组装成一个完整的"应用框架"。
