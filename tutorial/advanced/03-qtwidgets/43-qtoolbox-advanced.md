---
title: "3.43 QToolBox 进阶"
description: "入门篇我们用 QToolBox 搭了折叠面板导航，掌握了 addItem/insertItem/setCurrentIndex/currentChanged 信号/setItemEnabled 等基本操作。"
---

# 现代Qt开发教程（进阶篇）3.43——QToolBox 进阶

## 1. 前言 / 当折叠面板不再只是"展开/收起"

入门篇我们用 QToolBox 搭了折叠面板导航，掌握了 addItem / insertItem / setCurrentIndex / currentChanged 信号 / setItemEnabled 等基本操作。对于一个功能简单的侧边栏导航，入门篇那一套完全够用——点击标题栏展开对应面板，其余面板自动收起，互斥切换，一目了然。但 QToolBox 的默认外观有一个明显的问题：标题栏的样式完全由平台 QStyle 决定，你不能改变标题文字的字体大小、颜色、图标位置，展开和收起之间没有任何过渡效果，标题栏的点击热区也不可控。如果你想把 QToolBox 做成一个产品级的侧边栏——类似 Figma 的图层面板、或者 KDE 系统设置左侧那种带图标的分组导航——你就得绕过 QToolBox 的默认标题渲染，自己控制每一帧。

今天我们把 QToolBox 的定制能力拆透。核心内容是三个方面：通过 QSS 和 setItemIcon/setItemText 定制标题栏样式、QToolBox 内部 QToolBoxButton 的访问方式与事件拦截、以及用 QPropertyAnimation 实现面板展开/收起的过渡动画。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QToolBox 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。动画部分涉及 QPropertyAnimation 和 QEasingCurve（QtCore）。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 QSS 定制标题栏样式

QToolBox 的标题栏在 Qt 内部由 QToolBoxButton 渲染——这是一个私有的 QWidget 子类，不暴露在公共 API 中。但你可以通过 QSS 的 `QToolBox::tab` 选择器来定制它的外观。QSS 为 QToolBox 提供了以下伪状态：`:selected` 表示当前展开的面板标题，`:!selected` 表示折叠状态。

```css
/* QToolBox 标题栏定制 */
QToolBox::tab {
    background: #2d2d2d;
    color: #cccccc;
    padding: 8px 12px;
    border: none;
    border-bottom: 1px solid #3d3d3d;
    font-size: 13px;
    font-weight: bold;
}

QToolBox::tab:selected {
    background: #3d3d3d;
    color: #ffffff;
    border-bottom: 2px solid #4a9eff;
}

QToolBox::tab:!selected {
    background: #252525;
}

QToolBox::tab:hover {
    background: #383838;
}
```

这段 QSS 做了几件事：默认标题栏深灰色背景浅灰色文字，当前展开的面板标题背景稍亮且底部有蓝色指示条，鼠标悬停时背景变色。padding 控制标题文字的内边距——默认的标题栏非常紧凑，通过增大 padding 可以让标题栏更容易点击。

setItemIcon(int index, const QIcon& icon) 可以给标题栏添加图标。图标显示在文字左侧。一个实用技巧是准备两组图标（正常态和选中态），然后在 currentChanged 信号中动态切换图标，让用户能直观区分当前展开的是哪个面板。

```cpp
connect(toolbox, &QToolBox::currentChanged, this,
        [this](int index) {
    for (int i = 0; i < toolbox->count(); ++i) {
        bool active = (i == index);
        toolbox->setItemIcon(i, active ? m_activeIcons[i]
                                        : m_normalIcons[i]);
    }
});
```

QSS 对 QToolBox 的定制能力有一个明显的限制：你不能通过 QSS 改变标题栏的布局结构——比如把图标放到右侧、在标题文字旁边加一个计数标签、或者嵌入一个操作按钮。这些深度定制需要直接操作 QToolBoxButton，或者干脆不用 QToolBox 改用 QGroupBox + QParallelAnimationGroup 自己搭一套折叠面板。

### 3.2 访问内部 QToolBoxButton

QToolBox 没有暴露获取标题栏按钮的公共 API——没有 `tabButton(int index)` 这样的方法。但 QToolBoxButton 是作为 QToolBox 的子对象存在的，你可以通过遍历子对象来找到它。

```cpp
// 查找指定索引的标题栏按钮
QWidget *findToolBoxButton(QToolBox *toolbox, int index)
{
    // QToolBoxButton 是 QToolBox 的直接子对象
    // 它们的顺序和面板索引一致
    const auto children = toolbox->children();
    int btn_count = 0;
    for (auto *child : children) {
        auto *btn = qobject_cast<QWidget*>(child);
        if (btn && btn->inherits("QToolBoxButton")) {
            if (btn_count == index) {
                return btn;
            }
            ++btn_count;
        }
    }
    return nullptr;
}
```

找到按钮指针后你就可以做一些高级操作了：给按钮安装事件过滤器拦截鼠标事件、改变按钮的 sizePolicy、在按钮上覆盖一层自定义绘制。但这种做法本质上是在访问 Qt 的内部实现——QToolBoxButton 是一个私有类，它的类名和行为在不同 Qt 版本之间可能发生变化。如果你的项目需要长期维护，更稳健的做法是继承 QToolBox 并重写它的虚拟函数，或者完全用自定义控件替代 QToolBox。

一个更安全的方式是利用 QToolBox 的 itemTooltip、itemWhatsThis 等元数据接口——这些是公共 API，可以安全使用。比如 setItemToolTip(int index, const QString&) 给标题栏添加悬停提示，setItemAccessibleText 和 setItemAccessibleDescription 提升无障碍支持。

### 3.3 展开/收起动画实现

QToolBox 默认的展开/收起是瞬间切换——旧面板 hide()，新面板 show()，没有任何过渡。要实现类似手风琴（accordion）那种平滑展开收起效果，思路和 QStackedWidget 的滑动动画类似：不依赖 QToolBox 自带的切换逻辑，而是用 QPropertyAnimation 控制面板的 maximumHeight。

```cpp
void animateExpand(QWidget *panel, int target_height, int duration = 250)
{
    auto *anim = new QPropertyAnimation(panel, "maximumHeight");
    anim->setDuration(duration);
    anim->setStartValue(0);
    anim->setEndValue(target_height);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QPropertyAnimation::finished, panel, [panel]() {
        panel->setMaximumHeight(16777215);  // QWIDGETSIZE_MAX
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void animateCollapse(QWidget *panel, int duration = 200)
{
    auto *anim = new QPropertyAnimation(panel, "maximumHeight");
    anim->setDuration(duration);
    anim->setStartValue(panel->height());
    anim->setEndValue(0);
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
```

这里用 maximumHeight 而不是固定 height 是因为 QToolBox 内部用布局管理子控件的大小。如果你直接 setHeight，QToolBox 的布局可能在下一帧把它覆盖回去。通过动画 maximumHeight，面板会在布局允许的范围内逐渐变大或变小，视觉上就是平滑展开或收起。

动画结束后必须把 maximumHeight 恢复为默认值（16777215，即 QWIDGETSIZE_MAX），否则面板的最大高度被锁在动画终值，后续窗口大小变化时面板无法正常伸展。这个坑很容易踩——展开动画结束后如果不恢复，面板内容增多时会被截断，用户怎么拖窗口都看不到下面的内容。

这个方案有一个局限：它不能和 QToolBox 的内置切换逻辑直接集成。当你调用 setCurrentIndex 时，QToolBox 会立即 hide 旧面板、show 新面板，把你的动画覆盖掉。如果你要动画，就需要在 currentChanged 信号中阻止默认行为（用 blockSignals 或者子类化重写），改为手动控制面板显隐和动画播放。实际工程中更常见的做法是不用 QToolBox，改用一组自定义的 CollapsibleSection 控件——每个控件有自己的标题栏和内容区，点击标题栏触发动画，完全掌控行为。

## 4. 踩坑预防

第一个坑是 maximumHeight 动画结束后不恢复默认值。动画把 maximumHeight 设到了一个固定值（比如 300），动画结束后如果不恢复为 16777215，面板的高度就被永久限制住了。后果是面板内的内容超过 300 像素后会被截断，用户无法通过任何方式看到下面的内容——滚动条也不会出现，因为 QToolBox 的内部布局认为面板最大就 300。解决方案是展开动画的 finished 回调中一定要 setMaximumHeight(16777215)。

第二个坑是 QToolBoxButton 的访问依赖内部实现。QToolBoxButton 是私有类，没有公共头文件。你通过遍历子对象找到它之后，只能用 QWidget 的通用接口操作它，不能 static_cast 为 QToolBoxButton*。在不同 Qt 版本之间这个类的名字甚至可能不存在。如果你的代码需要跨 Qt 版本，不要直接访问 QToolBoxButton——改用 QSS 定制或者完全自定义实现。

第三个坑是 setItemEnabled(false) 的视觉效果不够明显。禁用的面板标题栏只是变灰了，但没有任何视觉提示告诉用户"这个面板不可用"。如果你的界面有禁用状态，建议在标题文字后面追加 "(已禁用)" 或者通过 setItemToolTip 提供说明，否则用户可能会反复点击那个灰色的标题栏以为程序卡了。

## 5. 练习项目

练习项目：自定义手风琴面板导航。我们不直接使用 QToolBox，而是用自定义的 CollapsibleSection 控件组合出类似但更灵活的效果。

完成标准是：创建一个 CollapsibleSection 类，每个实例包含一个可点击的标题栏（自定义绘制，带展开/收起箭头图标）和一个内容区（QVBoxLayout）。点击标题栏时，如果内容区是收起的就播放展开动画（maximumHeight 从 0 到内容实际高度），如果是展开的就播放收起动画。多个 CollapsibleSection 放在一个 QVBoxLayout 中纵向排列。标题栏悬停时背景变色，展开状态的标题栏有底部蓝色指示条。准备三个面板："快捷操作"（包含 4 个 QPushButton）、"最近文件"（包含一个 QListWidget 显示 5 个文件名）、"系统信息"（包含 QLabel 显示版本号和编译日期）。

提示几个关键点：展开动画的 endValue 需要先调用 content->sizeHint().height() 获取内容理想高度；收起动画的 startValue 用 content->height()；多个面板可以同时展开（不像 QToolBox 那样互斥），这反而更灵活；箭头图标可以用 QPainter 画一个三角形，展开时旋转 90 度。

## 6. 官方文档参考链接

[Qt 文档 · QToolBox](https://doc.qt.io/qt-6/qtoolbox.html) -- 工具箱折叠面板控件，包含 addItem/setItemIcon/setCurrentIndex 等接口

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画类，用于动画化 QWidget 的属性

[Qt 文档 · QEasingCurve](https://doc.qt.io/qt-6/qeasingcurve.html) -- 缓动曲线，控制动画的速度曲线

---

到这里，QToolBox 的进阶内容就拆完了。QSS 可以定制标题栏的颜色和间距，但改不了布局结构。QToolBoxButton 是私有类，能找到但不稳定，跨版本别碰它。想要真正的展开收起动画，QToolBox 本身的切换逻辑会碍手碍脚——不如自己用 QPropertyAnimation 控制面板高度，效果和可控性都好得多。掌握了这些定制路径后，你就能做出产品级的折叠导航面板——而不是一个"能用但丑"的 QToolBox。
