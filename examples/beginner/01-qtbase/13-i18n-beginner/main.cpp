// Qt6 国际化(i18n)入门示例
// 演示：
// 1. tr() 函数标记需要翻译的字符串
// 2. QTranslator 加载翻译文件
// 3. 运行时动态切换语言
// 4. 复数形式处理
// 5. 消歧义字符串用法

#include <QApplication>        // 应用程序类

#include "i18nwindow.h"

int main(int argc, char *argv[]) {
    // 创建应用程序实例
    QApplication app(argc, argv);

    // 创建并显示主窗口
    I18nWindow window;
    window.show();

    // 进入事件循环
    return app.exec();
}
