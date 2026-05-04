# 现代Qt开发教程（新手篇）3.36——QLCDNumber：液晶数字显示

## 1. 前言 / 那个复古的数字显示器

如果你用过老式的电子钟、计算器或者汽车仪表盘，一定对那种液晶风格的数字显示不陌生——七段数码管，每个数字由七条线段拼出来，有点工业仪表的味道。Qt 里的 QLCDNumber 就是这么一个控件：它把数字渲染成液晶数码管的样式，用来显示计数器、计时器、传感器读数这类"仪表盘数值"再合适不过了。

说实话，QLCDNumber 在 Qt 控件家族里算是个冷门角色，做常规业务界面基本用不到它。但如果你在做工业监控面板、仪器模拟器、游戏计分板或者任何需要"数字仪表"感觉的界面，QLCDNumber 就派上用场了——它自带七段数码管的渲染效果，省去了你自己用 QPainter 画线段的麻烦。而且它的 API 设计得比较精简，display() 一个方法就能搞定整数、浮点数和字符串的显示，配合 setDigitCount 控制位数、setMode 切换进制，一个小小的控件就能满足大部分仪表盘数字显示的需求。

今天我们要把 QLCDNumber 拆解成四个部分来讲：display() 系列方法的整数/浮点/字符串三种调用方式，setDigitCount 位数控制与 setSmallDecimalPoint 小数点显示策略，setMode 在十进制/十六进制/八进制/二进制之间切换的用法，以及一个仪表盘数字显示的典型实战场景——用 QLCDNumber + QTimer 做一个运行时间计数器。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QLCDNumber 属于 QtWidgets 模块，链接 Qt6::Widgets 即可，不需要额外的模块依赖。示例代码中还用到了 QLabel、QPushButton、QSlider、QComboBox、QTimer、QVBoxLayout、QHBoxLayout、QGroupBox 和 QLCDNumber 来搭建演示界面。

## 3. 核心概念讲解

### 3.1 display() 的三种调用方式

QLCDNumber 提供了三个重载的 display() 方法，分别接受 double、int 和 const QString& 类型的参数。这三个方法的行为在大部分情况下是一致的——把传入的值转换成字符串然后渲染到液晶数码管上——但在边界情况下有细微差异，值得逐个说清楚。

display(int) 是最直接的调用方式，传入一个整数值，QLCDNumber 直接把它渲染出来。display(double) 稍微复杂一点：浮点数会被转换成字符串，然后按照当前设置的 digitCount（位数）来决定显示多少位。如果浮点数的小数部分位数太多，超出 digitCount 能容纳的范围，QLCDNumber 会自动截断或者四舍五入。display(const QString&) 最灵活，它直接把字符串内容当作显示文本——你可以传入 "3.14159"、"HELLO"（QLCDNumber 能渲染少量字母）、"--"（负号）之类的特殊内容。

```cpp
auto *lcd = new QLCDNumber;
lcd->setDigitCount(6);

lcd->display(42);         // 显示 "42"
lcd->display(3.14);       // 显示 "3.14"
lcd->display(QString("HELLO"));  // 显示字母 HELLO
```

这里面有一个容易忽略的点：display(int) 和 display(double) 在显示负数时的行为不同。display(-42) 会把负号算在显示位数的占位里，如果 digitCount 不够大，负数会被截断或者触发 overflow 信号。display(double) 同理，负号和小数点都会占用显示空间。而 display(QString) 则是原样渲染字符串中的每个字符，QLCDNumber 内部有一个字符映射表，能渲染数字 0-9、负号、小数点、以及少量字母（A, B, C, D, E, F, H, L, O, P, S, U 和一些小写字母）。传入无法识别的字符时，该位置会显示为空。

另外一个细节是 overflow 信号。当 display() 传入的值转换成字符串后长度超过了 setDigitCount 设置的位数，QLCDNumber 不会截断显示——它会触发 overflow() 信号，并且在视觉上显示 "——"（全横线）来表示溢出。这一点跟很多初学者的预期不一样：你以为它会自动多显示几位，实际上它直接告诉你"放不下了"。

```cpp
auto *lcd = new QLCDNumber;
lcd->setDigitCount(3);
lcd->display(999);   // 正常显示 "999"
lcd->display(1000);  // 溢出！显示 "——"，触发 overflow 信号

connect(lcd, &QLCDNumber::overflow, this, []() {
    qDebug() << "LCD overflow!";
});
```

所以实际使用时一定要根据你的数值范围预留足够的 digitCount。比如显示 0-9999 的计数器，至少要设 setDigitCount(4)；显示带两位小数的浮点数，比如 -99.99 到 999.99，需要 setDigitCount(6)——6 个位置分别给：3 位整数 + 小数点 + 2 位小数，如果可能有负号还得再加一位变成 7 位。

### 3.2 setDigitCount 位数与 setSmallDecimalPoint 小数点

setDigitCount(int) 控制 QLCDNumber 的显示位数，也就是液晶数码管有多少个"数字槽位"。默认值是 5，也就是说默认情况下能显示 5 个字符。这个值包含数字、小数点和负号在内——也就是说 setDigitCount(4) 并不代表能显示 4 位整数，而是能显示 4 个"字符位"，如果你要显示一个带小数点的数字比如 12.3，小数点占一位，实际整数部分只有 2 位。

```cpp
auto *lcd = new QLCDNumber;

lcd->setDigitCount(3);
lcd->display(1.2);    // 显示 "1.2"（3 个位置：1 + . + 2）
lcd->display(12.3);   // 溢出，需要 4 个位置但只有 3 个

lcd->setDigitCount(6);
lcd->display(123.45); // 正常显示 "123.45"
```

setSmallDecimalPoint(bool) 是一个和 digitCount 配合使用的属性。默认情况下，小数点和负号都会独立占一个字符位。但如果 setSmallDecimalPoint(true)，小数点会被渲染在相邻两个数字之间的缝隙里，不再单独占用一个位置。这样一来，同样 4 个数字槽位，关闭 setSmallDecimalPoint 时最多显示 "12.3"（小数点占一位），开启 setSmallDecimalPoint 时最多显示 "123.4"（小数点挤在 3 和 4 之间）。

```cpp
auto *lcd = new QLCDNumber;
lcd->setDigitCount(4);

// setSmallDecimalPoint 默认 false
lcd->display(12.34);  // 溢出！小数点占位，需要 5 个位置

lcd->setSmallDecimalPoint(true);
lcd->display(12.34);  // 正常显示 "12.34"，小数点不占位
```

setSmallDecimalPoint 的视觉差异在不同平台上可能略有不同——有些渲染引擎把小数点画得很小，有些画得相对明显。如果你对显示精度有严格要求，建议实际编译运行一下看看效果。另外 setSmallDecimalPoint 只影响小数点的占位行为，不影响负号的占位——负号始终占用一个字符位，没有"小负号"的说法。

digitCount() 方法可以读取当前设置的位数。这个属性只影响显示，不影响内部存储——QLCDNumber 内部存储的是最后一次 display() 传入的值，你可以随时通过 value()（返回 double）或 intValue()（返回 int）来获取当前存储的值。

```cpp
auto *lcd = new QLCDNumber;
lcd->display(42);
lcd->value();    // 42.0
lcd->intValue(); // 42
```

这里有一个容易混淆的地方：value() 和 intValue() 返回的是最后一次 display() 调用时传入的数值（经过类型转换后的结果）。如果你用 display(QString("HELLO")) 显示了一个字符串，value() 和 intValue() 会返回 0——因为 "HELLO" 不是一个合法的数字，QLCDNumber 内部没有存储对应的数值。

### 3.3 setMode 十进制/十六进制/八进制/二进制

QLCDNumber 支持 four 种数字显示模式：十进制（Dec）、十六进制（Hex）、八进制（Oct）和二进制（Bin），通过 setMode(QLCDNumber::Mode) 来切换。默认模式是十进制。

```cpp
auto *lcd = new QLCDNumber;
lcd->setDigitCount(8);

lcd->setMode(QLCDNumber::Dec);
lcd->display(255);   // 显示 "255"

lcd->setMode(QLCDNumber::Hex);
lcd->display(255);   // 显示 "FF"

lcd->setMode(QLCDNumber::Oct);
lcd->display(255);   // 显示 "377"

lcd->setMode(QLCDNumber::Bin);
lcd->display(255);   // 显示 "11111111"
```

二进制模式需要特别大的 digitCount。255 的二进制是 8 位，1023 的二进制是 10 位，65535 的二进制是 16 位。如果你要显示一个 16 位二进制数，需要 setDigitCount(16) 甚至更多（考虑前导零或空格）。十六进制模式相对紧凑，0-65535 只需要最多 4 位。八进制介于两者之间。

setMode 只影响 display(int) 和 display(double) 的渲染方式，不影响 display(QString)。当你调用 display(QString) 时，字符串内容直接原样渲染，setMode 的设置被忽略。所以如果你想要在十六进制模式下显示 "0xFF"（带 0x 前缀），不能用 display(255)——它只会显示 "FF" 不带前缀。你需要自己把数值转换成带前缀的字符串然后 display(QString("0xFF"))。

```cpp
auto *lcd = new QLCDNumber;
lcd->setMode(QLCDNumber::Hex);
lcd->display(255);                   // 显示 "FF"（没有 0x 前缀）

// 手动构造带前缀的字符串
lcd->display(QString("0x%1").arg(255, 0, 16).toUpper());  // 显示 "0xFF"
```

mode() 方法返回当前的显示模式，枚举值是 QLCDNumber::Dec / Hex / Oct / Bin。实际工程中十六进制模式用得最多——在调试工具、寄存器查看器、内存地址显示这些场景下，十六进制是标配。二进制模式偶尔用在位操作的可视化上，但 digitCount 需求太大，不太实用。八进制模式基本没人用，但它确实存在。

还需要注意一点：setMode 对 display(double) 的影响。在非十进制模式下，display(double) 会先把浮点数截断为整数（取整），然后按指定的进制显示。也就是说 display(3.14) 在十六进制模式下会显示 "3"（因为 3.14 取整为 3，十六进制还是 3），而不是 "3.14" 的小数部分——因为十六进制浮点数表示不是 QLCDNumber 支持的功能。

### 3.4 仪表盘数字显示的典型场景

QLCDNumber 最典型的应用场景是各种仪表盘、监控面板和实时数据显示。我们来做一个具体的场景：用 QLCDNumber + QTimer 实现一个运行时间计数器，类似工业设备上的"累计运行时间"显示。

这种场景的核心逻辑很简单：一个 QTimer 以固定间隔（比如 1 秒）触发 timeout 信号，槽函数里递增一个计数器变量，然后调用 lcd->display() 更新显示。计数器的值可以格式化为 "时:分:秒" 的形式，用 display(QString) 直接传入格式化后的字符串。

```cpp
// 核心逻辑：每秒更新一次
m_elapsedSeconds = 0;
m_timer = new QTimer(this);
m_timer->setInterval(1000);
connect(m_timer, &QTimer::timeout, this, [this]() {
    ++m_elapsedSeconds;
    int hours = m_elapsedSeconds / 3600;
    int minutes = (m_elapsedSeconds % 3600) / 60;
    int seconds = m_elapsedSeconds % 60;
    m_lcd->display(QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
});
m_timer->start();
```

这里有一个容易踩的坑：QLCDNumber 渲染冒号时，冒号是作为七段数码管的一部分来绘制的。不同平台上冒号的渲染效果差异比较大——有些平台把冒号画成两个圆点（标准数码管风格），有些画成一条竖线或者干脆不显示。如果你发现冒号显示异常，可以考虑换成 display(QString("%1h%2m%3s")) 之类的格式，或者干脆不用 QLCDNumber 而改用 QLabel + 自定义字体。

QLCDNumber 的视觉风格可以通过 QSS 做一定程度的自定义，但定制空间有限。QSS 可以控制 QLCDNumber 的 background-color 和 color 属性——background-color 是整个控件的背景色（模拟 LCD 屏幕的底色），color 是数字线段的颜色。默认的配色是浅灰色背景加深灰色线段，很有老式计算器的感觉。

```css
QLCDNumber {
    background-color: #1a1a2e;
    color: #00ff88;
    border: 2px solid #333;
    border-radius: 4px;
}
```

上面这段 QSS 会把 QLCDNumber 变成深色背景配绿色数字——典型的仪表盘/工业显示器配色。需要注意的是，QLCDNumber 的 QSS 支持相当有限，你不能通过 QSS 改变线段的粗细、圆角、发光效果等。如果需要高度自定义的数码管效果，还是得自己用 QPainter 画，或者用自定义字体（比如搜索 "seven segment font"）。

另外一个常见的仪表盘用法是显示传感器的实时读数。比如一个温度传感器每 500ms 上报一次温度值，你可以直接 lcd->display(temperature)，温度值就会实时更新在数码管上。这种方式比 QLabel 更有"仪表"的感觉，特别是在深色主题的界面里。

## 4. 踩坑预防

第一个坑是 digitCount 不够导致 overflow。QLCDNumber 不会自动扩展显示位数，超出范围就直接显示溢出。解决方案是根据最大可能的数值仔细计算需要的位数，整数位数加上小数点（如果不开启 setSmallDecimalPoint）加上小数位数再加上可能的负号位，总和就是最小 digitCount。

第二个坑是 setSmallDecimalPoint 的视觉差异。开启后小数点确实不占位了，但在某些平台上小数点可能渲染得非常小，几乎看不见。如果你的界面要在多个平台上运行，建议测试一下各平台的实际效果。

第三个坑是二进制模式的位数需求。一个 quint16 的二进制表示需要 16 位，一个 quint32 需要 32 位——QLCDNumber 设 setDigitCount(32) 在界面上会占非常宽的空间，而且 32 个数码管排成一排可读性很差。如果确实需要显示二进制，考虑把它分成几段（比如每 8 位一段）分别显示。

第四个坑是 display(QString) 传入不支持的字符。QLCDNumber 只能渲染数字、少量字母、负号、小数点和冒号。其他字符（比如中文、特殊符号）会被显示为空白。如果你发现某个位置"空了"，检查一下传入的字符串里有没有不支持的字符。

第五个坑是 QSS 对 QLCDNumber 的支持有限。你只能改背景色和前景色，不能改线段粗细、不能加发光效果、不能改变数码管的基本形态。如果需要高度自定义的外观，QLCDNumber 可能不是最佳选择。

## 5. 练习项目

我们来做一个综合练习：创建一个"数字仪表盘演示"窗口。窗口顶部是一个大的 QLCDNumber（digitCount 设为 8），用来显示当前数值。中间区域分为三行：第一行是一个 QSlider（0-9999）和一个 QComboBox（选择显示模式：十进制/十六进制/八进制/二进制），滑块拖动时实时更新 LCD 显示；第二行是一个 QPushButton 启动/停止计时器，点击后 QTimer 以 1 秒间隔递增计数器，LCD 显示 "HH:MM:SS" 格式的运行时间；第三行是一个 QDoubleSpinBox（范围 -999.99 到 999.99），用来演示 display(double) 和 setSmallDecimalPoint 的效果。窗口底部有一个状态栏 QLabel，当 LCD 发生 overflow 时显示红色警告文字。

提示：切换显示模式时需要同时调整 digitCount，因为二进制模式比十进制需要更多位数；计时器显示需要用 display(QString) 因为冒号不在 display(int) 的渲染范围内。

## 6. 官方文档参考链接

[Qt 文档 -- QLCDNumber](https://doc.qt.io/qt-6/qlcdnumber.html) -- 液晶数字显示控件

[Qt 文档 -- QTimer](https://doc.qt.io/qt-6/qtimer.html) -- 定时器类（配合 QLCDNumber 实现计时器）

---

到这里，QLCDNumber 的核心用法就全部讲完了。display() 的三种重载覆盖了整数、浮点数和字符串的显示需求，其中 display(QString) 最灵活但要注意字符集限制。setDigitCount 控制显示位数，计算位数时一定要把小数点和负号考虑进去，或者开启 setSmallDecimalPoint 让小数点不占位。setMode 支持四种进制切换，十六进制在调试工具中最常用，二进制位数需求大要谨慎使用。仪表盘场景是 QLCDNumber 最适合的用武之地，配合 QTimer 和信号槽可以轻松实现计时器、计数器和实时数值显示。虽然它不能通过 QSS 做太多视觉定制，但在需要"工业仪表"风格的界面里，QLCDNumber 依然是 Qt 自带的最快上手方案。
