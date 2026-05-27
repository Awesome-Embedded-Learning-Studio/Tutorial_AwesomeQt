---
title: "3.65 QFontDialog 进阶"
description: "入门篇我们用 QFontDialog::getFont 让用户选择字体，拿到 QFont 后直接设置给控件的 font 属性。"
---

# 现代Qt开发教程（进阶篇）3.65——QFontDialog 进阶

## 1. 前言 / 当所有字体都挤在一个列表里

入门篇我们用 QFontDialog::getFont 让用户选择字体，拿到 QFont 后直接设置给控件的 font 属性。如果你的应用只是改改 QLabel 的字体，getFont 确实够用。但实际项目中的字体选择场景远比"弹框选字体"复杂。我曾经做过一个代码编辑器项目，系统上安装了两千多个字体，QFontDialog 打开后列表长到根本找不到想要的字体。更麻烦的是用户想要的其实是"等宽字体"或者"支持中文的字体"，而不是在两千个字体中大海捞针。另外 getFont 的预览区域太小了，只显示一行 "AaBbCc123"，根本看不出代码字体的等宽效果或者中文字体的实际渲染质量。

这篇进阶篇聚焦四个方面：getFont 的局限性分析、QFontDialog 的选项标志位和字体类型过滤、字体过滤策略（按书写系统、按等宽比例宽度等条件筛选）、以及 fontSelected 信号配合自定义预览组件实现更直观的字体选择体验。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QFontDialog 属于 QtWidgets 模块，QFontDatabase 和 QFontMetrics 属于 QtGui 模块。链接 Qt6::Widgets 即可。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 getFont 的局限——无过滤、无定制预览

QFontDialog::getFont 的签名很简单：传一个 bool 指针接收用户是否确认、初始字体、父控件、对话框标题和可选的 options。返回值是用户选择的 QFont。这个接口的问题不在返回值——而在对话框内部的展示逻辑。

第一个问题是没有字体过滤能力。系统上安装的所有字体全部列在一个 QComboBox 里，从 Arial 到各种冷门装饰字体混在一起。在中文环境下更混乱——中文字体、英文字体、日文字体、韩文字体全部混列。用户想找一个"好看的中文字体"，要在几百个条目中逐个点击预览。getFont 的 options 参数只支持 FontDialogOptions 枚举里的几个标志位，没有"只显示等宽字体"或"只显示支持中文的字体"这样的过滤选项。

第二个问题是预览区域太小。QFontDialog 内部的预览只是 QLineEdit 里显示一行固定文本。对于代码字体选择来说，你需要在预览中看到实际的代码效果——缩进对齐、字符间距、0 和 O 的区分度、1 和 l 的区分度。一行 "AaBbCc123" 看不出这些差异。

第三个问题是 getFont 是模态阻塞的，和 QColorDialog::getColor 一样——对话框打开期间用户看不到自己正在编辑的文档效果。当然对于字体选择来说，模态的问题没有颜色选择那么严重，因为字体选择通常是"选一次用很久"的操作。

### 3.2 QFontDialog 的选项标志位

QFontDialog::FontDialogOption 提供了一组有限的控制标志。MonospacedFonts 只显示等宽字体——这可能是最实用的过滤选项。如果你在选择代码编辑器字体或者终端字体，这个选项直接把几千个字体缩减到几十个。

```cpp
bool ok = false;
QFont font = QFontDialog::getFont(
    &ok, QFont("Consolas"), this,
    "选择代码字体",
    QFontDialog::MonospacedFonts);
```

ProportionalFonts 是 MonospacedFonts 的反面——只显示比例宽度字体，排除等宽字体。适合选择文档排版字体。NoButtons 移除对话框底部的 OK 和 Cancel 按钮，和 QColorDialog 中的用途一样——配合非模态 open() 使用，让字体选择器变成浮动面板。DontUseNativeDialog 强制使用 Qt 自己实现的字体对话框，行为跨平台一致。

ScalableFonts 只显示可缩放字体（TrueType、OpenType 等），排除位图字体。在现代系统上位图字体已经很少了，但在一些嵌入式 Linux 环境中仍然存在固定位图的点阵字体，用这个选项可以过滤掉它们。

这些选项可以通过按位或组合使用。比如选择代码字体的推荐组合是 MonospacedFonts | ScalableFonts——只显示可缩放的等宽字体。

```cpp
auto *dlg = new QFontDialog(initial_font, this);
dlg->setOption(QFontDialog::MonospacedFonts);
dlg->setOption(QFontDialog::ScalableFonts);
dlg->setOption(QFontDialog::DontUseNativeDialog);
```

但要注意一个跨平台陷阱：MonospacedFonts 和 ProportionalFonts 在原生对话框上可能不生效。Windows 的原生字体对话框有自己的分类逻辑，这些 Qt 选项会被静默忽略。所以如果依赖字体类型过滤，务必同时设置 DontUseNativeDialog。

### 3.3 字体过滤策略

当 QFontDialog 内置的过滤选项不够用时，我们需要自己实现字体过滤。QFontDatabase 是字体信息的查询中心，提供了按书写系统、样式、大小等条件枚举字体的能力。

按书写系统过滤是最常见的需求。QFontDatabase::families(QFontDatabase::WritingSystem) 返回支持指定书写系统的字体族列表。比如 QFontDatabase::SimplifiedChinese 返回所有支持简体中文的字体族。

```cpp
QFontDatabase db;
QStringList chinese_fonts = db.families(QFontDatabase::SimplifiedChinese);
// chinese_fonts 包含所有支持简体中文的字体族名
```

按等宽属性过滤需要自己实现。QFont 没有直接的 isMonospaced() 方法，但 QFontMetrics 提供了间接判断方式——测量不同字符的宽度，如果所有字符宽度一致就是等宽字体。

```cpp
bool isMonospaced(const QFont &font)
{
    QFontMetrics fm(font);
    // 等宽字体的所有可打印 ASCII 字符宽度相同
    const int width_a = fm.horizontalAdvance('a');
    const int width_W = fm.horizontalAdvance('W');
    const int width_0 = fm.horizontalAdvance('0');
    const int width_i = fm.horizontalAdvance('i');
    return (width_a == width_W) && (width_a == width_0) && (width_a == width_i);
}
```

更高效的方式是检查 QFont::fixedPitch() 属性。fixedPitch() 返回 true 表示字体声明自己为固定宽度。这个值来自字体文件的元数据，比逐字符测量快得多。但 fixedPitch() 依赖字体自身的声明，极少数字体可能声明不准确。

```cpp
QStringList getMonospacedFamilies()
{
    QFontDatabase db;
    QStringList result;
    for (const auto &family : db.families()) {
        QFont font(family);
        if (font.fixedPitch()) {
            result << family;
        }
    }
    return result;
}
```

有了过滤后的字体列表，如何把它注入到 QFontDialog 中？遗憾的是 QFontDialog 没有提供 setFamilies 这样的 API。如果你需要完全自定义的字体列表，只能自己构建字体选择对话框——一个 QComboBox 显示过滤后的字体族名，一个 QComboBox 显示样式（Regular/Bold/Italic 等），一个 QSpinBox 选择大小，加上自定义预览区域。QFontDatabase 提供了所有需要的数据（families()、styles()、pointSizes()），构建起来并不复杂。

### 3.4 fontSelected 信号与自定义预览组件

QFontDialog 提供两个字体相关的信号：currentFontChanged(const QFont &font) 在用户浏览字体时实时触发，fontSelected(const QFont &font) 在用户点确定后触发。如果需要实时预览，用 currentFontChanged；如果只需要最终选择，用 fontSelected。

```cpp
auto *dlg = new QFontDialog(initial_font, this);
dlg->setOption(QFontDialog::DontUseNativeDialog);
dlg->setOption(QFontDialog::MonospacedFonts);

connect(dlg, &QFontDialog::currentFontChanged,
        this, &CodeEditor::onFontPreview);

dlg->open();
```

自定义预览组件是字体选择体验的关键。一个好的字体预览不应该只显示一行固定文本，而应该展示目标场景的实际效果。比如代码编辑器的字体预览应该显示一段真实代码，文档排版字体预览应该显示一段带标题和正文的文本。

```cpp
class FontPreviewWidget : public QWidget
{
public:
    explicit FontPreviewWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        m_layout = new QVBoxLayout(this);
        // 标题标签
        m_title = new QLabel("The Quick Brown Fox");
        m_layout->addWidget(m_title);
        // 代码预览标签
        m_code_preview = new QLabel;
        m_code_preview->setText(
            "int main()\n"
            "{\n"
            "    return 0;\n"
            "}");
        m_code_preview->setWordWrap(false);
        m_layout->addWidget(m_code_preview);
        // 尺寸信息
        m_metrics_label = new QLabel;
        m_layout->addWidget(m_metrics_label);
    }

    void setPreviewFont(const QFont &font)
    {
        m_title->setFont(font);
        m_code_preview->setFont(font);

        QFontMetrics fm(font);
        m_metrics_label->setText(
            QString("Ascent: %1  Descent: %2  "
                    "LineSpacing: %3  AvgWidth: %4")
                .arg(fm.ascent())
                .arg(fm.descent())
                .arg(fm.lineSpacing())
                .arg(fm.averageCharWidth()));
    }

private:
    QVBoxLayout* m_layout;
    QLabel* m_title;
    QLabel* m_code_preview;
    QLabel* m_metrics_label;
};
```

预览组件中显示 QFontMetrics 的信息（ascent、descent、lineSpacing、averageCharWidth）非常有用——这些数值直接影响文本在界面中的实际布局。两个字体看起来差不多，但 lineSpacing 差 4 个像素，在紧凑的 UI 中就会导致换行差异。把这些信息直观地展示出来，用户选择字体时更有依据。

现在有一道调试题给大家。看下面这段代码：

```cpp
bool ok = false;
QFont font = QFontDialog::getFont(
    &ok, m_editor->font(), this,
    "选择字体",
    QFontDialog::MonospacedFonts);
if (ok) {
    m_editor->setFont(font);
}
```

在 Windows 上运行后，发现列表中出现了大量非等宽字体（比如 Arial、Times New Roman）。问题出在哪里？MonospacedFonts 选项在 Windows 原生字体对话框上被静默忽略了——Windows 系统的字体选择对话框不支持"只显示等宽字体"这种过滤。解决方案是加上 DontUseNativeDialog 选项，强制使用 Qt 自己的字体对话框实现。

## 4. 踩坑预防

第一个坑是 MonospacedFonts 等过滤选项在原生对话框上不生效。前面调试题已经分析过了。Windows 和 macOS 的原生字体选择器不支持字体类型过滤，QFontDialog 会把 MonospacedFonts 选项传给原生对话框但被忽略。后果是你以为只显示了等宽字体，实际上系统上所有字体都在列表里，用户选到了一个比例宽度字体。解决方案是：使用任何过滤选项时，务必同时设置 DontUseNativeDialog。

第二个坑是 QFontDialog::getFont 在用户取消时返回的字体。getFont 的返回值是一个 QFont——无论用户点是确认还是取消，它都返回一个合法的 QFont 对象。唯一的区分方式是那个 bool* 参数。如果你忘了检查 bool 值，或者把 bool 指针传了 nullptr，就无法知道用户到底确认了还是取消了。后果是用户点取消后字体仍然被应用。解决方案是始终检查 bool 返回值，且不要传 nullptr。

第三个坑是字体族名在不同系统上不一致。同一个字体在 Windows 上叫 "Microsoft YaHei"，在 Linux 上可能叫 "Noto Sans CJK SC"，在 macOS 上叫 "PingFang SC"。如果你在 QFontDialog 中选了一个字体，把 fontFamily 保存到配置文件，拿到另一台机器上 QFont(family_name) 可能找不到对应字体，会静默回退到默认字体。后果是配置看起来没问题但字体显示效果完全不同。解决方案是保存配置时同时记录字体的 styleHint 和 styleName 作为备选匹配依据，或者使用 QFontDatabase::addApplicationFont 嵌入关键字体。

第四个坑是 currentFontChanged 信号的高频触发。用户拖动字体大小 spinbox 或者按住键盘上下键切换字体时，currentFontChanged 会高频触发（每秒几十次）。如果你的预览更新逻辑比较重（比如重新排版整篇文档），直接在槽函数中做会很卡。解决方案是用 QTimer 做防抖——收到信号后启动一个 50ms 的 singleShot timer，连续信号只让最后一次真正触发更新。

## 5. 练习项目

练习项目：代码编辑器字体选择器。我们不使用 QFontDialog，而是自己构建一个带过滤和实时预览的字体选择面板。

完成标准是：左侧是一个经过等宽过滤的字体族列表（QListWidget），右侧上方是字体样式选择（Regular/Bold/Italic/Bold Italic），右侧中间是字号选择（QSpinBox，范围 8-72），右侧下方是自定义预览区域显示一段真实 Python 代码并使用 QTextEdit 设置只读模式渲染。预览区域实时跟随字体族、样式、字号的变化更新。底部显示当前字体的 QFontMetrics 信息（ascent、descent、lineSpacing、averageCharWidth）。过滤逻辑使用 QFont::fixedPitch() 筛选等宽字体。

提示几个关键点：QFontDatabase::families() 获取全部字体族，用 fixedPitch() 过滤；QFontDatabase::styles(family) 获取指定字体族的可用样式；QFontDatabase::pointSizes(family, style) 获取可用字号列表，或者直接让用户输入任意大小；预览区域的 QTextEdit 通过 setCurrentFont 设置字体；QFontMetrics 的数值信息帮助用户对比不同字体的实际渲染差异。

## 6. 官方文档参考链接

[Qt 文档 · QFontDialog](https://doc.qt.io/qt-6/qfontdialog.html) -- 字体对话框类，getFont/setOption/currentFontChanged/fontSelected 信号

[Qt 文档 · QFont](https://doc.qt.io/qt-6/qfont.html) -- 字体类，family/style/pointSize/fixedPitch 属性

[Qt 文档 · QFontDatabase](https://doc.qt.io/qt-6/qfontdatabase.html) -- 字体数据库，families/styles/pointSizes/writingSystem 查询

[Qt 文档 · QFontMetrics](https://doc.qt.io/qt-6/qfontmetrics.html) -- 字体度量，ascent/descent/lineSpacing/averageCharWidth/horizontalAdvance

[Qt 文档 · QFontDialog::FontDialogOption](https://doc.qt.io/qt-6/qfontdialog.html#FontDialogOption-enum) -- 选项标志位，MonospacedFonts/ProportionalFonts/NoButtons/DontUseNativeDialog

---

到这里，QFontDialog 的进阶用法就过了一遍。getFont 简单好用但没有过滤能力，在字体数量庞大的系统上体验很差。选项标志位中的 MonospacedFonts 是最常用的过滤，但必须配合 DontUseNativeDialog 才能跨平台可靠生效。自己用 QFontDatabase 构建过滤逻辑可以实现更精细的字体筛选——按书写系统、按等宽属性、按自定义规则。currentFontChanged 信号配合自定义预览组件，让用户在真实场景文本中评估字体效果，比 QFontDialog 自带的那一行 "AaBbCc123" 有用得多。字体选择器做到这个程度，用户才不用在几千个字体中大海捞针了。
