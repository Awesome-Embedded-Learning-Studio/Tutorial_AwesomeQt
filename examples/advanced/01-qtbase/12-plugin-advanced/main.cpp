/// @file    main.cpp
/// @brief   程序入口，演示 Qt 插件系统的高级概念。
///
/// @details 演示内容包括：
///          1. 插件接口定义与版本兼容性检查
///          2. QPluginLoader::load()/unload() 热加载与卸载
///          3. QPluginLoader::metaData() 读取 JSON 元数据
///          4. QDir 迭代插件目录自动发现
///          5. errorString() 诊断加载失败
///
///          注意：由于本示例是单个可执行文件（不生成独立的 .so/.dll），
///          这里通过内联插件实例和模拟场景来演示概念。

#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QPluginLoader>
#include <QTimer>
#include <QCoreApplication>

#include "plugin_interface.h"
#include "plugin_manager.h"
#include "sample_plugin.h"

/// @brief [演示 1] 插件接口与版本兼容性检查。
///
/// @details 演示通过 IPlugin 接口使用插件实例，以及版本兼容性检查。
///          在真实场景中，插件实例来自 QPluginLoader::instance()，
///          这里直接使用内联的 SamplePlugin 来演示接口交互。
static void demo1_interfaceAndVersion()
{
    qDebug() << "\n[演示 1] 插件接口与版本兼容性检查";

    // 创建内联插件实例（真实场景中由 QPluginLoader 加载）
    SamplePlugin plugin;

    // 通过 IPlugin 接口指针访问插件
    IPlugin* interface = &plugin;

    qDebug() << "[接口] 插件名称:" << interface->name();
    qDebug() << "[接口] 插件描述:" << interface->description();
    qDebug() << "[接口] 插件版本:" << interface->version();

    // 版本兼容性检查：主程序要求的最低版本
    int requiredVersion = 1;
    if (interface->version() >= requiredVersion) {
        qDebug() << "[接口] 版本兼容（插件 v" << interface->version()
                 << ">= 要求 v" << requiredVersion << "）";

        // 初始化并执行插件功能
        if (interface->initialize()) {
            QString result = interface->execute("Hello Plugin System");
            qDebug() << "[接口] 执行结果:" << result;
            interface->shutdown();
        }
    } else {
        qDebug() << "[接口] 版本不兼容，拒绝加载";
    }

    // 演示版本不兼容的场景
    qDebug() << "\n[接口] 模拟版本不兼容检查（要求 v99）";
    int highVersion = 99;
    if (interface->version() >= highVersion) {
        qDebug() << "[接口] 版本兼容（不会执行到这里）";
    } else {
        qDebug() << "[接口] 版本不兼容：插件 v" << interface->version()
                 << "< 要求 v" << highVersion << "，拒绝加载";
    }
}

/// @brief [演示 2] QPluginLoader::load()/unload() 热加载与卸载。
///
/// @details 演示 QPluginLoader 的基本 API。由于本示例不生成独立的 .so 文件，
///          演示加载不存在的插件时的错误处理，以及 load/unload 的概念。
static void demo2_loadUnload()
{
    qDebug() << "\n[演示 2] QPluginLoader load/unload 热加载与卸载";

    // 尝试加载一个不存在的插件文件
    qDebug() << "\n[热加载] 测试加载不存在的插件文件";
    {
        QPluginLoader loader("nonexistent_plugin.so");

        // load() 尝试加载共享库
        bool loaded = loader.load();
        qDebug() << "[热加载] load() 返回:" << loaded;

        if (!loaded) {
            // errorString() 提供详细的错误描述
            qDebug() << "[热加载] 错误信息:" << loader.errorString();
            qDebug() << "[热加载] 这是预期的行为（文件不存在）";
        }

        // unload() 尝试卸载已加载的库
        bool unloaded = loader.unload();
        qDebug() << "[热加载] unload() 返回:" << unloaded;
    }

    // 演示 PluginManager 的加载流程
    qDebug() << "\n[热加载] 使用 PluginManager 管理插件生命周期";
    {
        PluginManager manager;
        manager.setRequiredVersion(1);

        // 尝试加载不存在的文件
        bool result = manager.loadPlugin("fake_plugin.so");
        qDebug() << "[热加载] 加载不存在的文件:"
                 << (result ? "成功" : "失败（预期）");

        // 加载成功时可以执行操作
        qDebug() << "[热加载] 已加载插件数量:" << manager.pluginCount();
    }

    // 演示热加载概念
    qDebug() << "\n[热加载] 热加载概念说明";
    qDebug() << "[热加载] 在真实项目中，插件编译为独立 .so/.dll：";
    qDebug() << "[热加载]   1. QPluginLoader loader(\"myplugin.so\");";
    qDebug() << "[热加载]   2. loader.load();          // 加载到内存";
    qDebug() << "[热加载]   3. auto obj = loader.instance();  // 获取实例";
    qDebug() << "[热加载]   4. auto plugin = qobject_cast<IPlugin*>(obj);";
    qDebug() << "[热加载]   5. plugin->execute(\"...\"); // 使用插件";
    qDebug() << "[热加载]   6. loader.unload();        // 卸载（热替换时先卸载旧版）";
}

/// @brief [演示 3] QPluginLoader::metaData() 读取 JSON 元数据。
///
/// @details 每个 Qt 插件都可以通过 Q_PLUGIN_METADATA 的 FILE 参数
///          关联一个 JSON 文件。QPluginLoader::metaData() 可以在不加载
///          插件的情况下读取这些元数据，非常适合快速扫描插件目录。
static void demo3_metadata()
{
    qDebug() << "\n[演示 3] QPluginLoader::metaData() 读取 JSON 元数据";

    // 尝试读取不存在文件的元数据
    {
        QPluginLoader loader("nonexistent_plugin.so");
        QJsonObject metaData = loader.metaData();

        if (metaData.isEmpty()) {
            qDebug() << "[元数据] 无法读取元数据（文件不存在）";
            qDebug() << "[元数据] 错误:" << loader.errorString();
        }
    }

    // 展示元数据结构说明
    qDebug() << "\n[元数据] Qt 插件元数据结构说明";
    qDebug() << "[元数据] 一个典型的插件元数据 JSON：";
    qDebug() << "[元数据] {";
    qDebug() << "[元数据]   \"IID\": \"org.awesomeqt.tutorial.IPlugin/1.0\",";
    qDebug() << "[元数据]   \"MetaData\": {";
    qDebug() << "[元数据]     \"name\": \"MyPlugin\",";
    qDebug() << "[元数据]     \"version\": 1";
    qDebug() << "[元数据]   },";
    qDebug() << "[元数据]   \"className\": \"MyPluginClass\",";
    qDebug() << "[元数据]   \"debug\": false";
    qDebug() << "[元数据] }";

    qDebug() << "\n[元数据] 元数据读取流程（无需加载插件）：";
    qDebug() << "[元数据]   QPluginLoader loader(\"plugin.so\");";
    qDebug() << "[元数据]   QJsonObject meta = loader.metaData();";
    qDebug() << "[元数据]   QString iid = meta[\"IID\"].toString();";
    qDebug() << "[元数据]   QJsonObject customMeta = meta[\"MetaData\"].toObject();";

    // 使用 PluginManager 读取元数据的模式
    PluginManager manager;
    QJsonObject metaData = manager.readPluginMetaData("fake.so");
    qDebug() << "\n[元数据] PluginManager 读取结果:"
             << (metaData.isEmpty() ? "空（预期）" : "有数据");
}

/// @brief [演示 4] QDir 迭代插件目录自动发现 + errorString() 错误诊断。
///
/// @details 演示使用 QDir 扫描目录发现插件文件，以及使用 errorString()
///          诊断各种加载失败原因。
static void demo4_discoveryAndErrors()
{
    qDebug() << "\n[演示 4] 目录发现与错误诊断";

    PluginManager manager;
    manager.setRequiredVersion(1);

    // 扫描不存在的目录
    {
        qDebug() << "\n[发现] 扫描不存在的目录";
        QStringList found = manager.discoverPlugins("/nonexistent/plugin/dir");
        qDebug() << "[发现] 发现插件数量:" << found.size();
    }

    // 扫描当前目录（可能没有插件文件）
    {
        qDebug() << "\n[发现] 扫描当前目录";
        QStringList found = manager.discoverPlugins(QDir::currentPath());
        qDebug() << "[发现] 发现插件数量:" << found.size();
    }

    // 扫描 /tmp 目录（可能有 .so 文件也可能没有）
    {
        qDebug() << "\n[发现] 扫描 /tmp 目录";
        QStringList found = manager.discoverPlugins("/tmp");
        qDebug() << "[发现] 发现插件数量:" << found.size();
        for (const QString& path : found) {
            qDebug() << "[发现] 候选文件:" << path;
        }
    }

    // errorString() 诊断演示
    qDebug() << "\n[错误诊断] errorString() 各种失败场景";

    // 场景 1：文件不存在
    {
        QPluginLoader loader("/nonexistent/path/plugin.so");
        loader.load();
        qDebug() << "[错误诊断] 文件不存在:" << loader.errorString();
    }

    // 场景 2：不是有效的共享库（如普通文本文件）
    {
        // 创建一个临时文本文件，假装是插件
        QString tempFile = "/tmp/fake_plugin.so";
        QFile file(tempFile);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("This is not a real plugin file");
            file.close();
        }

        QPluginLoader loader(tempFile);
        loader.load();
        qDebug() << "[错误诊断] 无效文件格式:" << loader.errorString();

        // 清理临时文件
        QFile::remove(tempFile);
    }

    // 场景 3：目录路径而非文件路径
    {
        QPluginLoader loader("/tmp");
        loader.load();
        qDebug() << "[错误诊断] 目录路径:" << loader.errorString();
    }

    qDebug() << "\n[错误诊断] 总结：errorString() 是诊断插件加载问题的首选工具";
    qDebug() << "[错误诊断] 常见原因：文件不存在、格式错误、IID 不匹配、依赖缺失";

    // 最终演示：展示完整的插件系统架构
    qDebug() << "\n[架构] 完整的 Qt 插件系统架构";
    qDebug() << "[架构] 1. 定义纯虚接口（IPlugin）";
    qDebug() << "[架构] 2. 插件实现接口 + Q_PLUGIN_METADATA";
    qDebug() << "[架构] 3. 主程序用 QPluginLoader 加载";
    qDebug() << "[架构] 4. qobject_cast 获取接口指针";
    qDebug() << "[架构] 5. 通过接口调用插件功能";
    qDebug() << "[架构] 6. unload() 热卸载/替换插件";
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========== Qt 插件系统高级示例 ==========";
    qDebug() << "Qt 版本:" << QT_VERSION_STR;
    qDebug() << "注意：本示例使用内联插件演示概念";
    qDebug() << "真实项目中插件编译为独立 .so/.dll 文件";

    // 依次执行所有演示
    demo1_interfaceAndVersion();
    demo2_loadUnload();
    demo3_metadata();
    demo4_discoveryAndErrors();

    qDebug() << "\n========== 所有演示执行完毕 ==========";

    // 使用 QTimer::singleShot 延迟退出
    QTimer::singleShot(0, &app, &QCoreApplication::quit);

    return app.exec();
}
