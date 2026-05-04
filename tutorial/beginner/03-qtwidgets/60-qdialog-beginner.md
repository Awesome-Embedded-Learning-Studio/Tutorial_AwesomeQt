# 现代Qt开发教程（新手篇）3.60——QDialog：自定义对话框

## 1. 前言 / 对话框是桌面应用交互的基本单元

任何桌面应用都离不开对话框。当用户点击"新建"时弹出一个配置窗口让用户填写项目名称和路径，当用户点击"设置"时弹出一个面板让用户调整各项参数，当操作需要二次确认时弹出一个窗口问用户"确定要删除吗"——这些弹出来的独立窗口就是对话框。它们和 QMainWindow 的区别在于：对话框是临时的、有明确目标的、用完即关的交互单元，而主窗口是持久的、承载核心工作流的容器。

Qt 中所有对话框的基类是 QDialog。它继承自 QWidget，在窗口系统层面就是一个独立的顶层窗口，但 QDialog 在此基础上封装了一套专门服务于"对话式交互"的机制：模态阻塞、返回值约定、接受/拒绝语义。理解了这套机制，你就掌握了所有 Qt 弹窗交互的基础——因为 QMessageBox、QInputDialog、QFileDialog 这些常用对话框全部继承自 QDialog。

今天我们从四个方面展开。先看模态 exec 与非模态 show 的区别和各自的适用场景，然后研究 accept / reject / done 的返回值约定以及如何从对话框中把用户输入的数据带出来，接着讨论 setModal 和 setWindowModality 在控制模态范围上的差异。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QDialog 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QDialog、QLabel、QLineEdit、QSpinBox、QComboBox、QPushButton、QVBoxLayout、QFormLayout、QDialogButtonBox 和 QApplication。

## 3. 核心概念讲解

### 3.1 模态 exec vs 非模态 show

QDialog 有两种展示方式，分别对应两种截然不同的交互模式。调用 exec() 会打开一个模态对话框——所谓"模态"，就是这个对话框打开后，它会阻塞调用 exec() 的那行代码，用户无法操作同一应用中的其他窗口，直到对话框被关闭。exec() 的返回值是 int 类型，告诉你用户是点了"确定"还是"取消"。这适用于那些"必须先回答我才能继续"的场景——比如保存文件时弹出的"是否保存更改？"，用户必须做出选择才能继续后续流程。

```cpp
auto *dialog = new QDialog(this);
// ... 配置对话框的布局和控件 ...

int result = dialog->exec();
if (result == QDialog::Accepted) {
    // 用户点击了"确定"
} else {
    // 用户点击了"取消"或关闭了窗口
}
```

调用 show() 则会打开一个非模态对话框。show() 立刻返回，不会阻塞调用者，用户可以同时操作主窗口和对话框。这适用于那些"辅助工具窗口"的场景——比如查找/替换对话框，用户可能一边在编辑器里改代码一边在查找对话框里输入搜索词，两边来回切换。

```cpp
auto *dialog = new QDialog(this);
// ... 配置对话框 ...

dialog->show();
// show() 立刻返回，主窗口继续响应
// 对话框和主窗口可以同时操作
```

这两种模式的选择逻辑其实很直觉：如果用户的回答是后续操作的必要前提（比如"你确定要删除这个文件吗？"），用模态；如果对话框只是一个辅助工具（比如"查找"面板、"属性"面板），用非模态。大多数情况下我们用模态对话框，因为大多数弹窗交互的本质就是"问一个问题，等用户回答，然后根据回答决定下一步"。

这里有一个常见的误解需要澄清：exec() 的"阻塞"并不是忙等待。exec() 内部会启动一个局部的 QEventLoop——也就是说，对话框自己的事件循环仍然在跑，对话框内部的所有交互（点击按钮、输入文字、调整大小）都正常工作，只是同一个应用中其他窗口的事件被过滤掉了（对于 ApplicationModal 而言）。所以不要担心 exec() 会冻结 UI。

另外一个需要留意的细节是 exec() 和 show() 在对话框生命周期管理上的差异。exec() 返回后对话框仍然是存在的（只是被隐藏了），你可以继续从对话框中读取数据，也可以再次调用 exec() 让它重新弹出。show() 则是"弹出就不管了"，你需要自己处理对话框的关闭和销毁——通常的做法是给对话框设置 WA_DeleteOnClose 属性，让它在关闭时自动 delete。

```cpp
// 模态：exec() 返回后手动 delete
auto *dialog = new QDialog(this);
int result = dialog->exec();
// 读取数据...
dialog->deleteLater();

// 非模态：设置自动删除
auto *dialog = new QDialog(this);
dialog->setAttribute(Qt::WA_DeleteOnClose);
dialog->show();
// show() 后不要再用 dialog 指针——它可能在任何时候被 delete
```

如果你在 exec() 和 show() 之间犹豫不决，问自己一个问题：用户对这个对话框的操作结果是下一步操作的必要输入吗？如果是，exec()；如果不是，show()。这个判断标准在绝大多数场景下都适用。

### 3.2 accept / reject / done 的返回值约定

QDialog 的关闭方式不是简单的"关闭窗口"，而是有一套语义化的返回值机制。当你调用 accept() 时，对话框以"接受"状态关闭，exec() 返回 QDialog::Accepted（值为 1）。当你调用 reject() 时，对话框以"拒绝"状态关闭，exec() 返回 QDialog::Rejected（值为 0）。用户点击窗口标题栏的关闭按钮（X）时，QDialog 默认调用 reject()，所以 exec() 会返回 Rejected。

```cpp
class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        auto *layout = new QVBoxLayout(this);
        // ... 添加用户名、密码输入框 ...

        auto *okBtn = new QPushButton("登录");
        auto *cancelBtn = new QPushButton("取消");

        connect(okBtn, &QPushButton::clicked,
                this, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked,
                this, &QDialog::reject);

        // ... 把按钮加入布局 ...
    }
};

// 使用
LoginDialog dlg(this);
if (dlg.exec() == QDialog::Accepted) {
    // 用户点击了"登录"
} else {
    // 用户点击了"取消"或关闭了窗口
}
```

accept() 和 reject() 都会关闭对话框并让 exec() 返回，但它们的语义不同。accept() 表示"用户确认了对话框中的操作"——可以理解为"OK"、"确定"、"保存"、"是"。reject() 表示"用户取消了对话框中的操作"——可以理解为"Cancel"、"取消"、"否"、"关闭"。

如果你需要更细粒度的返回值——比如对话框有多个按钮，每个按钮对应不同的操作——可以使用 done(int result)。done() 接受一个 int 参数，它会关闭对话框并让 exec() 返回这个值。你可以自定义返回值常量来区分不同的退出路径。

```cpp
class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    enum Result {
        kExportPdf = 10,
        kExportHtml = 11,
        kExportMarkdown = 12
    };

    explicit ExportDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        // ...
        auto *pdfBtn = new QPushButton("导出 PDF");
        auto *htmlBtn = new QPushButton("导出 HTML");
        auto *mdBtn = new QPushButton("导出 Markdown");
        auto *cancelBtn = new QPushButton("取消");

        connect(pdfBtn, &QPushButton::clicked,
                this, [this]() { done(kExportPdf); });
        connect(htmlBtn, &QPushButton::clicked,
                this, [this]() { done(kExportHtml); });
        connect(mdBtn, &QPushButton::clicked,
                this, [this]() { done(kExportMarkdown); });
        connect(cancelBtn, &QPushButton::clicked,
                this, &QDialog::reject);
    }
};

// 使用
ExportDialog dlg(this);
int result = dlg.exec();
switch (result) {
    case ExportDialog::kExportPdf:
        exportAsPdf(); break;
    case ExportDialog::kExportHtml:
        exportAsHtml(); break;
    case ExportDialog::kExportMarkdown:
        exportAsMarkdown(); break;
    default:
        break;  // 用户取消
}
```

done() 比 accept() / reject() 更灵活，但也会增加复杂度。如果你的对话框只有"确定"和"取消"两个出口，用 accept() / reject() 就够了。只有在确实需要区分多种退出路径时才引入自定义的 done() 返回值——不要为了"灵活性"而过度设计。

还有一个重要的细节：accept() 实际上就是 done(Accepted)，reject() 实际上就是 done(Rejected)。它们是 done() 的语义化封装。所以你也可以统一用 done() 来处理所有关闭逻辑，用 Accepted / Rejected 以及自定义值来区分不同的退出路径。

### 3.3 从对话框返回用户输入数据

模态对话框最典型的用法是：弹出来让用户填一些信息，用户点"确定"后，调用者把用户填写的信息取出来继续处理。这个"从对话框中取出数据"的模式在 Qt 中有几种实现方式，我们逐一来看。

最直接的做法是在对话框类中暴露 getter 方法。对话框的控件作为私有成员，通过 const getter 方法让外部读取用户输入的值。这种方式的好处是简单、类型安全，调用者不需要知道对话框内部用了什么控件。

```cpp
class UserConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserConfigDialog(QWidget *parent = nullptr);

    /// @brief 获取用户名
    QString username() const { return m_nameEdit->text(); }

    /// @brief 获取年龄
    int age() const { return m_ageSpin->value(); }

    /// @brief 获取角色
    QString role() const { return m_roleCombo->currentText(); }

private:
    QLineEdit *m_nameEdit = nullptr;
    QSpinBox *m_ageSpin = nullptr;
    QComboBox *m_roleCombo = nullptr;
};

// 调用方
UserConfigDialog dlg(this);
if (dlg.exec() == QDialog::Accepted) {
    QString name = dlg.username();
    int age = dlg.age();
    QString role = dlg.role();
    qDebug() << "用户配置:" << name << age << role;
}
```

注意一个关键点：getter 方法必须在对话框关闭之后调用。exec() 返回后对话框只是被隐藏了，控件和数据都还在，所以你可以安全地读取。但如果你用了 WA_DeleteOnClose 或者手动 deleteLater() 了对话框，那在 delete 之后就不能再访问了。所以标准的流程是：先 exec()，然后读取数据，最后再 delete。

有些开发者会在对话框关闭前把数据保存到一个独立的结构体中，而不是依赖控件的生命周期。这种方式更安全，因为它解耦了数据存储和 UI 控件。

```cpp
struct UserInfo {
    QString username;
    int age;
    QString role;
};

class UserConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserConfigDialog(QWidget *parent = nullptr);

    /// @brief 获取用户填写的信息（在 exec() 返回后调用）
    UserInfo userInfo() const { return m_info; }

private slots:
    void onAccepted()
    {
        m_info.username = m_nameEdit->text();
        m_info.age = m_ageSpin->value();
        m_info.role = m_roleCombo->currentText();
    }

private:
    QLineEdit *m_nameEdit = nullptr;
    QSpinBox *m_ageSpin = nullptr;
    QComboBox *m_roleCombo = nullptr;
    UserInfo m_info;
};

// 在构造函数中连接
UserConfigDialog::UserConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    // ... 创建控件和布局 ...
    connect(this, &QDialog::accepted,
            this, &UserConfigDialog::onAccepted);
}
```

这种"在 accepted 信号中保存数据到结构体"的模式在复杂对话框中更有优势。它保证了即使对话框被提前 delete，你仍然有数据可用。而且在 onAccepted 中你可以做数据校验——如果校验失败，你可以阻止对话框关闭。

如果你想在 accept() 之前做校验，可以重写 accept() 虚函数。在这个重写版本中，先检查输入是否合法，合法才调用 QDialog::accept()，不合法就停留着让用户继续修改。

```cpp
void UserConfigDialog::accept()
{
    if (m_nameEdit->text().isEmpty()) {
        m_nameEdit->setFocus();
        // 可以给用户一个提示
        return;
    }
    if (m_ageSpin->value() < 18) {
        m_ageSpin->setFocus();
        return;
    }
    // 校验通过，保存数据并关闭
    m_info.username = m_nameEdit->text();
    m_info.age = m_ageSpin->value();
    m_info.role = m_roleCombo->currentText();
    QDialog::accept();
}
```

这几乎是自定义对话框的标准范式：在 accept() 中做校验，校验通过才真正关闭对话框。用户看到的效果就是——输入不合法时点"确定"没反应，必须修正后才能提交。这种做法比弹出另一个 QMessageBox 来告知"输入有误"要优雅得多，因为用户不需要在两个窗口之间来回切换。

### 3.4 setModal vs setWindowModality

前面我们说的"模态"和"非模态"是通过 exec() 和 show() 来区分的。但实际上 Qt 提供了更精细的模态控制——通过 setWindowModality 和 setModal 两个方法。

setModal(bool modal) 是一个简化的接口。如果设为 true，对话框会变成模态的——即使你是用 show() 打开的。这看起来有点奇怪：show() 本来是非模态的，但 setModal(true) 可以让它变成模态的？是的。区别在于，用 exec() 打开的模态对话框是同步的——exec() 会阻塞直到对话框关闭；而用 show() + setModal(true) 打开的模态对话框是异步的——show() 立刻返回，但用户仍然无法操作其他窗口。后者在你不想阻塞调用流程但又要阻止用户操作其他窗口时有意义。

```cpp
// 方式一：exec() 模态（同步）
dialog->exec();  // 阻塞在这里

// 方式二：show() + setModal(true) 模态（异步）
dialog->setModal(true);
dialog->show();  // 立刻返回，但用户无法操作其他窗口
// 需要通过信号来获取对话框的关闭结果
connect(dialog, &QDialog::finished,
        this, [](int result) {
    // result 就是 exec() 会返回的那个值
});
```

setWindowModality(Qt::WindowModality mode) 提供了三种级别的模态控制。Qt::NonModal 表示非模态，用户可以自由操作所有窗口，这是默认值。Qt::WindowModal 表示窗口级模态，对话框会阻塞它的父窗口以及父窗口的所有兄弟窗口，但不会影响其他独立的顶层窗口。Qt::ApplicationModal 表示应用级模态，对话框会阻塞整个应用中的所有窗口——用户除了操作这个对话框之外什么也做不了。

```cpp
// 应用级模态：用户只能操作这个对话框
dialog->setWindowModality(Qt::ApplicationModal);

// 窗口级模态：用户无法操作父窗口，但可以操作其他独立窗口
dialog->setWindowModality(Qt::WindowModal);
```

窗口级模态在多窗口应用中很有用。想象你有一个主窗口和一个独立的数据监控窗口，主窗口弹出了一个"导出配置"对话框。如果对话框是 ApplicationModal，用户无法操作监控窗口——这不合理，因为导出配置和监控是两个无关的任务。这时用 WindowModal，对话框只阻塞主窗口，监控窗口仍然可以正常交互。

实际开发中的选择建议是：大多数情况下用 exec() 就够了，它的 ApplicationModal 行为是用户预期的弹窗行为。只有在多窗口架构中需要精细控制模态范围时，才考虑 setWindowModality。而 setModal(true) + show() 这种方式比较少用——它介于 exec() 的同步模态和 show() 的非模态之间，容易让人困惑，建议只在确实需要异步模态的场景下使用。

还有一个相关的设置是 setParent。如果你在创建 QDialog 时传入了 parent，那么对话框默认会在 parent 所在的窗口上方居中显示（取决于窗口管理器的行为）。如果你没有设置 parent，对话框就是一个完全独立的窗口，用户可以自由移动它。通常的做法是传入 parent，这样对话框在视觉上和逻辑上都与父窗口关联。

## 4. 踩坑预防

第一个坑是在 exec() 内部再次调用 exec()。这是允许的——Qt 支持嵌套的事件循环。比如对话框 A 的按钮槽函数中弹出对话框 B，B 的 exec() 会在 A 的 exec() 内部启动一个新的局部事件循环。但嵌套 exec() 会让代码的执行流变得难以追踪，调试时经常一头雾水。如果你发现自己嵌套了三层以上的 exec()，应该考虑重构交互流程。

第二个坑是在非模态对话框中使用 deleteLater() 的时机。如果你在对话框的按钮槽函数中调用 deleteLater()，然后又访问了对话框的成员——这是未定义行为，因为 deleteLater() 可能在当前事件循环迭代结束后就执行析构。安全的做法是在 finished 信号中调用 deleteLater()，并且之后不再访问对话框的任何成员。

第三个坑是 accept() 和 close() 的区别。close() 只是关闭窗口（隐藏窗口），但不会设置返回值——exec() 仍然会返回，但返回值取决于 close 事件的处理。QDialog 的默认 closeEvent 会调用 reject()，所以 close() 通常等价于 reject()。但如果你重写了 closeEvent 并且没有调用 reject() 或者 accept()，exec() 可能永远不会返回。所以除非你确实需要自定义关闭行为，否则不要重写 closeEvent，直接用 accept() / reject() 来关闭对话框。

第四个坑是 exec() 和 WA_DeleteOnClose 的组合。如果你同时使用了 exec() 和 WA_DeleteOnClose，对话框在关闭时会被 delete，但 exec() 还会尝试访问对话框的数据来返回结果。这会导致未定义行为。exec() 和 WA_DeleteOnClose 是互斥的——用 exec() 就不要设 WA_DeleteOnClose，用 show() 时才需要设。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，主窗口上有一个"新建用户"按钮。点击按钮弹出一个模态对话框 UserConfigDialog，对话框中包含三个输入控件：用户名（QLineEdit）、年龄（QSpinBox，范围 0-150，默认值 25）、角色（QComboBox，选项为"管理员"、"编辑者"、"观察者"）。对话框底部有"确定"和"取消"两个按钮，点击"确定"时校验用户名不为空且年龄不小于 18，校验失败则不关闭对话框并将焦点移到出错的控件。校验通过后 accept() 关闭对话框，主窗口从对话框中读取用户填写的信息并显示在 QTextEdit 中。点击"取消"则 reject() 关闭，主窗口不做任何操作。

同时实现一个"查找"按钮，点击后弹出一个非模态的 FindDialog，包含一个 QLineEdit 输入搜索关键字和一个 QTextEdit 显示搜索结果（模拟即可）。FindDialog 设置 WA_DeleteOnClose，通过 finished 信号监听关闭事件。

提示：UserConfigDialog 使用 exec() 模态方式打开，重写 accept() 做校验。FindDialog 使用 show() 非模态方式打开，注意生命周期管理。

## 6. 官方文档参考链接

[Qt 文档 -- QDialog](https://doc.qt.io/qt-6/qdialog.html) -- 对话框基类

[Qt 文档 -- QDialog::exec](https://doc.qt.io/qt-6/qdialog.html#exec) -- 模态显示方法

[Qt 文档 -- Qt::WindowModality](https://doc.qt.io/qt-6/qt.html#WindowModality-enum) -- 窗口模态枚举

[Qt 文档 -- Dialog Examples](https://doc.qt.io/qt-6/examples-dialogs.html) -- 对话框示例集

---

到这里，QDialog 的核心用法就全部讲完了。exec() 和 show() 分别对应模态和非模态两种交互模式，accept() / reject() / done() 提供了语义化的返回值约定，getter 方法和 accepted 信号是从对话框中取出用户数据的标准手段，setWindowModality 在多窗口架构中提供精细的模态控制。掌握了这些，后续的 QMessageBox、QFileDialog、QInputDialog 等内置对话框的行为对你来说就是透明的——它们全部建立在这套机制之上。
