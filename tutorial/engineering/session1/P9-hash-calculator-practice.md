# 实战练习 · HashCalculator — 分块读取 + 哈希 + 进度条

## 前言：一个真正实用的文件工具

哈希校验听起来很底层，但它其实无处不在。你下载了一个大文件，怎么确认它没被篡改？对了，算一下 SHA-256 和官网给的值对比一下。你要检测两个文件内容是否相同？比较 MD5 就行。你要给密码做不可逆存储？哈希是第一步。无论哪种场景，核心操作都是同一个：把一段数据（不管多长）变成一个固定长度的"指纹"。

这篇练习我们要做一个图形化的哈希计算器，支持 MD5、SHA-1、SHA-256、SHA-512 四种算法。但这个练习真正的核心不是哈希 API 本身——那真的就几行代码——而是**分块读取文件**这个技术。如果你的文件有 4GB，一次性 readAll() 会直接把内存吃满甚至 OOM。正确的做法是每次读 64KB，喂给哈希计算器，循环直到文件读完。这个模式在文件处理类应用里太常见了，掌握了它，以后处理任何大文件都不慌。

---

## 出发前的装备清单

- **QCryptographicHash** — Qt 内置的加密哈希类，支持 MD5、SHA-1、SHA-256、SHA-512 等。核心 API：addData() 喂数据，result() 取结果。
- **QFile** — 文件读写。这次我们要用它的分块读取能力：read(maxSize) 每次最多读指定字节数。
- **QProgressBar** — 进度条，显示哈希计算进度。
- **QFileDialog** — 文件选择。
- **QComboBox** — 算法选择。
- **QCoreApplication::processEvents()** — 在长时间循环中手动处理事件，防止 UI 冻结。注意：这只是临时方案，真正的做法是用多线程，但那超出了本练习的范围。

---

## 我们的目标长什么样

```
┌──────────────────────────────────────────┐
│  文件: [C:/Users/test/video.mp4] [浏览]  │
│  算法: [SHA-256 ▼]                       │
│                                          │
│  [计算]  [复制哈希]                       │
│                                          │
│  进度: ████████████████░░░░░░░░  62%     │
│                                          │
│  结果:                                   │
│  a3f2b8c1d4e5f6...（可选中复制）          │
│                                          │
│  ── 文本哈希 ──                           │
│  输入: [________________] [计算文本哈希]  │
│  结果: b2e4d1c3...                       │
└──────────────────────────────────────────┘
```

完成标准：文件哈希结果和命令行工具（md5sum / sha256sum）一致，进度条正确显示，文本直接哈希也工作正常。

---

## 第一步 — 窗口布局

### 思考题

这个界面的布局思路是什么？从上到下：文件选择行、算法选择行、操作按钮行、进度条、结果显示区，最后还有一个文本哈希区。用什么布局能把这么多东西整齐地排列？提示：QVBoxLayout 做主轴，每行一个 QHBoxLayout 或直接用 QFormLayout。

### 动手写

```cpp
#pragma once
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QWidget>

class HashCalculator : public QWidget
{
    Q_OBJECT

public:
    explicit HashCalculator(QWidget *parent = nullptr);

private slots:
    void onBrowse();
    void onCalculateFile();
    void onCalculateText();
    void onCopy();

private:
    void setupUi();
    QByteArray hashFile(const QString &filePath,
                        QCryptographicHash::Algorithm algorithm);
    QByteArray hashText(const QString &text,
                        QCryptographicHash::Algorithm algorithm);
    QCryptographicHash::Algorithm currentAlgorithm() const;

    QLineEdit *m_filePathEdit;
    QComboBox *m_algorithmCombo;
    QProgressBar *m_progressBar;
    QLineEdit *m_resultEdit;      // 哈希结果显示
    QLineEdit *m_textInput;       // 文本哈希输入
    QLineEdit *m_textResultEdit;  // 文本哈希结果
};
```

布局的关键是把"文件哈希"和"文本哈希"分成两个视觉区域。可以用 QGroupBox 或者简单的分割线隔开。QFormLayout 或者 QGridLayout 都能把标签和控件对齐好——选你顺手的。

### 检查点

编译运行，检查布局是否符合预期。所有控件都可见，间距合理。

---

## 第二步 — 文件选择

### 动手写

```cpp
void HashCalculator::onBrowse()
{
    // TODO: QFileDialog::getOpenFileName(this, "选择文件")
    //       如果返回非空字符串，设置到 m_filePathEdit
}
```

没什么花哨的。QFileDialog::getOpenFileName 会弹出系统原生的文件选择对话框，返回用户选择的文件完整路径。用户取消的话返回空字符串。

### 检查点

点浏览按钮，选择一个文件，路径显示在输入框里。

---

## 第三步 — 文本哈希：最简单的情况

### 思考题

QCryptographicHash 有两种用法：静态方法 `QCryptographicHash::hash(data, algorithm)` 一次性计算，和实例方法 `hash.addData(chunk)` 分块喂入。对于文本哈希，数据量很小，用哪种更方便？对于文件哈希，又该用哪种？提示：静态方法一次性搞定但不支持分块；实例方法支持分块但需要自己管生命周期。

### 动手写

```cpp
QByteArray HashCalculator::hashText(const QString &text,
                                     QCryptographicHash::Algorithm algo)
{
    // TODO: 最简单的实现——一行静态方法调用
    //       return QCryptographicHash::hash(text.toUtf8(), algo);
    //
    //       就这么一行。toUtf8() 把 QString 转成字节数组，
    //       hash() 静态方法返回哈希结果（也是 QByteArray）。
}
```

然后在槽函数里调用并显示结果：

```cpp
void HashCalculator::onCalculateText()
{
    // TODO: 获取文本和当前算法
    //       调用 hashText()
    //       把结果的 toHex() 显示到 m_textResultEdit
}
```

关于 toHex()：QCryptographicHash::result() 返回的是原始字节（比如 32 字节的 SHA-256），不是人类可读的十六进制字符串。`toHex()` 把每个字节转成两位十六进制表示，比如 `\xa3\xf2` 变成 `"a3f2"`。这才是你在网上看到的哈希值格式。

### 检查点

输入 "Hello World"，选择 SHA-256，点计算。结果应该是 `a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e`。你可以用命令行验证：`echo -n "Hello World" | sha256sum`。

---

## 第四步 — 分块文件哈希：核心挑战

### 思考题

这是本篇最重要的思考题。假设你要算一个 4GB 文件的 SHA-256，你有两种策略：第一种是一次性 readAll() 然后调用静态 hash()；第二种是循环 read(65536) 每次读 64KB，调用 addData() 喂给哈希实例。第一种会发生什么？为什么分块大小选 64KB 而不是 1MB 或者 1KB？提示：64KB 是一个在大多数操作系统上读写效率的"甜蜜点"——它足够大以摊薄系统调用开销，又足够小不会浪费内存。

### 动手写

```cpp
QByteArray HashCalculator::hashFile(const QString &filePath,
                                     QCryptographicHash::Algorithm algo)
{
    // TODO: 打开文件
    //       QFile file(filePath);
    //       if (!file.open(QIODevice::ReadOnly)) {
    //           return QByteArray();  // 打开失败返回空
    //       }

    // TODO: 获取文件大小（用于进度计算）
    //       qint64 fileSize = file.size();

    // TODO: 创建哈希实例
    //       QCryptographicHash hash(algo);

    // TODO: 分块读取循环
    //       qint64 bytesRead = 0;
    //       const int chunkSize = 65536;  // 64KB
    //       while (!file.atEnd()) {
    //           QByteArray chunk = file.read(chunkSize);
    //           if (chunk.isEmpty()) break;
    //           hash.addData(chunk);
    //           bytesRead += chunk.size();
    //
    //           更新进度条：
    //           int progress = int(bytesRead * 100 / fileSize);
    //           m_progressBar->setValue(progress);
    //
    //           保持 UI 响应：
    //           QCoreApplication::processEvents();
    //       }

    // TODO: file.close();
    // TODO: return hash.result();
}
```

这个循环模式值得刻在脑子里：`while (!file.atEnd()) { chunk = read(N); process(chunk); }`。它适用于任何大文件处理场景——哈希、压缩、加密、校验、传输——只要你的处理逻辑是"对数据流逐块操作"，这个骨架就能用。

关于 `QCoreApplication::processEvents()`：这个函数强制 Qt 事件循环处理一次所有待处理的事件（重绘、鼠标点击、定时器等）。没有它，你的 while 循环会霸占 CPU，UI 完全冻结——进度条不动、窗口拖不动、按钮点不了。加上它之后，每处理一个 chunk 就给 UI 一个喘息的机会。这不是最优方案（真正的做法是用 QThread 在后台算），但对于这个练习够用了。

### 你可能会遇到的坑

如果你用 `file.readAll()` 代替分块读取，一个小文件（几十 KB）完全没问题，但一个大文件（比如 2GB 的视频）会直接吃掉 2GB 内存。32 位程序甚至可能因为地址空间不够而分配失败。这个坑在你测试的时候不容易发现（因为测试文件通常很小），但到了生产环境大文件就会炸。所以从现在开始养成习惯：只要涉及文件读取，优先用分块。

### 检查点

选一个已知文件（比如项目里的 main.cpp），选 MD5 算法，点计算。然后用命令行验证：`md5sum main.cpp`。两者的哈希值应该完全一致。

---

## 第五步 — 进度条和复制结果

### 动手写

进度条的更新已经在 hashFile 的循环里实现了。每次读一个 chunk 就更新一下百分比。

```cpp
void HashCalculator::onCopy()
{
    // TODO: 根据当前焦点区域，复制对应的哈希结果到剪贴板
    //       简单起见，直接复制文件哈希结果：
    //       QApplication::clipboard()->setText(m_resultEdit->text());
}
```

算法选择 QComboBox 的初始化放在 setupUi 里：

```cpp
// m_algorithmCombo->addItem("MD5", QVariant::fromValue(QCryptographicHash::Md5));
// m_algorithmCombo->addItem("SHA-1", QVariant::fromValue(QCryptographicHash::Sha1));
// m_algorithmCombo->addItem("SHA-256", QVariant::fromValue(QCryptographicHash::Sha256));
// m_algorithmCombo->addItem("SHA-512", QVariant::fromValue(QCryptographicHash::Sha512));
```

用 QComboBox 的 userData 功能（第二个参数）存储枚举值，这样 currentAlgorithm() 就是 `m_algorithmCombo->currentData().value<QCryptographicHash::Algorithm>()`。

### 检查点

选一个大一点的文件计算哈希。进度条应该从 0% 走到 100%，过程中窗口可以拖动、按钮可以点击（不会完全冻结）。计算完成后点复制，到别处粘贴能粘出哈希值。

---

## 最终组装

```cpp
#include <QApplication>
#include "hash_calculator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    HashCalculator calculator;
    calculator.resize(500, 400);
    calculator.show();
    return app.exec();
}
```

---

## 验收标准

文本哈希结果和命令行工具（echo -n "text" | sha256sum）一致。文件哈希结果和 md5sum / sha256sum 命令行工具一致。进度条正确反映文件读取进度（从 0% 到 100%）。计算过程中 UI 不会完全冻结（可以拖动窗口）。切换算法后重新计算结果正确。复制功能把十六进制哈希字符串写入剪贴板。

---

## 进阶挑战

用 QThread 把文件哈希计算移到后台线程，彻底解决 UI 冻结问题——提示：创建一个 Worker 类，.moveToThread()，用信号槽通信。或者加一个"拖拽文件"功能，直接把文件拖到窗口上开始计算。再或者，加一个"比较"模式：输入一个已知哈希值，计算完后自动比对并显示"匹配/不匹配"。

---

## 踩坑预防清单

> **坑 #1：一次性 readAll() 爆内存**
> readAll() 把整个文件读进内存。4GB 文件 = 4GB 内存占用，32 位程序直接 GG。分块读取是唯一正确的做法。即使你的测试文件很小，也要用分块——因为你不知道用户会拿什么文件来算哈希。

> **坑 #2：toHex() 的输出格式**
> QCryptographicHash::result() 返回原始字节。你必须调用 toHex() 转成十六进制字符串才能和网上的哈希值对照。如果你忘了这一步，显示的是一堆乱码，而且你还以为算法写错了。

> **坑 #3：processEvents() 不是银弹**
> processEvents() 能让 UI 不冻结，但它有副作用：在处理事件期间，用户可能点了"重置"或者关闭了窗口，而你的 while 循环还在跑。这可能导致访问已销毁的对象。解决办法：在循环里检查一个 "cancelled" 标志，processEvents 后检查它。但这只是权宜之计，真正的做法是用 QThread。

---

## 官方文档参考

- [QCryptographicHash Class](https://doc.qt.io/qt-6/qcryptographichash.html) — 哈希算法
- [QFile Class](https://doc.qt.io/qt-6/qfile.html) — 文件读取
- [QProgressBar Class](https://doc.qt.io/qt-6/qprogressbar.html) — 进度条

到这里就大功告成了。九篇实战练习全部完成——从最简单的 LED 指示灯到分块文件哈希计算器，你走了很长的路。如果你一路跟着做了下来，你现在应该能独立开发自定义控件、组合控件、多页向导、对话框，以及完整的文件处理工具。这些技能覆盖了 Qt Widgets 开发的核心场景，剩下的就是在实战中积累经验了。恭喜你坚持到了最后。
