# 现代Qt开发教程（新手篇）1.3——字符串与编码

## 1. 前言：C++ 字符串的坑，我踩过太多了

说实话，字符串处理这事儿，在 C++ 里就是个雷区。std::string？只知道是一堆字节，但啥编码没人知道。std::wstring？Windows 是 UTF-16，Linux 是 UTF-32，跨平台直接裂开。更别提各种 GBK、UTF-8 互相转换，转来转去最后全是问号。

我第一次做 Qt 项目的时候，从网络收到的 JSON 里的中文全是乱码，折腾了一整夜，最后发现是没搞清楚 QString 是 UTF-16 存储的，而我拿 UTF-8 的 QByteArray 直接去构造。从那以后我就发誓，一定要把 Qt 的字符串体系搞明白。

Qt 其实提供了一个非常完整的字符串处理体系，搞懂了它，编码问题基本就不存在了。这一篇我们就不讲虚的，直接把 QString、QByteArray、QStringView 这三兄弟的关系捋清楚，再加上编码转换的那些坑，包教包会。

## 环境说明

本文代码基于 Qt 6.5+ 版本。Qt 6 中字符串类的行为与 Qt 5 基本一致，但建议使用新式 API（如 `QStringView` 替代 `QStringRef`）。示例代码在 C++17 或更高版本下编译通过。

## 2. QString 是什么，和 std::string 有啥本质区别

先说结论：QString 内部存储的是 UTF-16 编码的 Unicode 字符，而 std::string 只是原始字节序列，不知道自己是什么编码。

这意味着什么？std::string 存一个中文字符 "你"，UTF-8 编码下需要 3 个字节，但 std::string 只知道自己是 3 个字节，不知道这是一个字符。而 QString 知道自己是 1 个 QChar（准确说是 1 个 Unicode 码位），可以用 size() 得到 1，用 at(0) 拿到这个字符。

这个区别看着简单，但实际影响非常大：

```cpp
// std::string 的困惑
std::string s = "你好";  // UTF-8 编码，实际 6 字节
size_t len = s.length(); // 得到 6，而不是字符数 2

// QString 的清晰
QString qs = QString::fromUtf8("你好");
size_t len = qs.length(); // 得到 2，真正的字符数
```

QString 内部用 UTF-16 存储，这意味着每个字符至少占 2 字节。对于常用汉字，一个 QChar 就够了。但对于超出基本多文种平面的字符（比如 emoji），可能需要两个 QChar（代理对），不过这是进阶话题，入门阶段我们先记住：QString 能正确处理 Unicode 字符。

你可能会问：为什么 std::string 的 length() 不能准确返回中文字符数，而 QString 可以？原因很简单——std::string 是字节序列，不知道编码，length() 返回的是字节数。UTF-8 中一个汉字占 3 字节，所以 length() 返回 6 而不是 2。QString 内部用 UTF-16，每个 QChar 代表一个 Unicode 码位，length() 返回的是真正的字符数。

## 3. QByteArray 是什么，什么时候用

QByteArray 也很简单：它就是一个能够自动管理内存的字节数组。你可以把它看作带内存管理的 char*。

那什么时候用 QByteArray，什么时候用 QString？记住一句话：如果是纯文本、需要显示给人看的，用 QString；如果是二进制数据、网络传输、文件原始内容，用 QByteArray。

```cpp
// 文本内容 → QString
QString username = "张三";

// 网络接收的原始数据 → QByteArray
QByteArray rawData = socket.readAll();

// 文件二进制内容 → QByteArray
QByteArray fileData = file.readAll();
```

QByteArray 还有一个常见用途：和 C 库交互。很多 C API 接受 const char*，QByteArray 可以直接提供：

```cpp
QByteArray data = "Hello";
C_Function(data.constData()); // 自动转 const char*
```

## 4. QStringView —— 零拷贝的字符串视图

Qt 5.12 引入了 QStringView，Qt 6 里大量使用。它的核心思想是：零拷贝的字符串只读视图。

什么叫零拷贝？当你有一个 QString，想调用一个函数来处理它，但又不想复制整个字符串，用 QStringView 就行：

```cpp
void processString(QStringView sv);  // 接受任何字符串类型的视图

QString s = "Hello";
processString(s);           // 从 QString 构造视图，零拷贝
processString(u"Hello");    // 从字符数组构造视图，零拷贝
```

QStringView 本身不拥有数据，只是指向别的数据。这意味着它非常轻量，复制一个 QStringView 只是复制两个指针（数据指针和长度），不复制实际内容。

Qt 6 里很多 API 都改成了接受 QStringView，比如 QString 的很多成员函数：

```cpp
// Qt 5 风格（会有拷贝）
bool contains(const QString &str) const;

// Qt 6 风格（零拷贝）
bool contains(QStringView str) const;
```

不过入门阶段你不需要过度优化，直接用 QString 传引用也没问题。QStringView 主要在你需要处理大量字符串操作、又想避免频繁分配内存时有用。

这里有个非常容易踩的坑——QStringView 不拥有数据，所以你必须确保被观察对象活得比视图久。比如你写了一个函数返回 QStringView，但返回的却是指向临时 QString 的视图，那临时对象一销毁，视图就悬空了。下面这种写法就会导致悬空引用，可能崩溃或读到垃圾数据：

```cpp
// 错误做法：返回一个指向临时对象的 QStringView
QStringView getView() {
    return QString("Hello");  // 临时对象被销毁，视图悬空
}
```

正确做法是直接返回 QString 值类型，或者确保被观察对象的生命周期足够长。一句话记住这个坑：QStringView 只是借别人的数据用一下，别让数据源先走了。

```cpp
// 正确做法：返回值类型，安全
QString getView() {
    return QString("Hello");  // 返回值，安全
}
```

## 5. 编码转换 —— GBK、UTF-8 互相转换的坑

编码转换是国内 Qt 开发者的必经之路。Windows 默认编码可能是 GBK，Linux/Mac 通常是 UTF-8，网络传输通常用 UTF-8，这之间来回转换确实容易出错。

### 5.1 QString 和 UTF-8 互转

Qt 6 里最简单的方式：

```cpp
// UTF-8 → QString
QByteArray utf8Data = "...";  // UTF-8 编码的字节
QString str = QString::fromUtf8(utf8Data);

// QString → UTF-8
QByteArray utf8Data = str.toUtf8();
```

这是最常见的用法，也是推荐用法。现代系统基本都统一到 UTF-8 了，能不用别的编码就不用。

### 5.2 GBK 转换（Windows 旧代码兼容）

Windows 下有些老旧接口或文件会用 GBK 编码：

```cpp
// GBK → QString（Qt 6 推荐方式）
QByteArray gbkData = "...";  // GBK 编码的字节
QStringDecoder decoder("GBK");  // Qt 6 新方式
QString str = decoder(gbkData);

// QString → GBK
QStringEncoder encoder("GBK");
QByteArray gbkData = encoder(str);
```

注意：如果系统没有对应编码的转换器，decoder/encoder 会处于无效状态，转换会得到空字符串。可以检查 isValid()：

```cpp
QStringDecoder decoder("GBK");
if (!decoder.isValid()) {
    qWarning() << "GBK encoding not supported";
}
```

来一道调试挑战。以下代码有什么问题？在 Windows 中文环境下会输出什么？

```cpp
QFile file("data.txt");  // 假设文件是 GBK 编码
if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    QString text = QString::fromUtf8(data);  // 直接按 UTF-8 解析
    qDebug() << text;
}
```

答案是文件是 GBK 编码，但代码用 fromUtf8 解析，中文字符会变成问号或乱码。正确做法是检测文件编码，或使用 QStringDecoder("GBK") 解析。这个坑我在做日志分析工具的时候踩过，收到的数据死活不对，最后发现是拿错了解码器。

### 5.3 QStringLiteral —— 编译期字符串字面量

如果你在代码里写固定的字符串字面量，用 QStringLiteral 宏：

```cpp
// 运行时构造，有开销
QString str = QString("Hello");

// 编译期生成，零运行时开销
QString str = QStringLiteral("Hello");
```

QStringLiteral 会在编译期就把字符串转成 UTF-16 格式，运行时直接使用，避免运行时转换。不过这只对字面量有用，不能用于变量：

```cpp
QString s = "World";
QString str = QStringLiteral(s);  // 错误！QStringLiteral 只接受字面量
```

## 6. QString 常用操作速查

字符串操作是日常开发的高频操作，这里列几个最常用的：

```cpp
QString str = "Hello,Qt,World";

// 分割
QStringList parts = str.split(',');  // ["Hello", "Qt", "World"]

// 查找
bool hasQt = str.contains("Qt");     // true
int pos = str.indexOf("Qt");         // 6

// 替换
str.replace("Qt", "C++");            // "Hello,C++,World"

// 格式化（类似 sprintf）
QString msg = QString("用户 %1 登录成功，代码 %2").arg("张三").arg(200);
// "用户 张三 登录成功，代码 200"

// 数字转换
QString numStr = QString::number(3.14, 'f', 2);  // "3.14"
double value = numStr.toDouble();                 // 3.14

// 去空格
QString trimmed = "  hello  ".trimmed();  // "hello"
```

这里有一个精度陷阱值得说一声。QString::number 默认只有 6 位精度，如果你直接 `QString::number(3.1415926535)`，拿到的只有 3.14159。浮点数精度就这么丢了，计算结果可能不正确。解决办法是显式指定精度格式，写成 `QString::number(3.1415926535, 'f', 10)`。一句话记住：数字转字符串记得指定精度格式，不然默认 6 位不够用。

## 7. 字符串拼接的性能问题

Qt 5 时代，大家都被教导不要用 + 号拼接字符串，因为会有多次内存分配。Qt 6 里虽然优化了很多，但了解一下还是有好处。

```cpp
// Qt 5/6 都可以，但可能有多次分配
QString s = "Hello";
s += " ";
s += "World";
s += "!";

// 推荐方式：一次构建
QString s = QString("Hello%1World%2").arg(" ").arg("!");

// 或者用 QStringList + join
QStringList parts;
parts << "Hello" << "World" << "!";
QString s = parts.join(" ");
```

不过说实话，对于日常使用，+ 号拼接的性能影响通常可以忽略，除非你在循环里拼接大量字符串。先写正确，再考虑优化。

## 8. 练习项目

练习项目：日志文件编码转换工具。写一个命令行工具，能够读取文本文件并转换编码。例如将 GBK 编码的日志文件转换为 UTF-8 编码。

完成标准是：程序接受两个命令行参数（输入文件路径和输出文件路径），能够检测或指定输入文件编码（至少支持 GBK 和 UTF-8），转换后输出为 UTF-8 编码，转换失败时给出清晰的错误提示，能够处理大文件（逐行读取，不一次性加载全部内容）。

实现上有几个方向可以用。用 QFile + QTextStream 逐行读取是个好起点，QTextStream 可以设置编码（比如 `stream.setEncoding(QStringConverter::Utf8)`），用 QStringDecoder/QStringEncoder 处理编码转换。错误处理方面，记得检查文件是否能打开、编码转换是否有效。

## 9. 官方文档参考

[Qt 6 QString 类文档](https://doc.qt.io/qt-6/qstring.html) · QString 完整 API 参考

[Qt 6 QByteArray 类文档](https://doc.qt.io/qt-6/qbytearray.html) · 字节数组处理，二进制数据必备

[Qt 6 QStringView 类文档](https://doc.qt.io/qt-6/qstringview.html) · 零拷贝字符串视图

[Qt 6 字符串处理概述](https://doc.qt.io/qt-6/string-processing.html) · 字符串体系全景图

[Qt 6 QStringEncoder/QStringDecoder 文档](https://doc.qt.io/qt-6/qstringencoder.html) · 现代 Qt 6 编码转换方式

*（链接已验证，2026-03-17 可访问）*

---

到这里你就掌握了 Qt 字符串处理的基础。编码转换这事儿说难不难，但一定要搞清楚每个环节的编码是什么，不然就是满天飞乱码。下一篇我们来讲 Qt 的容器类，和字符串配合使用非常频繁。
