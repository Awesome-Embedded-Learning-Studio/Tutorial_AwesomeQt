# 现代Qt开发教程（新手篇）1.6——内存管理

## 1. 前言：C++ 内存管理的痛

说实话，C++ 的内存管理是最让新手头疼，也最让老手翻车的部分。我自己刚学 Qt 的时候，没少在 `new` 和 `delete` 上栽跟头。要么忘了 `delete` 导致内存泄漏，要么同一个对象被 `delete` 了两次直接程序崩溃。

更绝的是，Qt 这套框架自己就有对象树机制，而 C++11 之后又有了 `std::unique_ptr`、`std::shared_ptr`，然后 Qt 自己又搞了一套 `QSharedPointer`、`QScopedPointer`。这堆东西混在一起，新手很容易迷失方向。

所以这一篇，我们要把这些东西捋清楚。不是让你死记硬背 API，而是理解每种方案背后的设计理念和使用场景。学会之后，你会发现 Qt 的内存管理其实比纯 C++ 要轻松很多。

## 2. 对象树自动清理——Qt 的第一道防线

Qt 最经典的内存管理机制就是对象树。简单来说，当你创建一个 `QObject` 派生类对象时，可以给它指定一个父对象。这个父对象就拥有了这个子对象的所有权，当父对象被销毁时，所有子对象会被自动销毁。

这听起来很抽象，我们来看个具体的例子。假设你在做一个窗口程序，里面有一个按钮、一个标签、一个输入框。在传统 C++ 里，你需要手动管理这些控件的生命周期：

```cpp
// 传统 C++ 风格，容易出错
QWidget* window = new QWidget();
QPushButton* button = new QPushButton(window);
QLabel* label = new QLabel(window);
QLineEdit* edit = new QLineEdit(window);

// 用完之后必须手动 delete，顺序还不能乱
delete edit;
delete label;
delete button;
delete window;
// 一旦中间抛异常，内存泄漏妥妥的
```

而在 Qt 里，只要在构造时指定父对象，就万事大吉了：

```cpp
// Qt 风格，自动管理
QWidget* window = new QWidget();
// 这里传入 window 作为父对象，这些控件就加入了 window 的对象树
QPushButton* button = new QPushButton("Click me", window);
QLabel* label = new QLabel("Hello", window);
QLineEdit* edit = new QLineEdit(window);

// 只需要 delete 窗口，所有子控件会自动被清理
delete window;  // button、label、edit 都会自动 delete
```

这个机制的核心在于 `QObject` 的构造函数：

```cpp
QObject::QObject(QObject* parent = nullptr);
```

当你传入一个非空的 `parent` 时，这个对象就会被添加到父对象的 `children()` 列表里。当父对象析构时，它会遍历所有子对象并逐个删除。

你可能已经意识到了，Qt 的对象树机制本质上解决了对象所有权不明确的问题——父对象拥有子对象，所以自动清理时不会遗漏，即使中间代码抛异常，对象树的析构链条仍然会正确触发。这比手动 delete 安全得多，因为手动管理的最大风险就是忘了或者顺序错了。

### 2.1 栈对象的自动管理

其实更推荐的做法是直接在栈上创建父对象：

```cpp
// 更好的做法，栈对象自动析构
QWidget window;
QPushButton button("Click me", &window);
QLabel label("Hello", &window);

// window 超出作用域自动析构，button 和 label 也会自动清理
// 不需要任何 delete，完美
```

这种写法结合了 C++ 的 RAII（资源获取即初始化）和 Qt 的对象树，是最安全的做法。栈对象在离开作用域时会自动调用析构函数，触发对象树的清理机制。

### 2.2 对象树的限制

对象树机制虽好，但不是万能的。首先它只能用于 `QObject` 及其派生类，如果你用的是标准 C++ 类或者第三方库的类，这套机制就用不上。其次子对象的生命周期完全绑定在父对象上，你不能单独删除一个子对象然后继续使用它，也不能在父对象删除后继续访问子对象。最后，对象树不适合共享所有权的场景，如果一个对象可能被多个地方同时引用，对象树就无法表示这种关系。

这里有一个很经典的坑：重复删除导致崩溃。你把一个按钮交给窗口的对象树管理，然后又手动 delete 了这个按钮，接下来窗口析构时会再次删除它，双重 free 直接炸掉。所以一旦子对象交给对象树，就别再手动 delete 了。

```cpp
// 典型的崩溃场景
QWidget* window = new QWidget();
QPushButton* button = new QPushButton("Click", window);
delete button;  // 手动删除子对象
delete window;  // window 析构时会再次删除 button，崩溃！

// 正确做法：只删除父对象
QWidget* window2 = new QWidget();
QPushButton* button2 = new QPushButton("Click", window2);
delete window2;  // 子对象自动清理
```

## 3. 智能指针——Qt 的第二道防线

当对象树不够用时，Qt 提供了一套智能指针来帮助管理内存。这套智能指针在 C++11 之前就存在了，比标准库的智能指针出现得更早。

### 3.1 QScopedPointer——独占所有权

`QScopedPointer` 是最简单的智能指针，它表示对某个对象的独占所有权。当 `QScopedPointer` 超出作用域时，它所指向的对象会被自动删除。你不能复制它，只能移动它（C++11 之后）。

这个类在 Qt 5.15 之后被标记为已废弃，推荐使用 C++11 的 `std::unique_ptr`。但既然是教程，我们还是了解一下，毕竟可能会在老代码里见到。

```cpp
QScopedPointer<QWidget> window(new QWidget());
// 使用 window.get() 获取原始指针
// 或者用 window-> 直接访问成员（重载了 -> 和 *）
window->show();
// window 超出作用域时，自动 delete
```

`QScopedPointer` 的使用场景很明确：当你需要一个对象，且这个对象的所有权不会被转移时，用它就很合适。

有一点要注意，`QScopedPointer` 是排他性的，不能复制。如果你写 `QScopedPointer<QWidget> ptr2 = ptr1;`，编译直接报错，这是设计上就防止所有权混乱的做法。如果确实需要转移所有权，用 `take()` 释放所有权再交给另一个 `QScopedPointer`：

```cpp
QScopedPointer<QWidget> ptr1(new QWidget());
QScopedPointer<QWidget> ptr2;
ptr2.reset(ptr1.take());  // take() 释放所有权但不删除对象
```

### 3.2 QSharedPointer——共享所有权

`QScopedPointer` 的局限在于它不支持共享所有权，而 `QSharedPointer` 正好解决了这个问题。它使用引用计数来追踪有多少个 `QSharedPointer` 指向同一个对象，当引用计数归零时，对象被删除。

```cpp
// 创建一个共享指针
QSharedPointer<QWidget> ptr1(new QWidget());
// 复制共享指针，引用计数增加到 2
QSharedPointer<QWidget> ptr2 = ptr1;

// 现在有两个指针指向同一个对象
// 无论哪个先离开作用域，对象都不会被删除
// 只有当两者都离开作用域，引用计数归零，对象才被删除
```

这个机制在处理不确定生命周期的对象时非常有用。比如你可能有一个全局的资源管理器，多个组件都需要访问同一个配置对象，但又没人真正"拥有"这个对象。这时候用 `QSharedPointer` 就很合适。

`QSharedPointer` 还支持自定义删除器，可以在删除对象时执行额外的操作：

```cpp
// 自定义删除器
auto customDeleter = [](QWidget* obj) {
    qDebug() << "About to delete widget";
    delete obj;
};
QSharedPointer<QWidget> ptr(new QWidget(), customDeleter);
```

### 3.3 QWeakPointer——弱引用

`QWeakPointer` 是 `QSharedPointer` 的伴生类，它持有对某个对象的弱引用，不会增加引用计数。这听起来很奇怪，为什么需要这样一个东西？

想象一个场景：两个对象互相持有对方的 `QSharedPointer`。A 对象有 B 的共享指针，B 对象也有 A 的共享指针。这样它们的引用计数永远不会归零，永远不会被删除，这就是典型的循环引用问题。

`QWeakPointer` 的作用就是打破这种循环。它指向一个由 `QSharedPointer` 管理的对象，但不参与引用计数。当原对象被删除后，`QWeakPointer` 会自动变为空：

```cpp
QSharedPointer<QWidget> strongPtr(new QWidget());
QWeakPointer<QWidget> weakPtr = strongPtr;

// weakPtr 不会影响 strongPtr 的引用计数
if (!weakPtr.isNull()) {
    // 安全地使用对象
    QSharedPointer<QWidget> locked = weakPtr.toStrongRef();
    if (locked) {
        locked->show();
    }
}

// 当 strongPtr 离开作用域，对象被删除
// weakPtr 会自动变为空，再调用 isNull() 会返回 true
```

我们来做一个填空练习：补全以下代码，使用 `QWeakPointer` 安全地访问可能已删除的对象。

```cpp
QSharedPointer<QLabel> labelPtr(new QLabel("Hello"));
QWeakPointer<QLabel> weakLabel = labelPtr;

// 一段时间后...
if (!weakLabel.isNull()) {
    QSharedPointer<QLabel> strong = weakLabel.toStrongRef();
    if (strong) {
        strong->show();
    }
}
```

这里的模式很固定：先用 `isNull()` 检查弱引用是否还有效，再通过 `toStrongRef()` 提升为共享指针来安全访问对象。提升失败说明对象已经被删除了，`strong` 会是空指针。

### 3.4 QPointer——弱引用的另一种选择

`QPointer` 是专门为 `QObject` 设计的弱指针。它比 `QWeakPointer` 更早出现，用法也更简单：

```cpp
QPointer<QLabel> label = new QLabel("Hello");
// label 不持有强引用，不影响对象生命周期

// 检查对象是否还存在
if (label) {
    label->show();
}

// 如果 label 指向的对象被删除，label 会自动变为空
```

`QPointer` 的优势是专门为 `QObject` 优化，当对象被删除时能立即感知。但它只能用于 `QObject` 及其派生类，而 `QWeakPointer` 更通用。

## 4. 何时用什么——实战指南

讲了这么多，我们来总结一下不同场景下的最佳实践。

场景一：UI 控件的父子关系。这是对象树的主战场，直接指定父对象就完事了。

```cpp
// 推荐用对象树
QWidget window;
QPushButton* button = new QPushButton("Click", &window);
```

场景二：短暂使用的临时对象。直接用栈对象或者 `std::unique_ptr`，简单粗暴。

```cpp
// 推荐用栈对象或 std::unique_ptr
std::unique_ptr<QWidget> window(new QWidget());
// 或者直接
QWidget window;
```

场景三：需要在多个地方共享的对象。用 `QSharedPointer`，引用计数帮你搞定生命周期。

```cpp
// 推荐用 QSharedPointer
QSharedPointer<Config> config(new Config());
// 多个模块都持有这个 config 的副本
```

场景四：观察某个可能被删除的 QObject。用 `QPointer`，访问前检查是否为空就行。

```cpp
// 推荐用 QPointer
QPointer<QLabel> label = findLabel();
// 需要使用时先检查
if (label) {
    label->setText("Updated");
}
```

再看一个调试题：下面这段代码有什么问题？

```cpp
class NetworkManager : public QObject {
public:
    void setReply(QNetworkReply* r) {
        reply = r;
    }

private:
    QNetworkReply* reply;
};

void processData() {
    QNetworkAccessManager* mgr = new QNetworkAccessManager();
    NetworkManager* handler = new NetworkManager();

    QNetworkReply* reply = mgr->get(QNetworkRequest(QUrl("http://example.com")));
    handler->setReply(reply);

    delete mgr;  //mgr 被删除，reply 也会被删除
    // handler->reply 现在是悬空指针！
}
```

问题很明确：`QNetworkAccessManager` 析构时会删除所有相关的 `QNetworkReply`，所以 `handler->reply` 变成了悬空指针，后续任何访问都会导致崩溃。解决办法是改用 `QPointer<QNetworkReply>` 来持有 reply 的引用，或者在 mgr 删除前把 reply 置空。实际上更好的做法是直接把 mgr 做成栈对象，让它的生命周期覆盖整个使用过程：

```cpp
QNetworkAccessManager mgr;  // 栈对象，不需要手动删除
QNetworkReply* reply = mgr.get(request);
// 或者用 QPointer 监听 reply
QPointer<QNetworkReply> replyPtr = reply;
```

一句话记住这个坑：`QNetworkAccessManager` 拥有其创建的 `QNetworkReply`，删除 manager 会导致 reply 也被删除。

## 5. 练习项目

练习项目：任务管理器。

创建一个简单的任务管理器程序，每个任务有一个名称、优先级和状态。主界面显示所有任务，用户可以添加、删除、完成任务。

完成标准是使用对象树管理 UI 控件的生命周期，使用 `QSharedPointer` 管理任务对象的共享所有权，使用 `QPointer` 安全地持有对可能被删除的任务项的引用。程序退出时无内存泄漏（可以用 Valgrind 或 Qt Creator 的内置工具检测），删除任务后所有对该任务的引用都能正确处理。

几个思路供参考：定义一个 `Task` 类存储任务信息，用 `QSharedPointer<Task>` 在主窗口和任务详情窗口之间共享任务数据，用 `QPointer<QListWidgetItem>` 安全地持有列表项的引用，记得在析构函数中打印调试信息来验证对象被正确删除。

## 6. 官方文档参考

[Object Trees & Ownership | Qt Core 6.10.2](https://doc.qt.io/qt-6/objecttrees.html) - Qt 对象树与所有权机制的官方文档

[QSharedPointer Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qsharedpointer.html) - QSharedPointer 完整参考

[QWeakPointer Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qweakpointer.html) - QWeakPointer 完整参考

[QScopedPointer Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qscopedpointer.html) - QScopedPointer 完整参考

（链接已验证，2026-03-17 可访问）

---

到这里，Qt 的内存管理基础就讲完了。掌握了对象树和智能指针，你的 Qt 程序就已经能够避免大部分内存相关的 bug。下一节我们会深入 Qt 的事件系统，这是理解 Qt 程序运行机制的关键。
