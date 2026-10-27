/// @file    plugin_interface.h
/// @brief   定义插件系统纯虚接口 IPlugin。
///
/// @details Qt 的插件系统基于 QPluginLoader 和纯虚接口。主程序定义接口，
///          插件实现接口，通过 Q_PLUGIN_METADATA 声明自己是插件。
///          运行时 QPluginLoader 动态加载 .so/.dll 并通过 qobject_cast
///          获取接口指针。

#pragma once

#include <QtPlugin>   // Q_DECLARE_INTERFACE 宏

/// 插件接口的唯一标识符，用于 qobject_cast 类型检查。
/// 字符串格式 "com.company.Project.InterfaceName:Version" 必须在整个系统中唯一。
#define PLUGIN_INTERFACE_IID "org.awesomeqt.tutorial.IPlugin/1.0"

/// @brief 插件系统纯虚接口，所有插件必须实现。
///
/// @details 主程序通过此接口与插件交互，而不直接依赖插件的具体实现类。
///          这种解耦使得插件可以独立编译和更新。
///
///          设计原则：
///          - 接口应当尽量稳定，避免频繁变更
///          - 使用版本号标识接口变更
///          - 虚析构函数确保通过基类指针删除时正确调用派生类析构
class IPlugin
{
public:
    virtual ~IPlugin() = default;

    /// @brief 获取插件名称，用于在主程序中显示和识别插件。
    /// @return 插件名称字符串。
    virtual QString name() const = 0;

    /// @brief 获取插件描述，详细说明插件的功能和用途。
    /// @return 插件描述字符串。
    virtual QString description() const = 0;

    /// @brief 获取插件版本号。
    /// @details 主程序在加载时检查此版本号，确保接口兼容性。
    ///          版本号规则：主版本.次版本.修订号。
    ///          主版本变更表示不兼容的 API 变更。
    /// @return 整数版本号。
    virtual int version() const = 0;

    /// @brief 初始化插件，在插件加载后调用。
    /// @return true 初始化成功，false 初始化失败。
    virtual bool initialize() = 0;

    /// @brief 执行插件的主要功能。
    /// @param[in] input 输入参数。
    /// @return 执行结果的描述信息。
    virtual QString execute(const QString& input) = 0;

    /// @brief 关闭插件，释放资源。在插件卸载前调用。
    virtual void shutdown() = 0;
};

Q_DECLARE_INTERFACE(IPlugin, PLUGIN_INTERFACE_IID)
