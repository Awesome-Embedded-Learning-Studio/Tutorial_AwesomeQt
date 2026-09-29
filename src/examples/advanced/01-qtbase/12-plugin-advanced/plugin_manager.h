/// @file    plugin_manager.h
/// @brief   定义 PluginManager 类，负责插件扫描、加载、卸载和版本检查。
///
/// @details PluginManager 封装了 QPluginLoader 的使用模式：
///          1. 使用 QDir 扫描插件目录
///          2. 对每个 .so/.dll 文件使用 QPluginLoader 尝试加载
///          3. 加载后通过 qobject_cast 获取 IPlugin 接口指针
///          4. 检查版本兼容性，不兼容则卸载
///          5. 提供 load/unload 的热管理功能

#pragma once

#include <QDir>                // 目录操作
#include <QFileInfoList>       // 文件信息列表
#include <QJsonArray>
#include <QJsonObject>         // JSON 对象（读取元数据）
#include <QMap>
#include <QObject>
#include <QPluginLoader>       // 插件加载器

#include "plugin_interface.h"

/// @brief 插件信息结构体，存储已加载插件的状态。
struct PluginInfo
{
    QString filePath;       ///< 插件文件路径
    QString name;           ///< 插件名称
    int version;            ///< 插件版本
    IPlugin* instance;      ///< 插件实例指针（通过接口访问）
    QPluginLoader* loader;  ///< 加载器指针（用于后续卸载）
};

/// @brief 插件管理器，提供插件发现、加载、版本检查、卸载的完整生命周期管理。
///
/// @details 在实际应用中，PluginManager 通常是单例，在程序启动时扫描
///          插件目录并加载所有兼容插件。
class PluginManager : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit PluginManager(QObject* parent = nullptr);

    /// @brief 析构函数，卸载所有已加载的插件。
    ~PluginManager() override;

    /// @brief 设置要求的最低插件版本。低于此版本的插件将被拒绝加载。
    /// @param[in] version 要求的最低版本号。
    void setRequiredVersion(int version);

    /// @brief 扫描目录发现插件文件。
    /// @details 使用 QDir 遍历指定目录下的所有共享库文件（.so/.dll/.dylib），
    ///          返回候选插件文件路径列表。实际的加载尝试在 loadPlugin() 中进行。
    /// @param[in] dirPath 插件目录路径。
    /// @return 发现的插件文件路径列表。
    QStringList discoverPlugins(const QString& dirPath);

    /// @brief 读取插件的 JSON 元数据（不加载插件）。
    /// @details QPluginLoader::metaData() 可以在不实际加载插件的情况下
    ///          读取其 JSON 元数据，适合快速扫描插件信息而不付出加载开销。
    ///          元数据来源于插件编译时 Q_PLUGIN_METADATA 宏指定的 JSON 文件。
    /// @param[in] filePath 插件文件路径。
    /// @return JSON 元数据对象。
    QJsonObject readPluginMetaData(const QString& filePath);

    /// @brief 加载单个插件文件。
    /// @details 完整的加载流程：
    ///          1. 读取元数据检查 IID 匹配
    ///          2. 调用 QPluginLoader::load() 加载共享库
    ///          3. qobject_cast 转换为 IPlugin 接口指针
    ///          4. 检查版本兼容性
    ///          5. 调用插件的 initialize() 方法
    /// @param[in] filePath 插件文件路径。
    /// @return 加载成功返回 true。
    bool loadPlugin(const QString& filePath);

    /// @brief 卸载指定插件（热卸载）。
    /// @details 调用插件的 shutdown() 方法释放资源，然后使用
    ///          QPluginLoader::unload() 卸载共享库。
    /// @param[in] filePath 插件文件路径。
    /// @return 卸载成功返回 true。
    bool unloadPlugin(const QString& filePath);

    /// @brief 卸载所有已加载的插件。
    void unloadAll();

    /// @brief 获取已加载插件的数量。
    /// @return 已加载插件数量。
    int pluginCount() const;

    /// @brief 获取所有已加载插件的信息。
    /// @return 已加载插件信息列表。
    QList<PluginInfo> loadedPlugins() const;

    /// @brief 根据名称查找插件。
    /// @param[in] name 插件名称。
    /// @return 插件实例指针，未找到返回 nullptr。
    IPlugin* findPlugin(const QString& name) const;

private:
    QMap<QString, PluginInfo> m_plugins;  ///< 已加载的插件映射
    int m_requiredVersion;                ///< 要求的最低版本号
};
