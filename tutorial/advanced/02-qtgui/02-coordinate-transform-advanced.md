---
title: "2.2 坐标变换进阶：矩阵组合与逆变换"
description: "入门篇我们聊了 QPainter 的 translate、rotate、scale 三个基本变换，用 save/restore 做变换隔离。说实话，画个旋转的文字、做个缩放动画确实够用了。"
---

# 现代Qt开发教程（进阶篇）2.2——坐标变换进阶：矩阵组合与逆变换

## 1. 前言 / 当「先移再转」已经不够用了

入门篇我们聊了 QPainter 的 translate、rotate、scale 三个基本变换，用 save/restore 做变换隔离。说实话，画个旋转的文字、做个缩放动画确实够用了。但当你需要在同一个控件里绘制多个有层级关系的图形——比如一个机械臂的关节系统，每个关节的旋转中心不同、旋转角度依赖父关节——用三层 save/restore 嵌套之后代码就变成了意大利面条。更别提当你需要从屏幕坐标反向推算原始坐标（比如鼠标点击反算回逻辑坐标）——入门篇完全没提过逆变换。

这篇我们把 QTransform 的 3x3 矩阵拆清楚，搞懂变换组合的正确顺序，学会用逆变换解决坐标反算问题。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QTransform 和 QPainter 的坐标变换 API 属于 QtGui 模块。示例需要 QtWidgets 来展示效果。QTransform 的 API 在 Qt 5 和 Qt 6 之间没有变化。

## 3. 核心概念讲解

### 3.1 QTransform 3x3 矩阵——变换的数学本质

QTransform 内部维护一个 3x3 矩阵，但只用到了前两行（第三行是 [0, 0, 1] 用于齐次坐标）。所有平移、旋转、缩放操作本质上都是矩阵乘法。

```cpp
// 平移矩阵乘以当前矩阵
// | 1  dx |     | m11 m12 m13 |
// | 0  1  |  ×  | m21 m22 m23 |
// | 0  0  |     | m31 m32 m33 |

QTransform transform;
transform.translate(100, 50);   // 平移
transform.rotate(45);           // 旋转
transform.scale(2.0, 1.5);     // 缩放
```

QTransform::translate/rotate/scale 都是将对应的变换矩阵右乘到当前矩阵上。这意味着变换的顺序是**从下往上应用的**——最后调用的 scale 最先作用于图形，然后 rotate，最后 translate。

### 3.2 变换组合顺序——为什么顺序至关重要

变换的组合顺序直接影响最终结果。旋转 + 平移 ≠ 平移 + 旋转，这是一个经常被搞错的点。

```cpp
// 情况 A：先平移再旋转
QTransform tA;
tA.translate(100, 0);  // 先往右移 100
tA.rotate(45);         // 再旋转 45 度
// 结果：图形先移到 (100, 0)，然后绕原点旋转 45 度

// 情况 B：先旋转再平移
QTransform tB;
tB.rotate(45);         // 先旋转 45 度
tB.translate(100, 0);  // 再往右移 100（在旋转后的坐标系中！）
// 结果：图形先旋转，然后沿旋转后的 X 轴方向移动
```

这个区别在制作动画和交互式图形时非常关键。如果你想让图形绕自身中心旋转，正确的做法是：先 translate 到中心，再 rotate，再 translate 回去。

```cpp
// 绕自身中心旋转的正确做法
QTransform t;
t.translate(centerX, centerY);  // 移到旋转中心
t.rotate(angle);                // 旋转
t.translate(-centerX, -centerY); // 移回原点
```

### 3.3 逆变换——从屏幕坐标反算逻辑坐标

当你需要把鼠标点击位置（屏幕坐标）转换回图形的逻辑坐标时，就需要用到逆变换。QTransform::inverted() 返回当前矩阵的逆矩阵。如果矩阵不可逆（比如 scale(0, 0) 把所有东西压缩到一个点），inverted() 会返回一个 identity 矩阵，通过 bool 参数可以检查是否成功。

```cpp
QTransform transform;
transform.translate(100, 100);
transform.rotate(30);
transform.scale(2.0, 2.0);

bool ok = false;
QTransform inverse = transform.inverted(&ok);
if (ok) {
    // 鼠标点击的屏幕坐标
    QPointF screenPoint(200, 150);
    // 反算回逻辑坐标
    QPointF logicalPoint = inverse.map(screenPoint);
}
```

这个技术在自定义图形控件中非常常用——比如你画了一个经过旋转和缩放的矩形，用户点击了屏幕上的某个位置，你需要判断这个点击是否在矩形内部。直接用屏幕坐标检查是不对的（矩形已经被变换了），需要把屏幕坐标用逆变换映射回逻辑空间再检查。

现在有一道调试题。下面这段代码的 `logicalPoint` 结果是什么？

```cpp
QTransform t;
t.translate(100, 0);
QTransform inv = t.inverted();
QPointF logical = inv.map(QPointF(150, 50));
```

答案是 `(50, 50)`。因为正向变换是 x+100，所以逆变换是 x-100。150-100=50，y 不变。这个例子很简单，但在多层嵌套变换中，手动计算几乎不可能——逆变换就是救星。

### 3.4 仿射变换与投影变换

QTransform 支持两种变换类型：仿射变换和投影变换。仿射变换保持平行线的平行性（平移、旋转、缩放、剪切都是仿射变换）。投影变换不保证这一点——它允许透视效果。

当你只用 translate/rotate/scale 时，QTransform 自动识别为仿射变换，可以进行某些优化。当你设置了 m13 或 m23（矩阵第三列的前两个元素）时，QTransform 切换为投影模式，可以创建透视效果。大多数 2D 绘图场景只需要仿射变换。

## 4. 踩坑预防

第一个坑是 save/restore 嵌套过深导致维护困难。当变换层级超过 3-4 层时，save/restore 的嵌套会让代码非常难读。后果是修改时很容易漏掉一个 restore 导致后续绘制全部错位。解决方案是对于复杂的变换组合，直接使用 QTransform 对象构建矩阵，用 `painter.setTransform(t)` 一次性设置，而不是反复 save/restore。

第二个坑是 rotate 的旋转中心不是图形中心。QPainter::rotate() 总是绕坐标原点旋转，不是绕图形的中心旋转。如果你想让一个矩形绕自己的中心旋转，必须先 translate 到矩形中心、再 rotate、再 translate 回去。后果是图形「飞」到了意想不到的位置。解决方案是记住旋转三步曲：translate 到中心 → rotate → translate 回原点。

第三个坑是 scale 使用负值导致坐标翻转。`scale(-1, 1)` 会水平翻转坐标系，这在制作镜像效果时很有用，但如果你不知道某个变换链中包含了负值 scale，后续的坐标计算就会完全反过来。后果是绘制位置与预期完全相反。解决方案是对包含负值 scale 的变换链做注释说明，或者用 inverted() 验证映射结果。

## 5. 练习项目

练习项目：关节动画系统。实现一个简单的 2D 机械臂可视化。

具体要求是：绘制一个两关节的机械臂（上臂 + 前臂），每个关节可以独立旋转。关节的旋转中心在连接处，前臂的变换依赖上臂的变换结果。鼠标拖动关节时通过逆变换将屏幕坐标映射回逻辑空间，判断拖动的是哪个关节。完成标准是两个关节独立旋转、前臂跟随上臂运动、鼠标交互正确识别目标关节。

提示几个关键点：用 QTransform 链式组合关节变换，`pushTransform` 传递父级变换到子级，鼠标事件用 inverted() 反算逻辑坐标。

## 6. 官方文档参考链接

[Qt 文档 · QTransform](https://doc.qt.io/qt-6/qtransform.html) -- 变换矩阵类参考

[Qt 文档 · QPainter Coordinate System](https://doc.qt.io/qt-6/coordsys.html) -- 坐标系统说明

---

到这里，坐标变换的进阶知识就拆完了。3x3 矩阵的本质、变换组合的正确顺序、逆变换的坐标反算——掌握这些后就能处理任意复杂度的 2D 变换场景。下一篇我们来看图像处理进阶。
