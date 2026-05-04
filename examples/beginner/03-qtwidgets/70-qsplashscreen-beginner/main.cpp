// QtWidgets 入门示例 70: QSplashScreen 启动画面
// 演示：show/showMessage/finish 启动画面生命周期
//       与耗时初始化操作配合
//       processEvents 确保画面刷新

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QSplashScreen>
#include <QThread>

#include "mainwindow.h"

// ============================================================================
// 辅助结构: 初始化步骤描述
// ============================================================================
struct InitStep
{
    QString message;
    int durationMs;
};

// ============================================================================
// 创建启动画面的 QPixmap（不依赖外部资源文件）
// ============================================================================
static QPixmap createSplashPixmap()
{
    QPixmap pixmap(500, 300);
    pixmap.fill(QColor("#1E1E2E"));

    QPainter painter(&pixmap);
    painter.setRenderHint(
        QPainter::Antialiasing);

    // 应用名称
    painter.setPen(QColor("#CDD6F4"));
    painter.setFont(
        QFont("Sans", 22, QFont::Bold));
    painter.drawText(
        QRect(0, 70, 500, 50),
        Qt::AlignCenter,
        "My Application");

    // 版本号
    painter.setFont(QFont("Sans", 11));
    painter.setPen(QColor("#A6ADC8"));
    painter.drawText(
        QRect(0, 130, 500, 30),
        Qt::AlignCenter,
        "Version 1.0.0");

    // 底部分割线
    painter.setPen(QColor("#45475A"));
    painter.drawLine(50, 250, 450, 250);

    painter.end();
    return pixmap;
}

// ============================================================================
// 主函数: 启动画面 + 初始化序列 + 主窗口
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("MyApp");
    app.setOrganizationName("MyOrg");

    // 创建启动画面
    QSplashScreen splash(createSplashPixmap());
    splash.show();

    // 确保画面立即绘制
    QCoreApplication::processEvents();

    // 启动计时
    QElapsedTimer timer;
    timer.start();

    // 定义初始化步骤
    const InitStep steps[] = {
        {"正在加载配置文件...",     800},
        {"正在初始化日志系统...",   400},
        {"正在连接数据库...",       1000},
        {"正在扫描插件目录...",     600},
        {"正在准备用户界面...",     300},
    };

    // 逐步执行初始化
    for (const auto &step : steps) {
        splash.showMessage(
            step.message,
            Qt::AlignBottom | Qt::AlignHCenter,
            Qt::white);
        QCoreApplication::processEvents();
        QThread::msleep(step.durationMs);
    }

    // 计算耗时
    const double elapsedSec =
        timer.elapsed() / 1000.0;

    splash.showMessage(
        "启动完成!",
        Qt::AlignBottom | Qt::AlignHCenter,
        QColor("#A6E3A1"));
    QCoreApplication::processEvents();

    // 创建并显示主窗口
    MainWindow mainWindow(elapsedSec);
    mainWindow.show();

    // 关闭启动画面
    splash.finish(&mainWindow);

    return app.exec();
}
