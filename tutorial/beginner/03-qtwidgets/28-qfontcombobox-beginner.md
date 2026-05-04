# 现代Qt开发教程（新手篇）3.28——QFontComboBox：字体选择下拉框

## 1. 前言 / 为什么需要一个专门的字体选择控件

我们在上一篇把 QComboBox 的基本能力全部过了一遍——addItem、currentData、setEditable、setModel，该覆盖的都覆盖了。你可能会想，字体选择不就是在 QComboBox 里填一堆字体名称嘛，为什么要单独拎出一个 QFontComboBox 来讲？这个问题问得合理，但答案也很直接：QFontComboBox 不是简单的"QComboBox + 字体名称列表"，它在几个关键维度上做了专门的封装和优化，这些事情你自己用 QComboBox 手撸要花不少功夫。

首先是字体列表的自动获取。QFontComboBox 在构造的时候会自动调用 QFontDatabase 拿到当前系统所有可用的字体家族名称，然后填充到下拉列表里——你不需要自己遍历 QFontDatabase::families() 再逐个 addItem。其次，每个选项的显示不是纯文本，而是用对应字体来渲染字体名称本身——也就是说"Arial"用 Arial 字体显示，"宋体"用宋体显示，用户一眼就能看到字体的真实外观。再然后，QFontComboBox 提供了字体类型过滤功能，你可以通过 setFontFilters 把列表限定为等宽字体、比例字体或者系统默认的某种类型，不需要自己去分类。最后，它的 currentFont 信号直接返回 QFont 对象，你拿到就能用，省去了从字符串到 QFont 的转换步骤。

我们今天把 QFontComboBox 的四个核心维度讲清楚：setFontFilters 过滤字体类型、currentFont 获取选中的字体对象、实时预览字体变化的完整流程、以及字体名称本地化的显示机制。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QFontComboBox 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。它内部依赖 QtGui 中的 QFontDatabase 来获取系统字体列表，不过因为 Widgets 已经依赖了 Gui，所以不需要额外处理链接关系。示例代码中用到了 QTextEdit 来做字体预览，QTextEdit 同样在 QtWidgets 中，没有额外的依赖。

## 3. 核心概念讲解

### 3.1 setFontFilters() 过滤字体类型

QFontComboBox 构造完成后，下拉列表里默认会显示当前系统安装的所有字体。在大多数桌面系统上这个列表可能有几十甚至上百个条目——各种西文字体、CJK 字体、符号字体混在一起，找起来很费劲。QFontComboBox 提供了 `setFontFilters()` 方法，让你可以按字体类型做一轮筛选。

setFontFilters 接受一个 QFontComboBox::FontFilter 枚举值的组合。可用的过滤器有四种：AllFonts 显示所有字体（默认值），ScalableFonts 只显示可缩放字体（也就是矢量字体，排除位图字体），NonScalableFonts 只显示不可缩放字体（通常是老式位图字体，现代系统上很少见了），MonospacedFonts 只显示等宽字体（每个字符占用相同宽度的字体，适合代码编辑器），ProportionalFonts 只显示比例字体（字符宽度不固定的字体，绝大多数文本字体都属于此类）。

```cpp
auto *fontCombo = new QFontComboBox();

// 只显示等宽字体——适合代码编辑器的字体选择
fontCombo->setFontFilters(QFontComboBox::MonospacedFonts);

// 只显示比例字体——适合正文排版
fontCombo->setFontFilters(QFontComboBox::ProportionalFonts);

// 组合使用：等宽 + 可缩放
fontCombo->setFontFilters(
    QFontComboBox::MonospacedFonts | QFontComboBox::ScalableFonts);
```

在典型应用场景中，等宽字体过滤是最常用的。代码编辑器、终端模拟器、日志查看器这类工具通常只关心等宽字体——因为非等宽字体在代码对齐上会造成视觉混乱。如果你做的是一个富文本编辑器，那比例字体过滤更合适。如果你不确定用户需要什么类型的字体，直接用 AllFonts 就行，这是 QFontComboBox 的默认行为。

一个容易忽略的细节：setFontFilters 会在内部重新查询 QFontDatabase 并重建下拉列表。如果你在 QFontComboBox 构造完成之后调用 setFontFilters，之前用户选中的字体可能会从列表中消失——比如用户当前选了一个比例字体，你突然把过滤器切到 MonospacedFonts，那个比例字体就不在列表里了，QFontComboBox 会自动回退到列表中的第一个条目。所以如果你要动态切换过滤器，需要处理好当前选中项的变化。

还有一个实际使用中的经验：不同操作系统返回的字体列表差异很大。Windows 上你会看到大量以 "@" 开头的字体名称（这些是竖排字体），macOS 上会有很多系统 UI 专用字体，Linux 上取决于你安装了哪些字体包。如果你希望跨平台保持一致的用户体验，可能需要自己做一轮额外的过滤或者排序。

### 3.2 currentFont() 获取选中字体

QFontComboBox 最核心的 getter 是 `currentFont()`，它返回一个 QFont 对象。这比从 QComboBox 的 currentText() 拿到一个字体名称字符串再手动构造 QFont 要方便得多——QFontComboBox 内部已经帮你完成了从字体家族名称到 QFont 对象的映射，包括处理字体替换（当请求的字体不存在时，Qt 会自动选择一个接近的替代字体）。

```cpp
auto *fontCombo = new QFontComboBox();

connect(fontCombo, &QFontComboBox::currentFontChanged,
        this, [](const QFont &font) {
    qDebug() << "字体家族:" << font.family();
    qDebug() << "字体风格:" << font.styleName();
    qDebug() << "字体大小:" << font.pointSize();
});
```

注意这里用的是 `currentFontChanged` 信号而不是 QComboBox 的 `currentIndexChanged` 或 `currentTextChanged`。currentFontChanged 的参数直接就是 QFont 对象，省去了你自己转换的步骤。它的触发时机和 currentIndexChanged 一致——当用户在下拉列表中选择了一个不同的字体时触发。

QFontComboBox 还继承了大量 QComboBox 的方法，所以你可以用 `setCurrentFont(const QFont &)` 来通过代码设置当前字体——比如你从配置文件中读取了用户上次选择的字体，想在启动时恢复：

```cpp
QFont savedFont("Consolas", 10);
fontCombo->setCurrentFont(savedFont);
```

setCurrentFont 会在下拉列表中查找匹配的字体家族名称并设置为当前选中项。如果传入的字体不在列表中（比如用户上次使用的字体已经被卸载了），QFontComboBox 会保持当前的选中项不变——它不会报错，但也不会切换。这种静默失败的行为在恢复配置时需要留意，你可能需要自己验证字体是否确实切换成功了。

QFontComboBox 内部使用的 QFont 默认大小是 12 磅（point size 12），这是 Qt 的默认字体大小。currentFont() 返回的 QFont 对象的 pointSize() 也是 12——但这个大小只是 QFontComboBox 自己内部的默认值，和你实际应用中使用的字体大小完全无关。QFontComboBox 只负责选择字体家族，字体大小、粗细、斜体这些属性需要你自己在应用中单独设置。

```cpp
// 正确做法：从 QFontComboBox 拿到字体家族名称，结合你自己的大小设置
QFont selectedFont = fontCombo->currentFont();
selectedFont.setPointSize(14);  // 使用你应用自己的大小
selectedFont.setBold(true);     // 按需设置粗体
textEdit->setFont(selectedFont);
```

这一点经常被误解——有人以为 setCurrentFont 传入一个带大小的 QFont 后，QFontComboBox 会记住这个大小。实际上 QFontComboBox 只关心 family 这一个属性，其他属性在 setCurrentFont 时会被忽略，currentFont 返回时也只会包含默认的大小和样式。

### 3.3 实时预览字体变化

QFontComboBox 最典型的应用场景就是和一个文本编辑/显示控件配合——用户在下拉框里选一个字体，旁边的文本区域立刻用这个字体重新渲染内容。这个交互在现代桌面应用中非常常见，几乎所有的富文本编辑器、代码编辑器的设置页面都有这个功能。

我们来构建一个完整的"字体预览面板"：左边是 QFontComboBox 加上一些辅助控件（字体大小选择、粗体/斜体开关），右边是一个 QTextEdit 用于展示预览效果。每当用户改变任何一个设置项时，预览区域的字体实时更新。

```cpp
class FontPreviewPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FontPreviewPanel(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        auto *mainLayout = new QHBoxLayout(this);

        // ---- 左侧控制面板 ----
        auto *controlPanel = new QWidget();
        auto *controlLayout = new QVBoxLayout(controlPanel);

        // 字体选择
        auto *fontLabel = new QLabel("字体:");
        m_fontCombo = new QFontComboBox();
        m_fontCombo->setFontFilters(QFontComboBox::ProportionalFonts);

        // 字体大小
        auto *sizeLabel = new QLabel("大小:");
        m_sizeCombo = new QComboBox();
        for (int s = 8; s <= 36; s += 2) {
            m_sizeCombo->addItem(QString::number(s), s);
        }
        m_sizeCombo->setCurrentText("14");

        // 预览区域
        m_previewEdit = new QTextEdit();
        m_previewEdit->setPlainText(
            "The quick brown fox jumps over the lazy dog.\n"
            "敏捷的棕色狐狸跳过了懒狗。\n"
            "0123456789  !@#$%^&*()");

        controlLayout->addWidget(fontLabel);
        controlLayout->addWidget(m_fontCombo);
        controlLayout->addWidget(sizeLabel);
        controlLayout->addWidget(m_sizeCombo);
        controlLayout->addStretch();

        // ---- 布局 ----
        mainLayout->addWidget(controlPanel, 1);
        mainLayout->addWidget(m_previewEdit, 2);

        // ---- 信号连接 ----
        connect(m_fontCombo, &QFontComboBox::currentFontChanged,
                this, &FontPreviewPanel::updatePreviewFont);
        connect(m_sizeCombo, &QComboBox::currentIndexChanged,
                this, &FontPreviewPanel::updatePreviewFont);
    }

private:
    /// @brief 根据当前设置更新预览区域的字体
    void updatePreviewFont()
    {
        QFont font = m_fontCombo->currentFont();
        int size = m_sizeCombo->currentData().toInt();
        font.setPointSize(size);
        m_previewEdit->setFont(font);
    }

    QFontComboBox *m_fontCombo = nullptr;
    QComboBox *m_sizeCombo = nullptr;
    QTextEdit *m_previewEdit = nullptr;
};
```

这个模式的关键在于 updatePreviewFont 方法：它从 QFontComboBox 拿到字体家族，从大小下拉框拿到字号，组合成一个完整的 QFont 后设置到 QTextEdit 上。所有影响字体的控件（字体选择、大小选择、粗体/斜体开关）都连接到同一个更新方法，保证任何一个设置变化都会触发预览刷新。

有一点需要注意：QTextEdit::setFont() 会影响编辑器中所有文本的字体。如果你只想影响选中的文本或者只影响后续输入的文本，应该使用 QTextCursor 配合 QTextCharFormat 来做局部格式设置。不过在字体预览面板的场景中，通常就是要全部文本一起变，所以 setFont() 是最简单直接的选择。

还有一个实际开发中的建议：预览文本应该包含不同类型的字符——拉丁字母、CJK 字符、数字、标点符号——这样用户在选择字体时可以同时看到该字体对这些字符集的渲染效果。有些字体只包含拉丁字符，选了之后中文会回退到系统默认字体，如果预览文本只有英文就发现不了这个问题。

### 3.4 字体名称的本地化显示

QFontComboBox 在渲染每个下拉选项时有一个非常贴心的设计：它用对应的字体来渲染字体名称本身。也就是说，"Times New Roman"这个选项会用 Times New Roman 字体来显示，"微软雅黑"会用微软雅黑字体来显示。这个效果是通过 QFontComboBox 内部的自定义 delegate 实现的——它在 paint 事件中为每个选项临时切换 painter 的字体。

这个"用自身字体渲染自身名称"的效果在视觉上非常有帮助。用户不需要记住字体名称长什么样——看到选项的渲染效果就能判断这是不是自己想要的字体。这在字体数量很多的情况下特别有用，因为纯文本列表里几十个字体名称长得都差不多，视觉区分度很低。

不过这里有一个潜在的坑：有些字体的渲染效果在某些语言环境下可能看起来很奇怪。比如一些装饰性字体（Dingbats、Webdings 之类）的字符集里根本没有常规的拉丁字母，用它们来渲染字体名称本身可能显示出一堆方块或者乱码。QFontComboBox 内部对此做了一定的处理——如果某个字体无法正确渲染自己的名称，它会回退到默认字体来显示——但这个回退机制并不是 100% 可靠的。

关于字体名称的本地化，Qt 内部是通过 QFontDatabase::families() 获取字体列表的。在一些平台上，字体的本地化名称和英文名称是不同的——比如"Microsoft YaHei"在中文系统上可能显示为"微软雅黑"。QFontComboBox 显示的是 QFontDatabase 返回的名称，这个名称取决于系统语言环境。如果你需要跨语言环境保持一致的字体名称（比如存储到配置文件中），建议使用 QFont::family() 返回的名称而不是显示文本，因为 family() 通常返回的是字体的规范英文名称。

```cpp
// 存储到配置文件
QSettings settings;
settings.setValue("font/family", fontCombo->currentFont().family());

// 恢复
QString family = settings.value("font/family").toString();
fontCombo->setCurrentFont(QFont(family));
```

最后提一下 QFontComboBox 的 `writingSystem` 属性。通过 `setWritingSystem(QFontDatabase::WritingSystem)` 方法，你可以让 QFontComboBox 只显示支持特定书写系统的字体。比如设置为 QFontDatabase::SimplifiedChineseScript 后，列表中只会出现支持简体中文的字体。这在需要确保用户选择的字体能正确显示中文内容的场景下非常有用——选了不支持中文的字体，中文文本会全部回退到系统默认字体，体验很差。

```cpp
// 只显示支持简体中文的字体
fontCombo->setWritingSystem(QFontDatabase::SimplifiedChineseScript);

// 只显示支持日文的字体
fontCombo->setWritingSystem(QFontDatabase::JapaneseScript);

// 显示所有字体（默认）
fontCombo->setWritingSystem(QFontDatabase::Any);
```

## 4. 踩坑预防

第一个坑是 currentFont() 返回的 QFont 对象的 pointSize 始终是默认值（通常是 12），不反映你应用中实际使用的字体大小。QFontComboBox 只负责选择字体家族，大小、粗细、斜体这些属性需要你自己在拿到 QFont 后单独设置。如果你直接用 currentFont() 的返回值设置到文本控件上，字体大小会变成 12 磅，不管你之前设的是什么。

第二个坑是 setCurrentFont 在目标字体不在列表中时静默失败。如果你从配置文件中恢复了一个用户上次使用的字体，但那个字体已经被卸载了，setCurrentFont 不会报错但也不会切换，QFontComboBox 会保持原来的选中项。建议在 setCurrentFont 之后检查 currentFont().family() 是否和预期一致。

第三个坑是 setFontFilters 会重建列表，可能导致当前选中项丢失。比如用户当前选了一个比例字体，你切换过滤器为 MonospacedFonts 后，那个比例字体不在新列表中了，QFontComboBox 会自动选中第一个等宽字体，同时触发 currentFontChanged 信号。

第四个坑是装饰性字体的显示问题。一些符号字体（Wingdings、Webdings 等）的字符集不包含常规拉丁字母，用它们来渲染自己的字体名称可能显示异常。虽然 QFontComboBox 内部有回退机制，但在某些情况下仍然可能出现乱码或方块。

第五个坑是跨平台字体名称差异。同一个字体在不同操作系统上的名称可能不同，特别是在 macOS 和 Windows 之间。如果你需要在配置文件中存储字体选择，使用 QFont::family() 获取的名称而不是 QFontComboBox 的 currentText()，因为后者可能是本地化后的显示名称。

## 5. 练习项目

我们来做一个综合练习：创建一个"字体预览工具"窗口，覆盖 QFontComboBox 的核心用法。窗口分为左右两个区域。左侧是控制面板，包含一个 QFontComboBox（默认显示所有字体，可通过下方的 QComboBox 切换过滤器：全部/等宽/比例），一个字体大小 QComboBox（8 到 72，步进 2，默认 14），两个 QCheckBox（粗体和斜体），以及一个"书写系统"QComboBox（可选：任意、简体中文、日文、韩文、拉丁文，对应 QFontDatabase::WritingSystem 枚举）。右侧是一个 QTextEdit，预览文本包含中英文混排的示例段落。每当左侧任何一个控件发生变化时，右侧预览区域的字体实时更新。窗口底部有一个状态栏，显示当前字体的 family、styleName 和 pointSize。

几个提示：把所有控件的变化信号都连接到同一个更新槽函数，在槽函数中统一收集所有控件的当前值来构建 QFont 对象；书写系统变化时需要同时调用 QFontComboBox::setWritingSystem 重建字体列表；粗体用 QFont::setBold，斜体用 QFont::setItalic；状态栏用 QFontInfo 获取字体的实际渲染信息比 QFont 更准确，因为 QFont 记录的是请求值而 QFontInfo 记录的是系统实际匹配到的值。

## 6. 官方文档参考链接

[Qt 文档 · QFontComboBox](https://doc.qt.io/qt-6/qfontcombobox.html) -- 字体选择下拉框控件

[Qt 文档 · QFontComboBox::FontFilter](https://doc.qt.io/qt-6/qfontcombobox.html#FontFilter-enum) -- 字体类型过滤器枚举

[Qt 文档 · QFont](https://doc.qt.io/qt-6/qfont.html) -- 字体对象

[Qt 文档 · QFontDatabase](https://doc.qt.io/qt-6/qfontdatabase.html) -- 字体数据库（查询系统可用字体）

[Qt 文档 · QFontDatabase::WritingSystem](https://doc.qt.io/qt-6/qfontdatabase.html#WritingSystem-enum) -- 书写系统枚举

---

到这里，QFontComboBox 的四个核心维度我们就全部讲完了。setFontFilters 让你可以按字体类型缩小选择范围，currentFont 直接返回可用的 QFont 对象免去了手动转换，和 QTextEdit 配合的实时预览是它最典型的使用模式，而 writingSystem 属性则帮助你在需要特定语言支持的场景下过滤掉不适用的字体。QFontComboBox 本质上是一个"高度特化"的 QComboBox——它封装了字体获取、字体渲染、字体过滤这一整套逻辑，让你不需要自己处理 QFontDatabase 的细节就能提供一个专业的字体选择体验。
