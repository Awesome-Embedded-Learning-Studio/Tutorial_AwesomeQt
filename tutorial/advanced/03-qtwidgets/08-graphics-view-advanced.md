---
title: "3.8 图形视图进阶"
description: "图元平时看着正常，一句 setRotation(45) 之后四个角被裁掉。itemChange 写好了却一次都不被回调。两颗五角星的角还没碰到，碰撞却报了出来。这些现象的根子分别在 boundingRect 的余量规则、itemChange 的通知条件、shape() 的矩形近似和 QGraphicsEffect 的所有权归属里，本篇把自定义 Item 的四个虚函数、精确碰撞检测、状态拦截回调和视觉特效讲清。配套示例在 examples/advanced/03-qtwidgets/08-graphics-view-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.8——图形视图进阶

[入门篇图形视图基础](../../beginner/03-qtwidgets/08-graphics-view-beginner.md) 把 Scene/View/Item 三层架构跑通了，怎么添加标准图元、怎么在三层坐标系之间来回换算、怎么在 Scene 层拦截鼠标事件，都摸过一遍。需求只是画几个矩形椭圆摆一摆的话，那些存货确实够用。可一旦您要画一张带圆角和投影的节点卡片，要判断两个不规则图形碰没碰上，要在节点被拖走时让连线的端点跟上来，这些定制在标准图元上都找不到入口。精确轮廓要走 shape()，位置联动要走 itemChange()，都得继承 QGraphicsItem、自己实现虚函数才做得到。本篇咱们把四件事走完整：自定义 Item 的实现套路、基于 shape() 的碰撞检测、itemChange() 的状态拦截，以及 QGraphicsEffect 的视觉特效。

## 继承 QGraphicsItem：两个纯虚必须写，两个默认实现值得重写

QGraphicsItem 值得认真对待的虚函数有四个：boundingRect()、paint()、shape()、type()。前两个是纯虚函数，不实现连编译都过不了。后两个带默认实现，重写与否直接决定碰撞检测的精度和类型识别的成败。咱们把分工摆成表：

| 虚函数 | 分工 | 不重写的后果 |
| ------ | ---- | ------------ |
| boundingRect | 向 Scene 报告图元在本地坐标系的包围矩形，供粗剔除和脏区域标记用 | 覆盖不住绘制区域时，图元被裁剪 |
| paint | 实际绘制，QPainter 的坐标系和裁剪已由 Scene 配好 | 纯虚函数，必须实现 |
| shape | 给出精确轮廓，碰撞检测和鼠标命中都问它 | 退化为包围矩形的矩形近似，非矩形图元误判 |
| type | 返回运行时类型 ID | 自定义子类与基类报同一个 ID，类型识别失效 |

boundingRect() 返回图元在本地坐标系里的包围矩形，作用是告诉 Scene“我可能占多大地方”，Scene 拿它做快速的粗略剔除和脏区域标记。要守的原则只有一条：报出去的矩形必须盖住 paint() 会画到的所有区域，包括 pen 宽度的一半——Qt 的 pen 是居中绘制的，线条内外各占一半，外边那一半就伸出了几何边界。落在节点卡片上，写法是这样：

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

pad 里多算的 2px 是咱们留给选中状态虚线框的，它画在图元外沿，同样得算进包围矩形。

paint() 负责实际绘制。传进来的 QPainter 已经被 Scene 配好了坐标系和裁剪区域，咱们只管在里面画。要留意的是调用频率：paint 每帧都可能进来，场景里有几百个 Item 时，每次都构造一遍 QFont、QPen 这类重量级局部变量，累积开销不小。不变的 Pen 和 Brush 值得提成成员变量，构造时初始化一次。配套示例的自定义图元一共才八个，paint() 里就图省事每次现构 QPen。咱们要是把它换到几百个 Item 的场景里，每帧几百次构造累积下来，开销一下子就上来了。

shape() 返回一个 QPainterPath，描述图元的精确轮廓。默认实现只把 boundingRect() 转成矩形路径，画圆、画五角星、画任意多边形都不够用。咱们重写之后，Scene 的碰撞检测和鼠标命中判断就改用精确路径来判断：

```cpp
QPainterPath NetworkNode::shape() const
{
    QPainterPath path;
    // 如果节点是圆角矩形，用 addRoundedRect 精确描述
    path.addRoundedRect(0, 0, kNodeSize, kNodeSize, kCornerRadius, kCornerRadius);
    return path;
}
```

type() 返回一个运行时类型 ID。惯例是咱们在子类里定义 enum { Type = UserType + 1 } 并返回它，其他代码就能拿 item->type() == NetworkNode::Type 做一次快速判断，再决定要不要 dynamic_cast。配套示例的 CustomShapeItem 走的是省事一路：type() 里直接 return UserType + 1，连 Type 枚举都没设。

### 一道调试题：平时完全正常，setRotation(45) 之后四个角被裁掉

咱们来看一段没留余量的写法：

```cpp
QRectF MyRect::boundingRect() const
{
    return QRectF(0, 0, 100, 50);  // 刚好是矩形本身的大小
}

void MyRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(QPen(Qt::black, 4));
    painter->drawRect(0, 0, 100, 50);
}
```

图元没旋转时咱们看不出任何问题，setRotation(45) 一调，矩形的四个角被裁掉了。咱们把病根找出来：boundingRect 只报了矩形本身的区域，pen 宽 4px 意味着线条各方向向外扩展 2px，扩出去的部分不在报告范围里。Scene 依据包围矩形安排重绘和裁剪，旋转把矩形的有效绘制区域顶出了报告边界，顶出去的部分就丢了。毛病还挑环境，普通显示器上不旋转，四条边与裁剪边平行，缺的那 2px 肉眼难辨，非得旋转或高 DPI 缩放把角顶到边界上，缺角才显出来。修法是让 boundingRect 向外扩 pen 半宽，再补 1px 给抗锯齿，图元若带选中虚线框，就再多留 1-2px，思路和上面 NetworkNode 的 pad 一致。

配套示例在 `examples/advanced/03-qtwidgets/08-graphics-view-advanced/`，`cmake -B build && cmake --build build` 构建后运行。裁剪这毛病当场就能复现：您把 custom_shape_item.cpp 里 boundingRect() 的 `const qreal pad = kPenWidth / 2.0 + kPadding;` 改成 `const qreal pad = 0.0;`，重新编译运行，图元的描边先薄一圈。再去 collision_scene.cpp 的 populateScene()，给里面创建矩形的局部变量 rect1 补一行 `setRotation(45)`，缺角就摆在那儿了。改回原值，一切如初。

## collidingItems()：boundingRect 粗筛在前，shape() 精判在后

QGraphicsScene 和图元自己都提供了 collidingItems()，能查出一个图元和场景里哪些图元发生了碰撞。检测怎么走，咱们跟着看两步：第一步用 boundingRect 做快速排除，两个包围矩形不相交就肯定没碰上。第二步用 shape() 做精确判断，看两个 QPainterPath 有没有交集。两步走的意义也在这儿，粗筛便宜，先挡掉绝大多数不相干的组合，昂贵的路径相交判断只留给少数候选。

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

咱们给 collidingItems 传的 Qt::IntersectsItemShape，语义是“两个 shape 有交集就算碰撞”。与之配对的 Qt::ContainsItemShape 要求更严，得一个 shape 把另一个完全包含才算。绝大多数场景咱们选 IntersectsItemShape 就对了。

性能上咱们要看的因素有两个：shape() 的复杂度，和场景里 Item 的数量。路径越复杂，精确判断的计算量越大，一条带 200 个控制点的贝塞尔曲线图元就能让碰撞检测明显变慢。真遇到复杂轮廓，可以在精度和性能之间取一个平衡：shape() 不追求完全精确，改返回一个稍大但形状简单的近似路径，拿凸包顶替凹多边形就是常见做法。

配套示例的窗口 CollisionScene 是一个自带场景的 QGraphicsView 子类，场景里摆着圆形、矩形、五角星三类图元。开场就有几对图元压在一起，它们顶着红色边框，左上角两个圆的初始位置就是重叠着设的。咱们把它们拖开，碰撞的红色一退，图元回到灰边浅蓝底。再拿两颗五角星慢慢贴近，角和角还没碰到、包围矩形已经搭上的位置，颜色不变——shape() 重写换来的正是这些位置的准确判断。检测的触发点也写在源码里：CollisionScene 的构造函数把 scene 的 changed 信号 connect 到 detectCollisions()，拖动、增删图元引发的任何场景变化都会触发一轮全量重检。

咱们还想看矩形近似有多离谱的话，就把 CustomShapeItem::shape() 的返回改成 `return QGraphicsItem::shape();`（基类默认实现），重新编译后再拖五角星靠近矩形：轮廓还没相碰，红色就亮了，包围矩形比星形大一圈，多出来的空当全被算成了碰撞。

## itemChange()：ItemPositionChange 还能改值，ItemPositionHasChanged 只能收通知

itemChange() 是 QGraphicsItem 的通用状态变化回调，位置、选中状态、可见性、父级关系的变化都会路过这里。change 落地之前，咱们就能把最终值改掉，工作模式是“拦截并修改”。

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

两个位置通知的时机容易搅在一起，咱们拉表对比：

| 通知 | 触发时机 | 能不能改值 | 适合干什么 |
| ---- | -------- | ---------- | ---------- |
| ItemPositionChange | 位置改变之前 | 能，返回值就是新位置 | 约束和修正，比如把坐标夹回场景边界 |
| ItemPositionHasChanged | 位置已经改完 | 不能，只能响应 | 联动和通知，比如让连线更新端点 |

代码里有个细节值得咱们停一句：ItemPositionChange 分支开头的 && scene() 不是顺手一写。图元还在构造、尚未 addItem 进场景的时候，位置通知就已经会到，这时 scene() 是空指针，不判空直接去调 scene()->sceneRect() 做 qBound，就是空指针解引用。

咱们要是想在 ItemPositionChange 里约束位置，记住别调 setPos()。setPos 会再次触发 ItemPositionChange，于是回调自己调自己，无限递归到栈溢出，调试时您会在 callstack 里看到满屏的 itemChange 帧。要修正位置，就像上面那样把修正后的值 return 出去，不碰任何会再次触发同一个 change 的函数。

要收到这些通知，标志位得先设好：位置变化需要 ItemSendsGeometryChanges，选中状态变化需要 ItemIsSelectable。标志没设，itemChange 对应的分支一次都不会进来。入门篇 3.8 已经把漏设标志的后果讲过，这里不重讲。配套示例里，设标志的地方是 CustomShapeItem 自己的构造函数（custom_shape_item.cpp 的 32-36 行），别去 CollisionScene 里找。ItemIsSelectable、ItemIsMovable、ItemSendsGeometryChanges 三个标志一次设齐，咱们照着抄就不会漏。

## QGraphicsEffect：paint 一行不动，特效挂上去就有

视觉效果这块，Qt 给咱们预备了四个现成的 QGraphicsEffect 子类，投影、模糊、着色、透明度各管一样，咱们往图元上一挂就生效，paint() 一行不用改：

| Effect 子类 | 做的事 |
| ----------- | ------ |
| QGraphicsDropShadowEffect | 投影，可调模糊半径、偏移和颜色 |
| QGraphicsBlurEffect | 模糊 |
| QGraphicsColorizeEffect | 着色覆盖 |
| QGraphicsOpacityEffect | 透明度控制 |

```cpp
// 给节点添加投影效果
auto *shadow = new QGraphicsDropShadowEffect;
shadow->setBlurRadius(12);
shadow->setOffset(4, 4);
shadow->setColor(QColor(0, 0, 0, 80));
node->setGraphicsEffect(shadow);
```

咱们把机制看一遍，它在渲染时拦截绘制：图元先被画进一块离屏缓冲区，效果算法作用在缓冲区上，再把结果合成回目标。所以每个挂了 Effect 的图元都额外占一块缓冲内存，几百个图元全挂上，内存占用会明显上去，挂多少个就得掂量了。

所有权归谁，咱们不用猜，Qt 6.9.1 的源码说得很直白：qgraphicsitem.cpp 里 setGraphicsEffect 的文档注释写着“QGraphicsItem takes ownership of effect”，代码也兑现——挂新效果会先 delete 旧效果，图元析构时把当前效果一并 delete。反过来的机制 Qt 也准备好了。QGraphicsEffect 的析构函数会把自己从图元上卸载，detach 时顺手清掉图元侧的指针，效果先走一步也不会让图元渲染到野指针，视觉上只是效果消失。

别在 Item 的所有权之外再叠一层管理：拿 std::unique_ptr 包住效果，图元析构先 delete 了一次，智能指针里剩下的就是野指针，轮到它析构就是二次释放。笔者一直照上面代码这么写，new 出来直接交给 setGraphicsEffect，不再多管。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，内容都在 QtWidgets 模块（QGraphicsView、QGraphicsScene、QGraphicsItem、QGraphicsEffect），链接 Qt6::Widgets 即可，渲染后端默认 raster，高 DPI 环境建议开启 setDevicePixelRatio 相关处理。

[Qt 文档 · QGraphicsItem](https://doc.qt.io/qt-6/qgraphicsitem.html) -- 图元基类，包含 boundingRect / paint / shape / itemChange 等核心虚函数

[Qt 文档 · QGraphicsScene](https://doc.qt.io/qt-6/qgraphicsscene.html) -- 场景管理器，包含 collidingItems 碰撞检测方法

[Qt 文档 · QGraphicsEffect](https://doc.qt.io/qt-6/qgraphicseffect.html) -- 视觉特效基类，包含四个内置子类

[Qt 文档 · QGraphicsDropShadowEffect](https://doc.qt.io/qt-6/qgraphicsdropshadoweffect.html) -- 投影效果，支持模糊半径、偏移和颜色

[Qt 文档 · Graphics View Framework](https://doc.qt.io/qt-6/graphicsview.html) -- 架构总览，包含坐标系和事件分发机制的详细说明

---

图形视图的进阶内容就到这里。想再动手的，请您把 CollisionScene 多玩一会儿：示例开着橡皮筋选择（setDragMode(QGraphicsView::RubberBandDrag)），在空白处拖个框能同时选中几个图元一起挪，每挪一步 changed 信号都带着全量碰撞重检跑一遍，把断点下在 detectCollisions() 里，检测的触发节奏看得一清二楚。下一篇 [3.9 动画进阶](./09-animation-advanced.md) 讲动画的驱动与编排，QStateMachine 配 QPropertyAnimation 做状态驱动的动画，动画组怎么编排队列，还有自定义 QVariantAnimation 的非线性插值。
