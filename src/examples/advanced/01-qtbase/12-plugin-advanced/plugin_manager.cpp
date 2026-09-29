/// @file    plugin_manager.cpp
/// @brief   PluginManager 类的实现，包含插件加载、卸载、发现和元数据读取。
///
/// @details 对应教程：进阶层 01-QtBase/12-插件系统。

#include "plugin_manager.h"

#include <QDebug>

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
    , m_requiredVersion(1)  // 默认要求的最低版本号
{
}

PluginManager::~PluginManager()
{
    unloadAll();
}

void PluginManager::setRequiredVersion(int version)
{
    m_requiredVersion = version;
}

QStringList PluginManager::discoverPlugins(const QString& dirPath)
{
    QStringList discovered;

    QDir pluginDir(dirPath);
    if (!pluginDir.exists()) {
        qDebug() << "[PluginManager] 插件目录不存在:" << dirPath;
        return discovered;
    }

    qDebug() << "[PluginManager] 扫描插件目录:" << dirPath;

    // 根据平台确定共享库后缀
    QStringList filters;
#ifdef Q_OS_LINUX
    filters << "*.so";
#elif defined(Q_OS_WIN)
    filters << "*.dll";
#elif defined(Q_OS_MAC)
    filters << "*.dylib";
#endif

    QFileInfoList files = pluginDir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fileInfo : files) {
        discovered.append(fileInfo.absoluteFilePath());
        qDebug() << "[PluginManager] 发现插件文件:" << fileInfo.fileName();
    }

    if (discovered.isEmpty()) {
        qDebug() << "[PluginManager] 目录中没有发现插件文件";
    }

    return discovered;
}

QJsonObject PluginManager::readPluginMetaData(const QString& filePath)
{
    QPluginLoader loader(filePath);

    QJsonObject metaData = loader.metaData();
    if (metaData.isEmpty()) {
        qDebug() << "[PluginManager] 无法读取元数据:" << filePath;
        qDebug() << "[PluginManager] 错误:" << loader.errorString();
    }

    return metaData;
}

bool PluginManager::loadPlugin(const QString& filePath)
{
    // 检查是否已经加载
    if (m_plugins.contains(filePath)) {
        qDebug() << "[PluginManager] 插件已加载，跳过:" << filePath;
        return true;
    }

    qDebug() << "[PluginManager] 尝试加载插件:" << filePath;

    QPluginLoader* loader = new QPluginLoader(filePath);

    // 步骤 1：读取元数据
    QJsonObject metaData = loader->metaData();
    if (metaData.isEmpty()) {
        qDebug() << "[PluginManager] 无法读取元数据:"
                 << loader->errorString();
        delete loader;
        return false;
    }

    // 检查 IID 匹配
    QString iid = metaData.value("IID").toString();
    if (iid != PLUGIN_INTERFACE_IID) {
        qDebug() << "[PluginManager] IID 不匹配:"
                 << "期望" << PLUGIN_INTERFACE_IID
                 << "实际" << iid;
        delete loader;
        return false;
    }

    qDebug() << "[PluginManager] IID 匹配:" << iid;

    // 读取元数据中的附加信息
    QJsonObject metaDataObj = metaData.value("MetaData").toObject();
    qDebug() << "[PluginManager] 元数据:" << metaDataObj;

    // 步骤 2：加载共享库
    if (!loader->load()) {
        qDebug() << "[PluginManager] 加载失败:"
                 << loader->errorString();
        delete loader;
        return false;
    }

    // 步骤 3：获取接口指针
    QObject* pluginObj = loader->instance();
    if (!pluginObj) {
        qDebug() << "[PluginManager] 获取实例失败:"
                 << loader->errorString();
        loader->unload();
        delete loader;
        return false;
    }

    IPlugin* plugin = qobject_cast<IPlugin*>(pluginObj);
    if (!plugin) {
        qDebug() << "[PluginManager] qobject_cast 失败，接口不兼容";
        loader->unload();
        delete loader;
        return false;
    }

    // 步骤 4：检查版本兼容性
    int pluginVersion = plugin->version();
    if (pluginVersion < m_requiredVersion) {
        qDebug() << "[PluginManager] 版本不兼容:"
                 << "插件版本" << pluginVersion
                 << "要求 >=" << m_requiredVersion;
        loader->unload();
        delete loader;
        return false;
    }

    // 步骤 5：初始化插件
    if (!plugin->initialize()) {
        qDebug() << "[PluginManager] 插件初始化失败";
        loader->unload();
        delete loader;
        return false;
    }

    // 记录插件信息
    PluginInfo info;
    info.filePath = filePath;
    info.name = plugin->name();
    info.version = pluginVersion;
    info.instance = plugin;
    info.loader = loader;
    m_plugins[filePath] = info;

    qDebug() << "[PluginManager] 插件加载成功:"
             << plugin->name()
             << "v" << pluginVersion;
    return true;
}

bool PluginManager::unloadPlugin(const QString& filePath)
{
    if (!m_plugins.contains(filePath)) {
        qDebug() << "[PluginManager] 插件未加载:" << filePath;
        return false;
    }

    PluginInfo& info = m_plugins[filePath];
    qDebug() << "[PluginManager] 卸载插件:" << info.name;

    // 先通知插件进行清理
    info.instance->shutdown();

    // 卸载共享库
    if (!info.loader->unload()) {
        qDebug() << "[PluginManager] 卸载失败:" << info.loader->errorString();
        return false;
    }

    delete info.loader;
    m_plugins.remove(filePath);
    qDebug() << "[PluginManager] 插件已卸载";
    return true;
}

void PluginManager::unloadAll()
{
    QStringList paths = m_plugins.keys();
    for (const QString& path : paths) {
        unloadPlugin(path);
    }
    qDebug() << "[PluginManager] 所有插件已卸载";
}

int PluginManager::pluginCount() const
{
    return m_plugins.size();
}

QList<PluginInfo> PluginManager::loadedPlugins() const
{
    return m_plugins.values();
}

IPlugin* PluginManager::findPlugin(const QString& name) const
{
    for (const PluginInfo& info : m_plugins) {
        if (info.name == name) {
            return info.instance;
        }
    }
    return nullptr;
}
