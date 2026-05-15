/// @file    dynamic_translator.cpp
/// @brief   QTranslator 动态翻译演示实现。
///
/// 对应教程：进阶层 01-QtBase/13-国际化。

#include "dynamic_translator.h"

#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QTranslator>

// ============================================================
// TranslatorManager 实现
// ============================================================

TranslatorManager::TranslatorManager(QCoreApplication* app)
    : m_app(app)
{
    // 支持的语言列表（BCP 47 格式）
    m_supportedLanguages = {
        {"zh_CN", "简体中文"},
        {"en_US", "English (US)"},
        {"ja_JP", "日本語"},
        {"de_DE", "Deutsch"},
    };
}

bool TranslatorManager::switchLanguage(const QString& localeName,
                                       const QString& searchPath)
{
    qDebug() << "  [TranslatorManager] 尝试切换语言到:" << localeName;

    // 第 1 步：移除当前已安装的翻译器
    // installTranslator 是追加式的，不移除旧的会导致多个翻译器叠加
    if (m_currentTranslator) {
        QCoreApplication::removeTranslator(m_currentTranslator.get());
        qDebug() << "  [TranslatorManager] 已移除旧翻译器";
    }

    // 第 2 步：构造 .qm 文件路径
    // 命名约定：前缀 + 区域名 + .qm，如 i18n/app_zh_CN.qm
    QString qmFile = QString("%1app_%2.qm").arg(searchPath, localeName);

    // 第 3 步：加载 .qm 文件
    if (!m_currentTranslator) {
        m_currentTranslator = std::make_unique<QTranslator>();
    }

    // load() 即使文件不存在也不会崩溃，只是返回 false
    bool loaded = m_currentTranslator->load(qmFile);
    if (!loaded) {
        // 尝试不带目录前缀的路径（兼容不同的部署结构）
        loaded = m_currentTranslator->load(
            QLocale(localeName),       // 目标区域
            "app",                     // 文件名前缀
            ".",                       // 前缀和区域名之间的分隔符
            searchPath                 // 搜索目录
        );
    }

    if (!loaded) {
        qDebug() << "  [TranslatorManager] 警告: 无法加载翻译文件:" << qmFile;
        qDebug() << "  [TranslatorManager] 将使用源语言（硬编码字符串）";
        qDebug() << "  [TranslatorManager] 提示: 使用 lupdate/lrelease 生成 .qm 文件";
        return false;
    }

    // 第 4 步：安装翻译器到应用程序
    // 后安装的翻译器优先级更高（LIFO 顺序）
    QCoreApplication::installTranslator(m_currentTranslator.get());
    m_currentLocale = localeName;

    qDebug() << "  [TranslatorManager] 翻译器已安装:" << qmFile;
    return true;
}

QString TranslatorManager::currentLocale() const
{
    return m_currentLocale;
}

const QList<QPair<QString, QString>>& TranslatorManager::supportedLanguages() const
{
    return m_supportedLanguages;
}

void TranslatorManager::printStatus() const
{
    qDebug() << "  [状态] 当前语言:" << m_currentLocale;
    qDebug() << "  [状态] 翻译器已加载:" << (m_currentTranslator != nullptr);
}

// ============================================================
// 翻译 API 演示
// ============================================================

/// @brief 演示 tr() 宏与 QT_TR_NOOP 的使用方式。
static void demoTranslationAPI()
{
    qDebug() << "  [tr() 宏用法] 演示可翻译字符串的标记方式";
    qDebug() << "  ----------------------------------------";

    // QCoreApplication::translate() 等价于 tr()，但可以在任何 C++ 代码中使用
    QString greeting = QCoreApplication::translate("Demo", "Hello, World!");
    qDebug() << "    translate(\"Demo\", \"Hello, World!\") =" << greeting;

    QString saveAction = QCoreApplication::translate("Menu", "Save");
    qDebug() << "    translate(\"Menu\", \"Save\") =" << saveAction;

    QString exitAction = QCoreApplication::translate("Menu", "Exit");
    qDebug() << "    translate(\"Menu\", \"Exit\") =" << exitAction;

    qDebug() << "";
    qDebug() << "  [QT_TR_NOOP 宏用法] 标记但不立即翻译";
    qDebug() << "  ----------------------------------------";

    // QT_TR_NOOP：只标记字符串为可翻译的，但不执行翻译
    // 用途：在数组、枚举等静态数据结构中预先标记，运行时再翻译
    qDebug() << "    QT_TR_NOOP(\"New File\") 在代码中标记";
    qDebug() << "    运行时通过 context->tr() 实际翻译";

    qDebug() << "";
    qDebug() << "  [翻译文件生成流程]";
    qDebug() << "  ----------------------------------------";
    qDebug() << "    1. 在源码中使用 tr() 或 QT_TR_NOOP() 标记字符串";
    qDebug() << "    2. 在 CMakeLists.txt 中使用 qt_add_translations() 或";
    qDebug() << "       手动运行 lupdate 提取字符串生成 .ts 文件";
    qDebug() << "    3. 使用 Qt Linguist 工具翻译 .ts 文件中的字符串";
    qDebug() << "    4. 运行 lrelease 将 .ts 编译为二进制 .qm 文件";
    qDebug() << "    5. 运行时通过 QTranslator::load() 加载 .qm 文件";
}

/// @brief 演示动态切换语言（模拟运行时切换）。
static void demoDynamicSwitching()
{
    qDebug() << "  [动态语言切换] 模拟运行时切换语言";
    qDebug() << "  ----------------------------------------";

    TranslatorManager manager(qApp);

    qDebug() << "  支持的语言:";
    for (const auto& [locale, name] : manager.supportedLanguages()) {
        qDebug() << "    " << locale << "-" << name;
    }

    qDebug() << "";

    // 模拟切换到每种语言
    // 由于没有实际的 .qm 文件，switchLanguage 会返回 false
    for (const auto& [locale, name] : manager.supportedLanguages()) {
        qDebug() << "  --- 切换到" << name << "(" << locale << ") ---";
        bool success = manager.switchLanguage(locale);
        qDebug() << "    加载结果:" << (success ? "成功" : "失败（无 .qm 文件）");
        qDebug() << "    当前 tr() 将返回源字符串（因为无翻译文件）";
        qDebug() << "";
    }

    // 展示在实际项目中如何触发 UI 更新
    qDebug() << "  [UI 更新策略]";
    qDebug() << "    实际项目中，安装新翻译器后需要：";
    qDebug() << "    1. 发射 languageChanged() 信号";
    qDebug() << "    2. 各 Widget 重新调用 tr() 获取翻译文本";
    qDebug() << "    3. 调用 QMainWindow::retranslateUi() 统一更新";
}

/// @brief 演示翻译系统中的复数处理。
///
/// 不同语言对复数的处理规则不同：
/// - 英语：1 item（单数）, 0/2/3... items（复数）—— 只区分两种
/// - 中文：不区分单复数
/// - 阿拉伯语：有 6 种不同的复数形式
/// - 俄语：有 4 种复数形式
static void demoPluralForms()
{
    qDebug() << "  [复数形式] 演示翻译系统中的复数处理";
    qDebug() << "  ----------------------------------------";

    QList<int> counts = {0, 1, 2, 5, 100};

    qDebug() << "  [模拟翻译结果（英语规则）]";
    for (int count : counts) {
        // 在实际项目中，这里使用 tr("%n file(s) found", nullptr, count)
        QString msg = QCoreApplication::translate("PluralDemo",
                                                   "%n message(s) found",
                                                   nullptr, count);
        qDebug() << "    n =" << count << "->" << msg;
    }

    qDebug() << "";
    qDebug() << "  [各语言的复数规则]";
    qDebug() << "    中文: 不区分（统一形式）";
    qDebug() << "    英语: n==1 单数, 其他 复数（2 种形式）";
    qDebug() << "    法语: n==0/1 单数, 其他 复数（2 种形式）";
    qDebug() << "    俄语: n%10==1 单数, n%10==2-4 双数, 其他 复数（4 种形式）";
    qDebug() << "    阿拉伯语: 6 种复数形式";
    qDebug() << "";
    qDebug() << "  Qt 会自动根据目标语言的复数规则选择正确的翻译条目";
    qDebug() << "  开发者只需传入 n 参数，无需手动判断语言";
}

void runTranslatorDemo()
{
    qDebug() << "\n[演示 3] tr() 宏与翻译 API 用法";
    qDebug() << "========================================";
    demoTranslationAPI();

    qDebug() << "\n[演示 4] 动态语言切换与复数形式";
    qDebug() << "========================================";
    demoDynamicSwitching();
    demoPluralForms();
}
