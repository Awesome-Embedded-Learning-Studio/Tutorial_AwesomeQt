#include <QApplication>

#include "nfcwindow.h"

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "Qt NFC 标签读写工具";
    qDebug() << "本示例演示 NDEF 消息的读取和写入";
    qDebug() << "注意：需要 NFC 硩件支持（主要在 Android 上工作）";

    NfcWindow window;
    window.show();

    return app.exec();
}
