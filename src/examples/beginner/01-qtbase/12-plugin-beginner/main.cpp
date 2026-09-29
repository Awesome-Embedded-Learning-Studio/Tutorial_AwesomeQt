/*
 * 12-plugin-beginner 示例
 *
 * 演示 Qt 插件系统的基础用法：
 * 1. 定义插件接口（TextProcessorInterface）
 * 2. 实现插件（UpperCasePlugin、ReversePlugin）
 * 3. 使用 QPluginLoader 加载插件
 * 4. 验证接口并调用插件功能
 * 5. 自动发现并加载多个插件
 */

#include <QCoreApplication>      // 应用程序核心类
#include <QPluginLoader>        // 插件加载器
#include <QDir>                 // 目录操作
#include <QDebug>               // 调试输出
#include <QJsonObject>          // JSON 对象
#include <QJsonArray>           // JSON 数组
#include <cstdio>               // 标准输入输出
#include "textprocessor_interface.h"  // 插件接口定义

// 自定义消息处理器：确保输出立即显示
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(type)
    Q_UNUSED(context)
    QTextStream stream(stdout);
    stream << msg << Qt::endl;
    stream.flush();
}

/**
 * @brief 演示手动加载单个插件
 * @param pluginPath 插件文件的绝对路径
 * @return 加载是否成功
 */
bool loadSinglePlugin(const QString &pluginPath) {
    qDebug() << "\n========== 加载单个插件 ==========";
    qDebug() << "插件路径:" << pluginPath;

    // 创建插件加载器
    QPluginLoader loader(pluginPath);

    // 加载插件并获取根对象
    QObject *plugin = loader.instance();

    if (!plugin) {
        qDebug() << "加载失败:" << loader.errorString();
        return false;
    }

    qDebug() << "加载成功！";

    // 使用 qobject_cast 验证并转换接口
    // 这是类型安全的转换，如果对象不支持目标接口，返回 nullptr
    TextProcessorInterface *processor =
        qobject_cast<TextProcessorInterface*>(plugin);

    if (!processor) {
        qDebug() << "错误：插件不实现 TextProcessorInterface 接口";
        return false;
    }

    qDebug() << "接口验证通过";
    qDebug() << "插件名称:" << processor->name();
    qDebug() << "插件版本:" << processor->version();

    // 测试插件功能
    QString testInput = "Hello Qt Plugins!";
    QString result = processor->process(testInput);
    qDebug() << "输入:" << testInput;
    qDebug() << "输出:" << result;

    return true;
}

/**
 * @brief 演示自动发现并加载多个插件
 * @param pluginsDir 插件所在目录
 * @return 成功加载的插件数量
 */
int discoverAndLoadPlugins(const QString &pluginsDir) {
    qDebug() << "\n========== 自动发现插件 ==========";
    qDebug() << "插件目录:" << pluginsDir;

    QDir dir(pluginsDir);
    if (!dir.exists()) {
        qDebug() << "错误：插件目录不存在";
        return 0;
    }

    // 根据平台设置插件文件扩展名
    QStringList filters;
#ifdef Q_OS_WIN
    filters << "*.dll";
#elif defined(Q_OS_MAC)
    filters << "*.dylib";
#else
    filters << "*.so";
#endif

    dir.setNameFilters(filters);
    QFileInfoList pluginFiles = dir.entryInfoList(QDir::Files);

    qDebug() << "找到" << pluginFiles.size() << "个可能的插件文件";

    int loadedCount = 0;
    for (const QFileInfo &fileInfo : pluginFiles) {
        QString pluginPath = fileInfo.absoluteFilePath();
        qDebug() << "\n--- 尝试加载:" << fileInfo.fileName() << "---";

        QPluginLoader loader(pluginPath);
        QObject *plugin = loader.instance();

        if (!plugin) {
            qDebug() << "跳过（加载失败）:" << loader.errorString();
            continue;
        }

        TextProcessorInterface *processor =
            qobject_cast<TextProcessorInterface*>(plugin);

        if (processor) {
            qDebug() << "成功加载:" << processor->name()
                     << "v" << processor->version();

            // 演示每个插件的功能
            QString testInput = fileInfo.fileName();
            QString result = processor->process(testInput);
            qDebug() << "  测试:" << testInput << "->" << result;

            loadedCount++;
        } else {
            qDebug() << "跳过（接口不匹配）";
        }

        // 卸载插件（释放资源）
        loader.unload();
    }

    return loadedCount;
}

/**
 * @brief 演示插件的元数据读取
 * @param pluginPath 插件路径
 */
void showPluginMetadata(const QString &pluginPath) {
    qDebug() << "\n========== 插件元数据 ==========";
    qDebug() << "插件路径:" << pluginPath;

    QPluginLoader loader(pluginPath);

    // 读取插件的元数据（来自 Q_PLUGIN_METADATA 宏）
    QJsonObject metaData = loader.metaData();
    qDebug() << "完整元数据 JSON:" << QJsonDocument(metaData).toJson(QJsonDocument::Compact);

    // 读取 IID（接口标识符）
    QString iid = metaData["IID"].toString();
    qDebug() << "插件 IID:" << iid;

    // 读取类名
    QJsonObject classNameData = metaData["className"].toObject();
    qDebug() << "类名:" << classNameData["stringValue"].toString();
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 安装自定义消息处理器，确保输出立即显示
    qInstallMessageHandler(messageHandler);

    qDebug() << "========== Qt 插件系统基础示例 ==========";
    qDebug() << "Qt 版本:" << QT_VERSION_STR;
    qDebug() << "当前平台:" <<
#ifdef Q_OS_WIN
        "Windows";
#elif defined(Q_OS_MAC)
        "macOS";
#elif defined(Q_OS_LINUX)
        "Linux";
#else
        "Unknown";
#endif

    // 获取插件目录（相对于可执行文件）
    QString pluginsDir;
#ifdef CMAKE_BUILD
    // CMake 构建时，插件输出到 build/plugins
    pluginsDir = app.applicationDirPath() + "/plugins";
#else
    // 其他情况：使用当前目录下的 plugins
    pluginsDir = QDir::currentPath() + "/plugins";
#endif

    // ========== 示例 1: 自动发现并加载所有插件 ==========
    int loadedCount = discoverAndLoadPlugins(pluginsDir);

    if (loadedCount > 0) {
        qDebug() << "\n成功加载" << loadedCount << "个插件";
    } else {
        qDebug() << "\n警告：没有成功加载任何插件";
        qDebug() << "请确保插件文件存在于:" << pluginsDir;
    }

    // ========== 示例 2: 手动加载单个插件 ==========
    QDir dir(pluginsDir);
    QStringList filters;
#ifdef Q_OS_WIN
    filters << "*.dll";
#elif defined(Q_OS_MAC)
    filters << "*.dylib";
#else
    filters << "*.so";
#endif

    dir.setNameFilters(filters);
    QFileInfoList pluginFiles = dir.entryInfoList(QDir::Files);

    if (!pluginFiles.isEmpty()) {
        // 加载第一个插件作为演示
        QString firstPlugin = pluginFiles.first().absoluteFilePath();
        loadSinglePlugin(firstPlugin);

        // ========== 示例 3: 读取插件元数据 ==========
        showPluginMetadata(firstPlugin);
    }

    // ========== 示例 4: 演示错误处理 ==========
    qDebug() << "\n========== 错误处理演示 ==========";
    QPluginLoader badLoader("/nonexistent/path/plugin.so");
    if (!badLoader.instance()) {
        qDebug() << "预期的错误（文件不存在）:" << badLoader.errorString();
    }

    qDebug() << "\n========== 所有示例执行完毕 ==========";
    qDebug() << "\n插件系统关键点总结：";
    qDebug() << "1. 定义接口（带 IID）-> Q_DECLARE_INTERFACE";
    qDebug() << "2. 实现插件 -> Q_PLUGIN_METADATA + Q_INTERFACES";
    qDebug() << "3. 加载插件 -> QPluginLoader::instance()";
    qDebug() << "4. 验证接口 -> qobject_cast<Interface*>";
    qDebug() << "5. 使用插件 -> 调用接口方法";

    return 0;
}
