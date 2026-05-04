# 现代Qt开发教程（新手篇）3.8——图形视图框架基础

## 1. 前言 / 为什么需要一套专门的图形视图框架

如果你只是需要在界面上画几个矩形和文字，QPainter + paintEvent 完全够用——我们在自定义绘制那一篇已经搞定了。但当你需要管理几十上百个可交互的图形元素，每个元素都能独立响应鼠标事件、能被拖拽移动、能叠加层级、能缩放旋转，再单纯用 paintEvent 管理就会变得非常痛苦。你需要自己维护一个图元列表、自己判断鼠标点击落在哪个图元上、自己处理图元之间的遮挡关系、自己实现选中和移动的逻辑——每一件事都不算特别难，但全部加在一起就是一座山。

Qt 的图形视图框架（Graphics View Framework）就是为了解决这个问题而存在的。它把"场景中的图元管理"抽象成了一个三层架构：QGraphicsScene 负责管理图元和处理事件分发，QGraphicsView 负责把场景渲染到屏幕上并处理用户的视图操作（缩放、平移），QGraphicsItem（以及它的各种子类）代表场景中一个个独立的图元对象。这三层各司其职，你只需要关注每一层该做的事，而不需要操心跨层的协调问题。

说实话我第一次接触这个框架的时候被它的 API 数量吓到了——QGraphicsScene 有几十个方法，QGraphicsView 更是有一百多个，QGraphicsItem 的子类加起来有二十多个。但实际写下来你会发现，日常开发用到的只是其中很小的一个子集。这篇文章我们聚焦四个核心议题：Scene/View/Item 三层架构的关系和各自职责、标准图元类型的创建和添加、三层坐标系之间的转换关系、以及鼠标事件在 Scene 层的拦截和响应机制。

## 2. 环境说明

本篇代码基于 Qt 6.5+，CMake 3.26+，C++17 标准。图形视图框架属于 QtWidgets 模块中的 QGraphicsView 相关类，链接 Qt6::Widgets 即可。Graphics View 的渲染后端在 Qt 6 中默认使用 raster（CPU 渲染），如果你需要 GPU 加速可以通过 `QGraphicsView::setViewport(new QOpenGLWidget)` 设置 OpenGL 视口，但对于本篇的入门示例来说 raster 渲染完全够用。所有代码在任何支持 Qt6 的桌面平台上行为一致。

## 3. 核心概念讲解

### 3.1 QGraphicsScene / QGraphicsView / QGraphicsItem 三角关系

这三者的关系可以用一个简单的类比来理解：QGraphicsScene 是"画布"，QGraphicsView 是"窗户"，QGraphicsItem 是"画布上的物体"。画布（Scene）是独立于显示设备的数据模型——它管理所有图元、处理碰撞检测、分发事件，但它本身不可见。窗户（View）是你看画布的方式——同一个画布可以有多个窗户从不同角度、不同缩放级别来观察。物体（Item）是画布上的实际内容——矩形、椭圆、文字、图片，每个物体有自己的坐标、层级、变换。

```cpp
// 创建场景
auto *scene = new QGraphicsScene(this);
scene->setSceneRect(0, 0, 800, 600);  // 场景的逻辑尺寸

// 创建视图并绑定场景
auto *view = new QGraphicsView(scene);
view->setRenderHint(QPainter::Antialiasing);
view->setDragMode(QGraphicsView::RubberBandDrag);

// 添加图元到场景
auto *rect = scene->addRect(50, 50, 200, 100,
    QPen(Qt::blue, 2), QBrush(QColor("#E8F0FE")));
auto *text = scene->addText("Hello Graphics View",
    QFont("Arial", 16));
text->setPos(80, 80);
```

你会发现这三行代码分别对应了三层：创建 Scene、创建 View 并关联 Scene、通过 Scene 添加 Item。View 的构造函数接收一个 Scene 指针，之后 View 会自动渲染 Scene 中的所有图元。当你修改 Scene 中的图元（移动、添加、删除）时，View 会自动更新显示，不需要你手动调 `update()`。

`setSceneRect()` 设置的是场景的逻辑坐标范围。场景坐标是一个"虚拟的"无限平面，`setSceneRect` 只是告诉 View 默认应该关注哪个区域。如果你不设置，Scene 会自动根据所有图元的包围矩形来调整。

`setDragMode(QGraphicsView::RubberBandDrag)` 让用户可以在视图上拖拽出一个选择框来批量选中图元。这是 Graphics View 内置的交互功能——你不需要写一行选择逻辑的代码。另一个常用的模式是 `QGraphicsView::ScrollHandDrag`，用户可以拖拽来平移视图。

### 3.2 添加标准图元：矩形、椭圆、文字、像素图

QGraphicsScene 提供了一系列便捷方法来快速添加标准图元。这些方法会创建对应的 QGraphicsItem 子类对象，添加到场景中，并返回图元指针供你进一步配置。

```cpp
// 矩形图元
auto *rect = scene->addRect(
    0, 0, 200, 120,
    QPen(QColor("#2C3E50"), 2),
    QBrush(QColor("#ECF0F1"))
);
rect->setFlag(QGraphicsItem::ItemIsMovable);      // 可拖拽
rect->setFlag(QGraphicsItem::ItemIsSelectable);    // 可选中
rect->setFlag(QGraphicsItem::ItemSendsGeometryChanges);  // 位置变化时通知

// 椭圆图元
auto *ellipse = scene->addEllipse(
    250, 50, 150, 150,
    QPen(QColor("#E74C3C"), 3),
    QBrush(QColor("#FADBD8"))
);
ellipse->setFlags(QGraphicsItem::ItemIsMovable |
                  QGraphicsItem::ItemIsSelectable);

// 文字图元
auto *text = scene->addText(
    "Graphics View",
    QFont("Arial", 20, QFont::Bold)
);
text->setDefaultTextColor(QColor("#2C3E50"));
text->setPos(50, 160);
text->setFlags(QGraphicsItem::ItemIsMovable |
               QGraphicsItem::ItemIsSelectable);

// 像素图（图片）图元
QPixmap pixmap(":/images/logo.png");
if (!pixmap.isNull()) {
    auto *pixmapItem = scene->addPixmap(pixmap);
    pixmapItem->setPos(300, 200);
    pixmapItem->setScale(0.5);  // 缩放到 50%
    pixmapItem->setFlags(QGraphicsItem::ItemIsMovable |
                         QGraphicsItem::ItemIsSelectable);
}
```

`ItemIsMovable` 标志让用户可以用鼠标拖拽移动图元。这个功能是 Graphics View 框架内置的——你不需要自己实现拖拽逻辑，只需要设置这个标志，框架就会帮你处理鼠标按下、移动、释放的完整流程。

`ItemIsSelectable` 标志让图元可以被选中。选中后图元周围会显示一个虚线框（选中状态的视觉反馈由 QGraphicsItem 的 `paint()` 方法自动绘制）。你可以通过 `scene->selectedItems()` 获取当前所有被选中的图元。

除了这些便捷方法之外，你也可以直接创建图元对象然后通过 `scene->addItem()` 添加。这种方式在需要精细控制图元属性的时候更常用。

```cpp
// 直接创建 QGraphicsRectItem 并添加
auto *rect = new QGraphicsRectItem(0, 0, 200, 120);
rect->setPen(QPen(Qt::blue, 2));
rect->setBrush(QBrush(QColor("#D6EAF8")));
rect->setPos(100, 100);
rect->setFlags(QGraphicsItem::ItemIsMovable |
               QGraphicsItem::ItemIsSelectable);
scene->addItem(rect);
```

这两种方式的区别是：`scene->addRect()` 等便捷方法会自动把 Scene 设为图元的 parent（也就是 Scene 拥有图元的所有权），而 `new` + `addItem()` 的方式你需要自己确保图元的 parent 设置正确。实际上 `addItem()` 也会把 Scene 设为图元的 scene，所以内存管理上没有区别。

### 3.3 场景坐标 vs 视图坐标 vs 图元坐标的转换

图形视图框架中有三套坐标系，理解它们之间的关系是用好这个框架的关键。

第一套是场景坐标（Scene Coordinates）。这是整个场景的全局坐标系，所有图元在场景中都有一个确定的位置。场景坐标就像地球上的经纬度——不管你用什么角度观察地球，北京的经纬度永远是那个值。图元的 `pos()` 返回的就是图元在场景坐标中的位置（实际上是图元在父图元坐标系中的位置，顶级图元的父坐标系就是场景坐标）。

第二套是视图坐标（View Coordinates）。这是 View 控件自身的坐标系，原点在 View 的左上角，单位是像素。视图坐标受 View 的缩放和平移影响——如果你放大了 View，同样大小的场景区域会占据更多的视图像素。

第三套是图元坐标（Item Coordinates）。这是每个图元自己的局部坐标系，原点在图元的左上角（对于 QGraphicsRectItem 来说就是矩形的左上角）。图元的绘制和鼠标事件处理都是在图元坐标系中进行的。

三者之间的转换通过 QGraphicsView 和 QGraphicsItem 提供的映射方法实现。

```cpp
// View 坐标 → Scene 坐标
QPointF scenePos = view->mapToScene(viewPosX, viewPosPosY);

// Scene 坐标 → View 坐标
QPoint viewPos = view->mapFromScene(scenePos);

// Scene 坐标 → Item 坐标
QPointF itemPos = item->mapFromScene(scenePos);

// Item 坐标 → Scene 坐标
QPointF scenePos2 = item->mapToScene(itemPos);

// View 坐标 → Item 坐标（需要经过 Scene 坐标中转）
QPointF scenePos3 = view->mapToScene(viewPos);
QPointF itemPos2 = item->mapFromScene(scenePos3);
```

这些映射方法在处理鼠标事件时非常重要。当用户在 View 上点击鼠标时，事件中携带的坐标是视图坐标，你需要先用 `mapToScene()` 转换到场景坐标，然后再根据需要转换到具体图元的坐标。Graphics View 框架在事件分发时会自动帮你做这个转换——如果你在 QGraphicsItem 子类中重写 `mousePressEvent()`，事件中的坐标已经是图元坐标了。

一个常见的使用场景是：用户在 View 上点击了一个位置，你想知道点击的是哪个图元。这可以通过 `QGraphicsView::itemAt(viewPos)` 或 `QGraphicsScene::itemAt(scenePos)` 来查询。前者接收视图坐标，后者接收场景坐标。

```cpp
void MyView::mousePressEvent(QMouseEvent *event)
{
    // 将视图坐标转换为场景坐标
    QPointF scenePos = mapToScene(event->pos());

    // 查找点击位置下的图元
    QGraphicsItem *item = scene()->itemAt(scenePos, transform());
    if (item) {
        qDebug() << "点击了图元，类型:" << item->type()
                 << "图元坐标:" << item->mapFromScene(scenePos);
    }

    QGraphicsView::mousePressEvent(event);
}
```

### 3.4 鼠标事件在 Scene 层拦截与响应

Graphics View 框架的事件分发遵循一条清晰的链路：View 收到原始鼠标事件 → 转换为场景坐标 → 交给 Scene 处理 → Scene 找到目标 Item → Item 处理事件。你可以在这条链路的任何一层拦截和处理事件。

如果你想在 Scene 层统一处理所有鼠标事件（比如实现一个自定义的选择逻辑或者显示鼠标位置信息），可以继承 QGraphicsScene 并重写鼠标事件处理函数。

```cpp
class InteractiveScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit InteractiveScene(QObject *parent = nullptr)
        : QGraphicsScene(parent)
    {
        // 场景初始化...
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        QPointF scenePos = event->scenePos();
        qDebug() << "Scene 收到鼠标按下，场景坐标:" << scenePos;

        // 查找点击位置的图元
        QGraphicsItem *item = itemAt(scenePos, views().first()->transform());
        if (item) {
            // 高亮被点击的图元
            item->setOpacity(0.6);
        }

        // 别忘了调用基类实现，否则图元不会收到事件
        QGraphicsScene::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        // 实时显示鼠标位置
        emit mousePositionChanged(event->scenePos());
        QGraphicsScene::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
    {
        // 恢复所有图元的不透明度
        for (auto *item : items()) {
            item->setOpacity(1.0);
        }
        QGraphicsScene::mouseReleaseEvent(event);
    }

signals:
    void mousePositionChanged(const QPointF &scenePos);
};
```

这里有一个非常重要的原则：如果你在 Scene 层重写了鼠标事件处理函数，一定要调用基类的实现（`QGraphicsScene::mousePressEvent(event)`）。基类的实现负责把事件转发给具体的图元。如果你忘了调用基类实现，所有图元都不会收到鼠标事件——包括 `ItemIsMovable` 的拖拽功能也会失效。这是一个极其常见的坑。

如果你想在 Item 层处理事件（比如某个特定的图元需要对点击做出特殊响应），可以继承具体的图元类并重写它的鼠标事件。

```cpp
class DraggableRect : public QGraphicsRectItem
{
public:
    DraggableRect(const QRectF &rect, QGraphicsItem *parent = nullptr)
        : QGraphicsRectItem(rect, parent)
    {
        setFlags(ItemIsMovable | ItemIsSelectable |
                 ItemSendsGeometryChanges);
        setAcceptHoverEvents(true);  // 启用悬停事件
    }

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override
    {
        // 双击改变颜色
        QBrush brush = this->brush();
        QColor color = brush.color();
        color.setRed((color.red() + 80) % 256);
        setBrush(QBrush(color));

        QGraphicsRectItem::mouseDoubleClickEvent(event);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override
    {
        // 鼠标进入时改变光标
        setCursor(Qt::PointingHandCursor);
        QGraphicsRectItem::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override
    {
        // 鼠标离开时恢复光标
        setCursor(Qt::ArrowCursor);
        QGraphicsRectItem::hoverLeaveEvent(event);
    }

    QVariant itemChange(GraphicsItemChange change,
                        const QVariant &value) override
    {
        // 位置变化时打印新位置
        if (change == ItemPositionHasChanged) {
            QPointF newPos = value.toPointF();
            qDebug() << "图元移动到:" << newPos;
        }
        return QGraphicsRectItem::itemChange(change, value);
    }
};
```

`setAcceptHoverEvents(true)` 是一个容易被忽略的设置。默认情况下图元不接收悬停事件（因为悬停事件的分发比较消耗性能——每次鼠标移动都要判断鼠标在哪个图元上）。如果你需要 `hoverEnterEvent` 和 `hoverLeaveEvent`，必须显式开启。

`itemChange()` 是一个非常有用的回调函数。当图元的各种属性发生变化时（位置、选中状态、可见性、层级等），这个函数都会被调用。你可以在里面监听位置变化来做一些联动操作，比如拖拽一个矩形时让连线的端点跟着移动。但前提是你设置了 `ItemSendsGeometryChanges` 标志，否则 `itemChange()` 不会收到位置变化的通知。

## 4. 踩坑预防

第一个坑是在 Scene 层重写鼠标事件后忘记调用基类实现。这个前面已经反复强调了。后果是所有图元的交互都失效——拖拽不了、选不了、自定义的鼠标事件也收不到。养成习惯：Scene 层的鼠标事件处理函数最后一定要调基类。

第二个坑是坐标系搞混。在 View 中拿到的是视图坐标，在 Scene 中拿到的是场景坐标，在 Item 中拿到的是图元坐标。如果你在错误的地方用了错误的坐标，计算出来的位置会完全对不上。调试这类问题的时候，打印出每个阶段的坐标值是最直接的方法。

第三个坑是图元的 Z 值（层级）冲突。当你添加多个重叠的图元时，后添加的图元会覆盖先添加的。默认的 Z 值都是 0，相同 Z 值的图元按添加顺序排列。你可以通过 `setZValue()` 手动设置层级——Z 值大的在上面。

第四个坑是忘记设置 `ItemSendsGeometryChanges` 标志。如果你需要在 `itemChange()` 中监听位置变化，这个标志是必须的，否则 `itemChange()` 的 `change` 参数永远不会是 `ItemPositionHasChanged`。

第五个坑是 Scene 的 `itemAt()` 方法在图元重叠时只返回最顶层的图元。如果你需要获取某个位置下的所有图元，用 `items()` 方法配合 `Qt::IntersectsItemShape` 或 `Qt::ContainsItemShape` 来过滤。

## 5. 练习项目

我们来做一个综合练习：实现一个简易的"图形画板"。Scene 中预设一些矩形和椭圆图元，用户可以拖拽移动它们。双击图元改变颜色。工具栏提供"添加矩形""添加椭圆""删除选中"三个按钮。底部状态栏显示当前鼠标的场景坐标和选中的图元数量。图元移动时在状态栏实时显示其新位置。

几个提示：添加图元用 `scene->addRect()` 或 `scene->addEllipse()`，设置 `ItemIsMovable | ItemIsSelectable` 标志；删除选中的图元用 `scene->selectedItems()` 获取列表，然后对每个图元调 `deleteLater()` 或者 `scene->removeItem()` 后再 `delete`；鼠标坐标通过继承 QGraphicsScene 并重写 `mouseMoveEvent` 获取，注意要调基类实现；选中图元数量通过 `scene->selectedItems().size()` 获取，可以连接 `selectionChanged` 信号来实时更新。

## 6. 官方文档参考链接

[Qt 文档 · QGraphicsScene](https://doc.qt.io/qt-6/qgraphicsscene.html) -- 场景管理器，负责图元管理和事件分发

[Qt 文档 · QGraphicsView](https://doc.qt.io/qt-6/qgraphicsview.html) -- 视图控件，负责渲染和用户交互

[Qt 文档 · QGraphicsItem](https://doc.qt.io/qt-6/qgraphicsitem.html) -- 图元基类，包含坐标映射、标志设置、事件处理

[Qt 文档 · Graphics View Framework Overview](https://doc.qt.io/qt-6/graphicsview.html) -- 官方架构总览，包含坐标系和事件分发机制的详细说明

[Qt 文档 · QGraphicsRectItem](https://doc.qt.io/qt-6/qgraphicsrectitem.html) -- 矩形图元

[Qt 文档 · QGraphicsEllipseItem](https://doc.qt.io/qt-6/qgraphicsellipseitem.html) -- 椭圆图元

[Qt 文档 · QGraphicsTextItem](https://doc.qt.io/qt-6/qgraphicstextitem.html) -- 文字图元

---

到这里，图形视图框架的基础你就掌握了。Scene 管图元、View 管显示、Item 管自身——三层各司其职；三套坐标系通过映射方法自由转换；鼠标事件从 View 传递到 Scene 再分发到 Item，每一层都可以拦截处理。这套架构看起来有点重，但当你需要管理大量可交互的图形元素时，它省下来的代码量是惊人的。下一篇我们进入属性动画框架，让控件和图元"动起来"。
