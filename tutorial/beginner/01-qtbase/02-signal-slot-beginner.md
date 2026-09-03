---
title: "1.2 信号与槽"
description: "Qt 对象间解耦通信的机制：信号是事件声明、槽是可调用目标，新式
函数指针 connect 自带编译期检查，跨线程时 Qt 自动把参数打包排队投递到目标线程。
四个坑各有各的崩法——Lambda 捕获野指针的偶发崩溃、漏 Q_OBJECT 信号连不上、
工作线程直接碰 GUI、重载信号连接歧义。配套可跑示例在
examples/beginner/01-qtbase/02-signal-slot-beginner/。"
---

# 现代Qt开发教程（新手篇）1.2——信号与槽

一个按钮被点击之后，该让谁知道这件事？最直接的写法：让按钮持有窗口的指针，点击时直接调窗口的函数。能跑，但代价是两头互相知道——按钮得清楚"谁"会响应它，窗口也得知道按钮的类型，换个窗口按钮跟着改，换个按钮窗口跟着改。

Qt 的答案是信号与槽。按钮只说"我被点击了"，谁来听、听完做什么，它一概不知；窗口只说"我关心点击事件"。两边互不相识却能协作，中间的连接交给元对象系统，咱们只要告诉它"把这两个连起来"。

从回调函数过来的朋友可能会问：注册个回调不也能解耦？能，但解耦只是表层收益。信号槽真正值钱的地方是跨线程天生安全——参数打包、投递到目标线程、排队执行，这些 Qt 全包了；再加上自动断开（对象销毁连接跟着断）和一发多收。这些下文都会用到。

其实笔者认为，信号槽的核心在于：发射者不需要知道接收者的存在。上一篇 [1.1 QObject 与元对象系统](./01-qobject-meta-system-beginner.md) 把 Q_OBJECT 与对象树备好了课，本篇专心讲通信本身。

## 信号与槽是什么：一个只声明，一个真干活

信号（Signal）是一句"事件声明"——按钮被点击、滑块值变、数据加载完成，声明本身不含任何实现；槽（Slot）是一个可调用目标，成员函数、静态函数、Lambda 都行。信号一响，连到它的槽全部被调用。

```cpp
class Counter : public QObject {
    Q_OBJECT

public:
    Counter(QObject *parent = nullptr) : m_value(0) {}

    void setValue(int value);

signals:
    void valueChanged(int newValue);

public slots:
    void printValue() const {
        qDebug() << "Current value:" << m_value;
    }

private:
    int m_value;
};
```

您在八竿子打不着的另一个类里定义槽、连上这个信号，照样工作——声明和定义完全分开，是这套机制松耦合的根源。signals: 这个关键字本质是 public（moc 预处理时会特殊对待），Qt 用它标记"这一段是信号"，属于约定成俗。

这份约定生效有个前提：moc 真的处理了这个类。图省事不写 Q_OBJECT 的话，约定落空——不少人先写 signals 和 connect，想着"跑起来再补"，结果编译通过、信号永远连不上，因为没被 moc 处理的类，signals: 下面就是一串普通成员函数声明，怎么连都不响。继承 QObject 的第一件事就是加 Q_OBJECT，没有例外；根子（moc 的扫描机制）[1.1 篇](./01-qobject-meta-system-beginner.md) 拆过了，这里不重讲。

## connect：让编译器替咱们守门

```cpp
Counter counter;
connect(&counter, &Counter::valueChanged,
        &app,    &QApplication::quit);
```

counter 发射 valueChanged 时，app 的 quit 被调用。注意咱们没写参数类型——新式语法用函数指针，参数匹配检查交给编译器：信号带的参数槽收不下，直接编译失败。这种错在编译期报，比运行期崩友好太多。一次性的小逻辑不必单写槽函数，Lambda 直接上：

```cpp
connect(&counter, &Counter::valueChanged, [](int newValue) {
    qDebug() << "Value changed to:" << newValue;
});
```

今年都 Qt6 了，网上还能刷到大量 SIGNAL/SLOT 宏的老写法：

```cpp
// 请不要这样写了，求你了
connect(sender, SIGNAL(valueChanged(int)),
        receiver, SLOT(onValueChanged(int)));
```

宏语法没有上面那道编译期检查，错误全拖到运行期才爆。比如手一抖把 valueChanged 打成 valuChanged：编译通过，运行不报错，信号发了槽永远不来，排查起来能耗掉一下午，最后发现只是少了个 e。同样的拼写错误放在函数指针语法下，编译期直接报错，定位就在那一行。

函数指针语法自己也有个小门槛：重载信号。QSlider 的 valueChanged 有重载时，`&QSlider::valueChanged` 让编译器犯了难——有好几个重载，您指的是哪个？直接报编译错误。用函数指针变量把地址取出来，或者 QOverload 显式指定，都行：

```cpp
// 错误：编译器无法确定是哪个重载
connect(slider, &QSlider::valueChanged, [](int value) {
    // 编译错误！
});

// 正确方式一：显式指定函数指针类型
void (QSlider::*valueChangedSignal)(int) = &QSlider::valueChanged;
connect(slider, valueChangedSignal, [](int value) {
    qDebug() << value;
});

// 正确方式二：用 QOverload
connect(slider, QOverload<int>::of(&QSlider::valueChanged), [](int value) {
    qDebug() << value;
});
```

## 同步、异步与跨线程：谁来排队，Qt 说了算

发射者和接收者同线程，默认同步——发射处阻塞到槽执行完，跟直接调函数没区别。跨线程时 Qt 自动转异步：发射立即返回，参数打包送到接收者线程排队执行。想显式控制也有：

```cpp
// 强制同步（直接调用）
connect(sender, &Sender::signal,
        receiver, &Receiver::slot,
        Qt::DirectConnection);

// 强制异步（排队执行）
connect(sender, &Sender::signal,
        receiver, &Receiver::slot,
        Qt::QueuedConnection);
```

绝大多数场景让 Qt 自动判断就好。"自动"背后怎么认线程归属，是 [进阶篇 1.02](../../advanced/01-qtbase/02-signal-slot-advanced.md) 和 [专家篇 1.02 源码拆解](../../expert/01-qtbase/02-signal-slot-internals-expert.md) 的主菜，这里只要求会用，机制细节留给上面两篇。

异步这一端有个容易被忽视的前提：排队执行靠的是事件循环在转。空口说不直观，咱们动手看：配套示例在 `examples/beginner/01-qtbase/02-signal-slot-beginner/`（`cmake -B build && cmake --build build` 跑起来，七段输出对应七种用法）。做个实验：示例 5 里等定时器的那行 `QCoreApplication::processEvents();` 注释掉再跑，程序卡死在示例 5——事件不派发，`QTimer::singleShot` 的回调永远不来，`timerDone` 永远不翻转，while 循环空转。排队调用能不能落地，取决于事件循环在不在转，这个实验比文字描述直观。

跨线程还有一条 Qt 的硬规定：所有 GUI 操作必须在主线程。您在工作线程里直接 `label->setText("Done")`，可能崩、可能界面诡异、也可能暂时没事然后在某个不可预测的时刻出错——而且崩溃位置常常不在碰 GUI 的那一行，排查难度翻倍。正确做法就是用信号槽：工作线程发信号，主线程的槽更新 UI，跨线程排队 Qt 自动做。

```cpp
// 错误：在工作线程中直接操作 UI
void WorkerThread::run() {
    label->setText("Done");  // 崩！
}

// 正确：使用信号槽让 Qt 自动跨线程
class WorkerThread : public QThread {
    Q_OBJECT
signals:
    void textChanged(const QString &);
};

// 连接到主线程的槽
connect(worker, &WorkerThread::textChanged,
        label, &QLabel::setText);
```

## Lambda 作槽：捕获的对象必须活到信号发射

```cpp
QSlider *slider = new QSlider(Qt::Horizontal);
QLabel *label = new QLabel;

connect(slider, &QSlider::valueChanged, [&](int value) {
    label->setText(QString("Value: %1").arg(value));
});
```

`[&]` 把 label 的引用捕进来，直观好用，但问题也出在捕获上：捕获指针或引用时，得保证信号发射那一刻对象还活着。函数里 new 了个 QLabel、Lambda 捕获它的指针连上信号，函数返回后对象在别处被删，信号再发射时，Lambda 摸到的就是野指针——不然您会收获一个非常漂亮的 segfault。这种崩溃还是偶发的，取决于信号什么时候来、对象什么时候死，排查起来相当磨人。

根子是连接的生命周期没人管，解法两条：connect 时把对象作为 context object 传进去，对象销毁连接自动断；或者用 QPointer 包装，访问前先验活。

```cpp
// 思路一：用 context object
QLabel *label = new QLabel;
connect(slider, &QSlider::valueChanged, label, [label](int value) {
    if (label) label->setText(QString::number(value));
});

// 思路二：用 QPointer
QPointer<QLabel> safeLabel = label;
connect(slider, &QSlider::valueChanged, [safeLabel](int value) {
    if (safeLabel) safeLabel->setText(QString::number(value));
});
```

## 连接的生命周期

连接不是永生的。发送者或接收者销毁，连接自动断——对象都没了，连接留着也没用。要手动管理时，connect 返回的句柄就是把手：

```cpp
QMetaObject::Connection conn = connect(sender, &Sender::someSignal, [](int value) {
    // 一次性处理
});

// 处理完后断开
disconnect(conn);
```

重构代码、临时屏蔽某条连接时用得上，其余时候交给自动断开就好。

## 官方文档参考

文中代码与行为均在 Qt 6.2+ / C++11 验证；Qt 5 迁移对照表见[环境搭建篇](../00-environment-setup/00-qt6-install-beginner.md)。

[Qt 文档 · Signals & Slots](https://doc.qt.io/qt-6/signalsandslots.html) · 信号槽的官方完整说明，包含所有连接类型和高级用法

[Qt 文档 · QObject::connect](https://doc.qt.io/qt-6/qobject.html#connect) · connect 函数的详细重载列表和参数说明

[Qt 文档 · QMetaObject::Connection](https://doc.qt.io/qt-6/qmetaobject-connection.html) · 连接对象的生命周期管理

[Qt 文档 · Qt::ConnectionType](https://doc.qt.io/qt-6/qt.html#ConnectionType-enum) · 所有连接类型的枚举定义和说明

---

日常开发里八成的信号槽场景，本篇的内容够用了。想动手的，跑配套示例：把某条 connect 的参数改错，看编译器报什么；或按上文的实验把事件循环掐断，看程序卡在哪。下一篇 [1.3 字符串与编码](./03-string-encoding-beginner.md)，讲 QString 和编码的常见陷阱。
