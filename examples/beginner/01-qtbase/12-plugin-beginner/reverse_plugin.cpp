/*
 * 文本反转插件实现
 *
 * 演示第二个插件的实现
 * 这个插件将输入的文本反转
 */

#include "textprocessor_interface.h"
#include <QObject>       // QObject 基类
#include <QtPlugin>      // Qt 插件宏
#include <algorithm>     // std::reverse

/**
 * @brief 文本反转插件
 *
 * 实现了 TextProcessorInterface 接口
 * 将输入的文本完全反转（"hello" -> "olleh"）
 */
class ReversePlugin : public QObject, public TextProcessorInterface {
    Q_OBJECT  // 必须有这个宏才能使用 Qt 的元对象系统

    // 声明插件元数据：IID 必须与接口一致
    Q_PLUGIN_METADATA(IID TextProcessorInterface_iid)

    // 声明实现的接口
    Q_INTERFACES(TextProcessorInterface)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    ReversePlugin(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * @brief 处理文本：反转字符串
     * @param input 输入文本
     * @return 反转后的文本
     */
    QString process(const QString &input) override {
        QString reversed = input;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }

    /**
     * @brief 获取插件名称
     * @return "Text Reverser"
     */
    QString name() const override {
        return "Text Reverser";
    }

    /**
     * @brief 获取插件版本
     * @return "1.0.0"
     */
    QString version() const override {
        return "1.0.0";
    }
};

// 插件由 CMake 的 qt_add_plugin 自动导出

// 当 Q_OBJECT 宏在 .cpp 文件中使用时，必须手动包含 MOC 生成的文件
#include "reverse_plugin.moc"
