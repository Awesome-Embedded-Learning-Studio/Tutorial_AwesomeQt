# 讨论QString的各种to...

## 🌐 聊一聊编码：什么是字符编码（Character Encoding）？

字符编码是 **将字符转为二进制数字** 的一种规则。

例如，字符 `'A'` 在 UTF-8 中对应二进制是 `01000001`（ASCII 的 65），而汉字 `'你'` 在 UTF-8 中是 `0xE4BD A0`，3 个字节。

不同编码会用不同的方式对字符进行编码：

| 字符 | UTF-8     | GBK    | UTF-16 |
| ---- | --------- | ------ | ------ |
| A    | 0x41      | 0x41   | 0x0041 |
| 你   | 0xE4BD A0 | 0xC4E3 | 0x4F60 |



------

## 🔤 Qt 的 `QString` 默认使用 **UTF-16**

`QString` 是 Qt 的字符串类型，它内部使用 **UTF-16** 存储所有字符。也就是说，你写 `QString str = "你好";` 时，这两个字其实是以 UTF-16 格式存在内存中的。

------

## 🧭 常见编码的简介与适用场景

| 编码                     | 特点                                                         | 适用                                  |
| ------------------------ | ------------------------------------------------------------ | ------------------------------------- |
| **UTF-8**                | 可变长度（1-4字节），兼容 ASCII，国际化标准                  | 网络传输、跨平台文件、现代软件标准    |
| **UTF-16**               | 每字符一般用2或4字节，适合存储所有语言                       | Windows内部API、`QString`内部、Java等 |
| **GBK/GB2312**           | 中文专用编码，单/双字节混合                                  | 中文Windows系统下老程序、简体中文文档 |
| **ISO-8859-1 (Latin1)**  | 单字节编码，适用于西欧字符                                   | 西欧国家文档、旧系统                  |
| **本地编码 (Local8Bit)** | 系统当前的编码，比如简体中文是 GBK、繁体中文是 Big5、英文是 UTF-8 | 与系统 API、控制台交互                |

​	在我们熟悉了这个知识点之后，我们就可以开始继续看一看Qt的各种to了：

# Qt的各种转化

## 一、编码相关的转换函数（返回 `QByteArray`）

这些函数将 `QString` 转换为不同编码格式的 `QByteArray`，适合与底层 C/C++ 接口或文件、网络等系统交互：

| 函数                            | 说明                                                         | 常见用途                                          |
| ------------------------------- | ------------------------------------------------------------ | ------------------------------------------------- |
| `toUtf8()`                      | 转换为 UTF-8 编码的 `QByteArray`                             | 网络传输、保存为 UTF-8 文件等                     |
| `toLatin1()`                    | 转换为 ISO-8859-1（Latin1）编码                              | 西欧语言环境使用                                  |
| `toLocal8Bit()`                 | 转换为本地编码（如 GBK、BIG5 等）                            | 与本地平台系统交互（如 Windows 控制台、文件路径） |
| `toStdString()`                 | 转换为 `std::string`，本质使用 `toUtf8()` 或 `toLocal8Bit()` | 和标准库接口交互                                  |
| `toWCharArray(wchar_t* buffer)` | 将字符串内容复制到宽字符数组中（`wchar_t[]`）                | Windows API 等宽字符接口                          |
| `utf16()`                       | 返回 UTF-16 编码的指针（`const ushort*`）                    | 与 Windows 或 Unicode 底层 API 交互               |

### 1. `toUtf8()` — 转成 UTF-8 格式的 QByteArray

```cpp
QString str = "你好";
QByteArray bytes = str.toUtf8();
```

- 输出：`bytes` 是 UTF-8 编码的二进制字节流。
- **用途：** 网络传输、保存为 UTF-8 文件、输出给支持 UTF-8 的系统。

------

### 2. `toLocal8Bit()` — 转成系统当前的编码（本地编码）

```cpp
QString str = "你好";
QByteArray localBytes = str.toLocal8Bit();
```

- 在 **简体中文 Windows** 上，结果是 GBK 编码。
- **用途：** 控制台输出、与本地 API、旧 C 接口交互。

✅ 特别提示：

```cpp
printf("%s", str.toLocal8Bit().data()); // 控制台输出中文（Windows）
```

------

### 3. `toLatin1()` — 转成 ISO-8859-1（Latin1）格式

```cpp
QString str = "Hello";
QByteArray latin = str.toLatin1();
```

- **仅支持西欧字符**（如英文字母、数字、法文等），中文会变成乱码。
- **用途：** 西欧语言环境系统或嵌入式旧平台。

------

### 4. `toStdString()` — 转成 `std::string`

```cpp
std::string s = str.toStdString(); // 实质等价于 str.toUtf8().toStdString()
```

- 适合和标准 C++ 库或旧接口混用。
- 不建议在处理中文时用 `std::string`，因它对编码无感。

## 🧷 举个场景例子

假设你要把 `QString` 写到文件或发到网络上，不同场景应选择不同编码：

| 场景                              | 应使用                            |
| --------------------------------- | --------------------------------- |
| 与本地 Windows 控制台打印中文     | `toLocal8Bit()`                   |
| 写入 UTF-8 编码的文本文件         | `toUtf8()`                        |
| 与 C++ 标准库（如 `fstream`）配合 | `toUtf8().toStdString()`          |
| 发送字符串到嵌入式设备使用 GBK    | `toLocal8Bit()`（需设置系统编码） |

> ✅ 注意：
>
> - `QString` 内部使用 **UTF-16** 存储，因此转换为其他编码都需要编码过程。
> - `toLocal8Bit()` 在不同系统下可能编码不同，比如 Windows 上是 GBK（简中系统）、Linux 下是 UTF-8 或其他本地编码。

------

## 二、转换为数值类型

用于将 `QString` 转换为基本数据类型，如整数、浮点数等：

| 函数                     | 返回类型  | 示例                                   |
| ------------------------ | --------- | -------------------------------------- |
| `toInt(&ok, base)`       | `int`     | 十进制或其他进制整数                   |
| `toUInt(&ok, base)`      | `uint`    | 无符号整数                             |
| `toLongLong(&ok, base)`  | `qint64`  | 大整数                                 |
| `toULongLong(&ok, base)` | `quint64` | 无符号大整数                           |
| `toFloat(&ok)`           | `float`   | 浮点数                                 |
| `toDouble(&ok)`          | `double`  | 双精度浮点数                           |
| `toBool()`               | `bool`    | 返回是否为 "true"、"1"（不区分大小写） |



> 参数 `ok` 是可选的，用于输出是否成功转换。
>
> ```
> bool ok;
> int val = str.toInt(&ok);
> if (ok) { ... }
> ```

------

## 三、转换为标准 C/C++ 类型

| 函数               | 返回类型         | 说明                     |
| ------------------ | ---------------- | ------------------------ |
| `toStdString()`    | `std::string`    | 转换为 UTF-8 字符串      |
| `toStdU16String()` | `std::u16string` | 对应 UTF-16              |
| `toStdU32String()` | `std::u32string` | 对应 UTF-32              |
| `toStdWString()`   | `std::wstring`   | 宽字符字符串（平台相关） |



------

## 四、与 C 风格字符串交互（使用临时的 `QByteArray`）

虽然没有 `toCharArray()`，但你可以这样使用：

```
const char* cstr = str.toLocal8Bit().constData();
```

- 注意这是临时变量，必须确保 `QByteArray` 生命周期足够长。

​	举个例子：

```
QString str = "你好123";

// 编码转换
QByteArray utf8 = str.toUtf8();       // UTF-8 编码
QByteArray local = str.toLocal8Bit(); // 本地编码（如GBK）

// 数值转换
bool ok;
int num = QString("42").toInt(&ok);   // 转成整数

// 与标准库兼容
std::string stdStr = str.toStdString();
```