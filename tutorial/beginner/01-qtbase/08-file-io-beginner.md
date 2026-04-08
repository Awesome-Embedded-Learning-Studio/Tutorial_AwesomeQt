# 现代Qt开发教程（新手篇）1.8——文件与IO

## 1. 前言：跨平台文件操作的痛

说实话，我第一次在 C++ 里写文件读写的时候，满脑子都是 "为什么不直接用 fopen"。后来真上了项目，在 Windows 和 Linux 之间来回切，才发现标准库的那些东西有多难用——路径分隔符不一样、编码不一样、连文件系统 API 都不一样。

Qt 把这些破事全包了。同样的代码，Windows 上跑完直接扔 Linux，连改都不用改。你不用担心路径是 `/` 还是 `\`，不用管文件名是 GBK 还是 UTF-8，Qt 都帮你处理好了。

这一篇我们要做的是：把 Qt 的文件 IO 体系摸透，从最简单的读写到一个完整的文件浏览器雏形，都能搞定。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要以下模块：

| 模块 | CMake 组件 | 用途 |
|------|-----------|------|
| QtCore | Qt6::Core | QFile、QDir、QFileInfo、QTextStream、QDataStream |

所有文件 IO 类都在 QtCore 里，不需要额外链接其他模块。

## 3. 核心概念讲解

### 3.1 QFile —— 文件读写的瑞士军刀

QFile 是 Qt 里最基本的文件操作类，继承自 QIODevice。它可以读写文本文件、二进制文件，还能直接通过路径操作。

先看一个最基本的写文件：

```cpp
// 创建文件对象
QFile file("output.txt");

// 打开文件：只写 | 文本模式
if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    // 写入数据
    file.write("Hello, Qt!\n");

    // 记得关闭
    file.close();
}
```

这里有几个细节需要说清楚。`open()` 的参数是标志位（flag）的组合，用 `|` 连接：

- `QIODevice::ReadOnly` —— 只读
- `QIODevice::WriteOnly` —— 只写
- `QIODevice::ReadWrite` —— 读写
- `QIODevice::Append` —— 追加模式
- `QIODevice::Truncate` —— 打开时清空文件
- `QIODevice::Text` —— 文本模式（自动处理换行符转换）

`Text` 标志在跨平台时特别重要。Windows 用 `\r\n` 换行，Linux 用 `\n`，开了 Text 模式 Qt 会自动转换。

> 📝 **随堂测验：口述回答**
> 用自己的话说说：`QIODevice::WriteOnly | QIODevice::Append` 和 `QIODevice::WriteOnly | QIODevice::Truncate` 有什么区别？
>
> *(请先自己想一下，再往下滑看答案)*
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> - `Append` 会在文件末尾追加内容，不删除原有内容
> - `Truncate` 会清空文件后再写入，原有内容全部丢失
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

读取文件也很简单：

```cpp
QFile file("input.txt");

if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    // 一次性读取全部内容
    QByteArray allData = file.readAll();

    // 或者逐行读取
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        qDebug() << line.trimmed();  // 去掉末尾的换行符
    }

    file.close();
}
```

`readAll()` 会把整个文件塞进内存，适合小文件。大文件建议用 `readLine()` 或者定长 `read()`。

> ⚠️ **坑 #1：忘记检查 open() 返回值**
> ❌ 错误做法：
> ```cpp
> QFile file("data.txt");
> file.open(QIODevice::ReadOnly);
> file.readAll();  // 如果打开失败，这里返回空
> ```
> ✅ 正确做法：
> ```cpp
> QFile file("data.txt");
> if (!file.open(QIODevice::ReadOnly)) {
>     qDebug() << "打开失败:" << file.errorString();
>     return;
> }
> ```
> 💥 后果：文件不存在或权限不足时，你拿到的空数据，后续代码可能一脸懵逼地报错
> 💡 一句话记住：永远检查 `open()` 的返回值，它是 `bool`，不是 void

### 3.2 QTextStream —— 文本流的优雅

QFile 直接操作 `QByteArray` 有点原始，读写文本文件推荐用 `QTextStream`。它支持流式操作，还能自动处理编码。

```cpp
QFile file("config.txt");

if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);

    // 设置编码（默认是系统编码）
    out.setEncoding(QStringConverter::Utf8);

    // 流式写入
    out << "Server = 192.168.1.1\n";
    out << "Port = " << 8080 << "\n";
    out << "Debug = " << true << "\n";

    // 可以直接写 QString
    QString message = "Hello, 世界";
    out << message << "\n";

    file.close();
}
```

读取同样方便：

```cpp
QFile file("config.txt");

if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    // 逐行读取
    while (!in.atEnd()) {
        QString line = in.readLine();
        qDebug() << line;
    }

    file.close();
}
```

`QTextStream` 的强大之处在于它像 C++ 的 `iostream` 一样支持 `<<` 和 `>>` 操作符，并且能正确处理 Unicode。

这里有个细节要注意：Qt 6 里编码设置用 `setEncoding()`，而 Qt 5 用的是 `setCodec()`。如果你在旧教程里看到 `setCodec()`，那是 Qt 5 的写法。

> ⚠️ **坑 #2：文本模式下的换行符混乱**
> ❌ 错误做法：
> ```cpp
> // Windows 下写入
> out << "Line1\nLine2\n";
> // 然后 Linux 下读取，发现多了 \r
> ```
> ✅ 正确做法：
> ```cpp
> // 写入时用 Text 标志
> file.open(QIODevice::WriteOnly | QIODevice::Text);
> // 或者手动用 Qt 的通用换行符
> out << "Line1" << Qt::endl << "Line2" << Qt::endl;
> ```
> 💥 后果：跨平台时换行符不一致，解析配置文件会出错
> 💡 一句话记住：用 `QIODevice::Text` 标志让 Qt 自动处理换行符，别自己硬编码 `\n`

### 3.3 QDir —— 目录操作的指挥官

QDir 用于目录操作：创建、删除、遍历、过滤。

```cpp
// 创建目录
QDir dir;
if (!dir.exists("data")) {
    dir.mkpath("data/subdir");  // mkpath 会创建所有必要的父目录
}

// 遍历目录
QDir dataDir("data");
dataDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);  // 只要文件，不要 . 和 ..
dataDir.setSorting(QDir::Name | QDir::Reversed);       // 按名称倒序

QFileInfoList fileList = dataDir.entryInfoList();
for (const QFileInfo &fileInfo : fileList) {
    qDebug() << fileInfo.fileName() << fileInfo.size();
}

// 用通配符过滤
dataDir.setNameFilters(QStringList() << "*.txt" << "*.md");
QStringList textFiles = dataDir.entryList();
```

`mkpath()` 和 `mkdir()` 的区别是：`mkpath()` 会递归创建父目录，`mkdir()` 只创建最后一级。

> 🔲 **随堂测验：代码填空**
> 补全以下代码，删除目录及其所有内容：
>
> ```cpp
> QDir dir("temp");
> if (dir.exists()) {
>     // 递归删除目录及其内容
>     bool success = dir.______("temp");
>     if (!success) {
>         qDebug() << "删除失败";
>     }
> }
> ```
>
> *(提示：三个字母的方法)*
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> ```cpp
> bool success = dir.removeRecursively();
> ```
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

### 3.4 QFileInfo —— 文件元数据的百科全书

QFileInfo 不是一个用来读写的类，而是用来获取文件信息的：大小、修改时间、权限、路径等。

```cpp
QFileInfo fileInfo("data/config.txt");

// 基本信息
qDebug() << "文件名:" << fileInfo.fileName();        // config.txt
qDebug() << "完整路径:" << fileInfo.absoluteFilePath();
qDebug() << "大小:" << fileInfo.size() << "字节";
qDebug() << "是否可读:" << fileInfo.isReadable();
qDebug() << "是否可写:" << fileInfo.isWritable();
qDebug() << "创建时间:" << fileInfo.birthTime().toString();
qDebug() << "修改时间:" << fileInfo.lastModified().toString();

// 路径操作
qDebug() << "目录名:" << fileInfo.path();            // data
qDebug() << "后缀名:" << fileInfo.suffix();          // txt
qDebug() << "完整后缀:" << fileInfo.completeSuffix(); // 对于 tar.gz 会是 tar.gz
```

QFileInfo 会缓存文件信息，如果你需要实时更新的信息，要调用 `refresh()`。

```cpp
QFileInfo fileInfo("data.txt");
qint64 size1 = fileInfo.size();

// ... 文件可能被外部修改 ...

fileInfo.refresh();  // 刷新缓存
qint64 size2 = fileInfo.size();
```

### 3.5 QDataStream —— 二进制数据的序列化利器

QTextStream 适合人类阅读的文本，QDataStream 适合机器读写二进制数据。它最大的优势是能直接序列化 Qt 的基本类型和容器。

```cpp
// 写入二进制数据
QFile file("data.bin");
if (file.open(QIODevice::WriteOnly)) {
    QDataStream out(&file);

    // 写入各种类型
    out << 42;                    // int
    out << 3.14;                  // double
    out << QString("Hello");      // QString
    out << QByteArray("ABC");     // QByteArray

    // 也可以写容器
    QList<int> numbers = {1, 2, 3, 4, 5};
    out << numbers;

    file.close();
}

// 读取二进制数据
QFile file2("data.bin");
if (file2.open(QIODevice::ReadOnly)) {
    QDataStream in(&file2);

    int intValue;
    double doubleValue;
    QString stringValue;
    QByteArray byteArrayValue;
    QList<int> numbers;

    // 按写入顺序读取
    in >> intValue;
    in >> doubleValue;
    in >> stringValue;
    in >> byteArrayValue;
    in >> numbers;

    qDebug() << intValue << doubleValue << stringValue << numbers;

    file2.close();
}
```

QDataStream 会自动处理字节序（大端/小端），你不用担心跨平台的问题。

> ⚠️ **坑 #3：QDataStream 读写顺序必须严格一致**
> ❌ 错误做法：
> ```cpp
> // 写入
> out << a << b << c;
> // 读取时顺序错了
> in >> c >> b >> a;  // 数据全乱了
> ```
> ✅ 正确做法：
> ```cpp
> // 写入时记住顺序
> out << a << b << c;
> // 读取时必须一模一样
> in >> a >> b >> c;
> ```
> 💥 后果：读取到的数据完全是错的，程序可能崩溃或产生莫名其妙的结果
> 💡 一句话记住：QDataStream 是强类型、强顺序的，写什么读什么，顺序不能乱

### 3.6 跨平台路径处理

Qt 提供了 `QPath`（不，Qt 6 里已经统一用 `QString` 处理路径），推荐使用以下方式拼接路径：

```cpp
// Qt 6 推荐方式
QString path = QDir::homePath() + "/data/config.txt";

// 或者用 QDir::separator()
QString path2 = "data" + QDir::separator() + "config.txt";

// 最干净的方式 —— 静态函数
QString path3 = QDir::cleanPath("data/../data/config.txt");  // 会变成 data/config.txt
```

获取系统标准路径：

```cpp
qDebug() << "当前工作目录:" << QDir::currentPath();
qDebug() << "用户主目录:" << QDir::homePath();
qDebug() << "临时目录:" << QDir::tempPath();
qDebug() << "根目录:" << QDir::rootPath();
```

> 🐛 **随堂测验：调试挑战**
>
> 以下代码有什么问题？会输出什么？
>
> ```cpp
> QFile file("C:/data/config.txt");  // 假设在 Linux 上运行
> if (file.open(QIODevice::ReadOnly)) {
>     qDebug() << "打开成功";
> }
> ```
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> - 问题：硬编码了 Windows 路径风格，在 Linux 上会尝试打开名为 "C:/data/config.txt" 的文件（实际上是一个合法的文件名）
> - 后果：文件可能找不到，路径不符合当前系统的习惯
> - 改进：使用 `QDir::homePath()` 或相对路径，避免硬编码绝对路径
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 4. 综合示例：简单的日志文件管理

把前面学的串起来，做一个简单的日志文件管理：

```cpp
class LogManager {
public:
    LogManager(const QString &logDir) {
        QDir dir;
        if (!dir.exists(logDir)) {
            dir.mkpath(logDir);
        }
        m_logDir = logDir;
    }

    void writeLog(const QString &message) {
        QString fileName = m_logDir + "/log_" +
                          QDate::currentDate().toString("yyyy-MM-dd") + ".txt";

        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << QDateTime::currentDateTime().toString("hh:mm:ss")
                << " - " << message << "\n";
            file.close();
        }
    }

    QStringList readTodayLogs() {
        QString fileName = m_logDir + "/log_" +
                          QDate::currentDate().toString("yyyy-MM-dd") + ".txt";

        QFile file(fileName);
        QStringList logs;

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                logs << in.readLine();
            }
            file.close();
        }

        return logs;
    }

private:
    QString m_logDir;
};
```

## 5. 练习项目

🎯 **练习项目：简易文件浏览器**

📋 **功能描述**：
做一个命令行文件浏览器，用户输入路径后，程序列出该目录下的所有文件和子目录，并显示每个文件的大小和修改时间。用户可以选择进入子目录或返回上级目录。

✅ **完成标准**：
- 程序启动时显示当前目录
- 支持命令：`cd 目录名` 进入目录，`cd ..` 返回上级，`ls` 列出当前目录，`exit` 退出
- 列表显示：文件名、大小（字节）、修改时间
- 目录和文件用不同的标识符区分（比如 `[DIR]` 和 `[FILE]`）
- 处理错误输入（目录不存在等）

💡 **提示**：
- 用 `QDir::entryInfoList()` 获取文件信息列表
- 用 `QFileInfo::isDir()` 区分目录和文件
- 用 `QDir::setCurrent()` 改变当前目录
- 主循环用 `while (true)` 读取用户输入，遇到 `exit` 跳出

## 6. 官方文档参考

📎 [Qt 文档 · QFile](https://doc.qt.io/qt-6/qfile.html) · 文件读写的核心类
📎 [Qt 文档 · QTextStream](https://doc.qt.io/qt-6/qtextstream.html) · 文本流操作
📎 [Qt 文档 · QDataStream](https://doc.qt.io/qt-6/qdatastream.html) · 二进制序列化
📎 [Qt 文档 · QDir](https://doc.qt.io/qt-6/qdir.html) · 目录操作与遍历
📎 [Qt 文档 · QFileInfo](https://doc.qt.io/qt-6/qfileinfo.html) · 文件元信息查询

*（链接已验证，2026-03-17 可访问）*

---

**到这里就大功告成了！** Qt 的文件 IO 体系其实不复杂，关键是用对工具：文本用 QTextStream，二进制用 QDataStream，目录操作用 QDir，信息查询用 QFileInfo。下一篇我们进入多线程的世界，看看 Qt 是如何优雅地处理并发编程的。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
