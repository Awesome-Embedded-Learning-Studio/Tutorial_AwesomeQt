---
title: "3.41 QStackedWidget 进阶"
description: "入门篇我们用 QStackedWidget 配合 QComboBox 和 QListWidget 搭了导航切换界面，掌握了 addWidget/setCurrentIndex/currentChanged 信号的基本用法，也搞清了它和 QTabWidget 的核心区别——没有导航头，一切导航逻辑你自己说了算。"
---

# 现代Qt开发教程（进阶篇）3.41——QStackedWidget 进阶

## 1. 前言 / 当页面切换不再"闪烁"

入门篇我们用 QStackedWidget 配合 QComboBox 和 QListWidget 搭了导航切换界面，掌握了 addWidget / setCurrentIndex / currentChanged 信号的基本用法，也搞清了它和 QTabWidget 的核心区别——没有导航头，一切导航逻辑你自己说了算。入门篇最后留了一个伏笔：QStackedWidget 切换页面时是直接 show/hide，没有任何过渡效果——旧页面瞬间消失、新页面瞬间出现，视觉上就是一帧闪切。对于一个系统设置界面来说这倒没什么，但如果你在做移动端风格的导航、向导式引导流程、或者任何需要"流畅感"的 UI，这种硬切就很难接受了。

今天我们解决的核心问题就是：给 QStackedWidget 加上滑动切换动画。围绕这个目标，我们会拆解几个关键技术点——QPropertyAnimation 控制 pos 和 geometry 实现平移滑入滑出、QGraphicsOpacityEffect 实现淡入淡出的组合动画、动画期间如何冻结用户交互防止状态混乱，以及 QStackedWidget 的 sizeHint 在动画过程中如何配合窗口大小变化。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。涉及的类分布在 QtWidgets（QStackedWidget、QPropertyAnimation、QGraphicsOpacityEffect、QParallelAnimationGroup）和 QtCore（QEasingCurve、QAbstractAnimation）模块中，链接 Qt6::Widgets 即可。动画相关代码在所有桌面平台上行为一致。

## 3. 核心概念讲解

### 3.1 用 QPropertyAnimation 实现页面滑动

QStackedWidget 的 setCurrentIndex 做的事情非常简单粗暴——旧页面 hide()，新页面 show()，完事。要实现滑动效果，我们需要自己控制这个过程：在切换时不直接调 setCurrentIndex，而是手动把新页面放在视口外，然后用 QPropertyAnimation 把新页面滑进来、旧页面滑出去。

实现滑动切换的思路是这样的：假设我们要从左往右切换（新页面从右侧滑入），先把新页面 geometry 的 x 坐标设为 QStackedWidget 的宽度（即视口右边缘之外），然后启动一个 QPropertyAnimation 将新页面的 pos 从 (width, 0) 动画到 (0, 0)，同时把旧页面从 (0, 0) 动画到 (-width, 0)。两个动画用 QParallelAnimationGroup 同步执行，结束时调用 setCurrentIndex 确保状态一致。

```cpp
class SlidingStackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    enum class SlideDirection {
        kLeftToRight,
        kRightToLeft,
        kTopToBottom,
        kBottomToTop
    };

    void slideToIndex(int index,
                      SlideDirection direction = SlideDirection::kLeftToRight)
    {
        if (index < 0 || index >= count() || index == currentIndex()) {
            return;
        }

        QWidget *current = currentWidget();
        QWidget *next = widget(index);

        // 先把新页面放到视口外面
        QPoint offset = calculateOffset(direction);
        next->setGeometry(current->geometry().translated(offset));
        next->show();
        next->raise();

        // 旧页面滑出，新页面滑入
        auto *group = new QParallelAnimationGroup(this);

        auto *anim_out = new QPropertyAnimation(current, "pos");
        anim_out->setDuration(300);
        anim_out->setStartValue(current->pos());
        anim_out->setEndValue(current->pos() - offset);
        anim_out->setEasingCurve(QEasingCurve::OutCubic);
        group->addAnimation(anim_out);

        auto *anim_in = new QPropertyAnimation(next, "pos");
        anim_in->setDuration(300);
        anim_in->setStartValue(current->pos() + offset);
        anim_in->setEndValue(current->pos());
        anim_in->setEasingCurve(QEasingCurve::OutCubic);
        group->addAnimation(anim_in);

        connect(group, &QParallelAnimationGroup::finished, this,
                [this, next, index]() {
            setCurrentWidget(next);
            // 重置所有页面的位置
            for (int i = 0; i < count(); ++i) {
                widget(i)->move(0, 0);
            }
            sender()->deleteLater();
        });

        group->start();
    }

private:
    QPoint calculateOffset(SlideDirection dir) const
    {
        int w = width();
        int h = height();
        switch (dir) {
        case SlideDirection::kLeftToRight:
            return {w, 0};
        case SlideDirection::kRightToLeft:
            return {-w, 0};
        case SlideDirection::kTopToBottom:
            return {0, h};
        case SlideDirection::kBottomToTop:
            return {0, -h};
        }
        return {w, 0};
    }
};
```

这里有几个关键细节值得注意。第一，新页面的 geometry 初始位置必须在调用 show() 之前设好，否则用户会看到新页面先在 (0, 0) 闪一下再跳到偏移位置——那一帧闪烁非常刺眼。第二，动画结束后必须调用 setCurrentWidget 或 setCurrentIndex 把 QStackedWidget 的内部状态同步过来，否则下次调用 currentIndex() 返回的还是旧页面的索引。第三，动画结束后要把所有页面的 pos 重置为 (0, 0)，因为 QStackedWidget 正常工作时所有页面的位置都是 (0, 0)，如果你不重置，下次手动调 setCurrentIndex 就会出现页面位置偏移的问题。

QEasingCurve 的选择直接决定了动画的体感。OutCubic 是最常用的——开始快、结束慢，给人一种"稳稳停下"的感觉。InOutCubic 更适合循环播放的动画。Linear 几乎不用，因为它看起来机械、没有物理感。你可以用 QEasingCurve::Type 枚举里提供的几十种曲线做实验，但实际项目中 90% 的场景用 OutCubic 或 OutQuad 就够了。

### 3.2 淡入淡出 + 缩放的组合过渡效果

纯滑动不是唯一选择。移动端应用大量使用的是淡入淡出（opacity 渐变）配合轻微的缩放（scale），这种过渡比滑动更轻量、更"现代"。Qt 中实现淡入淡出的标准做法是给 QWidget 设置 QGraphicsOpacityEffect，然后用 QPropertyAnimation 动画改变 effect 的 opacity 属性。

```cpp
void fadeIn(QWidget *target, int duration = 250)
{
    auto *effect = new QGraphicsOpacityEffect(target);
    target->setGraphicsEffect(effect);
    effect->setOpacity(0.0);

    auto *anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(duration);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);

    connect(anim, &QPropertyAnimation::finished, effect, [target]() {
        target->setGraphicsEffect(nullptr);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
```

这里有一个很容易踩的坑：QGraphicsOpacityEffect 会给 QWidget 加一层独立的绘制层。动画结束后如果你不把 effect 移除（setGraphicsEffect(nullptr)），这个额外的绘制层会一直存在，在某些平台和某些 QSS 配置下会导致渲染性能下降甚至显示异常。特别是当你同时使用了 QSS 的 background-image 或者半透明背景时，opacity effect 可能会和 QSS 的绘制逻辑产生冲突——表现为文字模糊、背景色叠加错误等。

如果你想把淡入淡出和滑动组合在一起——比如新页面从右侧滑入的同时还有从 0 到 1 的透明度变化——就把 pos 动画和 opacity 动画都加到 QParallelAnimationGroup 里同步执行。组合动画的重点是时长的协调：滑动和透明度的 duration 必须一致，否则一个动画结束了另一个还在跑，效果会非常违和。

### 3.3 动画期间冻结交互

动画播放过程中有一个棘手的问题：用户如果在动画还没结束时快速点击导航切换，就会出现状态混乱。比如当前正在从页面 0 滑到页面 1，动画进行到一半用户又点了页面 2，这时候你同时启动了两组动画——旧页面的位置已经不在 (0, 0) 了，新动画的偏移计算全部出错。

解决方案是在动画播放期间屏蔽用户输入。最简单的做法是维护一个 `m_animating` 标志位，slideToIndex 开始时设为 true，finished 回调中设为 false。在标志位为 true 期间，所有的切换请求直接忽略或者入队等待。

```cpp
bool m_animating = false;

void slideToIndex(int index, SlideDirection direction)
{
    if (m_animating || index == currentIndex()) {
        return;
    }
    m_animating = true;
    // ... 启动动画 ...

    connect(group, &QParallelAnimationGroup::finished, this,
            [this]() {
        m_animating = false;
    });
}
```

如果你不想直接丢弃用户在动画期间的输入，可以用一个 `m_pendingIndex` 记录用户最后一次请求的目标索引，动画结束后检查是否有待处理的切换请求，如果有就立即启动新的切换动画。

还有一种更粗暴但更省事的方案：在动画期间给整个 QStackedWidget 设置 setEnabled(false)，动画结束后恢复。这会阻止所有鼠标和键盘事件到达子控件，视觉上用户会看到控件变灰（取决于 QSS），清楚地知道"系统在忙"。

### 3.4 窗口大小变化时的动画适配

滑动动画的偏移量依赖 QStackedWidget 的当前尺寸（width 或 height）。如果动画播放过程中窗口大小发生了变化——比如用户拖拽了窗口边框——offset 计算的基础就变了。动画开始时偏移量是 800 像素（窗口宽 800），但动画还没结束窗口被拉宽到了 1000，新页面的终点位置 (0, 0) 仍然是正确的，但旧页面滑出的距离不够，会停在视口中间而不是完全滑出去。

解决这个问题的思路有两个。第一种是在动画期间禁止窗口大小变化——如果你的 QStackedWidget 在一个固定大小的 QDialog 里，这不是问题。第二种是监听 resizeEvent，在窗口大小变化时调整正在进行的动画的 endValue。但 QPropertyAnimation 在运行时修改 endValue 的行为在 Qt 文档中没有明确保证，实测中虽然可以生效但时间参数不会自动调整，可能导致动画速度突变。

更稳健的做法是在 resizeEvent 中重新计算所有页面的 geometry。因为 QStackedWidget 的页面 geometry 应该和 QStackedWidget 自身大小一致，所以在 resize 的时候把所有页面的 setGeometry 更新一下就行了。但注意，如果动画正在进行中且页面位置被动画控制着，你不能直接暴力 setGeometry——需要暂停动画、更新几何、再恢复动画。实际工程中最省事的做法就是在动画播放期间不让窗口 resize，或者把动画时长控制得足够短（200-300ms），让窗口 resize 和动画播放重合的概率降到最低。

## 4. 踩坑预防

第一个坑是动画结束后不调用 setCurrentIndex 或 setCurrentWidget 同步内部状态。QStackedWidget 内部维护着一个 currentIndex，你通过手动移动页面位置实现了视觉上的切换，但 QStackedWidget 的 currentIndex 还是旧值。这会导致 currentIndex() 和 currentWidget() 返回错误的结果，后续所有依赖这些方法的逻辑（比如导航控件的选中同步、页面状态判断）全部出错。后果就是下一次调用 slideToIndex 时 index == currentIndex() 判断为 true，切换直接被跳过。解决方案是在 finished 回调中必须调用 setCurrentWidget(next) 同步状态。

第二个坑是 QGraphicsOpacityEffect 和 QSS 背景的冲突。给一个有 QSS 半透明背景或 background-image 的控件加上 QGraphicsOpacityEffect 后，在某些平台（特别是 Linux + X11）上会出现文字模糊、背景色叠加异常。原因是 opacity effect 在绘制时引入了额外的合成层，和 QSS 的绘制路径产生了重叠。后果就是动画看起来正常但页面上文字像被涂了一层毛玻璃。如果你的页面使用了复杂的 QSS 样式，尽量用滑动动画而不是淡入淡出——滑动不依赖 effect，不会和 QSS 冲突。

第三个坑是动画结束时忘记重置页面 pos。QStackedWidget 正常工作时所有页面都在 (0, 0)，如果你在动画结束后不把旧页面和新页面的 pos 重置回 (0, 0)，下次用户手动调用 setCurrentIndex（不走你的 slideToIndex）时，页面会出现在偏移后的位置——你可能看到页面内容只显示了左半边或者完全不可见。养成习惯：finished 回调里遍历所有页面把 pos 重置。

## 5. 练习项目

练习项目：带动画的设置中心导航。我们要做一个视觉效果接近移动端的设置界面。

完成标准是：左侧 QListWidget 作为导航列表，包含六个项目："通用"、"账户"、"通知"、"外观"、"隐私"、"关于"。右侧是一个自定义的 SlidingStackedWidget，每个导航项对应一个页面。点击导航列表时，页面以滑动动画切换——从上到下切换时新页面从下方滑入，从下到上切换时新页面从上方滑入。动画期间导航列表不可点击（setEnabled(false)）。动画使用 OutCubic 缓动曲线，时长 280ms。窗口标题通过 currentChanged 信号动态更新为"设置中心 — [页面名]"。SlidingStackedWidget 需要在 resizeEvent 中正确处理页面几何，确保窗口大小变化时不会出现页面错位。

提示几个关键点：根据 currentIndex 和目标 index 的大小关系决定滑动方向——index 增大往左滑，index 减小往右滑；animation group 的 finished 回调里必须 setCurrentWidget + 重置 pos + 恢复导航列表 enabled；可以用 QSignalMapper 或者 Lambda 把导航项和页面索引绑定起来。

## 6. 官方文档参考链接

[Qt 文档 · QStackedWidget](https://doc.qt.io/qt-6/qstackedwidget.html) -- 堆叠页面控件，包含 addWidget/removeWidget/setCurrentIndex 等接口

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画类，用于动画化 QObject 的 Qt 属性

[Qt 文档 · QParallelAnimationGroup](https://doc.qt.io/qt-6/qparallelanimationgroup.html) -- 并行动画组，同步执行多个动画

[Qt 文档 · QEasingCurve](https://doc.qt.io/qt-6/qeasingcurve.html) -- 缓动曲线，控制动画的时间-值映射关系

[Qt 文档 · QGraphicsOpacityEffect](https://doc.qt.io/qt-6/qgraphicsopacityeffect.html) -- 透明度效果，用于实现淡入淡出

---

到这里，QStackedWidget 的进阶内容就拆完了。核心收获就是一个：QStackedWidget 本身不支持动画，但你可以绕过它的 setCurrentIndex，用 QPropertyAnimation 手动控制页面位置来实现滑动、淡入淡出甚至更复杂的过渡效果。关键细节在于动画结束后的状态同步（必须 setCurrentWidget）、页面位置的重置（所有页面回到 (0, 0)）、以及动画期间的交互冻结（防止状态混乱）。把这些环节串起来，你就能做出流畅的页面切换体验——而不只是一个"能切换但不流畅"的堆叠容器。
