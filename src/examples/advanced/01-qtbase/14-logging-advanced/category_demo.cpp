/// @file    category_demo.cpp
/// @brief   分类日志演示实现。
///
/// 对应教程：进阶层 01-QtBase/14-日志。

#include "category_demo.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QLoggingCategory>

#include <QStringList>

// ========== 声明日志类别 ==========
// 命名约定：使用点分层的模块路径，如 "app.module.submodule"
// 这与 Qt 内部的类别命名保持一致（如 "qt.core.io"）

// 网络模块日志类别——网络日志通常很频繁，生产环境需要关闭以避免性能损失
Q_DECLARE_LOGGING_CATEGORY(networkLog)

// 数据库模块日志类别——调试慢查询时重要，正常运行时应关闭
Q_DECLARE_LOGGING_CATEGORY(databaseLog)

// UI 模块日志类别——帮助追踪事件处理和渲染问题
Q_DECLARE_LOGGING_CATEGORY(uiLog)

// 性能分析日志类别——只在需要分析性能时开启
Q_DECLARE_LOGGING_CATEGORY(perfLog)

// ========== 定义日志类别 ==========
// 第三个参数是默认级别，不指定时为 Debug（即所有级别都启用）

Q_LOGGING_CATEGORY(networkLog, "app.network")                            // 默认启用所有级别
Q_LOGGING_CATEGORY(databaseLog, "app.database", QtWarningMsg)            // 默认只启用 Warning+
Q_LOGGING_CATEGORY(uiLog, "app.ui")                                      // 默认启用所有级别
Q_LOGGING_CATEGORY(perfLog, "app.performance", QtWarningMsg)             // 默认只启用 Warning+

/// @brief 演示分类日志的基本用法（不同模块使用独立的日志类别）。
static void demoBasicCategoryLogging()
{
    qDebug() << "  [分类日志基础] 不同模块使用独立的日志类别";
    qDebug() << "  " << QString(46, '-');

    // 网络模块日志
    qCDebug(networkLog) << "正在连接到服务器...";
    qCInfo(networkLog) << "连接已建立 - host: example.com, port: 443";
    qCWarning(networkLog) << "连接超时，正在重试（第 1 次）";
    qCCritical(networkLog) << "无法连接到服务器 - 所有重试已用尽";

    // 数据库模块日志——由于 databaseLog 默认级别是 QtWarningMsg，
    // qCDebug 和 qCInfo 的输出默认不会显示
    qCDebug(databaseLog) << "这条 Debug 消息默认不会显示";
    qCInfo(databaseLog) << "这条 Info 消息默认也不会显示";
    qCWarning(databaseLog) << "慢查询检测 - 耗时 > 1000ms";
    qCCritical(databaseLog) << "数据库连接失败 - 连接池已耗尽";

    // UI 模块日志
    qCDebug(uiLog) << "窗口大小改变 - width: 1920, height: 1080";
    qCInfo(uiLog) << "主题切换 - 从 Light 到 Dark";

    // 性能日志——默认只显示 Warning 及以上级别
    qCDebug(perfLog) << "这条 Debug 性能日志默认不显示";
    qCWarning(perfLog) << "帧率过低 - 当前: 15 FPS, 期望: 60 FPS";
}

/// @brief 演示 setFilterRules 运行时动态控制日志输出。
///
/// 规则格式：类别名.级别=true/false，支持通配符 *。
static void demoFilterRules()
{
    qDebug() << "  [日志过滤规则] 运行时动态控制日志开关";
    qDebug() << "  " << QString(46, '-');

    // 规则 1：启用所有网络日志
    qDebug() << "  规则: app.network.debug = true";
    QLoggingCategory::setFilterRules("app.network.debug=true");
    qCDebug(networkLog) << "网络 Debug 日志已启用 - 这条消息会显示";

    // 规则 2：禁用所有数据库日志
    qDebug() << "  规则: app.database.* = false";
    QLoggingCategory::setFilterRules("app.database.*=false");
    qCWarning(databaseLog) << "这条数据库警告不会显示（已禁用）";
    qCCritical(databaseLog) << "这条数据库严重错误也不会显示（已禁用）";

    // 规则 3：使用通配符控制所有模块
    qDebug() << "  规则: *.debug = false (禁用所有 Debug)";
    QLoggingCategory::setFilterRules("*.debug=false\n*.info=true");
    qCDebug(networkLog) << "这条 Debug 消息不会显示（全局禁用）";
    qCInfo(networkLog) << "这条 Info 消息会显示（全局启用）";
    qCWarning(uiLog) << "Warning 消息不受 *.debug 规则影响";

    // 规则 4：恢复默认（清除所有规则）
    qDebug() << "  规则: 清除所有过滤规则";
    QLoggingCategory::setFilterRules(QString());
    qCDebug(networkLog) << "规则清除后，恢复默认级别";

    // 环境变量方式也可以设置规则
    qDebug() << "  [提示] 也可以通过环境变量设置规则:";
    qDebug() << "    export QT_LOGGING_RULES=\"*.debug=true;app.database.debug=false\"";
}

/// @brief 演示条件日志，避免禁用日志时的性能损失。
///
/// 即使日志被禁用，<< 运算符右侧的表达式仍会执行。
/// 使用 isDebugEnabled() 进行预检查可以跳过字符串构造。
static void demoConditionalLogging()
{
    qDebug() << "  [条件日志] 避免禁用日志时的性能损失";
    qDebug() << "  " << QString(46, '-');

    QElapsedTimer timer;

    // 错误方式：无条件构造日志消息
    timer.start();
    for (int i = 0; i < 10000; ++i) {
        QStringList items;
        items << "CPU:50%" << "MEM:200MB" << "DISK:80%";
        qCDebug(perfLog) << items.join(", ");
    }
    qint64 unconditionalNs = timer.elapsed();

    // 正确方式：先检查级别
    timer.restart();
    for (int i = 0; i < 10000; ++i) {
        if (perfLog().isDebugEnabled()) {
            QStringList items;
            items << "CPU:50%" << "MEM:200MB" << "DISK:80%";
            qCDebug(perfLog) << items.join(", ");
        }
    }
    qint64 conditionalNs = timer.elapsed();

    qDebug() << "    无条件输出耗时:" << unconditionalNs << "ms";
    qDebug() << "    条件检查耗时:" << conditionalNs << "ms";
    qDebug() << "    性能差异:" << (unconditionalNs - conditionalNs) << "ms"
             << "(条件检查更快，因为跳过了字符串构造)";
}

/// @brief 演示 Release 模式下的日志配置建议。
static void demoReleaseStrategy()
{
    qDebug() << "  [Release 模式策略] 生产环境的日志配置建议";
    qDebug() << "  " << QString(46, '-');

    // 推荐的 Release 模式规则
    qDebug() << "  推荐的 Release 规则:";
    qDebug() << "    *.debug=false         (禁用所有 Debug)";
    qDebug() << "    *.warning=true        (保留 Warning)";
    qDebug() << "    *.critical=true       (保留 Critical)";
    qDebug() << "    app.network.info=true (关键模块保留 Info)";

    qDebug() << "";
    qDebug() << "  Release 模式额外建议:";
    qDebug() << "    1. 使用自定义处理器写入日志文件";
    qDebug() << "    2. 实现日志轮转（按大小或日期分割文件）";
    qDebug() << "    3. 过滤敏感信息（密码、Token、个人数据）";
    qDebug() << "    4. 编译时定义 QT_NO_DEBUG_OUTPUT 完全移除 qDebug";
    qDebug() << "    5. 编译时定义 QT_MESSAGELOGCONTEXT 保留文件名和行号";
}

void runCategoryDemo()
{
    qDebug() << "\n[演示 2] 分类日志基本用法";
    qDebug() << "========================================";
    demoBasicCategoryLogging();

    qDebug() << "\n[演示 3] 日志过滤规则";
    qDebug() << "========================================";
    demoFilterRules();

    qDebug() << "\n[演示 4] 条件日志与 Release 策略";
    qDebug() << "========================================";
    demoConditionalLogging();
    demoReleaseStrategy();
}
