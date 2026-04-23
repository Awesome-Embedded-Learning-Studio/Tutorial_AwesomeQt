# 现代Qt开发教程（新手篇）1.13——国际化

## 1. 前言：为什么一开始就要考虑国际化

说实话，我第一次做独立项目的时候，完全没想过国际化这回事。等程序写到一半，用户说「我们需要英文版」，我才发现所有的用户可见文本都硬编码成了中文。那时候我花了整整一个周末把所有字符串翻出来，用最笨拙的方式一一替换。

如果你一开始就做了国际化准备，后续添加语言支持只需要翻译一个 `.ts` 文件，改一行代码就能切换语言。但如果后期再补，工作量是指数级增长的。

Qt 的国际化系统非常成熟，从 `tr()` 函数到 `lupdate`/`lrelease` 工具链，再到 Qt Linguist 翻译工具，形成了一套完整的工作流。这一篇我们要做的就是把这个工作流跑通，让你的程序从一开始就具备走向世界的能力。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要 QtCore（CMake 组件 Qt6::Core，提供 `tr()` 和 `QTranslator`）以及 Qt Tools 中的 lupdate 和 lrelease 命令行工具。Qt Linguist 是一个独立的 GUI 工具，通常随 Qt 安装包一同安装。如果你用 Qt Creator，这些工具都已经配置好了。

## 3. 核心概念讲解

### 3.1 tr() 函数——国际化的基石

Qt 的国际化系统围绕 `tr()` 函数展开。这个函数的作用是标记需要翻译的字符串，并在运行时查找对应的翻译。

```cpp
// 在 QObject 派生类中
QString text = tr("Hello, World!");
QString greeting = tr("Welcome to %1").arg(applicationName);
```

`tr()` 是 `QObject` 的静态成员函数，所以所有继承自 `QObject` 的类都可以直接调用。它的基本签名是：

```cpp
QString tr(const char *sourceText,
           const char *disambiguation = nullptr,
           int n = -1) const;
```

三个参数各自有明确的用途：`sourceText` 是源文本（通常是英文），`disambiguation` 是消歧义字符串，用于同样文本在不同上下文有不同翻译的场景，`n` 则用于复数形式的数字。

消歧义是个很实用的功能。比如「File」这个词在菜单里是「文件」，在动词时是「文件操作」。你可以这样区分：

```cpp
QString menuText = tr("File", "Menu item");         // 菜单项
QString actionText = tr("File", "To file something"); // 动词
```

翻译者看到这两个条目时会分别处理，不会混淆。

### 3.2 复数形式处理

不同语言的复数规则差异很大，比如英语只有单数和复数，而阿拉伯语有六种复数形式。Qt 用一个特殊的 `tr()` 语法处理这个问题：

```cpp
int count = messages.size();
QString text = tr("%n message(s)", "", count);
```

这里的 `%n` 是特殊占位符，Qt 会根据 `count` 的值和语言规则自动选择正确的复数形式。翻译文件里会定义每种数字范围对应的翻译。

你可能会问，为什么不能用传统的 `if (count == 1)` 来处理复数？问题在于不同语言的复数规则完全不同——波兰语有单数、复数、少数等多种形式，阿拉伯语更复杂。如果你在代码里用 `if` 判断，就需要为每种语言写不同的逻辑分支，维护成本直接爆炸。Qt 的 `%n` 机制把复数规则放到翻译文件里，代码本身不需要知道目标语言的具体规则，这才是正确的解耦方式。

### 3.3 QTranslator——加载翻译文件

`QTranslator` 负责在运行时加载翻译文件（`.qm`）并提供翻译查询功能。

```cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 创建翻译器
    QTranslator translator;

    // 加载翻译文件
    if (translator.load(":/translations/zh_CN.qm")) {
        app.installTranslator(&translator);
    }

    // 之后所有 tr() 调用都会使用这个翻译
    return app.exec();
}
```

`load()` 的参数可以是文件路径，也可以是资源路径（`:` 开头）。加载成功后，用 `installTranslator()` 把翻译器安装到应用程序实例上。之后所有 `tr()` 调用都会通过这个翻译器查找翻译。

你可以安装多个翻译器，Qt 会按照安装顺序依次查找，找到第一个匹配的翻译就使用。

```cpp
QTranslator baseTranslator;
QTranslator appTranslator;

baseTranslator.load(":/qtbase_zh_CN.qm");
appTranslator.load(":/app_zh_CN.qm");

app.installTranslator(&baseTranslator);  // 先安装 Qt 基础翻译
app.installTranslator(&appTranslator);   // 再安装应用翻译
```

这个顺序很重要——如果应用翻译覆盖了 Qt 基础翻译（比如你重新翻译了标准对话框的按钮），应该把应用翻译放在后面。

这里有个很容易踩的坑：翻译器必须在任何需要翻译的 QObject 创建之前安装。如果你在 `main()` 之前创建了一个全局的 QObject，它的 `tr()` 调用发生在翻译器安装之前，永远只能拿到源文本。正确的做法是把所有需要翻译的对象放到翻译器安装之后再创建：

```cpp
// 正确做法：先安装翻译器，再创建对象
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QTranslator translator;
    translator.load("zh_CN.qm");
    app.installTranslator(&translator);
    // 在安装翻译器之后再创建对象
    MyGlobalWidget globalWidget;
}
```

### 3.4 lupdate/lrelease 工作流

Qt 的翻译工具链分三个阶段：首先用 `lupdate` 扫描源代码，找出所有 `tr()` 调用，生成 `.ts` 文件；然后在 `.ts` 文件中用 Qt Linguist 添加翻译文本；最后用 `lrelease` 把 `.ts` 编译成 `.qm` 二进制文件供运行时使用。

`.ts` 文件是 XML 格式，人类可读；`.qm` 是二进制格式，体积小加载快。

完整的 CMakeLists.txt 配置：

```cmake
cmake_minimum_required(VERSION 3.26)
project(MyApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 添加可执行文件
qt_add_executable(myapp main.cpp)

target_link_libraries(myapp PRIVATE Qt6::Core Qt6::Widgets)

# 设置翻译源文件
set(TS_FILES translations/myapp_zh_CN.ts)

# 让 lupdate 能找到源文件
target_sources(myapp PRIVATE ${TS_FILES})

# 添加 lupdate 和 lrelease 目标
qt_add_lupdate(myapp
    TS_FILES ${TS_FILES}
    OPTIONS -no-obsolete
)

qt_add_lrelease(myapp
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE qm_files
)

# 把 .qm 文件添加到资源（可选）
#qt_add_resources(myapp "translations"
#    FILES ${qm_files}
#    PREFIX "/translations"
#)
```

这样配置后，你可以运行 `cmake --build . --target lupdate` 更新 `.ts` 文件，运行 `cmake --build . --target lrelease` 生成 `.qm` 文件。`-no-obsolete` 选项会从 `.ts` 文件中删除已经不存在的翻译条目，保持文件整洁。

这里我们顺便看一个动态切换语言的代码片段，补全关键部分就能跑：

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        m_translator = new QTranslator(this);
        switchLanguage("zh_CN");
    }

    void switchLanguage(const QString &language) {
        // 移除旧翻译器
        qApp->removeTranslator(m_translator);

        // 加载新翻译
        QString fileName = QString(":/translations/%1.qm").arg(language);
        if (m_translator->load(fileName)) {
            qApp->installTranslator(m_translator);  // 安装翻译器
        }

        // 重新构建 UI，让 tr() 重新执行
        retranslateUi();
    }

private:
    QTranslator *m_translator;
};
```

核心逻辑就是三步：先 `removeTranslator` 拿掉旧的，再 `load` 加载新的，最后 `installTranslator` 装上去。装完之后别忘了调用 `retranslateUi()` 让界面刷新。

### 3.5 字符串字面量 vs tr() 的选择

不是所有字符串都需要翻译，只有用户可见的文本才需要 `tr()` 包裹。

```cpp
// 需要翻译
setLabel(tr("File Name:"));
setWindowTitle(tr("Open File"));
showMessage(tr("Operation completed successfully"));

// 不需要翻译
qDebug() << "Debug: value changed to" << value;  // 调试输出
setObjectName("mainButton");                      // 对象名
writeLog("User logged in");                       // 日志
```

一个常见的错误是对所有字符串都加 `tr()`，这会增加翻译工作量，也会让 `.ts` 文件变得臃肿。

### 3.6 上下文和翻译条目组织

`tr()` 调用有一个默认的「上下文」，就是类的名称。这有助于避免相同文本在不同类中的翻译冲突。

```cpp
// 在 FileMenu 类中
tr("Open")  // 上下文是 "FileMenu"

// 在 Dialog 类中
tr("Open")  // 上下文是 "Dialog"
```

翻译者会看到两个不同的条目：`FileMenu::Open` 和 `Dialog::Open`，可以分别翻译。如果你想使用自定义上下文，可以用 `QT_TR_CONTEXT_NOOP` 宏：

```cpp
// 在类的顶部定义自定义上下文
static const char * const my_context = "CustomContext";
#define tr(s) QObject::tr(s)

// 使用时
tr("Save")  // 上下文是 "CustomContext"
```

不过大多数情况下，默认的类名上下文就够用了。

说到上下文，还有一个常见的坑：`tr()` 在非 QObject 派生类中直接调用会编译错误，因为它本质上是 `QObject` 的成员函数。如果你的类没有继承 QObject，需要用 `QObject::tr()` 这个静态版本来调用，或者把上下文对象传进去：

```cpp
struct Data {
    QString getStatus() {
        return QObject::tr("Active");  // 使用静态成员
    }
};
// 或者更好的做法，传递翻译上下文
struct Data {
    QString getStatus(QObject *ctx) {
        return ctx->tr("Active");
    }
};
```

### 3.7 动态语言切换的完整流程

如果你的应用需要在运行时切换语言，需要做一点额外的工作。因为 `tr()` 调用只发生在执行时刻，切换翻译器不会自动更新已经显示的文本。

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        m_translator = new QTranslator(this);

        // 创建语言切换菜单
        QMenu *langMenu = menuBar()->addMenu(tr("Language"));
        QAction *enAction = langMenu->addAction("English");
        QAction *zhAction = langMenu->addAction("中文");

        connect(enAction, &QAction::triggered, [this]() { switchLanguage("en"); });
        connect(zhAction, &QAction::triggered, [this]() { switchLanguage("zh_CN"); });

        // 创建 UI
        m_label = new QLabel(tr("Hello, World!"), this);
        setCentralWidget(m_label);
    }

    void switchLanguage(const QString &language) {
        // 移除旧翻译
        qApp->removeTranslator(m_translator);

        // 加载新翻译
        if (m_translator->load(QString(":/translations/%1.qm").arg(language))) {
            qApp->installTranslator(m_translator);
        }

        // 重新翻译 UI
        m_label->setText(tr("Hello, World!"));  // tr() 会查找新翻译

        // 如果有菜单或标准按钮，也要重新设置
        menuBar()->clear();
        // 重建菜单...
    }

private:
    QTranslator *m_translator;
    QLabel *m_label;
};
```

关键点是：切换翻译器后，需要重新执行所有 `tr()` 调用。这通常意味着你需要一个 `retranslateUi()` 方法来更新所有用户可见的文本。

很多朋友会在这里翻车——装完翻译器就以为万事大吉，结果界面文本纹丝不动。原因很简单：`tr()` 在构造函数里已经执行过了，当时拿到的就是源文本，后面换翻译器不会让已经设置好的文本自动变。必须手动重新调一遍 `tr()` 并更新 UI 才行。

## 4. 踩坑预防

我们在前面已经提到了几个常见的坑，这里再补充几个容易忽略的细节。

`tr()` 的参数必须是字符串字面量，动态拼接的字符串 `lupdate` 无法提取。所以如果你写了 `tr("Loading file " + fileName)`，翻译文件里根本不会有这个条目。正确的做法是用占位符：`tr("Loading file %1").arg(fileName)`。同样的道理，`tr("File: ") + fileName` 也是有问题的——虽然 `"File: "` 这部分能被提取，但整个翻译流程就被拼接打断了，不如直接用 `%1` 占位符干净利落。

还有一个容易被忽略的问题是构造函数初始化列表中的 `tr()` 调用。在初始化列表执行时，派生类的虚表还没完全建立，`tr()` 的行为可能不符合预期。所以如果要在构造函数里设置文本，最好在函数体内用 `setText(tr("..."))` 的方式，而不是在初始化列表里直接传 `tr("...")` 给成员对象。

最后，翻译文件的编码问题。`.ts` 文件本质上是 XML，务必用 UTF-8 编码保存。如果你用文本编辑器直接修改 `.ts` 文件然后保存成了 GBK 之类的编码，非 ASCII 字符在运行时会变成乱码。最省心的办法还是用 Qt Linguist 编辑翻译文件，它会自动处理编码。

## 5. 练习项目

做一个简单的时钟应用，支持中英文切换。显示当前时间，并提供菜单选项切换语言。

完成标准：窗口标题随语言变化（「时钟」/「Clock」），时间标签下方显示语言切换后的提示文本，菜单栏有「Language」菜单包含「English」和「中文」选项，切换语言后界面立即更新无需重启，正确生成和加载 `.qm` 翻译文件。

几个提示：用 `QTimer` 每秒更新时间显示，创建一个 `retranslateUi()` 方法统一处理所有文本更新，用 `QAction::setData()` 存储语言代码简化槽函数，记得在 CMakeLists.txt 中配置 `qt_add_lupdate` 和 `qt_add_lrelease`。

## 6. 官方文档参考

- [Qt 文档 · Internationalization with Qt](https://doc.qt.io/qt-6/internationalization.html) —— Qt 国际化的完整概述，包含所有相关工具和最佳实践
- [Qt 文档 · QTranslator](https://doc.qt.io/qt-6/qtranslator.html) —— QTranslator 类的详细说明，包含 load() 和 installTranslator() 的用法
- [Qt 文档 · Writing Source Code for Translation](https://doc.qt.io/qt-6/i18n-source-translation.html) —— 如何编写可翻译的源代码，tr() 函数的完整说明
- [Qt 文档 · Qt Linguist Manual](https://doc.qt.io/qt-6/qtlinguist-index.html) —— Qt Linguist 翻译工具的使用指南

---

到这里就大功告成了！Qt 的国际化系统虽然步骤多，但每一步都很清晰：用 `tr()` 标记字符串、lupdate 提取、Qt Linguist 翻译、lrelease 编译、QTranslator 加载。这套工作流掌握后，你的程序从一开始就具备了走向世界的能力。下一篇我们进入日志系统，看看 Qt 是如何帮助开发者调试和诊断问题的。
