---
title: "1.13 国际化进阶：复数规则与动态语言切换"
description: "入门篇我们聊了 QTranslator 的基本用法——用 tr() 包裹字符串、lupdate 生成 .ts 文件、lrelease 编译成 .qm 文件。说实话，单语言或者双语言的应用，这些确实够用了。"
---

# 现代Qt开发教程（进阶篇）1.13——国际化进阶：复数规则与动态语言切换

## 1. 前言 / i18n 不只是 tr() 包一层

入门篇我们聊了 QTranslator 的基本用法——用 tr() 包裹字符串、lupdate 生成 .ts 文件、lrelease 编译成 .qm 文件。说实话，单语言或者双语言的应用，这些确实够用了。但当你面对一个需要支持 30+ 语言、需要处理不同语言的复数规则、需要在运行时动态切换语言而不重启程序的场景时，入门知识就远远不够了。

我之前在一个跨国产品团队里踩过一个坑：俄语有三种复数形式（1 个、2-4 个、5 个以上），我们的代码只处理了「1 个」和「N 个」两种情况，俄语用户看到「2 文件已删除」的翻译语法完全错误。更棘手的是日语根本没有复数形式——一段翻译逻辑要同时适配这些完全不同的语言规则，入门篇的 %n 参数完全不够用。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。国际化相关类（QTranslator、QLocale）属于 QtCore 模块。lupdate 和 lrelease 工具随 Qt 安装提供。示例中涉及的 .ts/.qm 文件需要用 Qt Linguist 或命令行工具生成。

## 3. 核心概念讲解

### 3.1 复数规则——不同语言的复数形式远比你想象的多

Qt 的 tr() 函数支持通过 `%n` 或 `Ln` 占位符处理复数形式。关键在于 Qt 的翻译系统会根据目标语言的复数规则自动选择正确的翻译字符串。

```cpp
// 代码中：使用 %n 占位符
label->setText(tr("Deleted %n file(s)", nullptr, count));
```

在 .ts 文件中，同一个源字符串会有多个翻译条目，对应不同语言的复数形式。比如英语有两种（1 file / N files），俄语有三种，日语只有一种，阿拉伯语有六种。Qt Linguist 会根据目标语言自动显示正确数量的翻译输入框。

复数规则由 QLocale 在运行时根据目标语言的规则自动选择。你不需要在代码中判断语言——只需要在 .ts 文件中提供完整的复数翻译。

### 3.2 动态语言切换——运行时切换而不重启

动态语言切换的核心思路是：安装新的 QTranslator 后，触发所有 UI 组件重新加载翻译文本。Qt 没有内置的「重新翻译所有控件」机制，你需要自己实现。

常见的做法是定义一个 `changeEvent` 处理器，监听 `QEvent::LanguageChange` 事件。当新的翻译器被安装时，Qt 会向所有 QWidget 发送 LanguageChange 事件。

```cpp
void MyWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        // 重新设置所有翻译文本
        m_titleLabel->setText(tr("Application Title"));
        m_closeButton->setText(tr("Close"));
    }
    QWidget::changeEvent(event);
}
```

切换语言的操作序列是：移除旧翻译器 -> 安装新翻译器 -> Qt 自动发送 LanguageChange 事件 -> 各控件在 changeEvent 中更新文本。

### 3.3 QLocale——区域感知的数字和日期格式

QLocale 提供了区域感知的格式化功能——根据用户的语言和地区设置，自动选择正确的数字格式、日期格式、货币符号等。

```cpp
QLocale locale(QLocale::German, QLocale::Germany);
QString number = locale.toString(1234.56);  // "1.234,56"
QString date = locale.toString(QDate::currentDate(), QLocale::LongFormat);
```

QLocale 和 QTranslator 是独立的两套系统——QLocale 影响格式化，QTranslator 影响翻译字符串。在实践中，切换语言时通常需要同时更新两者。

### 3.4 翻译上下文与 QT_TR_NOOP

默认情况下，tr() 使用当前类的类名作为翻译上下文。但在某些场景下（比如在全局函数或枚举翻译中），你需要在非 QObject 类中使用 tr()。这时可以用 `QCoreApplication::translate("Context", "string")` 显式指定上下文。

`QT_TR_NOOP` 和 `QT_TRANSLATE_NOOP` 宏用于标记需要翻译的字符串但不立即翻译——比如在数组初始化时标记字符串，运行时再用 tr() 翻译。

## 4. 踩坑预防

第一个坑是动态切换语言后 QLabel 的文本没有更新。如果你在构造函数中用 `setText(tr("Hello"))` 设置了文本，切换语言后 QLabel 不会自动更新——你必须手动在 changeEvent 中重新 setText。后果是切换语言后部分 UI 还是旧语言。解决方案是确保所有需要翻译的控件都在 changeEvent 中更新文本，或者使用信号槽机制统一管理翻译更新。

第二个坑是 .ts 文件中缺少复数形式翻译。如果你在代码中用了 `%n` 占位符但 .ts 文件只提供了单数翻译，Qt 会回退到英文源字符串。后果是某些语言下复数显示不正确。解决方案是用 Qt Linguist 打开 .ts 文件，确保每种语言的复数形式都已填写。

第三个坑是 QTranslator::load() 的搜索路径问题。load() 需要指定 .qm 文件的完整路径或正确的搜索前缀。如果路径不对，load() 静默返回 false，程序显示未翻译的源字符串。后果是看起来「翻译没生效」但其实是文件没找到。解决方案是在 load 后检查返回值，或者用 `QCoreApplication::applicationDirPath()` 构建绝对路径。

## 5. 练习项目

练习项目：多语言控制面板。实现一个支持中英日三语动态切换的控制台信息输出程序。

具体要求是：定义一组带 tr() 的消息字符串（包含复数形式），支持运行时通过命令输入切换语言。切换后所有消息使用新语言输出，复数形式正确。完成标准是三种语言切换无遗漏、复数在不同语言下规则正确。

提示几个关键点：用 QCoreApplication::translate 指定上下文，%n 处理复数，QLocale 处理数字格式。

## 6. 官方文档参考链接

[Qt 文档 · QTranslator](https://doc.qt.io/qt-6/qtranslator.html) -- 翻译器类参考

[Qt 文档 · QLocale](https://doc.qt.io/qt-6/qlocale.html) -- 区域设置类参考

[Qt 文档 · Writing Source Code for Translation](https://doc.qt.io/qt-6/i18n-source-translation.html) -- 翻译源码编写指南

---

到这里，国际化的进阶知识就拆完了。复数规则、动态语言切换、区域感知格式化——这些知识让应用真正具备全球化的能力。下一篇我们来看日志系统进阶。
