/// @file    sample_plugin.h
/// @brief   定义内联示例插件 SamplePlugin，演示 Q_PLUGIN_METADATA 和 Q_INTERFACES 的使用。
///
/// @details 在真实项目中，插件会编译为独立的 .so/.dll 文件，由主程序
///          通过 QPluginLoader 动态加载。本示例是单个可执行文件，
///          因此用内联方式演示插件的概念和 API 使用。
///
///          关键宏说明：
///          - Q_PLUGIN_METADATA: 声明插件的元数据，包括 IID 和 JSON 文件
///          - Q_INTERFACES: 声明此类实现了哪些接口（用于 qobject_cast）
///          - Q_OBJECT: 启用信号/槽和元对象系统支持

#pragma once

#include <QDebug>
#include <QObject>

#include "plugin_interface.h"

/// @brief 示例插件实现，模拟真实插件的完整生命周期。
///
/// @details 实现了 IPlugin 接口的所有虚函数，包含初始化、执行、关闭的完整生命周期。
///          Q_PLUGIN_METADATA 宏必须出现在私有部分（private section），这是 Qt 的要求。
///          FILE 参数指向一个 JSON 文件，包含插件元数据。
class SamplePlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
    // IID 必须与接口声明的 IID 完全一致
    // FILE 指向包含额外元数据的 JSON 文件
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID FILE "sample_plugin.json")

public:
    /// @brief 构造函数。
    SamplePlugin();

    /// @brief 析构函数。
    ~SamplePlugin() override;

    /// @brief 获取插件名称。
    /// @return 插件名称字符串。
    QString name() const override;

    /// @brief 获取插件描述。
    /// @return 插件描述字符串。
    QString description() const override;

    /// @brief 获取版本号。版本 1 表示与当前主程序接口兼容。
    /// @return 整数版本号。
    int version() const override;

    /// @brief 初始化插件。在真实插件中，这里可能打开数据库连接、加载配置等。
    /// @return 初始化成功返回 true。
    bool initialize() override;

    /// @brief 执行插件功能。将输入文本转换为大写并附加插件信息。
    /// @param[in] input 输入文本。
    /// @return 执行结果字符串。
    QString execute(const QString& input) override;

    /// @brief 关闭插件，释放资源。
    void shutdown() override;

private:
    bool m_initialized;  ///< 初始化状态标志
};
