---
title: "3.8 图形视图进阶"
description: "入门篇我们用 QGraphicsView/QGraphicsScene 做了基础图形操作——拖拽、选中、坐标映射。但 Graphics View Framework 的真正威力在于自定义 Item 和碰撞检测。"
---

# 现代Qt开发教程（进阶篇）3.8——图形视图进阶

## 1. 前言 / 为什么要自己写 QGraphicsItem

入门篇我们把 Scene/View/Item 三层架构跑通了，知道了怎么添加标准图元、怎么在三层坐标系之间来回转换、怎么在 Scene 层拦截鼠标事件。说实话，如果你的需求就是画几个矩形椭圆摆一摆，那些知识确实够用。但一旦你需要画一个带圆角和阴影的节点卡片、需要判断两个不规则图形是否碰撞、需要在图元被拖拽时联动更新连线的端点——标准图元就撑不住了。你必须继承 QGraphicsItem，自己实现全套虚函数。这篇我们集中火力攻克四件事：自定义 Item 的完整实现套路、基于 shape() 的碰撞检测、itemChange() 的状态拦截机制、以及 QGraphicsEffect 的视觉特效。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。所有内容依赖 QtWidgets 模块中的 QGraphicsView、QGraphicsScene、QGraphicsItem 及 QGraphicsEffect 相关类，链接 Qt6::Widgets 即可。渲染后端默认 raster，高 DPI 环境下建议开启 `setDevicePixelRatio` 相关处理。

## 3. 核心概念讲解

### 3.1 继承 QGraphicsItem 的四个关键虚函数

QGraphicsItem 是一个纯虚基类（严格说是有少量默认实现的接口类），你要自己写的核心虚函数有四个：boundingRect()、paint()、shape()、type()。前两个是必须重写的，后两个强烈建议重写——不重写 shape() 碰撞检测会用矩形近似，不重写 type() 运行时类型识别就会失效。

boundingRect() 返回的是图元在本地坐标系中的包围矩形。这个矩形的作用是告诉 Scene "我可能占多大地方"——Scene 用它做快速的粗略剔除和脏区域标记。关键原则是 boundingRect 必须覆盖所有 paint() 会绘制到的区域，包括 pen 宽度的一半（因为 Qt 的 pen 是居中绘制的，线条的一半会超出几何边界）。

```cpp
QRectF NetworkNode::boundingRect() const
{
    // kNodeSize 是节点边长，kPenWidth 是画笔宽度
    // 留出 pen 半宽的余量，再加 2px 给选中状态的虚线框
    qreal pad = kPenWidth / 2.0 + 2.0;
    return QRectF(-pad, -pad,
                  kNodeSize + pad * 2,
                  kNodeSize + pad * 2);
}
```

paint() 负责实际绘制。它的 QPainter 已经被 Scene 配置好了坐标系和裁剪区域，你只需要在这个空间里画。这里有一个非常重要的细节：paint() 中不要创建 QFont、QPen 等重量级对象当局部变量然后每次调用都构造——如果你的场景有几百个 Item，每帧每个 Item 都构造一遍，累积开销不小。正确的做法是把不变的 Pen 和 Brush 做成成员变量，构造时初始化一次。

shape() 返回一个 QPainterPath，描述图元的精确轮廓。默认实现返回的是 boundingRect() 转换成的矩形路径，这对于圆形、多边形等非矩形图元来说精度不够。如果你重写了 shape()，Scene 在做碰撞检测和鼠标命中判断时就会用这个精确路径而不是包围矩形。

```cpp
QPainterPath NetworkNode::shape() const
{
    QPainterPath path;
    // 如果节点是圆角矩形，用 addRoundedRect 精确描述
    path.addRoundedRect(0, 0, kNodeSize, kNodeSize, kCornerRadius, kCornerRadius);
    return path;
}
```

type() 返回一个 int 类型的 ID，用于运行时类型识别。建议在子类中定义一个 enum { Type = UserType + 1 }，然后返回 Type。这样其他代码就能通过 item->type() == NetworkNode::Type 来安全地做 dynamic_cast 之前的快速判断。

现在有一道调试题给大家。看下面这段代码：

```cpp
QRectF MyRect::boundingRect() const
{
    return QRectF(0, 0, 100, 50);  // 刚好是矩形本身的大小
}

void MyRect::paint(QPainter *painter, const QStyleOptionItem *, QWidget *)
{
    painter->setPen(QPen(Qt::black, 4));
    painter->drawRect(0, 0, 100, 50);
}
```

这个 Item 在没有旋转的时候看起来完全正常，但一旦调了 setRotation(45)，你会发现矩形的四个角被裁掉了。问题出在 boundingRect 没有给 pen 留余量——pen 宽度 4px 意味着线条各方向向外扩展 2px，而 boundingRect 只报告了矩形本身的区域。Scene 根据这个包围矩形设置裁剪区域，旋转后矩形的有效绘制区域超出了包围矩形，超出的部分就被裁掉了。修复方法是让 boundingRect 向外扩展 pen 半宽。

### 3.2 碰撞检测——collidingItems() 与自定义 shape() 的配合

QGraphicsScene 提供了 collidingItems() 方法，它可以查询一个 Item 与场景中哪些其他 Item 发生了碰撞。碰撞检测分两步：第一步用 boundingRect 做快速排除（如果两个包围矩形不相交就肯定不碰撞），第二步用 shape() 做精确判断（两个 QPainterPath 是否有交集）。

```cpp
// 查询与当前节点碰撞的所有图元
auto colliders = node->collidingItems(Qt::IntersectsItemShape);
for (auto *item : colliders) {
    if (item->type() == NetworkNode::Type) {
        auto *other = static_cast<NetworkNode*>(item);
        other->setHighlight(true);  // 碰撞时高亮
    }
}
```

这里的 Qt::IntersectsItemShape 标志很重要。它表示"只要 shape 有交集就算碰撞"，与之对应的是 Qt::ContainsItemShape，要求一个 shape 完全包含另一个才算。绝大多数场景用 IntersectsItemShape 就对了。

碰撞检测的性能取决于两个因素：shape() 的复杂度和场景中 Item 的数量。shape() 返回的 QPainterPath 越复杂，精确碰撞的计算量越大。如果你有一个带 200 个控制点的贝塞尔曲线图元，碰撞检测就会明显变慢。对于复杂图元，一个实用的优化是：shape() 不返回完全精确的轮廓，而是返回一个稍大但形状简单的近似路径（比如用凸包代替凹多边形），在精度和性能之间取一个平衡。

### 3.3 itemChange()——拦截图元状态变化的核心回调

itemChange() 是 QGraphicsItem 提供的一个通用的状态变化通知机制。当图元的位置、选中状态、可见性、父级关系等属性发生变化时，这个函数都会被调用。它的工作模式是"拦截并修改"——你可以在 change 被应用之前修改最终值。

```cpp
QVariant NetworkNode::itemChange(GraphicsItemChange change,
                                  const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        // 限制节点只能在场景范围内移动
        QPointF newPos = value.toPointF();
        QRectF bounds = scene()->sceneRect();
        newPos.setX(qBound(bounds.left(), newPos.x(), bounds.right() - kNodeSize));
        newPos.setY(qBound(bounds.top(), newPos.y(), bounds.bottom() - kNodeSize));
        return newPos;  // 返回修改后的值
    }
    if (change == ItemPositionHasChanged) {
        // 位置已经改变，通知连线更新端点
        updateConnections();
    }
    if (change == ItemSelectedChange) {
        bool selected = value.toBool();
        // 选中/取消选中时改变视觉效果
        update(selected ? kSelectedColor : kNormalColor);
    }
    return QGraphicsItem::itemChange(change, value);
}
```

这里面有一个关键的区分：ItemPositionChange 是在位置改变之前触发的，你可以修改值；ItemPositionHasChanged 是在位置已经改变之后触发的，你只能响应不能修改。这两个的用途完全不同——前者做约束和修正，后者做联动和通知。

要启用 itemChange 通知，必须设置对应的标志。位置变化需要 ItemSendsGeometryChanges，选中状态变化需要 ItemIsSelectable。如果你忘了设标志，itemChange 根本不会被调用，这是入门篇提到过的老坑，这里不再重复。

### 3.4 QGraphicsEffect——视觉特效的标准化方案

Qt 提供了四个内置的 QGraphicsEffect 子类，它们可以挂载到任何 QGraphicsItem 上，在不修改 paint() 逻辑的情况下添加视觉效果。QGraphicsDropShadowEffect 用于投影，QGraphicsBlurEffect 用于模糊，QGraphicsColorizeEffect 用于着色覆盖，QGraphicsOpacityEffect 用于透明度控制。

```cpp
// 给节点添加投影效果
auto *shadow = new QGraphicsDropShadowEffect;
shadow->setBlurRadius(12);
shadow->setOffset(4, 4);
shadow->setColor(QColor(0, 0, 0, 80));
node->setGraphicsEffect(shadow);
```

Effect 的原理是在渲染图元时拦截绘制过程，先把图元绘制到一个离屏缓冲区，然后对缓冲区应用效果算法，最后把结果绘制到目标。这意味着每个挂了 Effect 的 Item 都额外消耗一份内存用于缓冲区。如果你的场景有几百个 Item 都挂了 Effect，内存占用会显著增加。

Effect 和 Item 的生命周期管理有一个容易踩的坑：setGraphicsEffect 接收的是指针，Item 不会接管 Effect 的所有权。如果你在某个函数里 new 了一个 Effect 然后挂到 Item 上，函数结束后 Effect 的局部指针丢失了，但 Item 还在引用它——只要你没有把 Effect 的 parent 设对或者用智能指针管理，这就是一个悬空指针隐患。

## 4. 踩坑预防

第一个坑是 boundingRect 返回值比实际绘制区域小。前面调试题已经展示了这个问题的典型场景：pen 的宽度没有被算进 boundingRect。后果是 Item 在特定角度或缩放下被裁剪，边缘消失，而且这个问题只在旋转或高 DPI 设备上才会暴露，在普通显示器上直线方向完全看不出来。解决方案是 boundingRect 向外扩展 penWidth / 2 + 1（多出的 1px 留给抗锯齿），如果你的 Item 还有选中状态的虚线框，再多留 1-2px。

第二个坑是 itemChange 中修改自身属性导致递归调用。比如你在 ItemPositionChange 里调了 setPos() 来约束位置——setPos 又会触发 ItemPositionChange，于是无限递归，栈溢出崩溃。调试时你会发现 callstack 里全是 itemChange，非常壮观。解决方案是在 itemChange 里永远不要调用会再次触发同一个 change 的函数。如果需要修正位置，直接返回修正后的 QVariant 值，不要调 setPos。

第三个坑是 QGraphicsEffect 的 Effect 对象先于 Item 析构。最常见的情况是 Effect 被设为局部变量或者挂在了一个比 Item 寿命短的对象上。Item 内部保存了 Effect 的裸指针，Effect 析构后 Item 还在用它来做渲染——结果就是访问已释放内存，而且是时序相关的崩溃，不是每次必现，调试起来血压拉满。解决方案是确保 Effect 的 parent 设为持有它的 Item，或者用 std::unique_ptr 管理 Effect 的生命周期。

## 5. 练习项目

练习项目：带碰撞检测的拓扑图编辑器。我们要实现一个可以可视化编辑网络拓扑图的工具，核心功能是节点可拖动、节点之间有连线、连线随节点移动自动更新、节点碰撞时高亮提示。

具体来说，场景中预设若干个 NetworkNode（自定义 QGraphicsItem，画一个带圆角和标题文字的矩形卡片），每两个节点之间有一条 NetworkEdge（也是自定义 QGraphicsItem，画一条从源节点中心到目标节点中心的折线）。拖动节点时连线自动跟随更新。当两个节点的 shape() 发生碰撞时，两个节点同时变为红色边框并在状态栏提示碰撞信息。节点选中时显示投影效果。

完成标准是：拖动节点时连线平滑跟随无闪烁，碰撞检测准确（圆角部分碰撞也能检测到），碰撞高亮在碰撞解除后恢复正常，内存无泄漏。提示几个关键点：NetworkNode 的 itemChange 监听 ItemPositionHasChanged 来通知 NetworkEdge 更新端点；NetworkEdge 需要保存源节点和目标节点的指针；碰撞检测可以在 itemChange 的 ItemPositionHasChanged 里调用 collidingItems()；Effect 用 QGraphicsDropShadowEffect 并设好 parent。

## 6. 官方文档参考链接

[Qt 文档 · QGraphicsItem](https://doc.qt.io/qt-6/qgraphicsitem.html) -- 图元基类，包含 boundingRect / paint / shape / itemChange 等核心虚函数

[Qt 文档 · QGraphicsScene](https://doc.qt.io/qt-6/qgraphicsscene.html) -- 场景管理器，包含 collidingItems 碰撞检测方法

[Qt 文档 · QGraphicsEffect](https://doc.qt.io/qt-6/qgraphicseffect.html) -- 视觉特效基类，包含四个内置子类

[Qt 文档 · QGraphicsDropShadowEffect](https://doc.qt.io/qt-6/qgraphicsdropshadoweffect.html) -- 投影效果，支持模糊半径、偏移和颜色

[Qt 文档 · Graphics View Framework](https://doc.qt.io/qt-6/graphicsview.html) -- 架构总览，包含坐标系和事件分发机制的详细说明

---

到这里，图形视图框架的进阶内容我们就过了一遍。自定义 QGraphicsItem 的四个虚函数搞清楚了——boundingRect 必须留余量、shape() 决定碰撞精度、type() 支撑运行时类型识别。collidingItems() 配合自定义 shape() 做精确碰撞、itemChange() 做状态拦截和联动通知、QGraphicsEffect 做视觉特效——这四招组合起来，足够应对绝大多数图形编辑场景。下一篇我们看动画框架的进阶内容，让控件和图元动得更漂亮。
