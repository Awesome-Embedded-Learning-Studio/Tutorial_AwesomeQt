/*
 * 文本处理插件接口定义
 *
 * 这个头文件定义了文本处理插件必须实现的接口
 * 主程序和所有插件都依赖这个接口
 */

#ifndef TEXTPROCESSOR_INTERFACE_H
#define TEXTPROCESSOR_INTERFACE_H

#include <QObject>       // 必须包含 QObject 才能使用 Q_DECLARE_INTERFACE
#include <QString>       // 字符串类

/**
 * @brief 文本处理插件接口
 *
 * 所有文本处理插件都必须实现这个接口
 * 接口提供了三个必须实现的方法：
 * 1. process() - 处理文本
 * 2. name() - 返回插件名称
 * 3. version() - 返回插件版本
 */
class TextProcessorInterface {
public:
    /**
     * @brief 虚析构函数
     *
     * 接口类必须有一个虚析构函数，确保通过接口指针删除对象时
     * 会调用派生类的析构函数，避免内存泄漏
     */
    virtual ~TextProcessorInterface() = default;

    /**
     * @brief 处理输入的文本
     * @param input 待处理的文本
     * @return 处理后的文本
     *
     * 这是一个纯虚函数，所有插件必须实现它
     * 具体的处理逻辑由各插件自行决定（如转大写、反转等）
     */
    virtual QString process(const QString &input) = 0;

    /**
     * @brief 获取插件名称
     * @return 插件的显示名称
     *
     * 用于在界面上显示插件的友好名称，如 "大写转换器"
     */
    virtual QString name() const = 0;

    /**
     * @brief 获取插件版本
     * @return 版本字符串，如 "1.0.0"
     *
     * 用于版本管理和兼容性检查
     */
    virtual QString version() const = 0;
};

// 定义插件接口的唯一标识符（IID）
// 这个字符串必须在整个系统中保持唯一，用于在加载插件时验证接口类型
#define TextProcessorInterface_iid "org.example.TextProcessorInterface.1.0"

// 声明接口：让 Qt 的元对象系统知道这个接口的存在
// 这个宏是必需的，它使得 qobject_cast 能够正确识别接口类型
// 注意：第二个参数必须是字符串字面量，不能是宏展开后的字符串
Q_DECLARE_INTERFACE(TextProcessorInterface, "org.example.TextProcessorInterface.1.0")

#endif // TEXTPROCESSOR_INTERFACE_H
