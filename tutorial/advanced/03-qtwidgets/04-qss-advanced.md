---
title: "3.4 QSS 进阶"
description: "规则明明写了，按钮还是老颜色。qproperty 配上 QSizeF 类型的属性，静默失败，一行警告都没有。主题切换完，总有几个控件还是旧配色。界面搬上 4K 屏，px 值不跟着 DPI 缩放走。这些现象的根子分别在样式表的作用域边界、qproperty 的类型限制、polish/unpolish 的刷新链路和 QSS 像素值的 DPI 语义里，本篇逐个讲清。配套示例在 examples/advanced/03-qtwidgets/04-qss-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.4——QSS 进阶

[入门篇 QSS 基础](../../beginner/03-qtwidgets/04-qss-stylesheet-beginner.md) 把选择器、伪状态、子控件选择器过了一遍，也从文件加载过 QSS，做过亮暗主题切换的小练习。只做内部工具，那些知识确实够用。可项目一大，麻烦就不在语法上了。明明 QSS 写了，样式就是不生效，咱们排查一个下午，最后发现两条规则一条在全局样式表里，一条在控件自己的 setStyleSheet() 里，特异性根本没上场。动态切完主题，界面上总有几个控件还是旧配色，因为它们在 C++ 代码里被单独设过样式。界面放到 4K 显示器上跑，控件尺寸缩了一圈，QSS 里硬编码的 px 值没跟着 DPI 走。咱们把四个维度走完整：特异性在哪个范围内作数，qproperty-* 的类型边界在哪里，polish/unpolish 怎么决定主题切换刷不刷得干净，高 DPI 下的 px 值怎么处理。

## 样式不生效的根子：特异性只在同一张样式表里作数

入门篇讲过 QSS 优先级的雏形：控件级别的 setStyleSheet() 优先于全局的 QApplication::setStyleSheet()，更具体的选择器优先于更宽泛的选择器。进阶要把边界补齐。官方文档没有给出 CSS 那样的权重公式，咱们手里只有一组相对关系，摆成一张表。

| 选择器类型 | 写法                          | 特异性关系                 |
| ---------- | ----------------------------- | -------------------------- |
| ID 选择器  | QPushButton#deleteButton      | 远高于属性选择器和类选择器 |
| 属性选择器 | QPushButton[flat="true"]      | 低于 ID 选择器             |
| 类选择器   | QPushButton                   | 基准档                     |
| 后代选择器 | QGroupBox QPushButton         | 比单独的类选择器更具体     |

组合选择器的权重是各档相加，QGroupBox QPushButton#deleteButton 就能压过单用的 QPushButton#deleteButton。两条规则的特异性完全相同时怎么算，得看它们住哪儿。同一张样式表里，后加载的覆盖先加载的，入门篇教的规则照旧成立。跨表就没有“后写”可讲了，比的是出身层级，应用级的全局样式表算浅一层，控件自己 setStyleSheet() 写的那张算深一层，层级深的压过层级浅的。还有时机要留意：QSS 的样式解析发生在控件第一次被 show 或者样式表被 set 的时候，不是每次重绘都重新来一遍。咱们在窗口构造函数里设了全局样式表，又在某个按钮的初始化里调了 setStyleSheet()，控件级别的样式从此就落在这个按钮身上，对它设过的属性，之后再改全局样式表都不响应，控件级别的优先级更高。

咱们拿一道调试题把作用域的问题演出来。全局样式表里放着最具体的一条，按钮自己身上又调了 setStyleSheet()，写的是最宽泛的一条：

```css
/* 全局样式表：最具体的规则 */
QPushButton#deleteButton {
    background-color: #E74C3C;
}

/* 按钮自身 setStyleSheet()：最宽泛的规则 */
QPushButton {
    background-color: #3498DB;
    color: white;
    border-radius: 4px;
}
```

按“ID 选择器更具体”的直觉，咱们会赌红色赢，实际跑起来按钮是蓝色。跨表时特异性不参赛，比的是出身层级。两类表的选择器会进同一张加权规则表一起排序，可每条规则的权重里，出身占的分量压过特异性一大截，控件自己的表永远压过全局表，全局表里那条 ID 规则再具体也翻不了盘。特异性只在各自的表内决定先后，咱们把两条规则挪进同一张样式表，它才说了算。开篇那个“排查一个下午”的场景，根子就出在把规则写进了两张样式表。

正确的做法是把两条规则放进同一张样式表，要么都放全局，要么都放控件级别，不要混用。同一张样式表内的特异性是确定可预测的，QPushButton#deleteButton 一定优先于 QPushButton。往工程里推一步就是一条纪律：所有样式规则都收进同一张样式表，最好是全局的，用 objectName 和选择器区分控件，不在 C++ 代码里散落 setStyleSheet() 调用。控件级别样式表里的选择器要是写得不够具体，还可能匹配不到咱们想改的那个控件，排查起来又是一轮。

配套示例在 `examples/advanced/03-qtwidgets/04-qss-advanced/`，`cmake -B build && cmake --build build` 构建后运行。示例程序叫 ThemeSwitcher，窗口从上到下三个演示区：动态主题切换、选择器特异性、样式级联。选择器特异性演示区把三条规则全写在同一张全局 QSS 里。左边按钮只由 QPushButton 匹配，是默认蓝色。中间按钮的 objectName 是 descendantButton，由 QGroupBox QPushButton#descendantButton 匹配，紫色底加虚线边框。右边按钮的 objectName 是 deleteButton，由 QPushButton#deleteButton 匹配，红色。咱们挨个点过去，下方说明标签会把每个按钮命中的规则原样报出来，谁压过谁一目了然。

## qproperty-*：把属性值搬进样式表，类型对不上就静默失败

QSS 有一个不太为人知但很有用的能力：通过 `qproperty-<属性名>` 语法，直接在样式表里设置控件的 Q_PROPERTY 值。咱们把原本要在 C++ 代码里设的属性值下沉到 QSS 里，设计师或最终用户改一个 QSS 文件就能定制控件行为，不用重新编译代码。

```css
MyCustomProgressBar {
    qproperty-barColor: #27AE60;
    qproperty-barHeight: 8;
    qproperty-showText: false;
}
```

这段 QSS 的前提是 MyCustomProgressBar 类用 Q_PROPERTY 声明了这些属性，而且属性类型得是 QSS 能解析的。咱们把类型边界摆成表：

| 属性类型                             | qproperty-* 的结果        |
| ------------------------------------ | ------------------------- |
| QString / int / double / bool        | 能正常设置                |
| QColor / QFont                       | 能正常设置                |
| QIcon / QImage / QPixmap / QRect / QBrush / QKeySequence | 能正常设置 |
| 用 Q_ENUM 声明的枚举（按常量名写值） | 能正常设置                |
| QSizeF / QRectF                      | 静默失败，不生效也不报错  |

“静默失败”值得您多看一眼。咱们要是把属性声明成 `Q_PROPERTY(QSizeF iconSize READ iconSize WRITE setIconSize)`，再在 QSS 里写 `qproperty-iconSize: 32px 32px`，它就无声无息地没了：QSizeF 没有专门的解析分支，落进默认分支被静默忽略，不报错不警告，控件行为不变，调试时完全找不到线索。QSize 反而有专门分支，同样的 `32px 32px` 能正常设上，一字母之差，一个能设一个没动静。对策也直白，只对表里能解析的类型用 qproperty-*，复杂类型的自定义属性要么拆成多个简单类型的属性，要么放弃 QSS 设置，改用 C++ 代码赋值。

类型之外还有设置时机。QSS 的属性赋值发生在 polish() 阶段，也就是控件第一次被样式化的时候。咱们在 C++ 代码里先设了属性值，QSS 又设了同一个属性，QSS 的值会覆盖代码设置。反过来，QSS 设完之后咱们又改了属性，代码的修改会一直生效，直到下一次样式刷新，比如调用 style()->unpolish(widget) 再 style()->polish(widget)，QSS 的值才重新接管。动态主题切换正好会触发样式刷新，这个时序您得提前想清楚。

## 主题切换切不干净：polish/unpolish 与控件级样式表的残留

入门篇的主题切换很直接，读一个 QSS 文件，调一次 QApplication::setStyleSheet() 就完了。小应用里这么做没毛病，应用一大咱们就会碰到切换后部分控件不刷新的问题，根子在 Qt 样式系统的 polish/unpolish 机制。

两个函数各自管什么，咱们分开看。控件的样式要被应用时，QStyle 的 polish(QWidget*) 会被调用，负责初始化控件的视觉属性，比如设置调色板、安装事件过滤器。样式要被移除或更换时，unpolish(QWidget*) 负责清理之前 polish 设置的东西。咱们调用 QApplication::setStyleSheet() 替换全局样式表时，Qt 会自动对所有现有控件执行 unpolish 然后重新 polish，理论上所有控件都应该刷新。实际上，控件级样式表的声明权重更高，它设过的属性，全局表改不动，它没设的属性，全局表照样生效。主题切不干净的残留，正是控件级表设过的那几个属性一直压着新主题。

咱们能做的根治办法只有一件，就是彻底避免在 C++ 代码里对单个控件调用 setStyleSheet()，所有样式规则都写在全局样式表里，通过选择器区分控件。您确实要对某个控件单独设样式，用 setObjectName() 配 #id 选择器在全局 QSS 里定位它：

```cpp
void apply_theme(QApplication &app, const QString &theme_name)
{
    // 所有样式都在全局 QSS 中，不在 C++ 代码里单独设
    QString path = QString(":/themes/%1.qss").arg(theme_name);
    QString qss = load_qss(path);
    if (!qss.isEmpty()) {
        app.setStyleSheet(qss);
    }

    // 如果有特殊控件需要强制刷新，手动触发 unpolish + polish
    for (auto *widget : app.allWidgets()) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
}
```

手动遍历所有控件执行 unpolish/polish 是比较暴力的做法，控件数量特别多时可能有一瞬间的闪烁。应用结构允许的话，更好的方案是让所有控件响应 QEvent::StyleChange 事件自行刷新，不依赖手动触发。不过对大多数中型项目来说，咱们用上面的暴力方案就应付得来了。

配套示例走的就是根治路线。theme_switcher.cpp 里亮、暗两套主题各是一整段全局 QSS，同一个选择器在两套里各对应一份颜色值，applyTheme() 里只有 qApp->setStyleSheet() 这一招，整个工程找不到第二处样式设置。咱们在下拉框里把 Light 切成 Dark，三个演示区连同输入框、按钮一起当场换装，没有任何控件留在旧主题里。最下面的级联演示区还能多看一层父子关系：#cascadeParent 容器有自己的底色和边框，里面的 QLabel 把背景设成 transparent，透出容器底色，同容器里的 QPushButton 走 QPushButton 规则自己的背景色，不吃继承，主题切换时连容器带子控件一起变。

## 4K 屏上控件缩了一半：QSS 的 px 值与 DPI 缩放脱节

QSS 在工程实践里最隐蔽的问题之一就出在高 DPI 上。咱们在 1080p 屏幕上调得好好的界面，放到 4K 屏（150% 或 200% 缩放）上，控件尺寸全缩了，间距变小，字体大小正常但边框和 padding 不对，按钮看起来比文字小了一圈，整块界面的视觉比例失调。开发机上完全看不出来，部署到高缩放率的设备上才原形毕露。

原因得从 px 的名义语义说起。QSS 里的 px 名义上是设备无关像素（device-independent pixels），Qt 的自动缩放机制会处理布局管理器中的尺寸、字体大小这一类，但 QSS 里硬编码的 px 值在某些 Qt 版本和平台组合下，不会自动跟随 DPI 缩放。咱们写了 `padding: 8px`，1x 缩放下看起来正好，2x 缩放下它可能还是 8 个物理像素而不是 16 个，控件的内容区域就比预期小。它不算 Qt 的 bug，来由是 QSS 的像素值语义在不同渲染后端中行为不一致。

一个方向是尽量用相对单位替代绝对 px。QSS 对相对单位的支持很有限，没有 CSS 的 em 和 rem，咱们能借道的是 font-size：把关键的间距值和 font-size 挂钩，在代码里根据 DPI 动态设置字体大小，间距跟着字体走。

```cpp
// 根据 DPI 计算基础字号，所有 QSS 中的尺寸基于此缩放
// dark.qss 里的基础字号占位写作 font-size: %1px
qreal dpr = screen()->devicePixelRatio();
int base_font_size = qRound(13 * dpr);
app.setStyleSheet(load_qss(":/themes/dark.qss").arg(base_font_size));
```

另一个方向是在代码里做预处理：咱们在加载 QSS 文件之后，用一个简单的字符串替换函数把所有 px 值乘上当前的 DPI 缩放系数。

```cpp
QString scale_qss(const QString &qss, qreal factor)
{
    // 把 "8px" 替换为 "16px"（factor=2 时）
    QRegularExpression re("(\\d+)px");
    QString result = qss;
    QRegularExpressionMatchIterator it = re.globalMatch(qss);
    int offset = 0;
    while (it.hasNext()) {
        auto match = it.next();
        int scaled = qRound(match.captured(1).toInt() * factor);
        QString replacement = QString::number(scaled) + "px";
        result.replace(match.capturedStart() + offset,
                       match.capturedLength(), replacement);
        offset += replacement.length() - match.capturedLength();
    }
    return result;
}
```

替换方案有点粗暴，但对大多数项目来说最快见效。咱们要是知道应用只跑在固定的几个 DPI 档位（比如 1x、1.5x、2x），还可以直接准备三套不同 px 值的 QSS 文件，运行时按 DPI 选一套加载，省掉运行时字符串替换的开销。再退一步，尽量减少 QSS 里硬编码的绝对尺寸，能不写死的就不写死。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，内容依赖 QtWidgets 模块，动态主题部分涉及 QStyle 的 polish/unpolish 机制，高 DPI 部分涉及 QScreen 的 devicePixelRatio，示例可在任何支持 Qt6 的桌面平台上编译运行。

[Qt 文档 · Qt Style Sheets](https://doc.qt.io/qt-6/stylesheet.html) -- QSS 完整参考，包含选择器和属性列表

[Qt 文档 · Qt Style Sheets Reference](https://doc.qt.io/qt-6/stylesheet-reference.html) -- 按控件分类的所有可样式化属性列表

[Qt 文档 · The Style Sheet Syntax](https://doc.qt.io/qt-6/stylesheet-syntax.html) -- QSS 语法详解，包含选择器类型和 qproperty-* 语法

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- QStyle 基类，包含 polish / unpolish 方法说明

[Qt 文档 · High DPI](https://doc.qt.io/qt-6/highdpi.html) -- Qt 高 DPI 支持概述，包含 QT_SCALE_FACTOR 环境变量说明

---

QSS 进阶就到这里。想再动手的，请您把 ThemeSwitcher 完整玩一轮：在下拉框里把主题切成 Dark 再切回 Light，看全局样式表怎么把三个演示区一次换干净，再把“选择器特异性”区的三个按钮各点一遍，对照下方说明标签看每个按钮命中了哪条规则，最后看级联演示区里 transparent 的 QLabel 怎么透出父容器的底色。上一篇 [3.3 Model/View 进阶](./03-model-view-advanced.md) 讲数据与视图的分工，下一篇 [3.5 自定义控件进阶](./05-custom-widget-advanced.md) 把绘制拿到自己手里，QSS 覆盖不到的效果，要去 paintEvent 里画。
