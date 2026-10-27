/*
 * 大写转换插件实现
 *
 * 演示如何实现一个简单的 Qt 插件
 * 这个插件将输入的文本转换为大写
 */

#include "textprocessor_interface.h"
#include <QObject>       // QObject 基类
#include <QtPlugin>      // Qt 插件宏

/**
 * @brief 大写转换插件
 *
 * 实现了 TextProcessorInterface 接口
 * 将输入的文本全部转换为大写字母
 */
class UpperCasePlugin : public QObject, public TextProcessorInterface {
    Q_OBJECT  // 必须有这个宏才能使用 Qt 的元对象系统

    // 声明插件元数据：
    // - IID: 必须与接口的 IID 完全一致
    // - FILE: 可选的元数据文件（这里省略）
    Q_PLUGIN_METADATA(IID TextProcessorInterface_iid)

    // 声明实现的接口：让 qobject_cast 能正确识别
    Q_INTERFACES(TextProcessorInterface)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    UpperCasePlugin(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief 处理文本：转换为全大写
     * @param input 输入文本
     * @return 大写后的文本
     */
    QString process(const QString &input) override {
        return input.toUpper();
    }

    /**
     * @brief 获取插件名称
     * @return "Upper Case Converter"
     */
    QString name() const override {
        return "Upper Case Converter";
    }

    /**
     * @brief 获取插件版本
     * @return "1.0.0"
     */
    QString version() const override {
        return "1.0.0";
    }
};

// 插件导出：这个宏是必需的，它让插件系统能够找到这个插件
// 对于使用 qt_add_plugin 的 CMake 项目，这个宏可以省略
// 但显式写出有助于理解插件导出机制

// 当 Q_OBJECT 宏在 .cpp 文件中使用时，必须手动包含 MOC 生成的文件
#include "uppercase_plugin.moc"
