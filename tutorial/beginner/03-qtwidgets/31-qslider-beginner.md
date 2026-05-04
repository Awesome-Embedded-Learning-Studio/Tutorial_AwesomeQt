# 现代Qt开发教程（新手篇）3.31——QSlider：滑动条

## 1. 前言 / 当你需要一个"拖动调参"的控件

前面几篇我们一直在讲各种输入框——QLineEdit 让用户输入文本，QSpinBox 让用户输入数字，QDateEdit 让用户选日期。这些控件有一个共同特征：用户需要精确地输入一个具体值。但还有很多场景下我们并不需要精确值，而是需要一个"大致的范围"——音量调节、亮度调节、进度控制、透明度设置、播放进度跳转。这些场景的共同特征是：值域是连续的（或者至少是密集离散的），用户通过拖动滑块来快速调整到一个"差不多"的位置，而不关心它是 73 还是 74。

QSlider 就是 Qt 为这类场景提供的标准控件。它呈现为一个带刻度槽的滑动条，中间有一个可拖拽的手柄。用户可以拖动手柄，也可以点击槽的任意位置来跳转，还可以用键盘的方向键微调。和 QSpinBox 一样，QSlider 也有 setRange / setValue 来设定范围和当前值，但它没有文本输入框——它纯粹是一个可视化调节控件，强调的是"拖拽操作"而不是"精确输入"。

今天我们要把 QSlider 讲透四个维度：水平/垂直方向的创建与选择，setRange / setValue / setSingleStep / setPageStep 的行为机制，valueChanged / sliderMoved / sliderReleased 三个信号的区别和适用场景，以及通过 QSS 自定义滑块外观。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QSlider 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。它继承自 QAbstractSlider，这个基类也是 QScrollBar 和 QDial 的父类，所以今天学到的 setRange / setValue / setSingleStep / setPageStep 以及信号机制，在 QScrollBar 和 QDial 中完全通用。示例代码中用到了 QLabel 和 QFormLayout 来展示数值变化，同样在 QtWidgets 中。

## 3. 核心概念讲解

### 3.1 水平与垂直方向

QSlider 的构造函数接受一个 `Qt::Orientation` 参数来决定方向：`Qt::Horizontal` 创建水平滑动条（从左到右），`Qt::Vertical` 创建垂直滑动条（从下到上）。如果不传参数，默认是水平的。

```cpp
// 水平滑动条
auto *horizontalSlider = new QSlider(Qt::Horizontal);

// 垂直滑动条
auto *verticalSlider = new QSlider(Qt::Vertical);
```

方向的选择取决于你的布局空间和交互习惯。水平 Slider 适合放在窗口的底部或中间区域——比如音乐播放器的进度条、图片编辑器的透明度调节。垂直 Slider 适合放在窗口的左侧或右侧——比如文本编辑器的缩放控制、音频混音器的音量推子。

有一个容易忽略的细节：对于水平 Slider，最小值在左侧、最大值在右侧，从左往右拖是增大；对于垂直 Slider，最小值在底部、最大值在顶部，从下往上拖是增大。这个方向可以用 `setInvertedAppearance(bool)` 来反转——设为 true 后，水平 Slider 的最小值会出现在右侧，垂直 Slider 的最小值出现在顶部。但说实话这个功能在实际项目中几乎用不到，默认方向符合绝大多数人的直觉。

QSlider 还可以显示刻度标记，通过 `setTickPosition(QSlider::TickPosition)` 控制。常用的选项有 `QSlider::NoTicks`（无刻度，默认值）、`QSlider::TicksBelow`（水平 Slider 下方显示刻度）、`QSlider::TicksAbove`（上方显示刻度）、`QSlider::TicksBothSides`（两侧都显示）。`setTickInterval(int)` 设置刻度之间的间隔值——注意它和 singleStep / pageStep 是独立的概念，刻度标记只是视觉提示，不影响实际的步进行为。

```cpp
auto *slider = new QSlider(Qt::Horizontal);
slider->setRange(0, 100);
slider->setTickPosition(QSlider::TicksBelow);
slider->setTickInterval(10);  // 每 10 个单位画一个刻度标记
```

刻度标记在高精度调节场景下很有用——它给用户一个视觉参考，让用户大致判断"当前滑块在整条滑动条的什么位置"。但如果你只是做一个简单的音量调节，不需要刻度，保持默认的 NoTicks 就行。

### 3.2 setRange / setValue / setSingleStep / setPageStep

这四个方法定义了 QSlider 的数值行为。和 QSpinBox 一样，`setRange(int min, int max)` 设定取值范围，`setValue(int)` 设定当前值。但 QSlider 有两个步进概念：`setSingleStep(int)` 和 `setPageStep(int)`。

```cpp
auto *slider = new QSlider(Qt::Horizontal);
slider->setRange(0, 1000);
slider->setValue(500);
slider->setSingleStep(1);
slider->setPageStep(50);
```

singleStep 控制的是"小步进"——当用户按键盘左右方向键时，每次移动的数值量。默认值是 1。pageStep 控制的是"大步进"——当用户按键盘 PageUp / PageDown 键时，每次移动的数值量。默认值是 10。另外，当用户点击滑块槽（而不是手柄）时，手柄也会以 pageStep 为步长向点击位置跳一格。

这里的交互模型可以这样理解：拖拽手柄是"自由调节"，不涉及步进，用户可以把值设为范围内的任意整数；键盘方向键是"精细调节"，按 singleStep 步进；点击槽或者按 PageUp/PageDown 是"粗略调节"，按 pageStep 步进。这种分层操作模型让同一个 Slider 同时满足"快速定位"和"精确微调"两种需求。

```cpp
auto *volumeSlider = new QSlider(Qt::Horizontal);
volumeSlider->setRange(0, 100);
volumeSlider->setValue(50);
volumeSlider->setSingleStep(1);   // 方向键每次 ±1
volumeSlider->setPageStep(10);    // PageUp/Down 每次跳 ±10

auto *seekSlider = new QSlider(Qt::Horizontal);
seekSlider->setRange(0, 3600);    // 假设视频 1 小时，以秒为单位
seekSlider->setSingleStep(5);     // 方向键每次 ±5 秒
seekSlider->setPageStep(30);      // PageUp/Down 每次跳 ±30 秒
```

有一点需要特别注意：QSlider 只支持 int 类型的值。如果你需要处理浮点范围（比如 0.0 到 1.0 的透明度），需要手动做映射——把 0.0-1.0 映射到 0-100 或者 0-1000 的整数范围，在取值时再换算回来。这不是 Qt 的设计缺陷，而是 QAbstractSlider 作为底层基类选择了最通用的整数类型。

```cpp
// 透明度 Slider: 0.0 - 1.0 映射到 0 - 100
auto *opacitySlider = new QSlider(Qt::Horizontal);
opacitySlider->setRange(0, 100);
opacitySlider->setValue(80);

// 取值时转换
double opacity = opacitySlider->value() / 100.0;  // 0.8

// 设值时转换
opacitySlider->setValue(static_cast<int>(opacity * 100 + 0.5));
```

setValue 时如果传入的值超出了 setRange 的范围，行为和 QSpinBox 一样——会被静默钳位到最近的边界。setValue 也会触发 valueChanged 信号。

### 3.3 valueChanged / sliderMoved / sliderReleased 信号区别

QSlider 继承自 QAbstractSlider，而 QAbstractSlider 提供了三个和值变化相关的信号。这三个信号的触发时机不同，理解它们的区别是用好 QSlider 的关键。

`valueChanged(int)` 是最常用的信号。每当 Slider 的值发生变化时触发——无论是用户拖动手柄、点击槽、按键盘，还是代码调用 setValue。它反映的是"值已经确定性地变了"。在实际项目中，你大部分时候连接的就是这个信号。

`sliderMoved(int)` 只在用户拖动手柄的过程中触发。它是一个"实时追踪"信号——用户每拖动一点，sliderMoved 就发射一次，携带的是手柄当前所在位置的值。注意：sliderMoved 不会在代码调用 setValue 或者键盘操作时触发，它纯粹是鼠标拖拽行为的信号。

`sliderReleased()` 在用户松开鼠标手柄时触发，不带参数。它只告诉你"用户完成了一次拖拽操作"，但你需要自己调用 value() 来获取最终值。

```cpp
auto *slider = new QSlider(Qt::Horizontal);
slider->setRange(0, 100);

// 值变化（所有来源都触发）
connect(slider, &QSlider::valueChanged,
        this, [](int value) {
    qDebug() << "值变为:" << value;
});

// 用户正在拖拽（只有鼠标拖拽触发）
connect(slider, &QSlider::sliderMoved,
        this, [](int position) {
    qDebug() << "拖拽中，位置:" << position;
});

// 用户松开手柄
connect(slider, &QSlider::sliderReleased,
        this, [slider]() {
    qDebug() << "松开手柄，最终值:" << slider->value();
});
```

为什么要区分这三个信号？因为不同场景对"实时性"和"确定性"的要求不同。拿音量调节举例：如果你希望在用户拖动的过程中实时听到音量变化，连接 sliderMoved；如果你只需要在用户选定了最终值之后才更新音量（避免拖动过程中频繁触发昂贵的音频系统重新配置），连接 sliderReleased 后调用 value() 或者连接 valueChanged 但加一个防抖逻辑。

还有一种常见的模式是"sliderMoved 做实时预览，valueChanged 做最终提交"。比如图片编辑器中的亮度调节——拖动过程中 sliderMoved 信号实时更新预览（轻量操作），拖动结束后的 valueChanged 信号触发实际的图像重计算（重量操作）。但这种模式有一个陷阱：如果用户是通过键盘操作而不是鼠标拖拽来改变值，sliderMoved 不会触发，你的预览就不会更新。所以更安全的做法是统一用 valueChanged 做所有事情，只在"操作频率过高导致性能问题"时才考虑拆分。

还有一个辅助信号 `actionTriggered(int)` 会告诉你触发了哪种动作（SliderSingleStepAdd、SliderPageStepAdd、SliderMove 等），但这个信号在实际开发中很少用到。

### 3.4 QSS 自定义滑块外观

默认的 QSlider 外观是平台原生风格——Windows 上是那种蓝色的系统滑块，macOS 上是灰色的圆角滑块。但在自定义 UI 中，我们经常需要完全控制滑块的外观。QSS 提供了完整的子控件选择器来定制 QSlider 的每个视觉组成部分。

QSlider 的 QSS 子控件有三个：`groove`（滑槽，即背景轨道）、`handle`（手柄，即用户拖拽的那个小方块或小圆点）、`add-page` 和 `sub-page`（已填充区域和未填充区域）。

```css
QSlider::groove:horizontal {
    height: 6px;
    background: #E0E0E0;
    border-radius: 3px;
}

QSlider::handle:horizontal {
    width: 16px;
    height: 16px;
    margin: -5px 0;
    background: #1976D2;
    border-radius: 8px;
}

QSlider::sub-page:horizontal {
    background: #1976D2;
    border-radius: 3px;
}
```

这段样式定义了一个蓝色的圆形手柄和灰色的滑槽，手柄左侧的槽填充为蓝色。groove 的 height 控制滑槽的粗细，handle 的 margin 负值让手柄在垂直方向上超出滑槽（这样手柄才能被拖拽——如果手柄和滑槽一样大就太小了）。sub-page 是手柄左侧（已滑过的区域），add-page 是手柄右侧（未滑过的区域）。

对于垂直方向的 Slider，子控件名称变成 `groove:vertical`、`handle:vertical` 等。垂直 Slider 的 groove 宽度用 width 而不是 height，handle 的 margin 也要相应调整。

```css
QSlider::groove:vertical {
    width: 6px;
    background: #E0E0E0;
    border-radius: 3px;
}

QSlider::handle:vertical {
    width: 16px;
    height: 16px;
    margin: 0 -5px;
    background: #1976D2;
    border-radius: 8px;
}

QSlider::sub-page:vertical {
    background: #1976D2;
    border-radius: 3px;
}
```

伪状态也可以用——`:hover` 和 `:pressed` 让手柄在鼠标悬停和按下时呈现不同的视觉效果。

```css
QSlider::handle:horizontal:hover {
    background: #1565C0;
    transform: scale(1.1);
}

QSlider::handle:horizontal:pressed {
    background: #0D47A1;
}
```

不过 QSS 不支持 transform 属性——要实现悬停时手柄放大的效果，你需要用更大的 handle 尺寸配合透明边框来模拟。或者更彻底地，通过继承 QSlider 并重写 paintEvent 来实现完全自定义的绘制。QSS 方案适合大部分中等定制需求的场景；如果你需要非常复杂的滑块外观（比如带渐变色、带纹理、带动画），自定义绘制是唯一选择。

还有一个实用的小技巧：可以通过 `setStyleSheet("QSlider::groove:horizontal { ... } QSlider::handle:horizontal { ... }")` 的方式把样式直接写在代码里，也可以通过 `qApp->setStyleSheet()` 全局设置。后者适合需要统一主题的项目。

## 4. 踩坑预防

第一个坑是 QSlider 只支持 int。如果你需要浮点范围，必须自己做整数映射。设范围时要考虑精度——比如你需要 0.00 到 1.00 的精度，就把 range 设为 0 到 100；需要 0.000 到 1.000，就设为 0 到 1000。

第二个坑是 sliderMoved 只在鼠标拖拽时触发。如果你用 sliderMoved 做"实时预览"，用户通过键盘改值时你的预览不会更新。要么把预览逻辑也接到 valueChanged 上，要么确保 sliderMoved 和 valueChanged 的处理逻辑一致。

第三个坑是 QSS 中 handle 的 margin 负值。如果 margin 没设成负值，手柄会被夹在滑槽的范围内——看起来会非常小，几乎无法拖拽。对于水平 Slider，`margin: -5px 0` 让手柄在垂直方向超出 5px；对于垂直 Slider，`margin: 0 -5px` 让手柄在水平方向超出。

第四个坑是 setValue 会触发 valueChanged。在初始化阶段如果你不想让信号处理逻辑被意外触发，记得用 blockSignals(true) 临时屏蔽。这一点和 QSpinBox 的处理方式完全一样。

第五个坑是 setRange 的默认值是 (0, 99)。如果你不手动设范围，Slider 只能在 0 到 99 之间滑动。这个默认值在大部分场景下都太小了——养成创建 Slider 后立刻设 setRange 的习惯。

## 5. 练习项目

我们来做一个综合练习：创建一个"颜色调色板"窗口，覆盖 QSlider 的核心用法。窗口左侧是三组水平 Slider，分别控制颜色的 R / G / B 分量（范围 0-255），每个 Slider 旁边有一个 QLabel 显示当前数值（如 "R: 128"）。窗口右侧是一个大号的 QLabel，背景色实时跟随三个 Slider 的值变化。窗口底部还有两个 Slider：一个是"透明度"Slider（范围 0-100，对应 0.0-1.0），用来调整颜色预览区域的透明度；另一个是"笔刷大小"Slider（范围 1-50，默认 10），旁边显示当前笔刷大小的像素值。所有的 Slider 都使用自定义 QSS 样式——R 通道用红色风格，G 通道用绿色风格，B 通道用蓝色风格，透明度和笔刷用灰色风格。

几个提示：RGB Slider 的值变化时需要构建一个 QColor 并调用 QPalette 或者 setStyleSheet 来更新 QLabel 的背景色；透明度 Slider 需要做整数到浮点的映射；笔刷大小 Slider 可以添加刻度标记帮助用户判断位置。

## 6. 官方文档参考链接

[Qt 文档 -- QSlider](https://doc.qt.io/qt-6/qslider.html) -- 滑动条控件

[Qt 文档 -- QAbstractSlider](https://doc.qt.io/qt-6/qabstractslider.html) -- 抽象滑动条基类（信号和步进行为定义在此）

[Qt 文档 -- QSlider::TickPosition](https://doc.qt.io/qt-6/qslider.html#TickPosition-enum) -- 刻度位置枚举

---

到这里，QSlider 的四个核心维度就全部讲完了。水平/垂直方向的选择取决于你的布局空间和交互设计。setRange / setValue / setSingleStep / setPageStep 四个方法覆盖了从"范围约束"到"分层步进"的完整数值行为。valueChanged / sliderMoved / sliderReleased 三个信号各有分工——valueChanged 是"值变了就通知"的全能选手，sliderMoved 是"用户正在拖"的实时追踪器，sliderReleased 是"用户拖完了"的终点通知。QSS 自定义外观则让你可以在不写绘制代码的前提下把默认的系统风格滑块改造成任何你想要的样子。QSlider 看起来只是一个简单的拖拽控件，但用好它涉及数值映射、信号选择、样式定制等多层细节——把这些搞清楚后，音量调节、进度跳转、参数微调这类需求都不在话下了。
