# 现代Qt开发教程（新手篇）3.68——QErrorMessage：错误消息对话框

## 1. 前言 / 不是所有错误都需要弹窗轰炸用户

上两节我们聊了 QFileDialog 和 QProgressDialog，一个是文件操作的标准入口，一个是长任务的标准反馈。这一节轮到 QErrorMessage——Qt 提供的一个专门的错误消息展示对话框。你可能会问：上一节我们已经有了 QMessageBox::critical() 可以弹错误提示，为什么还需要一个专门的 QErrorMessage？这个问题问得好，答案在于两者的定位不同。

QMessageBox::critical() 是一个"一次性"的错误弹窗——每次调用都会弹出一个新的对话框，用户必须点击 OK 才能继续。这种模式适合致命错误——程序遇到了无法继续的情况，必须停下来让用户知道。但不是所有错误都这么严重。有些错误是"非致命的"——某个配置项读取失败但可以用默认值、某个可选模块加载失败但不影响核心功能、某个网络请求超时但可以重试。这类错误如果每次都用 QMessageBox 弹窗，用户会被无休止的弹窗轰炸到崩溃——特别是当同一个错误在短时间内反复出现的时候。

QErrorMessage 就是来解决这个问题的。它的核心特性是内置了一个"不再显示这个消息"的复选框——用户可以勾选"以后别再烦我这个错误"，之后同一个错误消息就不会再弹出了。这种"可抑制的非致命错误通知"机制在很多大型应用中都有——IDE 的警告日志、浏览器的控制台错误、操作系统的系统日志，都是类似的思路：错误发生了系统知道，用户第一次看到也知道了，但如果用户选择忽略，以后就不打扰。

除了基本的 showMessage 方法之外，QErrorMessage 还可以和 Qt 的全局消息处理器 qInstallMessageHandler 结合使用，把 qDebug / qWarning / qCritical / qFatal 的输出重定向到 QErrorMessage 的对话框中。这种组合让你在开发调试阶段可以把所有警告和错误都可视化展示，同时又能通过"不再显示"机制过滤掉重复的噪音。

今天我们从四个方面展开。先看 showMessage 的基本用法，然后讨论"不再显示"复选框的记忆机制，接着研究如何与 qInstallMessageHandler 结合捕获全局错误，最后看看 QErrorMessage 的适用场景和局限性。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QErrorMessage 在 QtWidgets 模块中，qInstallMessageHandler 在 QtCore 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QErrorMessage、QApplication、QMainWindow、QPushButton、QLabel、QVBoxLayout、QTextEdit、QMessageBoxHandler（全局消息处理函数）和 QDebug。

## 3. 核心概念讲解

### 3.1 showMessage：显示错误消息

QErrorMessage 的基本用法非常简单——创建一个 QErrorMessage 实例，调用 showMessage 方法：

```cpp
QErrorMessage *errorMsg = new QErrorMessage(this);
errorMsg->showMessage("无法加载配置文件 config.ini，将使用默认配置");
```

showMessage 接受一个 QString 参数作为错误消息内容。对话框弹出后会显示消息文字、一个"不要再显示"复选框和一个 OK 按钮。用户点击 OK 后对话框关闭，但如果用户勾选了"不要再显示"，下次对同一条消息调用 showMessage 时对话框就不会再弹出了。

QErrorMessage 默认是一个非模态对话框——调用 showMessage 后代码会立即继续执行，不会阻塞等待用户点击 OK。这和 QMessageBox::critical() 的模态行为不同。非模态的设计是合理的：非致命错误不应该阻塞用户的工作流，用户可以在方便的时候再看错误信息。

QErrorMessage 还提供了一个静态方法 qtHandler()，它返回一个全局的 QErrorMessage 实例，专门用于和 qInstallMessageHandler 配合。我们在 3.3 节会详细讨论这个方法。

另一个需要注意的地方是 QErrorMessage 的复用。你可以对同一个 QErrorMessage 实例多次调用 showMessage 来显示不同的错误消息——每次调用都会在对话框中追加一条消息。但如果前一条消息还没被用户确认就调用了 showMessage，新的消息会排队等待——QErrorMessage 内部有一个消息队列，逐条展示给用户。如果用户对某条消息勾选了"不再显示"，这条消息的后续 showMessage 调用会被静默跳过。

showMessage 有两个重载版本。第一个版本接受一个 QString 参数，就是上面演示的基本用法。第二个版本额外接受一个 QString 类型的 type 参数，用于消息分类：

```cpp
errorMsg->showMessage("网络连接超时", "NetworkError");
errorMsg->showMessage("磁盘空间不足", "DiskError");
```

type 参数和消息文字一起构成了"不再显示"的判重键——即使两条消息的文字完全相同但 type 不同，它们也会被视为两条独立的消息，用户可以分别选择是否抑制。如果你不传 type（使用第一个重载版本），默认的 type 是空字符串，所有没有 type 的消息共享同一个命名空间。

### 3.2 "不再显示"复选框的记忆机制

QErrorMessage 的"不再显示"功能是通过 QSettings 实现持久化的。当用户勾选了复选框并点击 OK 后，QErrorMessage 会把这条消息的内容（以及 type，如果有的话）写入 QSettings。下次再对同一条消息调用 showMessage 时，QErrorMessage 会先检查 QSettings 中是否已经标记了"不再显示"，如果是就跳过弹窗直接返回。

这个持久化机制意味着"不再显示"的选择在应用重启后依然有效——用户今天勾选了"不再显示：网络连接超时"，明天重新打开应用，同样的错误不会再弹出。这既是优点也是潜在的问题。优点是用户体验连贯——他不需要每次打开应用都重新设置一遍。问题在于如果用户误勾选了"不再显示"（或者错误的情况发生了变化），他需要手动清除 QSettings 中的记录才能重新看到那条消息。

QErrorMessage 使用的是应用默认的 QSettings（组织名和应用名由 QCoreApplication::setOrganizationName 和 QCoreApplication::setApplicationName 设置）。存储的键值格式是 Qt 内部实现细节，文档中没有明确说明。如果你想清除所有"不再显示"的记录，最直接的方式是删除 QSettings 中对应的键组：

```cpp
QSettings settings;
settings.remove("QtErrorMessageList");
```

QtErrorMessageList 是 QErrorMessage 在 QSettings 中使用的键名。删除这个键后，所有之前被抑制的消息都会恢复弹出。你也可以通过 QSettings 的 API 来查看哪些消息被抑制了——但这依赖于 Qt 的内部实现细节，不推荐在生产代码中这样做。

一个常见的需求是提供"重置所有被抑制的错误消息"的功能——比如在设置界面中加一个按钮"重置错误提示"。上面的 QSettings::remove 调用就是实现这个功能的核心代码。但需要注意：如果你有一个正在显示的 QErrorMessage 实例，它在内存中也维护了一份被抑制消息的缓存。QSettings 中删除了记录不会影响内存中的缓存——你需要重新创建 QErrorMessage 实例或者调用它的某些内部方法来刷新缓存。最简单的做法是 delete 掉旧的 QErrorMessage 再 new 一个新的。

另一个需要注意的地方是"不再显示"复选框的文案。在 Qt 6 中这个复选框的默认文字是 "Do not show again" 或者"不再显示这个消息"（取决于系统语言）。这个文案不可自定义——QErrorMessage 没有提供设置复选框文字的接口。如果你需要自定义文案或者自定义"不再显示"的行为（比如只在本次会话中抑制而不是持久化），你需要自己用 QDialog + QCheckBox + QTextBrowser 来实现。

### 3.3 与 qInstallMessageHandler 结合捕获全局错误

qInstallMessageHandler 是 Qt 提供的全局消息处理机制。Qt 内部（以及你的代码中）所有通过 qDebug()、qWarning()、qCritical()、qFatal() 输出的消息都会经过这个消息处理器。默认的消息处理器只是把消息打印到 stderr，但你可以安装自定义的处理器来改变消息的目的地——比如写入日志文件、发送到远程服务器、或者弹出一个 QErrorMessage。

QErrorMessage 提供了一个静态方法 qtHandler() 来方便这种集成。qtHandler() 返回一个全局的 QErrorMessage 实例，同时安装一个全局消息处理器，把 qWarning 和 qCritical 的消息转发到这个 QErrorMessage 实例上：

```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 安装 QErrorMessage 作为全局消息处理器
    QErrorMessage::qtHandler();

    // 之后所有的 qWarning 和 qCritical 都会弹窗
    qWarning() << "这是一条警告消息";
    qCritical() << "这是一条严重错误";

    return app.exec();
}
```

qtHandler() 做了两件事：首先创建一个全局的 QErrorMessage 实例（如果还没创建的话），然后调用 qInstallMessageHandler 安装一个内部的消息处理器。这个处理器会检查消息的类型——QtDebugMsg 被忽略（调试消息太多了，全部弹窗会疯掉），QtWarningMsg 和 QtCriticalMsg 通过 showMessage 弹窗展示，QtFatalMsg 会调用 abort() 终止程序（这是 qFatal 的默认行为，QErrorMessage 不会改变它）。

安装了 qtHandler() 之后，你的代码中所有通过 qWarning() 和 qCritical() 输出的消息都会自动弹窗——包括 Qt 内部的警告。这意味着你可能会看到一些你之前不知道的 Qt 内部警告弹出来——比如某个属性名拼写错误、某个信号连接失败、某个图像格式不支持。这些警告在没安装 qtHandler() 的时候只是默默打印到 stderr，很容易被忽略。安装之后它们就变得"可见"了，帮你发现代码中潜伏的问题。

但这里有一个权衡需要考虑。qtHandler() 把所有警告都弹窗展示，在生产环境中可能会让用户感到困惑——用户看到了一堆他看不懂的内部警告，不知道该怎么办。所以 qtHandler() 更适合用在开发和调试阶段。在生产环境中，你可以安装自定义的消息处理器，只把你认为用户需要知道的错误转发给 QErrorMessage，其他消息走日志系统：

```cpp
QErrorMessage *errorDialog = nullptr;

void customMessageHandler(
    QtMsgType type,
    const QMessageLogContext &context,
    const QString &msg)
{
    // 所有消息都写入日志文件
    writeToFile(type, context, msg);

    // 只有严重错误才弹窗
    if (type == QtCriticalMsg && errorDialog) {
        errorDialog->showMessage(msg);
    }
}
```

这种自定义处理器给了你完全的控制权——你可以根据消息类型、来源文件、函数名等条件决定哪些消息弹窗、哪些消息只写日志、哪些消息完全忽略。context 参数包含了消息的来源信息（文件名、行号、函数名），你可以用它来实现更精细的过滤策略。

还有一个需要注意的点：qInstallMessageHandler 返回的是上一个消息处理器的函数指针。你可以保存这个返回值，在自定义处理器中调用它来形成处理链——先做你的自定义处理，然后调用原来的处理器保持默认行为（打印到 stderr）。这样你的日志系统和 Qt 的默认输出都正常工作：

```cpp
QtMessageHandler previousHandler = nullptr;

void chainedMessageHandler(
    QtMsgType type,
    const QMessageLogContext &context,
    const QString &msg)
{
    // 自定义处理：写入日志
    writeToFile(type, context, msg);

    // 对特定错误弹 QErrorMessage
    if (type == QtCriticalMsg && errorDialog) {
        errorDialog->showMessage(msg);
    }

    // 调用上一个处理器（保持默认的 stderr 输出）
    if (previousHandler) {
        previousHandler(type, context, msg);
    }
}

// 安装
previousHandler = qInstallMessageHandler(
    chainedMessageHandler);
```

### 3.4 适用场景与局限性

QErrorMessage 适合的场景是"可抑制的非致命错误通知"。拆开来看这三个限定词。"可抑制"意味着用户有权选择不再看到这条错误——如果你有一个错误用户必须每次都看到（比如"密码错误"），那不应该用 QErrorMessage 而应该用 QMessageBox::warning。"非致命"意味着程序可以继续运行——如果错误导致程序无法继续（比如关键配置文件损坏、数据库连接失败且没有 fallback），用 QMessageBox::critical 更合适。"错误通知"意味着这是告诉用户"出了点问题"，而不是请求用户做决策——如果需要用户选择"重试还是取消"，QMessageBox::question 是正确的选择。

具体的应用场景包括：配置文件解析错误（用默认值继续运行）、可选插件加载失败（核心功能不受影响）、非关键网络请求失败（可以后台重试）、文件格式兼容性警告（可以降级处理）、第三方服务调用失败（有本地缓存可用）。

QErrorMessage 也有明显的局限性需要了解。第一个局限性是消息内容的格式——showMessage 只接受纯文本，不支持 HTML 或者富文本格式。如果你需要显示格式化的错误信息（比如带颜色的日志、可点击的链接、嵌入的代码块），需要自己用 QTextBrowser 或者 QLabel 来实现。

第二个局限性是缺乏消息分级。QErrorMessage 不区分"警告"和"严重错误"——所有消息都是同一级别。如果你需要让用户对不同级别的错误有不同的"不再显示"策略（比如警告可以抑制但严重错误不能），需要自己实现。

第三个局限性是"不再显示"的粒度太粗。它只按消息文字和 type 来判重——如果你的错误消息包含动态内容（比如"无法连接到 192.168.1.100:8080"），每次 IP 和端口不同都被视为不同的消息，"不再显示"就失效了。解决方案是把动态内容从消息中剥离出来，只保留固定的模板部分作为判重键，或者使用 type 参数来分类。

## 4. 踩坑预防

第一个坑是在多线程中调用 showMessage。QErrorMessage 是一个 QWidget，所有 QWidget 的操作都必须在主线程中执行。如果你的后台线程检测到错误需要通过 QErrorMessage 通知用户，不能直接调用 showMessage——需要通过信号槽（自动队列连接）把错误消息从后台线程传递到主线程，在主线程中调用 showMessage。

第二个坑是 qtHandler() 的全局影响。安装了 qtHandler() 后，所有 qWarning 和 qCritical 都会弹窗——包括 Qt 内部的、第三方库的、你自己的。Qt 内部的警告有时候非常啰嗦（比如 QStyleSheet 解析错误，一次能弹十几条），这些弹窗在调试阶段有用但在正常运行时会严重干扰用户。所以在生产环境中不要直接用 qtHandler()，而是安装自定义的消息处理器做过滤。

第三个坑是"不再显示"状态的清除。QErrorMessage 把被抑制的消息列表存在 QSettings 中，如果你的应用没有正确设置 QCoreApplication::setOrganizationName 和 setApplicationName，QSettings 可能使用了默认值，导致不同应用之间的"不再显示"记录互相干扰。确保在创建 QErrorMessage 之前就设置好了组织名和应用名。

第四个坑是 QErrorMessage 实例的生命周期。如果你用 new 创建了一个 QErrorMessage 并传了 parent，它会在 parent 销毁时自动销毁。但如果你用了 qtHandler() 返回的全局实例，它的生命周期由 Qt 内部管理——它在 QApplication 销毁时被清理。不要手动 delete qtHandler() 返回的实例。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，中央是一个 QTextEdit 用于显示日志输出，上方是工具栏按钮。"触发错误"按钮调用 QErrorMessage::showMessage 显示一条非致命错误。"重复错误"按钮连续三次调用 showMessage 显示同一条消息——第一次会弹出，用户勾选"不再显示"后，后续两次调用被静默跳过。"不同类型"按钮调用 showMessage 的重载版本，分别显示两条文字相同但 type 不同的消息，验证它们被视为独立的错误。"安装全局处理器"按钮调用 qInstallMessageHandler 安装自定义处理器——qDebug 只写日志不弹窗、qWarning 写日志并弹 QErrorMessage、qCritical 写日志并弹 QErrorMessage。"重置抑制列表"按钮清除 QSettings 中的 QtErrorMessageList 键，恢复所有被抑制的错误消息。

QTextEdit 用于显示所有通过自定义消息处理器处理的消息，充当一个简易的日志查看器。QErrorMessage 实例作为主窗口的成员变量，在整个生命周期内复用。

提示：自定义消息处理器中用 QMetaObject::invokeMethod 把错误消息投递到主线程的 QErrorMessage。重置抑制列表后需要 delete 旧的 QErrorMessage 实例并创建新实例，因为旧实例的内存缓存中还有被抑制消息的记录。

## 6. 官方文档参考链接

[Qt 文档 -- QErrorMessage](https://doc.qt.io/qt-6/qerrormessage.html) -- 错误消息对话框类

[Qt 文档 -- QErrorMessage::showMessage](https://doc.qt.io/qt-6/qerrormessage.html#showMessage) -- 显示错误消息方法

[Qt 文档 -- QErrorMessage::qtHandler](https://doc.qt.io/qt-6/qerrormessage.html#qtHandler) -- 全局消息处理器安装

[Qt 文档 -- qInstallMessageHandler](https://doc.qt.io/qt-6/qtglobal.html#qInstallMessageHandler) -- 全局消息处理函数

[Qt 文档 -- QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 持久化设置类

---

到这里，QErrorMessage 的核心用法就全部讲完了。showMessage 提供了带"不再显示"复选框的错误消息展示，QSettings 持久化确保抑制记录在应用重启后依然有效，qInstallMessageHandler 配合自定义处理器让你可以全局捕获并分类处理 Qt 的调试消息，而"可抑制的非致命错误通知"的定位让 QErrorMessage 在错误处理工具箱中占据了一个不可替代的位置——它填补了"致命错误弹 QMessageBox"和"静默写日志"之间的空白地带。
