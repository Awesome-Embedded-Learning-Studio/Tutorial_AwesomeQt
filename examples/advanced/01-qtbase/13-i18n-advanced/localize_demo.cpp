/// @file    localize_demo.cpp
/// @brief   QLocale 格式化演示函数实现。
///
/// 对应教程：进阶层 01-QtBase/13-国际化。

#include "localize_demo.h"

#include <QDebug>
#include <QDateTime>
#include <QLocale>

/// @brief 打印分隔线，让输出更清晰。
static void printSeparator()
{
    qDebug() << "  " << QString(50, '-');
}

/// @brief 展示不同区域对数字的格式化方式。
///
/// 用户期望看到符合自己区域习惯的数字表示，
/// 例如：1,234.56（英语）vs 1.234,56（德语）vs 1 234,56（法语）。
static void demoNumberFormatting()
{
    qDebug() << "  [数字格式化] 展示不同区域的数字表示方式";
    printSeparator();

    double pi = 3.14159265358979;
    qint64 largeNumber = 1234567890;

    QList<QLocale> locales = {
        QLocale(QLocale::Chinese, QLocale::China),
        QLocale(QLocale::English, QLocale::UnitedStates),
        QLocale(QLocale::German, QLocale::Germany),
        QLocale(QLocale::Japanese, QLocale::Japan),
        QLocale(QLocale::French, QLocale::France),
    };

    for (const auto& locale : locales) {
        // toString(double, char, int) 第二个参数是格式字符，第三个是精度
        // 'f' 表示定点表示法（不用科学计数法）
        qDebug() << "  " << locale.name()
                 << "| pi =" << locale.toString(pi, 'f', 4)
                 << "| large =" << locale.toString(largeNumber);
    }

    printSeparator();

    QLocale enUS(QLocale::English, QLocale::UnitedStates);
    QLocale deDE(QLocale::German, QLocale::Germany);

    qDebug() << "  [小数位数控制]";
    qDebug() << "    en_US pi(2位):" << enUS.toString(pi, 'f', 2);
    qDebug() << "    en_US pi(6位):" << enUS.toString(pi, 'f', 6);
    qDebug() << "    de_DE pi(2位):" << deDE.toString(pi, 'f', 2);
    qDebug() << "    de_DE pi(6位):" << deDE.toString(pi, 'f', 6);

    // QLocale::measurementSystem() 返回该区域默认的度量系统
    qDebug() << "  [度量系统]";
    qDebug() << "    en_US:" << (enUS.measurementSystem() == QLocale::MetricSystem
                                    ? "公制" : "英制");
    qDebug() << "    de_DE:" << (deDE.measurementSystem() == QLocale::MetricSystem
                                    ? "公制" : "英制");
}

/// @brief 展示不同区域对日期和时间的格式化方式。
///
/// 日期格式差异很大，如 MM/DD/YYYY vs DD.MM.YYYY vs YYYY-MM-DD。
static void demoDateTimeFormatting()
{
    qDebug() << "  [日期时间格式化] 展示不同区域的日期时间表示";
    printSeparator();

    QDateTime dt = QDateTime(QDate(2025, 6, 15), QTime(14, 30, 45));

    QList<QLocale> locales = {
        QLocale(QLocale::Chinese, QLocale::China),
        QLocale(QLocale::English, QLocale::UnitedStates),
        QLocale(QLocale::German, QLocale::Germany),
        QLocale(QLocale::Japanese, QLocale::Japan),
    };

    // QLocale::LongFormat   - 完整格式（包含星期、月份全名）
    // QLocale::ShortFormat  - 短格式（数字缩写）
    // QLocale::NarrowFormat - 极简格式（可能不唯一）
    for (const auto& locale : locales) {
        qDebug() << "  " << locale.name()
                 << "| 长格式:" << locale.toString(dt, QLocale::LongFormat)
                 << "| 短格式:" << locale.toString(dt, QLocale::ShortFormat);
    }

    printSeparator();

    QLocale zhCN(QLocale::Chinese, QLocale::China);
    qDebug() << "  [自定义格式 (zh_CN)]";
    qDebug() << "    日期:" << zhCN.toString(dt.date(), "yyyy年MM月dd日 dddd");
    qDebug() << "    时间:" << zhCN.toString(dt.time(), "HH:mm:ss");
    qDebug() << "    组合:" << zhCN.toString(dt, "yyyy-MM-dd HH:mm:ss");

    qDebug() << "  [默认格式字符串]";
    qDebug() << "    zh_CN 日期长格式:" << zhCN.dateTimeFormat(QLocale::LongFormat);
    qDebug() << "    zh_CN 日期短格式:" << zhCN.dateTimeFormat(QLocale::ShortFormat);
}

/// @brief 展示不同区域的货币表示方式。
///
/// 货币符号位置、小数位数、分隔符都因区域而异。
static void demoCurrencyFormatting()
{
    qDebug() << "  [货币格式化] 展示不同区域的货币表示";
    printSeparator();

    double amount = 12345.67;

    QList<QLocale> locales = {
        QLocale(QLocale::Chinese, QLocale::China),
        QLocale(QLocale::English, QLocale::UnitedStates),
        QLocale(QLocale::German, QLocale::Germany),
        QLocale(QLocale::Japanese, QLocale::Japan),
        QLocale(QLocale::English, QLocale::UnitedKingdom),
    };

    for (const auto& locale : locales) {
        // toCurrencyString 自动使用该区域的货币符号和格式
        qDebug() << "  " << locale.name()
                 << "|" << locale.toCurrencyString(amount);
    }

    printSeparator();

    // currencySymbol() 可以获取不同格式的货币符号
    QLocale zhCN(QLocale::Chinese, QLocale::China);
    QLocale enUS(QLocale::English, QLocale::UnitedStates);

    qDebug() << "  [货币符号]";
    qDebug() << "    zh_CN:" << zhCN.currencySymbol(QLocale::CurrencySymbol)
             << " (" << zhCN.currencySymbol(QLocale::CurrencyIsoCode) << ")";
    qDebug() << "    en_US:" << enUS.currencySymbol(QLocale::CurrencySymbol)
             << " (" << enUS.currencySymbol(QLocale::CurrencyIsoCode) << ")";

    // 日元通常没有小数位
    QLocale jaJP(QLocale::Japanese, QLocale::Japan);
    qDebug() << "    ja_JP:" << jaJP.toCurrencyString(amount)
             << "（注意：日元通常没有小数位）";
}

/// @brief 展示区域的其他有用信息（文本方向、语言/国家名称等）。
static void demoLocaleInfo()
{
    qDebug() << "  [区域信息] 展示 QLocale 的其他实用功能";
    printSeparator();

    QList<QLocale> locales = {
        QLocale(QLocale::Chinese, QLocale::China),
        QLocale(QLocale::English, QLocale::UnitedStates),
        QLocale(QLocale::Arabic, QLocale::SaudiArabia),
        QLocale(QLocale::Hebrew, QLocale::Israel),
    };

    for (const auto& locale : locales) {
        // textDirection() 返回该语言的文本方向
        QString dir = (locale.textDirection() == Qt::LeftToRight)
                          ? "从左到右 (LTR)" : "从右到左 (RTL)";

        qDebug() << "  " << locale.name()
                 << "| 语言:" << locale.nativeLanguageName()
                 << "| 国家:" << locale.nativeTerritoryName()
                 << "| 文本方向:" << dir;
    }

    printSeparator();

    // 系统默认区域设置
    QLocale systemLocale = QLocale::system();
    qDebug() << "  系统默认区域:" << systemLocale.name()
             << "| 语言:" << systemLocale.nativeLanguageName()
             << "| 国家:" << systemLocale.nativeTerritoryName();

    // BCP 47 格式：language[-script][-country]
    QLocale fromString("zh_Hans_CN");
    qDebug() << "  从字符串构造:" << fromString.name()
             << "| 语言:" << fromString.nativeLanguageName();
}

void runLocaleDemo()
{
    qDebug() << "\n[演示 1] QLocale 数字格式化";
    qDebug() << "========================================";
    demoNumberFormatting();

    qDebug() << "\n[演示 2] QLocale 日期时间格式化";
    qDebug() << "========================================";
    demoDateTimeFormatting();

    qDebug() << "\n[演示 3] QLocale 货币格式化";
    qDebug() << "========================================";
    demoCurrencyFormatting();

    qDebug() << "\n[演示 4] QLocale 区域信息与文本方向";
    qDebug() << "========================================";
    demoLocaleInfo();
}
