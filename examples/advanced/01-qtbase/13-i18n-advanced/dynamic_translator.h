/// @file    dynamic_translator.h
/// @brief   QTranslator 动态翻译演示声明。
///
/// 提供 TranslatorManager 类和 runTranslatorDemo() 入口，演示 QTranslator 的
/// 动态加载/移除/安装流程、tr() 宏用法、QT_TR_NOOP 标记以及复数形式翻译。
/// 由于没有实际的 .qm 文件，主要展示 API 用法和工作流程。
///
/// 对应教程：进阶层 01-QtBase/13-国际化。

#pragma once

#include <QCoreApplication>
#include <QTranslator>

#include <memory>

/// @brief 翻译管理器，封装 QTranslator 的安装/移除/加载操作。
///
/// 实际项目中需要管理多个翻译器（如 Qt 基础翻译 + 应用翻译），
/// 并在切换语言时保证正确的加载顺序和错误处理。
class TranslatorManager
{
public:
    /// @brief 构造函数。
    /// @param[in] app 应用程序指针，不拥有所有权。
    explicit TranslatorManager(QCoreApplication* app);

    /// @brief 尝试加载并安装指定语言的翻译文件。
    /// @param[in] localeName BCP 47 格式的区域名称，如 "zh_CN"。
    /// @param[in] searchPath .qm 文件的搜索目录，默认为 "i18n/"。
    /// @return 加载成功返回 true，否则返回 false。
    bool switchLanguage(const QString& localeName,
                        const QString& searchPath = "i18n/");

    /// @brief 获取当前语言的区域名称。
    /// @return 当前区域名称字符串。
    QString currentLocale() const;

    /// @brief 获取支持的语言列表。
    /// @return 区域名 -> 显示名的键值对列表。
    const QList<QPair<QString, QString>>& supportedLanguages() const;

    /// @brief 打印当前翻译器的状态信息。
    void printStatus() const;

private:
    QCoreApplication* m_app;                                  ///< 应用程序指针（不拥有所有权）
    std::unique_ptr<QTranslator> m_currentTranslator;         ///< 当前翻译器
    QString m_currentLocale;                                  ///< 当前区域名称
    QList<QPair<QString, QString>> m_supportedLanguages;      ///< 支持的语言列表
};

/// @brief 综合运行翻译 API 与动态切换演示。
void runTranslatorDemo();
