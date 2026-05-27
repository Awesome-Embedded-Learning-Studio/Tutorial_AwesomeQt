---
title: "3.60 QDialog 进阶"
description: "入门篇我们学会了 QDialog 的基本创建和 exec() 模态调用。进阶篇我们要深入 QDialog 的模态策略——exec() 的阻塞事件循环、show()/open() 的非阻塞模式、setWindowModality 的三种粒度，以及如何用信号和 lambda 回调构建异步对话框的结果处理链。"
---

# 现代Qt开发教程（进阶篇）3.60——QDialog 进阶

## 1. 前言 / exec() 用着用着就翻车了

入门篇我们学会了 QDialog 的基本操作——new 一个对话框、调 exec()、拿返回值。说实话，如果你只写简单的"弹个框问用户点确定还是取消"这种逻辑，exec() 确实够用。但一旦你的程序复杂起来，exec() 就开始露出獠牙了。

为什么？因为 exec() 的底层机制是开启一个**嵌套事件循环**。它会阻塞调用它的那个函数，直到对话框关闭才返回。这听起来很方便，但实际上埋了一堆定时炸弹：如果你的主窗口有个 QTimer 在跑，exec() 期间定时器照样触发；如果有个网络请求回来触发了一个回调，回调里恰好操作了正在等待对话框返回的对象——恭喜你，你可能会收获一个非常漂亮的使用已释放内存的场景。Qt 官方文档现在都开始建议开发者优先使用 open() 而不是 exec()，原因就在这里。

这一篇我们要做的事情是：彻底搞清楚 exec()、show()、open() 三者的区别，理解 setWindowModality 的三种模态粒度，掌握异步对话框的结果回调模式，最后把 QDialogButtonBox 和 accept/reject 的集成吃透。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。所有内容依赖 QtWidgets 模块。示例可在任何支持 Qt6 的桌面平台上编译运行。异步回调模式会用到 lambda 和信号槽的函数指针连接语法。

## 3. 核心概念讲解

### 3.1 exec() vs show() ——阻塞事件循环与非阻塞

我们先来搞清楚这三兄弟到底干了什么。exec() 调用后，QDialog 内部会创建一个 QEventLoop 并调用它的 exec()，这意味着调用 QDialog::exec() 的那个函数被挂起了——代码停在 exec() 那一行，直到对话框被 accept() 或 reject() 关闭后才继续往下走。但挂起的只是你的代码，不是整个程序。Qt 的事件分发器仍然在处理事件：QTimer 的 timeout 照样触发，QSocketNotifier 的网络事件照样分发，其他窗口的 repaint 请求照样执行。这就是所谓的"嵌套事件循环"——你在事件循环 A 中开启了一个事件循环 B，B 关闭之前 A 中的后续代码不会执行，但 A 中已经注册的事件源仍然活跃。

show() 就简单多了。它调用 QWidget::show() 把对话框显示出来，然后立即返回。你的代码继续往下走，对话框就在那里待着，用户想操作主窗口也行，想操作对话框也行。show() 本身不涉及任何事件循环的嵌套。

```cpp
// 模态用法——exec() 阻塞
auto* dlg = new MyDialog(this);
if (dlg->exec() == QDialog::Accepted) {
    // 用户点了确定，拿到数据
    auto result = dlg->getResult();
    process(result);
}
delete dlg;  // exec() 返回后安全删除

// 非模态用法——show() 立即返回
auto* dlg = new MyDialog(this);
dlg->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动删除
connect(dlg, &MyDialog::dataReady, this, &MainWindow::onDataReady);
dlg->show();
// show() 返回了，继续做别的事
```

现在问题来了：exec() 的嵌套事件循环到底危险在哪？最典型的翻车场景是这样的——你在主窗口有个 slot 处理网络回调，回调里调用了某个对象的成员函数，而这个对象的析构刚好挂在 exec() 返回之后的代码路径上。如果网络回调在 exec() 期间触发了，它访问了一个"逻辑上应该已经不存在但物理上还没被 delete"的对象，行为就是未定义的。这不是理论上的风险，是我在实际项目里踩过的坑，调了一整天才发现是 exec() 期间一个 deferred delete 事件被提前处理了。

Qt 还提供了第三个选择：open()。open() 的行为介于 exec() 和 show() 之间——它设置了 WindowModal 模态，然后用 show() 显示对话框，不阻塞。当对话框关闭时，它会发射 finished() 信号。这是 Qt 官方推荐的异步模态对话框方式，我们后面会详细讲。

### 3.2 setWindowModality 三种模式

QDialog 的模态行为不是非黑即白的，Qt 给了我们三种粒度的控制。这个属性继承自 QWidget，通过 setWindowModality() 设置，对应的枚举是 Qt::WindowModality。

第一种是 Qt::NonModal，顾名思义就是非模态。对话框显示后用户可以自由操作所有窗口，对话框只是漂浮在上面。show() 默认就是这个模式。

第二种是 Qt::WindowModal，窗口级模态。对话框会阻塞它的父窗口以及父窗口的所有兄弟窗口，但不会影响其他独立的顶层窗口。这在多窗口应用中非常有用——比如你的应用有一个主窗口和一个独立工具面板，弹出的对话框只需要阻塞主窗口但不影响工具面板，WindowModal 就是正确的选择。open() 内部就是设置了 WindowModal。

第三种是 Qt::ApplicationModal，应用级模态。对话框会阻塞整个应用中的所有窗口，用户必须先处理完对话框才能继续操作任何窗口。exec() 内部就是设置了 ApplicationModal。

```cpp
// 窗口级模态——只阻塞父窗口链
dlg->setWindowModality(Qt::WindowModal);
dlg->show();  // 非阻塞，但父窗口被冻结

// 应用级模态——不调 exec() 也能实现
dlg->setWindowModality(Qt::ApplicationModal);
dlg->show();  // 非阻塞，但所有窗口被冻结

// open() 等价于 WindowModal + show() + finished 信号连接
dlg->open(this, SLOT(onDialogFinished()));
```

这里有一个容易搞混的点：setModal(true) 和 setWindowModality(Qt::ApplicationModal) 的效果是一样的。setModal() 是一个便捷方法，内部就是调 setWindowModality()。但 setModal() 不会改变 show() 的非阻塞行为——你仍然需要配合 show() 而不是 exec()。如果你调了 setModal(true) 再调 show()，对话框是模态的（其他窗口被冻结），但 show() 立即返回，你的代码不会被阻塞。

还有一个细节值得注意：WindowModal 的阻塞范围是以"窗口层级链"来计算的。如果对话框 A 的父窗口是主窗口，对话框 B 的父窗口是对话框 A，那么 B 的 WindowModal 只会阻塞 A 和主窗口，不会影响其他独立窗口。这个层级关系在复杂的多对话框场景中很重要。

### 3.3 异步对话框的结果回调模式

既然 exec() 有风险，那我们就用异步模式来处理对话框的结果。异步模式的核心思路是：显示对话框（不阻塞），通过信号来通知调用者对话框的结果。QDialog 提供了三个关键信号来完成这件事。

finished(int result) 信号在对话框关闭时触发，参数是对话框的结果码。accepted() 在 accept() 被调用时触发，rejected() 在 reject() 被调用时触发。这三个信号的关系是：accept() 会同时触发 accepted() 和 finished(Accepted)，reject() 会同时触发 rejected() 和 finished(Rejected)。

```cpp
// 方式一：用 finished 信号 + lambda 回调
auto* dlg = new MyDialog(this);
dlg->setAttribute(Qt::WA_DeleteOnClose);

connect(dlg, &QDialog::finished, this, [this](int result) {
    if (result == QDialog::Accepted) {
        qDebug() << "用户确认了操作";
        // 处理确认逻辑
    } else {
        qDebug() << "用户取消了操作";
    }
});

dlg->open();  // 异步模态，不阻塞

// 方式二：用 accepted/rejected 分开连接
auto* dlg2 = new ConfirmDialog(this);
dlg2->setAttribute(Qt::WA_DeleteOnClose);

connect(dlg2, &QDialog::accepted, this, [this, dlg2]() {
    // 注意：dlg2 此时还没被 delete，WA_DeleteOnClose 的 delete 是 deferred
    auto data = dlg2->collectData();
    applyData(data);
});

connect(dlg2, &QDialog::rejected, this, []() {
    qDebug() << "取消";
});

dlg2->open();
```

这里有一个非常重要的细节：当你使用 WA_DeleteOnClose 时，对话框在关闭后会被标记为待删除，但实际的 delete 操作是延迟的（通过 deleteLater()）。这意味着在 finished/accepted/rejected 信号的槽函数中，对话框对象仍然有效——你可以安全地从中提取数据。但如果你的槽函数中启动了任何异步操作（比如另一个 open() 的对话框或者一个定时器），在那个异步操作回调时对话框可能已经被删除了。所以如果你需要在延迟回调中使用对话框数据，一定要在信号槽里把数据拷贝出来，而不是保存对话框的指针。

方式二的 lambda 中有一个容易忽略的陷阱——lambda 捕获了 dlg2 指针。因为 WA_DeleteOnClose 的存在，如果槽函数中的逻辑比较重（比如调用了 processEvents 或者打开了另一个模态对话框），dlg2 可能在槽函数执行期间就被 delete 了。安全做法是在 lambda 中只拷贝你需要的数据，或者不设 WA_DeleteOnClose 而是在槽函数末尾手动 deleteLater。

```cpp
// 安全的异步回调模式
auto* dlg = new SettingsDialog(this);

// 在连接信号前准备好数据提取函数
connect(dlg, &QDialog::accepted, this, [this, dlg]() {
    // 立即拷贝数据，不保存 dlg 指针
    auto settings = dlg->currentSettings();
    m_appSettings = settings;  // 拷贝到成员变量
    applySettings(settings);
});

// 用 finished 信号统一做清理
connect(dlg, &QDialog::finished, dlg, &QObject::deleteLater);

dlg->open();
```

open() 还有一个历史悠久的重载版本：open(QObject* receiver, const char* member)。这个版本会在对话框关闭时自动调用 receiver 的 member 槽函数，并且连接是自动断开的（一次性连接）。但在 Qt6 中，我们更推荐用函数指针连接 + lambda 的方式，可读性和类型安全性都更好。

### 3.4 QDialogButtonBox 与 accept/reject 的集成

QDialogButtonBox 是 Qt 为对话框按钮提供的一个标准容器，它的核心价值不只是帮你排列按钮——更重要的是它能自动触发 QDialog 的 accept() 和 reject()。当你在 QDialogButtonBox 中放入 AcceptRole 的按钮（比如 Ok、Save）并把它放进 QDialog 时，点击这个按钮会自动调用 QDialog 的 accept()；RejectRole 的按钮（比如 Cancel、Close）会自动调用 reject()。这个自动连接是在 QDialogButtonBox 被 show() 时建立的，不需要你手动 connect。

```cpp
auto* dialog = new QDialog(this);
auto* layout = new QVBoxLayout(dialog);

// ... 添加对话框内容 ...

auto* buttons = new QDialogButtonBox(
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

// 不需要手动连接 accepted/rejected！
// QDialogButtonBox 会自动触发 dialog 的 accept/reject

layout->addWidget(buttons);
dialog->open();

connect(dialog, &QDialog::accepted, this, [this]() {
    // 在这里处理确认逻辑
});
```

这个自动连接的机制是通过 QDialogButtonBox 的 accepted() 和 rejected() 信号实现的。当按钮的 role 是 AcceptRole 时，点击它会触发 QDialogButtonBox::accepted() 信号，而这个信号在 QDialog 内部被自动连接到了 QDialog::accept()。这个连接发生在 QDialog::showEvent() 中——所以如果你在 show() 之前就在按钮上连了自己的 clicked 信号槽，需要注意执行顺序：QDialogButtonBox 的 accepted/rejected 信号先触发，然后 QDialog 的 accept()/reject() 被调用，最后对话框关闭。

如果你需要在 accept 之前做数据验证，不要直接 connect Ok 按钮的 clicked 信号——更好的做法是重写 QDialog 的 accept() 虚函数，在里面做验证，只有验证通过才调用 QDialog::accept()。

```cpp
class ValidatedDialog : public QDialog {
protected:
    void accept() override {
        if (!validateInput()) {
            // 验证失败，不关闭对话框
            highlightErrors();
            return;
        }
        // 验证通过，调用基类 accept 关闭对话框
        QDialog::accept();
    }

    bool validateInput() {
        // 验证逻辑
        return !m_input->text().isEmpty();
    }
};
```

这样做的好处是把验证逻辑和按钮的信号解耦了。不管用户是点 Ok 按钮、按 Enter 键、还是通过代码调用 accept()，都会经过同一套验证逻辑。如果你把验证逻辑放在 Ok 按钮的 clicked 槽里，用代码调用 accept() 就会绕过验证。

## 4. 踩坑预防

第一个坑是 exec() 期间的对象生命周期问题。当你调用了 exec()，嵌套事件循环会处理所有排队的事件，包括 DeferredDelete 事件。这意味着如果你的代码逻辑是"exec() 返回后 delete 某个对象"，但 exec() 期间某个事件处理函数又访问了这个对象，你就会踩到未定义行为。解决方案是：尽量避免在 exec() 期间触发与正在等待的对象有交互的回调，或者改用 open() 异步模式从根本上避免这个问题。

第二个坑是 WA_DeleteOnClose 和信号槽时序的冲突。当你给对话框设置了 WA_DeleteOnClose，对话框关闭后会在事件循环的下一轮迭代中被 delete。但 finished/accepted/rejected 信号是在关闭过程中同步触发的，所以在这些信号的槽函数中对话框对象仍然有效。问题出在如果你在槽函数里又做了一些会导致事件循环处理的事情（比如调了另一个 exec()），对话框可能在那个嵌套调用中被 delete 了。解决方案是不使用 WA_DeleteOnClose，改用 connect(dlg, &QDialog::finished, dlg, &QObject::deleteLater)——这样可以保证 deleteLater 一定在所有信号槽处理完毕后才被调度。

第三个坑是 open() 的 receiver/member 重载版本在 Qt6 中依然可用但不够安全。这个重载版本内部使用了 QMetaObject::invokeMethod 的字符串查找机制，没有编译期类型检查。如果你改了槽函数的名字但忘了更新 open() 的参数，编译能过但运行时回调不会触发——而且不会有任何警告。解决方案是统一使用 open() 的无参版本 + connect() 函数指针语法。

第四个坑是在 accept() 重写中忘记调用基类的 accept()。我们前面建议在 accept() 中做验证，验证通过后才调 QDialog::accept()。但如果你忘了调基类的 accept()，对话框不会关闭，finished 信号不会触发，open() 的回调也不会执行。这个 bug 非常隐蔽，因为对话框看起来"什么都没发生"，不会报错也不会崩溃。养成一个习惯：每次重写 accept() 都检查一下是否在所有代码路径上要么调了 QDialog::accept() 要么明确想阻止关闭。

## 5. 练习项目

练习项目：异步设置对话框。我们要做一个模拟应用设置对话框的小项目，重点练手异步对话框的结果回调。

我们要实现的功能是：主窗口有一个按钮"打开设置"，点击后用 open() 异步弹出设置对话框。设置对话框包含两个 QLineEdit（用户名、服务器地址）和一个 QDialogButtonBox（Ok + Cancel + Apply）。Ok 按钮关闭对话框并通过 accepted 信号传递数据，Cancel 按钮关闭对话框不做任何事，Apply 按钮不关闭对话框但立即应用当前设置（这需要你用 QDialogButtonBox 的 clicked 信号配合 buttonRole 来判断）。主窗口在对话框打开期间仍然可以响应（但被 WindowModal 冻结），设置对话框关闭后主窗口更新显示。完成标准是三种按钮行为正确——Ok 关闭并应用、Cancel 关闭不应用、Apply 不关闭但应用——代码结构清晰，信号槽连接合理，没有内存泄漏。

提示几个关键点：Apply 按钮的 role 是 ApplyRole，它不会自动触发 accept 或 reject，你需要通过 clicked(QAbstractButton*) 信号判断点击的是哪个按钮；验证逻辑放在 accept() 重写中而不是按钮的信号槽中；用 connect(dlg, &QDialog::finished, dlg, &QObject::deleteLater) 管理对话框生命周期。

## 6. 官方文档参考链接

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- QDialog 类参考，exec/show/open 三种显示方式和模态行为

[Qt 文档 · QDialogButtonBox](https://doc.qt.io/qt-6/qdialogbuttonbox.html) -- QDialogButtonBox 类参考，StandardButton 和 ButtonRole 枚举

[Qt 文档 · Qt::WindowModality](https://doc.qt.io/qt-6/qt.html#WindowModality-enum) -- 窗口模态枚举值 NonModal/WindowModal/ApplicationModal

[Qt 文档 · QWidget](https://doc.qt.io/qt-6/qwidget.html) -- QWidget 基类参考，setWindowModality 和 WA_DeleteOnClose 属性

[Qt 文档 · QEventLoop](https://doc.qt.io/qt-6/qeventloop.html) -- 事件循环类，exec() 嵌套机制的基础

---

到这里我们就把 QDialog 的模态策略和异步回调模式彻底理清楚了。exec() 不是不能用，但你要清楚它的嵌套事件循环会带来什么后果；open() 是 Qt 官方推荐的替代方案，配合 finished 信号和 lambda 回调能写出更安全的代码；三种 WindowModality 让你在多窗口场景中有精确的控制粒度。下一篇我们来看 QDialogButtonBox 的进阶用法——自定义按钮角色、动态状态控制，以及 clicked 信号的分角色处理。
