# 现代Qt开发教程（新手篇）2.6——拖放系统基础

## 1. 前言 / 为什么需要拖放

说实话，拖放这个功能在很多应用里看起来像是"锦上添花"，但真正做项目的时候你会发现它几乎是刚需。想想看，文件管理器里把文件从一个文件夹拖到另一个文件夹、图片编辑器里拖入一张图片直接打开、看板工具里拖动任务卡片改变状态——这些操作如果没有拖放支持，用户体验会瞬间回到上个世纪。

Qt 的拖放系统设计得相当完整，但完整也意味着概念多。你需要理解的东西包括：怎么让一个 Widget 接受拖入的数据、怎么从你的 Widget 发起一次拖放操作、怎么用 QMimeData 在拖放过程中携带各种格式的数据。这些东西拼在一起，就是一套完整的拖放交互链路。

OK，咱们那就开始这次的学习吧！

## 2. 环境说明

本篇代码适用于 Qt 6.5+ 版本，CMake 3.26+，C++17 或更高标准。拖放相关的类分布在 QtGui（QDrag、QMimeData）和 QtWidgets（QWidget 的 dragEnterEvent 等）两个模块中，所以示例代码需要同时链接这两个模块。桌面平台上均可正常编译运行，但需要注意的是 Qt 的拖放功能依赖于底层窗口系统的支持，在无窗口系统的嵌入式环境下行为可能不同。

## 3. 核心概念讲解

### 3.1 启用拖放接收：setAcceptDrops

默认情况下，QWidget 是不接受拖放操作的。如果你不做任何设置，当用户把东西拖到你的 Widget 上方时，光标会显示一个"禁止"图标，表示这里不允许放下。要让一个 Widget 变成合法的拖放目标，第一步就是在构造函数里调用 `setAcceptDrops(true)`。

```cpp
class DropWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DropWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAcceptDrops(true);  // 这是第一步，没有这一步后面全白搭
        setMinimumSize(300, 200);
    }
};
```

这个设置的含义很直白：告诉 Qt 的事件系统，这个 Widget 对拖进来的东西感兴趣，请把相关的拖放事件分发给我。如果你忘了这一步，后面不管怎么重写 `dragEnterEvent` 和 `dropEvent`，它们都不会被调用——因为 Qt 在事件分发的早期阶段就帮你把这个事件忽略掉了。

### 3.2 dragEnterEvent 和 dropEvent 处理拖放接收

`setAcceptDrops(true)` 只是开了门，真正的逻辑需要我们通过重写两个事件处理函数来实现。

第一个是 `dragEnterEvent(QDragEnterEvent *event)`。这个事件在拖拽操作进入 Widget 区域时触发，你需要在这里决定：这个拖进来的东西我收不收？判断的依据通常是检查拖放数据中包含的 MIME 类型。如果决定接受，就调用 `event->acceptProposedAction()`；如果不接受，什么都不做就行，Qt 会自动显示禁止图标。

```cpp
void dragEnterEvent(QDragEnterEvent *event) override
{
    // 检查拖入的数据是否包含我们想要的格式
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();  // 告诉 Qt：这个我要了
    }
    // 如果不调用 acceptProposedAction()，Qt 会认为你拒绝了这次拖放
}
```

第二个是 `dropEvent(QDropEvent *event)`。这个事件在用户松开鼠标、真正把东西放下时触发。到这一步说明 `dragEnterEvent` 已经放行了，所以你可以放心地从 `event->mimeData()` 里提取数据。

```cpp
void dropEvent(QDropEvent *event) override
{
    QString text = event->mimeData()->text();
    qDebug() << "收到拖放文本:" << text;
    // 把文本存起来，然后触发重绘或更新 UI
    m_droppedText = text;
    update();
    event->acceptProposedAction();  // 确认接收
}
```

你可能会注意到，这两个函数里都调用了 `acceptProposedAction()`。这个函数的作用是告诉 Qt：我接受了这次拖放，并且同意执行拖放源提议的动作（复制、移动还是链接）。如果你想在接收端改变动作类型，可以用 `event->setDropAction(Qt::CopyAction)` 先覆盖，再 accept。

还有一个事件你可能偶尔会用到：`dragMoveEvent(QDragMoveEvent *event)`。它在拖拽过程中鼠标在 Widget 内移动时持续触发。大多数简单场景你不需要重写它，但在某些情况下——比如你想根据鼠标位置高亮显示某个区域——这个事件就派上用场了。

```cpp
void dragMoveEvent(QDragMoveEvent *event) override
{
    // 可以根据 event->pos() 判断鼠标在哪个区域
    // 用于实现拖放时的视觉反馈
    event->acceptProposedAction();
}
```

### 3.3 用 QDrag 发起拖放操作

上面讲的是"接收端"的逻辑，现在我们看看"发送端"——怎么让你的 Widget 成为一个拖放源，让用户可以从这里拖走数据。

发起拖放的标准做法是在 `mousePressEvent` 或 `mouseMoveEvent` 中创建一个 `QDrag` 对象。通常我们会选择在 `mouseMoveEvent` 里做这件事，因为这样可以加入一个移动阈值，避免用户只是想点击结果触发了拖拽。

```cpp
void mouseMoveEvent(QMouseEvent *event) override
{
    // 检查移动距离是否超过了拖拽阈值
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    if ((event->pos() - m_dragStartPos).manhattanLength()
         < QApplication::startDragDistance()) {
        return;  // 移动距离太短，不算拖拽
    }

    // 创建拖放对象
    QDrag *drag = new QDrag(this);

    // 创建 MIME 数据并设置内容
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(m_textToDrag);
    drag->setMimeData(mimeData);

    // 可选：设置拖拽时跟随鼠标的缩略图
    // drag->setPixmap(QPixmap::fromImage(someImage));

    // 执行拖放操作，这个调用会阻塞直到拖放结束
    Qt::DropAction result = drag->exec(Qt::CopyAction | Qt::MoveAction);

    qDebug() << "拖放结果:" << result;
    // result 是 Qt::CopyAction / Qt::MoveAction / Qt::IgnoreAction 之一
}
```

这里有几个要点需要理清。首先，`QDrag` 和 `QMimeData` 的父子关系是通过 `setMimeData` 自动建立的——`QMimeData` 的所有权会转移给 `QDrag`，所以你不需要手动 delete。`QDrag::exec()` 是一个阻塞调用，它会启动一个局部的嵌套事件循环来处理整个拖拽过程，直到用户松开鼠标或取消操作才返回。返回值告诉你最终执行了什么动作。

另外别忘了在 `mousePressEvent` 里记录起始位置，这是判断拖拽阈值的基础：

```cpp
void mousePressEvent(QMouseEvent *event) override
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
    }
    QWidget::mousePressEvent(event);
}
```

### 3.4 QMimeData：拖放数据的载体

QMimeData 是拖放系统中的数据容器，它用 MIME 类型来标识数据格式。Qt 内置支持了几种常见的 MIME 类型，你可以通过对应的便利函数直接访问：`hasText()` / `text()` 处理纯文本、`hasUrls()` / `urls()` 处理文件路径列表、`hasHtml()` / `html()` 处理 HTML 富文本、`hasImage()` / `imageData()` 处理图片数据。

处理纯文本拖放是最简单的场景，比如我们把一段文字从一个 Widget 拖到另一个 Widget：

```cpp
// 发送端
QMimeData *mimeData = new QMimeData;
mimeData->setText("Hello from drag source!");
drag->setMimeData(mimeData);

// 接收端
void dropEvent(QDropEvent *event) override
{
    if (event->mimeData()->hasText()) {
        QString received = event->mimeData()->text();
        // 处理接收到的文本...
    }
}
```

文件拖放稍微特殊一些。当用户从系统文件管理器拖入文件时，MIME 类型是 `text/uri-list`，Qt 把它封装成了 `QList<QUrl>`。你需要在 `dragEnterEvent` 里检查 `hasUrls()`，然后在 `dropEvent` 里通过 `toLocalFile()` 把 URL 转成本地路径：

```cpp
void dragEnterEvent(QDragEnterEvent *event) override
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void dropEvent(QDropEvent *event) override
{
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        if (url.isLocalFile()) {
            QString filePath = url.toLocalFile();
            qDebug() << "收到文件:" << filePath;
        }
    }
    event->acceptProposedAction();
}
```

当你需要传递自定义数据时，可以使用 `setData(const QString &mimeType, const QByteArray &data)` 来设置任意 MIME 类型的数据。比如你想传递一个自定义的应用内数据格式：

```cpp
// 发送端：自定义 MIME 类型
QMimeData *mimeData = new QMimeData;
mimeData->setData("application/x-myapp-task", taskData.toByteArray());
drag->setMimeData(mimeData);

// 接收端：检查自定义 MIME 类型
void dragEnterEvent(QDragEnterEvent *event) override
{
    if (event->mimeData()->hasFormat("application/x-myapp-task")) {
        event->acceptProposedAction();
    }
}
```

按照惯例，自定义 MIME 类型应该使用 `application/x-` 前缀加上你的应用名和具体数据类型，这样不会和标准 MIME 类型冲突。

到这里你可以停下来想一想：一次完整的拖放操作中，发送端和接收端各自需要做什么？QMimeData 在两者之间扮演什么角色？把这三件事——启用接收、处理事件、携带数据——在脑子里串成一条链路，后面的实战练习就会顺很多。

## 4. 踩坑预防

第一个坑是忘了调 `setAcceptDrops(true)`。这个太常见了，你把 `dragEnterEvent` 和 `dropEvent` 都重写好了，拖拽的时候就是不触发，调试半天才发现构造函数里少了这一行。Qt 的设计是：如果一个 Widget 没有声明自己接受拖放，系统就不会把拖放事件分发给它，你的重写函数根本不会被调用。

第二个坑是 `QDrag::exec()` 的阻塞行为。前面说了，这个函数会启动一个嵌套事件循环。这意味着在你的 `exec()` 返回之前，外层函数不会继续往下走。如果你把它放在一个不合适的时机调用——比如在一个正在执行的动画回调里——可能会产生意料之外的递归事件处理。绝大多数简单场景下这不是问题，但如果你有复杂的事件逻辑，这一点需要留意。

第三个坑跟文件拖放有关：从文件管理器拖入的 URL 是用 `QUrl` 表示的，而不是直接的文件路径字符串。如果你直接用 `event->mimeData()->text()` 去读取文件路径，大多数情况下你会得到一个空字符串或者一堆 URL 编码后的路径。正确的做法是用 `urls()` 获取 `QUrl` 列表，然后对每个 URL 调用 `toLocalFile()` 转成本地文件路径。

第四个坑是拖拽阈值。如果你直接在 `mousePressEvent` 里发起 `QDrag`，用户每次点击都会触发拖拽——连正常的点击选中都做不到。标准做法是在 `mouseMoveEvent` 里判断移动距离是否超过 `QApplication::startDragDistance()`（通常 4 像素），只有超过阈值才算真正的拖拽。

接下来做一个练习：假设你要实现一个简单的看板，卡片可以从"待办"列拖到"完成"列。思考一下：卡片 Widget 的发送端需要做什么？列 Widget 的接收端需要检查什么？拖放的数据应该用什么格式携带？

## 5. 练习项目

我们来做一个实战练习：创建一个包含两个文本框的窗口，支持从一个文本框拖拽文字到另一个文本框。这听起来简单，但它涵盖了拖放系统的完整流程——发起拖拽、携带数据、接收判断、提取数据。

完成标准是：创建一个 `DragSourceWidget` 作为拖拽源，继承 QWidget，在构造函数中设置一段默认文本并显示，支持鼠标拖拽发起操作，拖拽时携带文本数据；创建一个 `DropTargetWidget` 作为拖放目标，同样继承 QWidget，构造函数中调用 `setAcceptDrops(true)`，重写 `dragEnterEvent` 接受文本类型的拖放，重写 `dropEvent` 提取文本并显示在 Widget 上；主窗口把这两个 Widget 水平排列，中间用标签标注"拖拽源"和"接收目标"。

几个提示：`DragSourceWidget` 需要重写 `mousePressEvent` 和 `mouseMoveEvent`，前者记录起始位置，后者判断阈值并创建 `QDrag`；`DropTargetWidget` 接收到文本后可以用 `update()` 触发 `paintEvent` 把文本画出来，也可以用 QLabel 显示；拖拽源的文本可以用半透明的 `QPixmap` 作为拖拽预览图，视觉效果会好很多。

## 6. 官方文档参考链接

[Qt 文档 · Drag and Drop](https://doc.qt.io/qt-6/dnd.html) -- Qt 拖放系统的完整概述，包含拖放操作的完整流程和各平台注意事项

[Qt 文档 · QDrag Class](https://doc.qt.io/qt-6/qdrag.html) -- 拖放操作发起端的 API 文档，包含 exec()、setMimeData()、setPixmap() 等方法说明

[Qt 文档 · QMimeData Class](https://doc.qt.io/qt-6/qmimedata.html) -- MIME 数据容器的完整文档，涵盖内置类型便利函数和自定义 MIME 类型的使用

[Qt 文档 · QDragEnterEvent](https://doc.qt.io/qt-6/qdragenterevent.html) -- 拖拽进入事件文档，解释 acceptProposedAction() 的行为

[Qt 文档 · QDropEvent](https://doc.qt.io/qt-6/qdropevent.html) -- 放下事件文档，包含 drop action 和 MIME 数据提取的详细说明

---

到这里，QtGui 部分的六篇基础教程就全部完成了。拖放系统是 GUI 编程中非常实用的交互模式，掌握了接收端和发送端两套流程、理解了 QMimeData 作为数据载体的设计，后面不管遇到什么样的拖放需求，你都有能力把它拆解成这几个标准步骤来实现。接下来的 QtWidgets 篇章我们会进入布局系统和事件处理，那才是真正构建复杂界面的开始。
