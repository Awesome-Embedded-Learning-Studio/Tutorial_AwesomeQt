// QtGui 入门示例 06: 拖放系统基础
// 演示：拖放源发起拖拽、拖放目标接收数据、QMimeData 文本携带、文件拖入接收

#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "dragsourcewidget.h"
#include "droptargetwidget.h"

// ============================================================================
// 主函数：创建窗口展示拖放交互
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto *window = new QWidget;
    window->setWindowTitle("Qt 拖放系统演示");
    window->resize(600, 400);

    auto *mainLayout = new QVBoxLayout(window);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 顶部说明
    auto *infoLabel = new QLabel(
        "从左侧拖拽文本到右侧，或从文件管理器拖入文件到右侧区域");
    infoLabel->setStyleSheet("color: #666; font-size: 11px; padding: 4px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // 水平排列两个 Widget
    auto *hLayout = new QHBoxLayout;
    hLayout->setSpacing(20);

    auto *source = new DragSourceWidget;
    auto *target = new DropTargetWidget;

    hLayout->addWidget(source, 1);
    hLayout->addWidget(target, 1);

    mainLayout->addLayout(hLayout, 1);

    window->show();
    return app.exec();
}
