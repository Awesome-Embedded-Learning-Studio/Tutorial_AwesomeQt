/// @file    sample_plugin.cpp
/// @brief   SamplePlugin 类的实现，包含插件生命周期方法。
///
/// @details 对应教程：进阶层 01-QtBase/12-插件系统。

#include "sample_plugin.h"

SamplePlugin::SamplePlugin()
    : m_initialized(false)
{
    qDebug() << "[SamplePlugin] 构造函数调用";
}

SamplePlugin::~SamplePlugin()
{
    qDebug() << "[SamplePlugin] 析构函数调用";
}

QString SamplePlugin::name() const
{
    return "SamplePlugin";
}

QString SamplePlugin::description() const
{
    return "示例插件 - 演示 Qt 插件系统的接口实现";
}

int SamplePlugin::version() const
{
    return 1;
}

bool SamplePlugin::initialize()
{
    if (m_initialized) {
        qDebug() << "[SamplePlugin] 已经初始化，跳过";
        return true;
    }

    qDebug() << "[SamplePlugin] 初始化中...";
    // 模拟初始化操作
    m_initialized = true;
    qDebug() << "[SamplePlugin] 初始化完成";
    return true;
}

QString SamplePlugin::execute(const QString& input)
{
    qDebug() << "[SamplePlugin] 执行功能，输入:" << input;
    QString result = QString("[%1 v%2] %3")
        .arg(name())
        .arg(version())
        .arg(input.toUpper());
    qDebug() << "[SamplePlugin] 执行结果:" << result;
    return result;
}

void SamplePlugin::shutdown()
{
    qDebug() << "[SamplePlugin] 关闭中...";
    m_initialized = false;
    qDebug() << "[SamplePlugin] 已关闭";
}
