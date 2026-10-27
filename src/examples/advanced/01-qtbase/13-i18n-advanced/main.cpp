/// @file    main.cpp
/// @brief   13-i18n-advanced 示例程序入口。
///
/// 演示 Qt 国际化（i18n）系统的高级用法：
/// 1. QLocale 的数字、日期、货币格式化（不依赖 .qm 文件）
/// 2. QTranslator 的动态加载、移除、安装流程
/// 3. tr() 宏与 QT_TR_NOOP 的使用方式
/// 4. 复数形式（Plural Forms）的翻译机制
///
/// 对应教程：进阶层 01-QtBase/13-国际化。

#include <QDebug>                // 调试输出
#include <QLocale>               // 区域设置
#include <QCoreApplication>      // 核心应用类（非 GUI）

#include "dynamic_translator.h"  // 翻译器管理演示
#include "localize_demo.h"       // QLocale 格式化演示

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 设置应用程序信息（翻译系统会使用这些信息）
    QCoreApplication::setApplicationName("i18n-advanced-demo");
    QCoreApplication::setApplicationVersion("1.0");

    qDebug() << "========================================";
    qDebug() << "  Qt6 国际化（i18n）高级示例";
    qDebug() << "========================================";
    qDebug() << "  Qt 版本:" << QT_VERSION_STR;
    qDebug() << "  系统区域:" << QLocale::system().name();
    qDebug() << "";

    // 演示 1-2：QLocale 格式化（无需 .qm 文件）
    runLocaleDemo();

    // 演示 3-4：翻译 API 与动态切换
    runTranslatorDemo();

    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << "  所有演示执行完毕";
    qDebug() << "========================================";

    return 0;
}
