// QtWidgets 入门示例 69: QWizard 向导对话框
// 演示：addPage 添加向导页
//       initializePage / validatePage 页面生命周期
//       registerField / field 页面间数据传递
//       自定义按钮文字与样式

#include <QApplication>
#include <QDebug>
#include <QWizard>

#include "welcomepage.h"
#include "userinfopage.h"
#include "preferencepage.h"
#include "confirmpage.h"

// ============================================================================
// 主函数: 组装向导
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWizard wizard;
    wizard.setWindowTitle("用户注册向导");
    wizard.resize(600, 450);

    // 添加四个页面（线性向导，addPage 自动分配 ID）
    wizard.addPage(new WelcomePage);
    wizard.addPage(new UserInfoPage);
    wizard.addPage(new PreferencePage);
    wizard.addPage(new ConfirmPage);

    // 自定义按钮文字
    wizard.setButtonText(
        QWizard::BackButton, "上一步");
    wizard.setButtonText(
        QWizard::NextButton, "下一步");
    wizard.setButtonText(
        QWizard::FinishButton, "完成注册");
    wizard.setButtonText(
        QWizard::CancelButton, "放弃");

    // 启用帮助按钮
    wizard.setOption(
        QWizard::HaveHelpButton, true);

    // 帮助按钮的简单演示: 连接到 qDebug
    QObject::connect(
        &wizard, &QWizard::helpRequested,
        [&wizard]() {
            const int currentId =
                wizard.currentId();
            qDebug() << "帮助请求: 当前页面 ID ="
                     << currentId;
        });

    if (wizard.exec() == QWizard::Accepted) {
        qDebug() << "用户完成了注册向导";
    } else {
        qDebug() << "用户取消了注册向导";
    }

    return 0;
}
