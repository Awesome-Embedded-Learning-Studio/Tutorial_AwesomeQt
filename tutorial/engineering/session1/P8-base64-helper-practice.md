# 实战练习 · Base64Helper — 一个真正有用的编码工具

## 前言：Base64 无处不在，不如自己写一个

如果你做过 Web 开发、接触过邮件附件、或者看过 JWT token，你一定见过 Base64 编码——那串 `SGVsbG8gV29ybGQ=` 看起来像乱码但其实是 "Hello World"。虽然网上有一大堆在线编解码工具，但你总不能每次都打开浏览器吧？而且处理中文、处理二进制文件、处理不同编码（UTF-8 vs Latin-1）这些场景，在线工具往往力不从心。

这篇练习我们要写一个桌面端的 Base64 编解码工具。它不只让你练 QByteArray 的编解码 API，更重要的是你会碰到文本编码这个在 C++ 里永远绕不开的话题——同一个字符串，用 UTF-8 编码和用 Latin-1 编码，Base64 结果完全不同。搞清楚这背后的原理，比写十个 CRUD 都有价值。

---

## 出发前的装备清单

- **QByteArray** — Qt 的字节数组，核心的编解码 API 都在这里：toBase64() 和 fromBase64()。
- **QTextEdit** — 多行文本编辑框，用来显示输入和输出。注意它获取文本的方法是 toPlainText() 不是 text()。
- **QSplitter** — 可拖拽分割的面板。上下分割，上面是输入，下面是输出，用户可以拖拽调整比例。
- **QFileDialog** — 文件选择对话框，用来选择要编码/解码的文件。
- **QClipboard** — 剪贴板，用来复制结果。
- **QComboBox** — 下拉选择框，用来切换 UTF-8 / Latin-1 编码。

---

## 我们的目标长什么样

```
┌──────────────────────────────────────────┐
│  [编码] [解码] [交换] [复制]  编码:[UTF-8▼]│
├──────────────────────────────────────────┤
│  输入：                                   │
│  ┌──────────────────────────────────────┐│
│  │ Hello World                          ││
│  │                                      ││
│  ├──────────────────────────────────────┤│
│  │ 输出：                               ││
│  │ SGVsbG8gV29ybGQ=                    ││
│  │                                      ││
│  └──────────────────────────────────────┘│
├──────────────────────────────────────────┤
│  [文件编码] [文件解码]                     │
└──────────────────────────────────────────┘
```

完成标准：文本编解码往返正确（编码再解码回来结果一致），中文文本用 UTF-8 编码正确，交换按钮能互换输入输出内容，复制按钮把输出写到剪贴板。

---

## 第一步 — 窗口布局：QSplitter 上下面板

### 思考题

为什么用 QSplitter 而不是 QVBoxLayout 里放两个 QTextEdit？QSplitter 的核心优势是什么？提示：用户可以拖拽分割条来调整上下区域的比例，而且 QSplitter 还可以记住这个比例。普通布局做不到这一点。

### 动手写

```cpp
#pragma once
#include <QComboBox>
#include <QMainWindow>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>

class Base64Helper : public QMainWindow
{
    Q_OBJECT

public:
    explicit Base64Helper(QWidget *parent = nullptr);

private slots:
    void onEncode();
    void onDecode();
    void onSwap();
    void onCopy();
    void onFileEncode();
    void onFileDecode();

private:
    void setupUi();

    QTextEdit *m_inputEdit;
    QTextEdit *m_outputEdit;
    QComboBox *m_encodingCombo;
};
```

setupUi 的布局思路是：顶部一个 QHBoxLayout 放按钮和编码选择，中间一个 QSplitter 放上下两个 QTextEdit（带标签），底部放文件操作按钮。

```cpp
void Base64Helper::setupUi()
{
    // TODO: 创建中央 QWidget
    // TODO: 创建主 QVBoxLayout

    // 按钮行：
    // QHBoxLayout *btnLayout = new QHBoxLayout;
    // 添加按钮：编码、解码、交换、复制
    // 添加 QComboBox（UTF-8 / Latin-1）
    // 添加 stretch 让按钮靠左

    // QSplitter 区域：
    // QSplitter *splitter = new QSplitter(Qt::Vertical);
    // m_inputEdit = new QTextEdit;   // 输入
    // m_outputEdit = new QTextEdit;  // 输出
    // m_outputEdit->setReadOnly(true);  // 输出区只读
    // splitter->addWidget(m_inputEdit);
    // splitter->addWidget(m_outputEdit);
    // splitter->setSizes(QList<int>{200, 200});  // 初始 1:1

    // 文件操作行：
    // QHBoxLayout *fileLayout = new QHBoxLayout;
    // 添加：文件编码、文件解码按钮

    // 组装主布局：
    // mainLayout->addLayout(btnLayout);
    // mainLayout->addWidget(splitter);
    // mainLayout->addLayout(fileLayout);
}
```

### 你可能会遇到的坑

QSplitter::setSizes() 设置的是像素值，不是比例。如果你想设置 1:1 的初始比例，需要传入两个相同的值（比如 {200, 200}）。如果你传 {1, 1}，两个面板会变成各 1 像素高，基本看不见。另外，setSizes 只在控件已经有足够大的尺寸时才生效——如果窗口还没 show 出来，这个设置可能被覆盖。解决办法是在 showEvent 里再设一次。

### 检查点

编译运行。你应该能看到一个窗口，上面有按钮行，中间是上下分割的两个文本区（可以拖拽分割条），底部有文件操作按钮。

---

## 第二步 — 编码逻辑：text → Base64

### 思考题

QString 到 QByteArray 有多种转换方式：toUtf8()、toLatin1()、toLocal8Bit()。你知道它们的区别吗？如果输入是 "你好"，用 toLatin1() 会发生什么？提示：Latin-1 只能表示 0x00-0xFF 的字符，中文字符不在 Latin-1 范围内。

### 动手写

```cpp
void Base64Helper::onEncode()
{
    // TODO: 获取输入文本
    //       QString input = m_inputEdit->toPlainText();
    //
    //       if (input.isEmpty()) return;

    // TODO: 根据编码选择转换成 QByteArray
    //       if (m_encodingCombo->currentText() == "UTF-8")
    //           QByteArray bytes = input.toUtf8();
    //       else
    //           QByteArray bytes = input.toLatin1();

    // TODO: Base64 编码
    //       QByteArray encoded = bytes.toBase64();

    // TODO: 显示结果
    //       m_outputEdit->setPlainText(QString::fromLatin1(encoded));
    //       注意：Base64 的输出只包含 ASCII 字符，
    //       用 fromLatin1 或 fromUtf8 都可以。
}
```

这里有一个必须理解的点：Base64 编码的输入是字节数组（QByteArray），不是字符串（QString）。所以编码流程是三步：QString → QByteArray（编码选择）→ Base64 → QString（显示）。解码是反过来的三步：QString → QByteArray（Base64 文本）→ 原始字节 → QString（编码选择）。

### 检查点

输入 "Hello World"，点编码，输出应该是 `SGVsbG8gV29ybGQ=`。你可以用任何在线 Base64 工具验证这个结果。

---

## 第三步 — 解码逻辑：Base64 → text

### 思考题

如果输入不是有效的 Base64 字符串（比如包含 @#$% 这种非法字符），QByteArray::fromBase64() 会返回什么？它会把非法字符忽略还是直接报错？去文档查一下 fromBase64 的行为描述。

### 动手写

```cpp
void Base64Helper::onDecode()
{
    // TODO: 获取输入的 Base64 文本
    //       QString input = m_inputEdit->toPlainText();
    //
    //       if (input.isEmpty()) return;

    // TODO: Base64 解码
    //       QByteArray decoded = QByteArray::fromBase64(input.toLatin1());
    //
    //       提示：fromBase64 对非法字符的行为——
    //       它会忽略非 Base64 字符，如果剩余的有效字符不够凑成完整分组，
    //       返回的字节数组末尾可能会丢失数据。

    // TODO: 根据编码选择把字节转回 QString
    //       QString result;
    //       if (m_encodingCombo->currentText() == "UTF-8")
    //           result = QString::fromUtf8(decoded);
    //       else
    //           result = QString::fromLatin1(decoded);

    // TODO: 显示结果
    //       m_outputEdit->setPlainText(result);
}
```

关于错误处理：如果输入的 Base64 无效，解码后的字节可能无法正确转成 UTF-8 字符串。QString::fromUtf8 遇到无效 UTF-8 序列时会把非法字节替换为 Unicode 替换字符（那个问号方块）。这是一个"优雅降级"而不是崩溃的行为。

### 检查点

先编码一段文字，然后点交换把编码结果移到输入区，再点解码——输出应该和最初输入的文本完全一致。这就是一个完整的"往返测试"。

---

## 第四步 — 交换、复制、编码选择

### 动手写

这三个功能比较简单：

```cpp
void Base64Helper::onSwap()
{
    // TODO: 交换输入和输出的内容
    //       QString temp = m_inputEdit->toPlainText();
    //       m_inputEdit->setPlainText(m_outputEdit->toPlainText());
    //       m_outputEdit->setPlainText(temp);
}

void Base64Helper::onCopy()
{
    // TODO: 复制输出到剪贴板
    //       QClipboard *clipboard = QApplication::clipboard();
    //       clipboard->setText(m_outputEdit->toPlainText());
}
```

编码选择的 QComboBox 不需要额外的 slot——它的值在 onEncode/onDecode 里每次读取就行。用户切换编码后不需要立即触发什么操作，下次点编码/解码按钮时自然会用新选择的编码。

### 检查点

点交换，输入和输出内容互换。点复制，到其他地方粘贴能粘出输出区的文本。切换编码为 Latin-1，输入中文后编码，再解码回来——你会发现结果不对（因为中文不适合 Latin-1 编码），这正是这个功能想让你体验的。

---

## 第五步 — 文件模式：二进制文件的 Base64

### 思考题

文件模式和文本模式的本质区别是什么？提示：文本模式下我们先把 QString 转成 QByteArray 再 Base64；文件模式下我们直接读文件的原始字节做 Base64，不需要经过 QString。这意味着文件模式可以处理任何二进制文件——图片、PDF、ZIP——不限于文本。

### 动手写

```cpp
void Base64Helper::onFileEncode()
{
    // TODO: 用 QFileDialog::getOpenFileName() 选择文件
    //
    // TODO: 用 QFile 读取文件内容
    //       QFile file(fileName);
    //       if (!file.open(QIODevice::ReadOnly)) { /* 错误处理 */ }
    //       QByteArray data = file.readAll();
    //       file.close();
    //
    //       ⚠️ 注意：readAll() 会把整个文件读进内存。
    //       对于大文件（比如几百 MB 的视频）这会爆内存。
    //       但对于这个练习我们先不处理这个问题，
    //       后面的 hash-calculator 练习会教你分块读取。
    //
    // TODO: data.toBase64() 编码
    // TODO: 显示到输出区
}
```

文件解码类似，但是反过来：读 Base64 文本 → fromBase64 → 写到新文件（用 QFileDialog::getSaveFileName 选择保存位置）。

### 你可能会遇到的坑

QFile::open() 默认用文本模式打开。在 Windows 上，文本模式会自动把 `\r\n` 转成 `\n`，这会破坏二进制文件的内容。必须用 `QIODevice::ReadOnly` 就行——Qt 的 QFile 在所有平台上默认都是二进制模式，不存在文本模式转换的问题。但如果你从 C 标准库迁移过来，可能会习惯性地担心这个——在 Qt 里不用担心。

### 检查点

选择一个小图片文件，点文件编码。输出区应该出现一长串 Base64 文本。然后把这段文本复制，点文件解码，选择保存位置——解码出来的文件应该和原始文件一模一样（你可以用 md5sum 或文件大小验证）。

---

## 最终组装

main.cpp 标准 MainWindow 模式：

```cpp
#include <QApplication>
#include "base64_helper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Base64Helper window;
    window.resize(600, 450);
    window.show();
    return app.exec();
}
```

---

## 验收标准

文本编码/解码往返正确（编码再解码回来和原文一致）。中文文本在 UTF-8 模式下正确编解码。Latin-1 模式下中文表现异常（这是预期行为，体会编码差异）。交换按钮互换输入输出。复制按钮把输出写入系统剪贴板。文件模式能正确编解码二进制文件。无效 Base64 输入解码时不崩溃。

---

## 进阶挑战

加一个拖拽功能——把文件直接拖到窗口上自动编码。查一下 dragEnterEvent 和 dropEvent。或者加一个"实时验证"——输入 Base64 文本时，如果格式不合法就在输入框边框显示红色。再或者，加一个输出长度的进度指示——大文件编码时输出区可能非常长，显示一下编码后文本的字节数。

---

## 踩坑预防清单

> **坑 #1：中文 + Latin-1 乱码**
> Latin-1 只能表示 256 个字符（0x00-0xFF），中文不在其中。如果你用 toLatin1() 转换中文字符串，超出范围的字符会被替换为 `?`。编解码后再也恢复不回来——这不是 bug，是编码本身的限制。所以处理中文一定要用 UTF-8。

> **坑 #2：QSplitter::setSizes 的时机**
> setSizes 需要在控件已经显示之后调用才有效。在构造函数里调用可能被窗口的初始大小覆盖。如果发现分割比例不对，试试在 showEvent 里重新设一次。

> **坑 #3：readAll() 爆内存**
> 几百 MB 的文件直接 readAll() 会导致内存瞬间暴涨。正确的做法是分块读取——这在后面的 hash-calculator 练习里会详细讲解。这个练习我们先不管，但心里要有这个意识。

---

## 官方文档参考

- [QByteArray::toBase64](https://doc.qt.io/qt-6/qbytearray.html) — Base64 编码
- [QByteArray::fromBase64](https://doc.qt.io/qt-6/qbytearray.html) — Base64 解码
- [QString::toUtf8](https://doc.qt.io/qt-6/qstring.html) — UTF-8 编码
- [QSplitter Class](https://doc.qt.io/qt-6/qsplitter.html) — 可分割面板

到这里就大功告成了。这个工具虽然简单，但它涉及了文本编码这个 C++ 开发中永远绕不开的话题。下一篇是本系列的最后一篇——哈希计算器，我们会学习分块文件读取和 QCryptographicHash，把"文件处理"这个主题推向深入。
