# 现代Qt开发教程（新手篇）3.65——QFontDialog：字体选择对话框

## 1. 前言 / 字体选择是文本编辑类工具的刚需

上一节我们聊了 QColorDialog，用户选颜色用的。这一节轮到 QFontDialog——让用户选字体。这两个对话框在文本编辑器、绘图工具、排版软件中几乎总是成对出现：先选字体决定文字长什么样，再选颜色决定文字是什么颜色。

和 QColorDialog 一样，QFontDialog 也提供了一个静态方法 getFont()，一行代码弹出一个字体选择对话框，用户选完字体后返回一个 QFont 对象。但字体选择比颜色选择要复杂一些——字体不是一个单一的数值，它包含字体族（family）、字号（point size）、粗细（weight）、是否斜体（italic）、是否带下划线（underline）、是否带删除线（strikeout）等多个属性。QFontDialog 把这些属性全部打包到了一个对话框中，用户可以在一个界面里同时调整所有属性，最后返回一个完整的 QFont。

除了基本的字体选择之外，QFontDialog 还支持通过 setCurrentFont 设置初始预选字体，让用户在已有字体的基础上做微调而不是从零开始选；通过 currentFontChanged 信号实现实时预览，用户在调整字体时目标文本就跟着变化，不需要反复打开关闭对话框；通过字体过滤机制限制可选字体的范围，比如只显示等宽字体或者只显示中文字体，避免用户在几百个字体中大海捞针。

今天我们从四个方面展开。先看 getFont 静态方法的基本用法，然后讨论 setCurrentFont 设置初始预选字体的方式，接着研究通过 currentFontChanged 信号实现实时预览，最后看看如何通过 QFontDatabase 过滤可选字体范围。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QFontDialog 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QFontDialog、QFont、QFontDatabase、QApplication、QMainWindow、QPushButton、QLabel、QVBoxLayout、QHBoxLayout、QTextEdit 和 QFrame。

## 3. 核心概念讲解

### 3.1 getFont 静态方法：一行代码选字体

QFontDialog::getFont() 弹出一个模态的字体选择对话框，用户在其中选择字体族、字号、样式（普通/粗体/斜体/粗斜体）后，点击 OK 返回 QFont 对象。getFont 有两个重载版本，最常用的版本接受一个 bool* 参数来返回用户是否点击了 OK。

```cpp
bool ok = false;
QFont font = QFontDialog::getFont(&ok, this);

if (ok) {
    qDebug() << "用户选择了:" << font.family()
             << "字号:" << font.pointSize();
    setTextFont(font);
}
```

另一个重载版本允许你指定初始字体——对话框弹出时会预选这个字体：

```cpp
bool ok = false;
QFont initialFont("Microsoft YaHei", 12);
QFont font = QFontDialog::getFont(
    &ok, initialFont, this, "选择标题字体");

if (ok) {
    setTitleFont(font);
}
```

getFont 的参数依次是：bool 指针（接收用户是否确认）、初始字体、父窗口、对话框标题。其中对话框标题参数是可选的，不传的话默认是"Select Font"或者"选择字体"（取决于系统语言）。

这里有一个需要注意的地方：getFont 的 bool 参数不是可选的。如果你不关心用户是确认还是取消，也必须传入一个 bool 变量。你不能传 nullptr——虽然文档说可以传 nullptr，但为了代码的清晰性，建议总是检查返回值。原因和 QColorDialog::getColor 的 isValid() 一样——你不检查的话，用户取消操作后你的代码可能会用默认字体覆盖掉用户之前的设置。

当用户点击 Cancel 时，ok 为 false，返回的 QFont 是初始字体（如果指定了的话）或者应用的默认字体。所以即便用户取消了，返回的 QFont 也是有效的——你可以安全地使用它，但逻辑上你不应该用这个值去覆盖任何东西。

### 3.2 setCurrentFont：设置初始预选字体

getFont 的重载版本已经支持传入初始字体。但当你使用 QFontDialog 的实例化方式（而不是静态方法）时，需要通过 setCurrentFont 来设置初始预选字体。这个功能在"修改已有字体"的场景中非常重要——用户打开字体选择器时看到的应该是当前的字体设置，而不是某个默认值，这样用户可以在现有基础上做微调。

```cpp
QFontDialog dialog(this);
dialog.setWindowTitle("选择正文字体");
dialog.setCurrentFont(m_textEdit->currentFont());

if (dialog.exec() == QDialog::Accepted) {
    QFont selectedFont = dialog.selectedFont();
    m_textEdit->setCurrentFont(selectedFont);
}
```

setCurrentFont 接受一个 QFont 参数。对话框弹出后，字体族列表会滚动到当前字体族所在位置，字号滑块会停在当前字号上，样式选项（粗体、斜体）也会按照当前字体的设置来勾选。这种"所见即所得"的初始状态让用户的操作更加顺畅——特别是当用户只是想把字号从 12 调到 14 的时候，他不需要从几百个字体族中重新找到当前正在用的那个。

selectedFont() 返回用户最终选择的字体。这个方法和 currentFont() 的区别在于：selectedFont() 返回的是用户点击 OK 时的字体（只在 exec() 返回 Accepted 后有效），而 currentFont() 返回的是对话框中当前显示的字体（在对话框还打开时就有效，适合配合 currentFontChanged 信号使用）。

如果你用 getFont 静态方法，初始字体通过第二个参数传入就行了，效果和 setCurrentFont 一样。静态方法内部就是调用 setCurrentFont 来设置初始字体的。选择静态方法还是实例化方式，取决于你是否需要更多的控制——比如设置选项、连接信号、设置自定义按钮文字。

### 3.3 currentFontChanged 信号：实时预览字体变化

和 QColorDialog 的 currentColorChanged 一样，QFontDialog 也提供了 currentFontChanged 信号，在用户修改对话框中的字体设置时不断发射。你可以连接这个信号来实现实时预览——用户在字体选择器中滚动字体列表、调整字号、切换粗体斜体时，目标文本的字体就跟着实时变化。

```cpp
class FontPickerWindow : public QMainWindow
{
    Q_OBJECT
public:
    FontPickerWindow()
    {
        m_textEdit = new QTextEdit(this);
        m_textEdit->setPlainText(
            "The quick brown fox jumps over the lazy dog.\n"
            "-font-");
        setCentralWidget(m_textEdit);

        // 创建工具栏按钮来触发字体选择
        auto *toolbar = addToolBar("格式");
        auto *fontAction = toolbar->addAction("选择字体");
        connect(fontAction, &QAction::triggered,
                this, &FontPickerWindow::onFontDialog);
    }

private:
    void onFontDialog()
    {
        if (!m_fontDialog) {
            m_fontDialog = new QFontDialog(this);
            m_fontDialog->setOption(
                QFontDialog::NoButtons);
            m_fontDialog->setCurrentFont(
                m_textEdit->currentFont());

            connect(m_fontDialog,
                    &QFontDialog::currentFontChanged,
                    this,
                    &FontPickerWindow::onFontChanged);
        }

        m_fontDialog->show();
        m_fontDialog->raise();
    }

    void onFontChanged(const QFont &font)
    {
        // 实时更新文本编辑器的字体
        m_textEdit->setCurrentFont(font);
    }

    QTextEdit *m_textEdit = nullptr;
    QFontDialog *m_fontDialog = nullptr;
};
```

和 QColorDialog 的实时预览模式一样，我们使用了 QFontDialog::NoButtons 选项去掉 OK/Cancel 按钮，配合 show()（非模态）而不是 exec()（模态）来显示对话框。这样对话框就像一个浮动的字体调整面板——用户可以一边编辑文字一边调整字体。

currentFontChanged 的触发频率比 currentColorChanged 低一些——它只在用户执行以下操作时触发：点击字体族列表中的某个字体、修改字号输入框的值、点击粗体/斜体/下划线/删除线按钮。这些操作不像拖动 HSV 色轮那样连续触发，所以通常不需要防抖。

但有一个例外：当用户在字体族列表中使用键盘上下键快速滚动时，currentFontChanged 会在每次按键时触发。如果你的预览操作涉及复杂的文本排版计算（比如整个文档重新分页），这种快速连续触发可能会导致短暂的卡顿。解决方式和颜色选择一样——用 QTimer 防抖，延迟 100 毫秒左右再更新预览。

另一个值得注意的信号是 fontSelected，它在用户最终点击 OK 时触发（只触发一次）。如果你需要同时处理"实时预览"和"最终确认"两种逻辑——比如实时预览只更新当前选中文字的字体，最终确认时更新整个文档的默认字体——可以同时连接 currentFontChanged 和 fontSelected。

### 3.4 过滤可选字体范围

系统上安装的字体可能多达几百种——中文字体、英文字体、日文字体、等宽字体、衬线字体、无衬线字体、装饰性字体、符号字体……如果用户只需要选择一个"代码编辑器字体"，那他只需要看到等宽字体，其他几百个字体都是干扰。QFontDialog 本身没有提供字体过滤的接口——没有 setFontFilter() 这样的方法——但我们可以通过 QFontDatabase 来实现自定义的字体选择方案。

QFontDatabase 提供了查询系统字体的能力。你可以用它来获取所有字体族的列表，然后根据字体的属性（是否等宽、是否支持中文、是否是符号字体等）来过滤出你需要的子集。

```cpp
QFontDatabase fontDb;
const QStringList allFamilies = fontDb.families();

// 过滤出等宽字体
QStringList monoFamilies;
for (const QString &family : allFamilies) {
    if (fontDb.isFixedPitch(family)) {
        monoFamilies.append(family);
    }
}

// monoFamilies 就是你需要的等宽字体列表
// 比如: "Courier New", "Consolas", "Monaco",
//        "Source Code Pro", "JetBrains Mono" 等
```

fontDb.families() 返回系统上所有可用的字体族名称。fontDb.isFixedPitch(family) 判断某个字体族是否是等宽字体——等宽字体中每个字符占用的宽度相同，非常适合代码编辑器和终端模拟器。

拿到过滤后的字体列表后，你有两种方式来使用它。第一种是放弃 QFontDialog，自己用 QComboBox + QSpinBox + QCheckBox 搭建一个自定义的字体选择面板，只包含过滤后的字体。这种方式最灵活，但需要你手动实现字体预览、样式选择等功能——工作量不小。

第二种方式更取巧：利用 QFontDialog 的子类化或者样式提示来限制显示范围。不过坦率讲，QFontDialog 没有提供公开的接口来直接过滤字体族列表。如果你确实需要在 QFontDialog 中只显示特定字体，唯一的办法是通过 QFontDatabase 的 writingSystem 参数来缩小范围：

```cpp
QFontDatabase fontDb;
// 只获取支持简体中文的字体
QStringList cnFamilies =
    fontDb.families(QFontDatabase::SimplifiedChinese);
```

families() 的参数是 QFontDatabase::WritingSystem 枚举，包括 Latin、SimplifiedChinese、Japanese、Arabic、Cyrillic 等几十种书写系统。传入特定的书写系统后，返回值只包含支持该书写系统的字体族。这虽然不是精确的"只显示等宽字体"，但对于"只显示中文字体"或者"只显示日文字体"的需求来说是够用的。

如果你需要更精确的过滤——比如"只显示等宽字体并且支持中文"——那就只能自定义字体选择面板了。一个实用的方案是用 QComboBox 显示过滤后的字体列表，配合一个 QTextEdit 作为预览区域，用户在 QComboBox 中选择字体时实时更新预览。这个方案比 QFontDialog 简单很多，而且在特定场景下（比如代码编辑器的字体设置）用户体验更好——因为用户不需要在几百个字体中大海捞针，只看到他关心的等宽字体。

## 4. 踩坑预防

第一个坑是忽略 getFont 的 bool 返回值。当用户点击 Cancel 时，ok 为 false，返回的 QFont 是初始字体或默认字体。如果你不检查 ok 就直接使用返回值，用户取消操作后你的代码可能用默认字体覆盖掉用户之前的设置。在"修改字体"的场景中这尤其危险——用户本来用了精心调好的字体，手滑打开了字体选择器然后点了取消，结果字体被重置了。

第二个坑是字体族名称的平台差异。同一个字体在不同操作系统上的名称可能不同——比如微软雅黑在 Windows 上叫 "Microsoft YaHei"，在 Linux 上可能没有这个字体。如果你的代码中硬编码了字体族名称（比如用于 setCurrentFont 的默认值），需要做好跨平台兼容——用 QFontDatabase::families() 检查字体是否存在，不存在就用 fallback 字体。

第三个坑是 pointSize 和 pixelSize 的混淆。QFont 有两种设置字号的方式：setPointSize() 和 setPixelSize()。QFontDialog 使用的是 pointSize（磅值，跟 DPI 无关），但有些场景下你可能习惯用 pixelSize（像素值，跟 DPI 相关）。两者不能同时使用——设置了其中一个，另一个会被设为 -1。从 QFontDialog 拿到字体后，如果你需要用 pixelSize，需要手动转换。

第四个坑是 QFontDialog::NoButtons 配合非模态显示时的生命周期管理。如果你用 show() 显示了一个 QFontDialog，用户关闭主窗口时如果 QFontDialog 还开着，它会被自动销毁（因为它的 parent 是主窗口）。但如果你的代码中保存了 QFontDialog 的指针并且在主窗口析构后试图访问它，就会崩溃。建议在主窗口的析构函数或者 closeEvent 中检查并清理 QFontDialog 实例。

第五个坑是 isFixedPitch 的判断结果不一定准确。某些字体的等宽属性元数据不正确——它们声称自己是等宽的但实际上不是，或者反过来。如果你需要严格的等宽判断，可能需要通过 QFontMetrics 逐字符测量宽度来验证。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，中央是一个 QTextEdit，上方是工具栏按钮。"选择字体"按钮调用 QFontDialog::getFont() 弹出模态字体选择对话框，初始字体为当前 QTextEdit 的字体，用户确认后更新 QTextEdit 的字体。"实时预览"按钮创建一个非模态的 QFontDialog 实例，设置 NoButtons 选项，连接 currentFontChanged 信号到 QTextEdit 的 setCurrentFont 槽——用户在字体选择器中调整时，QTextEdit 的字体实时变化。"等宽字体"按钮弹出一个自定义的 QDialog，其中包含一个只列出等宽字体的 QComboBox（通过 QFontDatabase::isFixedPitch 过滤）、一个字号 QSpinBox（范围 8~72）、粗体和斜体两个 QCheckBox，以及一个预览标签用选中的字体显示示例文字。用户在自定义对话框中调整任何设置时，预览标签的字体实时更新。

QTextEdit 的初始文本包含多行内容，既有英文也有中文，方便用户观察不同字体的效果。初始字体设为 "Microsoft YaHei" 12pt（Linux 上 fallback 到系统默认字体）。

提示：等宽字体对话框通过 QFontDatabase::families() 获取所有字体，然后用 isFixedPitch 过滤。自定义对话框的信号连接使用 QComboBox::currentTextChanged、QSpinBox::valueChanged 和 QCheckBox::stateChanged 来实时更新预览。

## 6. 官方文档参考链接

[Qt 文档 -- QFontDialog](https://doc.qt.io/qt-6/qfontdialog.html) -- 字体选择对话框类

[Qt 文档 -- QFontDialog::getFont](https://doc.qt.io/qt-6/qfontdialog.html#getFont) -- 获取字体静态方法

[Qt 文档 -- QFont](https://doc.qt.io/qt-6/qfont.html) -- 字体类

[Qt 文档 -- QFontDatabase](https://doc.qt.io/qt-6/qfontdatabase.html) -- 字体数据库查询

[Qt 文档 -- QFontDatabase::WritingSystem](https://doc.qt.io/qt-6/qfontdatabase.html#WritingSystem-enum) -- 书写系统枚举

---

到这里，QFontDialog 的核心用法就全部讲完了。getFont 静态方法一行代码完成最基本的字体选择，setCurrentFont 让用户在已有字体的基础上做微调，currentFontChanged 信号实现了实时预览的流畅体验，QFontDatabase 提供了字体过滤的能力让你可以根据业务需求缩小可选范围。这三层能力——静态方法处理简单场景、实例化加信号处理交互场景、QFontDatabase 过滤处理定制场景——覆盖了字体选择从简单到复杂的全部需求。
