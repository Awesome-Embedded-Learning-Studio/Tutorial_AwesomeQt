// Qt6 国际化(i18n)入门示例
// 演示：
// 1. tr() 函数标记需要翻译的字符串
// 2. QTranslator 加载翻译文件
// 3. 运行时动态切换语言
// 4. 复数形式处理
// 5. 消歧义字符串用法

#include <QApplication>        // 应用程序类
#include <QMainWindow>         // 主窗口类
#include <QLabel>              // 标签控件
#include <QVBoxLayout>         // 垂直布局
#include <QTranslator>         // 翻译器类
#include <QComboBox>           // 下拉框控件
#include <QPushButton>         // 按钮控件
#include <QDebug>              // 调试输出

// 自定义主窗口类，演示国际化用法
class I18nWindow : public QMainWindow {
    Q_OBJECT  // 必须有这个宏才能使用 tr() 函数

public:
    // 构造函数：初始化窗口和翻译器
    I18nWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        // 创建翻译器对象，父对象为 this，确保自动内存管理
        m_translator = new QTranslator(this);

        // 设置窗口属性
        setWindowTitle(tr("Internationalization Demo"));  // 窗口标题需要翻译
        resize(400, 200);

        // 创建中心控件和布局
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        // 创建标签显示需要翻译的文本
        m_label = new QLabel(tr("Hello, World!"), this);  // tr() 标记需要翻译的字符串
        m_label->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_label);

        // 创建标签演示复数形式
        m_countLabel = new QLabel(tr("You have %n message(s)", "", 0), this);
        m_countLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_countLabel);

        // 创建消息计数显示
        m_messageCount = 0;

        // 创建按钮增加消息数量（演示复数形式）
        QPushButton *addButton = new QPushButton(tr("Add Message"), this);
        connect(addButton, &QPushButton::clicked, this, &I18nWindow::addMessage);
        layout->addWidget(addButton);

        // 创建语言选择下拉框
        m_langCombo = new QComboBox(this);
        m_langCombo->addItem("English", "en");         // 显示文本，数据是语言代码
        m_langCombo->addItem("中文", "zh_CN");         // 中文选项
        connect(m_langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &I18nWindow::onLanguageChanged);
        layout->addWidget(m_langCombo);

        // 创建标签演示消歧义用法
        QLabel *fileMenuLabel = new QLabel(tr("File", "Menu item"), this);  // 菜单项的 "File"
        QLabel *fileVerbLabel = new QLabel(tr("File", "To file something"), this);  // 动词的 "File"
        layout->addWidget(fileMenuLabel);
        layout->addWidget(fileVerbLabel);

        // 尝试加载中文翻译（如果存在）
        // 翻译文件通常位于 :/translations/ 目录
        if (m_translator->load(":/translations/i18n-example_zh_CN.qm")) {
            qDebug() << "Translation file loaded successfully";
            // 注意：这里不安装翻译器，默认使用英文
            // 用户可以通过下拉框切换到中文
        } else {
            qDebug() << "Translation file not found, using default language";
        }

        // 添加弹性空间，让控件居中
        layout->addStretch();
    }

    // 槽函数：处理语言切换
    void onLanguageChanged(int index) {
        // 获取选择的语言代码
        QString langCode = m_langCombo->itemData(index).toString();
        qDebug() << "Language changed to:" << langCode;

        // 移除旧的翻译器
        if (QApplication::installTranslator(m_translator)) {
            QApplication::removeTranslator(m_translator);
        }

        // 加载新的翻译文件
        QString translationFile = QString(":/translations/i18n-example_%1.qm").arg(langCode);
        if (m_translator->load(translationFile)) {
            // 安装翻译器，之后所有 tr() 调用都会使用这个翻译
            QApplication::installTranslator(m_translator);
            qDebug() << "Translation installed:" << translationFile;
        } else {
            qDebug() << "Failed to load translation:" << translationFile;
        }

        // 重新翻译 UI：因为 tr() 只在执行时刻查找翻译
        // 切换翻译器后需要重新执行 tr() 才能看到效果
        retranslateUi();
    }

    // 槽函数：增加消息数量（演示复数形式）
    void addMessage() {
        m_messageCount++;
        // %n 是特殊占位符，Qt 会根据语言规则自动选择正确的复数形式
        m_countLabel->setText(tr("You have %n message(s)", "", m_messageCount));
    }

private:
    // 重新翻译所有用户可见的文本
    void retranslateUi() {
        // 重新设置窗口标题
        setWindowTitle(tr("Internationalization Demo"));

        // 重新设置标签文本
        m_label->setText(tr("Hello, World!"));

        // 重新设置复数形式标签
        m_countLabel->setText(tr("You have %n message(s)", "", m_messageCount));

        // 注意：下拉框的选项文本不在 tr() 中，因为它们是在代码中直接设置的
        // 实际项目中，这些也应该被翻译
    }

    QTranslator *m_translator;    // 翻译器对象
    QLabel *m_label;              // 显示主要文本的标签
    QLabel *m_countLabel;         // 显示消息数量的标签（演示复数形式）
    QComboBox *m_langCombo;       // 语言选择下拉框
    int m_messageCount;           // 消息计数
};

int main(int argc, char *argv[]) {
    // 创建应用程序实例
    QApplication app(argc, argv);

    // 创建并显示主窗口
    I18nWindow window;
    window.show();

    // 进入事件循环
    return app.exec();
}

// 编译和运行说明：
//
// 1. 构建项目：
//    cmake -B build
//    cmake --build build
//
// 2. 生成/更新翻译文件：
//    cmake --build build --target lupdate
//    这会扫描源代码中的 tr() 调用，更新 translations/i18n-example_zh_CN.ts 文件
//
// 3. 翻译字符串：
//    使用 Qt Linguist 打开 .ts 文件进行翻译：
//    linguist translations/i18n-example_zh_CN.ts
//
// 4. 编译翻译文件：
//    cmake --build build --target lrelease
//    这会将 .ts 文件编译成 .qm 二进制文件供运行时使用
//
// 5. 运行程序：
//    ./build/bin/i18n-example
//
// 翻译文件内容示例（i18n-example_zh_CN.ts）：
// <?xml version="1.0" encoding="utf-8"?>
// <!DOCTYPE TS>
// <TS version="2.1" language="zh_CN">
// <context>
//     <name>I18nWindow</name>
//     <message>
//         <source>Internationalization Demo</source>
//         <translation>国际化演示</translation>
//     </message>
//     <message>
//         <source>Hello, World!</source>
//         <translation>你好，世界！</translation>
//     </message>
//     <message>
//         <source>You have %n message(s)</source>
//         <translation>您有 %n 条消息</translation>
//     </message>
//     <message numerus="yes">
//         <source>You have %n message(s)</source>
//         <translation>
//             <numerusform>您有 %n 条消息</numerusform>
//         </translation>
//     </message>
//     <message>
//         <source>Add Message</source>
//         <translation>添加消息</translation>
//     </message>
// </context>
// </TS>

#include "main.moc"  // 因为我们在 .cpp 文件中使用了 Q_OBJECT 宏