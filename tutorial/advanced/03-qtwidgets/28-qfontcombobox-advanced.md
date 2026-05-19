---
title: "3.28 QFontComboBox 进阶"
description: "入门篇我们把 QFontComboBox 当作一个封装好的字体选择器来用——setCurrentFont、currentFontChanged、setFontFilters，该调的 API 调完了就能干活。"
---

# 现代Qt开发教程（进阶篇）3.28——QFontComboBox 进阶

## 1. 前言 / 为什么需要深入字体枚举机制

入门篇我们把 QFontComboBox 当作一个封装好的字体选择器来用——setCurrentFont、currentFontChanged、setFontFilters，调完就能干活。说实话，如果你的场景只是"让用户选个字体然后设到 QTextEdit 上"，入门篇那点东西确实够了。但如果你做过稍微专业一点的排版工具或者代码编辑器的设置页面，你大概率会遇到这些进阶需求：用户只关心支持中文的字体，列表里那堆装饰性字体和系统 UI 字体看着就烦；下拉展开后几百个字体名称密密麻麻，用户根本分不清哪个长什么样，你需要在下拉列表中直接用对应字体渲染一个预览文本；有些场景下你只想展示等宽的可缩放字体同时还要排除掉不支持 CJK 的条目——单一的 setFontFilters 不够用了。

本篇我们要搞清楚三件事：setFontFilters 背后的字体枚举过滤机制到底怎么工作，怎样通过自定义 delegate 在下拉列表中实现真正的字体预览，以及 QFontDatabase::writingSystems 如何帮你按书写系统做更精细的字体筛选。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。依赖 QtWidgets 模块（QFontComboBox、QStyledItemDelegate）和 QtGui 模块（QFontDatabase、QFontMetrics）。所有内容不需要额外链接，Qt6::Widgets 已经包含了这两个模块。

## 3. 核心概念讲解

### 3.1 setFontFilters 背后的枚举过滤机制

入门篇我们知道 setFontFilters 可以传入 MonospacedFonts、ProportionalFonts、ScalableFonts 等枚举值来过滤字体列表。现在我们要搞清楚这些过滤器在底层是怎么实现的。

QFontComboBox 内部维护了一个私有模型，这个模型在构建时会遍历 QFontDatabase::families() 返回的所有字体家族，然后对每一个家族做一轮条件检查。条件检查的核心逻辑涉及 QFontDatabase 的三个查询方法：isScalable() 判断字体是否为矢量字体，isFixedPitch() 判断字体是否等宽，以及 styleString() 获取字体的风格描述。setFontFilters 设置的枚举值就是在这一轮条件检查中作为筛选条件使用的。

这里面有一个容易被忽略的细节：MonospacedFonts 和 ProportionalFonts 互斥吗？答案是不互斥。如果你同时传入 MonospacedFonts | ProportionalFonts，等价于 AllFonts——所有字体会被包含进来，因为每个字体要么等宽要么比例，二者的并集就是全集。这意味着如果你想做"只显示等宽字体"的过滤，单独传 MonospacedFonts 就够了，不需要加 ProportionalFonts。

还有一个关键点：ScalableFonts 和 NonScalableFonts 的分界线。在现代操作系统上，几乎所有人安装的字体都是矢量字体（TrueType、OpenType），属于 ScalableFonts 的范畴。NonScalableFonts 基本只出现在老式位图字体上，现代系统中极少见到。所以如果你传入 ScalableFonts 做过滤，效果和 AllFonts 几乎没区别。真正有过滤意义的组合是 MonospacedFonts 和 ProportionalFonts——它们能显著缩小列表范围。

setFontFilters 的一个重要行为是：每次调用都会触发内部模型的完整重建。QFontComboBox 会重新查询 QFontDatabase、重新应用过滤条件、重新填充列表。如果你在构造 QFontComboBox 之后就调用了 setFontFilters，之前用户选中的字体可能会因为不在新的过滤结果中而丢失——QFontComboBox 会回退到列表的第一个条目并触发 currentFontChanged。

### 3.2 自定义预览 Delegate 实现

QFontComboBox 默认的渲染行为是用对应字体来显示字体名称本身——"Arial"用 Arial 显示，"宋体"用宋体显示。这在入门篇已经提到过了。但这个默认渲染有一个明显的局限：它只渲染字体名称，不渲染示例文本。如果你想让下拉列表的每一项不仅显示字体名称，还在右侧用该字体渲染一段预览文本（比如 "AaBbCc 123"），你就需要替换 QFontComboBox 的 item delegate。

替换 delegate 的方式是调用 QComboBox::setItemDelegate，但这里有一个坑：QFontComboBox 内部已经在构造时设置了自己的 delegate 来实现"用对应字体渲染字体名称"的效果。如果你直接用 setItemDelegate 传入一个普通的 QStyledItemDelegate，你会丢失默认的字体渲染效果——所有条目都会回退到用系统默认字体显示。所以你的自定义 delegate 必须自己处理字体渲染逻辑。

下面是自定义预览 delegate 的核心思路。我们需要在 paint 方法中根据当前条目的字体家族名称构造一个 QFont，然后用这个 QFont 来渲染预览文本。

```cpp
class FontPreviewDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        // 先让默认实现绘制背景和选中高亮
        QStyledItemDelegate::paint(painter, opt, index);

        // 获取字体家族名称
        QString family = index.data(Qt::DisplayRole).toString();
        QFont previewFont(family);

        // 在右侧区域绘制预览文本
        QRect textRect = opt.rect.adjusted(
            opt.rect.width() / 2, 2, -4, -2);
        painter->save();
        painter->setFont(previewFont);
        painter->setPen(opt.palette.text().color());
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignRight,
                          "AaBbCc 123");
        painter->restore();
    }
};
```

使用方式很简单：

```cpp
auto* fontCombo = new QFontComboBox();
fontCombo->setItemDelegate(new FontPreviewDelegate(fontCombo));
```

这里有一个性能方面的考量。当字体列表中有几百个条目时，每次下拉展开都会对所有可见条目调用 paint 方法。每个 paint 调用都会构造一个 QFont 对象并用它来渲染文本。QFont 的构造本身开销不大（它只是一个 font family 的描述符），但 QFontMetrics 的计算和文本布局是有开销的。如果你的预览文本很长或者渲染逻辑更复杂，在低端设备上可能会感觉到下拉展开时有轻微的延迟。如果性能确实成问题，可以考虑在 delegate 中维护一个 QFont 到 QFontMetrics 的缓存，避免重复计算。

现在有一道调试题给大家。你用上面的 FontPreviewDelegate 替换了 delegate，但发现下拉列表中所有条目的字体名称都变成了系统默认字体显示，只有右侧的预览文本正确使用了对应字体。问题出在哪里？

原因在于 QStyledItemDelegate::paint 的默认实现会使用 QComboBox 的 font 属性来渲染 display text。我们把 paint 调用委托给了基类，基类用的是 QComboBox 自身的 font 而不是每个条目对应的 font。要修复这个问题，你需要在调用基类 paint 之前临时把 opt.font 替换为对应字体的 font。

### 3.3 Writing System 过滤与多语言字体筛选

setFontFilters 解决的是"字体类型"层面的过滤（等宽/比例/可缩放），但它无法按"语言支持"来过滤。如果你的应用需要用户选择一个能正确显示中文的字体，setFontFilters 做不到这一点——它只能区分等宽和比例，不能区分"支持 CJK"和"不支持 CJK"。

这时候 QFontComboBox::setWritingSystem 就派上用场了。入门篇我们简单提过这个方法，现在我们深入看看它的实现机制。

setWritingSystem 接受一个 QFontDatabase::WritingSystem 枚举值。QFontDatabase 内部为每个字体家族维护了一个支持的书写系统列表。当你调用 setWritingSystem(QFontDatabase::SimplifiedChineseScript) 后，QFontComboBox 在构建内部模型时会查询每个字体家族是否支持简体中文书写系统——不支持的字体直接被排除在列表之外。

```cpp
auto* fontCombo = new QFontComboBox();
// 只显示支持简体中文的字体
fontCombo->setWritingSystem(QFontDatabase::SimplifiedChineseScript);
```

QFontDatabase::writingSystems() 静态方法返回当前系统支持的所有书写系统的列表。如果你需要构建一个"书写系统选择器"让用户先选语言再选字体，可以这样做：

```cpp
auto* wsCombo = new QComboBox();
for (auto ws : QFontDatabase::writingSystems()) {
    wsCombo->addItem(QFontDatabase::writingSystemName(ws),
                     static_cast<int>(ws));
}
connect(wsCombo, &QComboBox::currentIndexChanged,
        this, [this, wsCombo, fontCombo]() {
    auto ws = static_cast<QFontDatabase::WritingSystem>(
        wsCombo->currentData().toInt());
    fontCombo->setWritingSystem(ws);
});
```

这里有一个非常重要的行为细节：setWritingSystem 和 setFontFilters 是独立生效的。也就是说，如果你同时调用了 setFontFilters(MonospacedFonts) 和 setWritingSystem(SimplifiedChineseScript)，最终的列表是两个条件的交集——只显示同时满足"等宽"和"支持简体中文"的字体。这两个过滤条件不会互相覆盖。

但要注意：setWritingSystem 的过滤不是即时生效的。它会在下一次内部模型重建时才被应用。所谓"模型重建"，包括 setFontFilters 的调用、setWritingSystem 自身的调用，以及某些平台上的字体变更事件。如果你在 setWritingSystem 之前就获取了 model() 的 rowCount()，你拿到的还是过滤前的数量。

还有一个实用技巧：QFontDatabase::families(QFontDatabase::WritingSystem) 这个重载版本可以直接返回支持指定书写系统的字体家族列表，不需要通过 QFontComboBox。如果你要在非 QFontComboBox 的场景下做同样的过滤，可以直接用这个 API。

```cpp
// 直接查询支持日文的字体家族
QStringList japaneseFonts =
    QFontDatabase::families(QFontDatabase::JapaneseScript);
```

## 4. 踩坑预防

第一个坑是 setFontFilters 的 MonospacedFonts 判断依据是 QFontDatabase::isFixedPitch()，但某些字体在系统报告上标为 fixedPitch 而实际上并不是严格的等宽字体（比如某些 CJK 字体在拉丁字符上不是严格等宽）。如果你的场景要求"真正的等宽"，不能完全信任 isFixedPitch 的结果，可能需要自己用 QFontMetrics 逐字符验证宽度。

第二个坑是自定义预览 delegate 在字体数量很多时的性能问题。当列表中有 300-500 个字体时，每次下拉展开会触发大量 paint 调用。如果每个 paint 中都做 QFontMetrics 计算和复杂文本布局，在低端设备上可能出现明显的卡顿。解决方案是在 delegate 中缓存 QFontMetrics，或者限制预览文本的长度。

第三个坑是 setWritingSystem 的过滤在模型重建前不生效。如果你先调了 setWritingSystem，紧接着就通过 model()->rowCount() 来判断过滤后的数量，你拿到的可能是过滤前的值。如果需要立即生效，可以在 setWritingSystem 之后手动触发一次模型刷新——比如临时切换一下 setFontFilters 再切回来，强制内部重建模型。

## 5. 练习项目

练习项目：多语言字体选择面板。我们要实现一个字体选择界面，上方有一个"书写系统"QComboBox（从 QFontDatabase::writingSystems() 动态获取），下方有一个 QFontComboBox 和一个 QTextEdit 预览区域。QFontComboBox 根据选中的书写系统过滤字体列表。QFontComboBox 的下拉列表使用自定义 delegate，在每个条目右侧用对应字体渲染一行预览文本，左侧用粗体显示字体名称。预览区域的文本包含选中书写系统的示例文字（比如简体中文用"你好世界"，日文用"こんにちは"）。

完成标准是切换书写系统后字体列表正确过滤、delegate 预览文本正确使用对应字体渲染、预览区域实时更新，并且在有 300+ 字体的系统上下拉展开不卡顿。提示几个关键点：delegate 的 paint 中用 opt.font 设为对应字体的 family 来渲染字体名称，预览文本用单独的 QFont 渲染；书写系统到示例文本的映射可以用 QHash 预先构建；性能优化可以在 delegate 中用 static QHash 缓存已构造的 QFont 对象。

## 6. 官方文档参考链接

[Qt 文档 · QFontComboBox](https://doc.qt.io/qt-6/qfontcombobox.html) -- 字体选择下拉框控件，包含 fontFilters 和 writingSystem 属性

[Qt 文档 · QFontComboBox::FontFilter](https://doc.qt.io/qt-6/qfontcombobox.html#FontFilter-enum) -- 字体类型过滤器枚举定义

[Qt 文档 · QFontDatabase](https://doc.qt.io/qt-6/qfontdatabase.html) -- 字体数据库，提供 families / isFixedPitch / writingSystems 等查询接口

[Qt 文档 · QFontDatabase::WritingSystem](https://doc.qt.io/qt-6/qfontdatabase.html#WritingSystem-enum) -- 书写系统枚举，包含 SimplifiedChineseScript 等值

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 自定义 delegate 基类，用于重写下拉列表的渲染逻辑

---

到这里，QFontComboBox 的进阶内容就全部讲完了。setFontFilters 的过滤机制搞清楚后，你就知道为什么 MonospacedFonts | ProportionalFonts 等于不过滤。自定义 delegate 让你可以突破默认渲染的限制，在下拉列表中提供真正的字体预览效果。setWritingSystem 则填补了 setFontFilters 在语言支持维度上的空白——二者组合使用可以实现精确的字体筛选。这些能力组合起来，足以构建一个专业级的字体选择体验。
