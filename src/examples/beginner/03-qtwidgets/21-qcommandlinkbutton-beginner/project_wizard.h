// QtWidgets 入门示例 21: QCommandLinkButton 命令链接按钮
// 演示：setDescription() 设置副标题描述文字
//       向导对话框中的功能选项场景
//       图标按钮 + clicked 信号响应
//       QSS 统一跨平台外观

#pragma once

#include <QWidget>

class QCommandLinkButton;
class QLabel;

// ============================================================================
// ProjectWizard: 模拟项目创建向导的选择页面
// ============================================================================
class ProjectWizard : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectWizard(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 处理选项按钮点击
    void onOptionSelected(const QString &projectType);

private:
    QCommandLinkButton *m_emptyBtn = nullptr;
    QCommandLinkButton *m_widgetsBtn = nullptr;
    QCommandLinkButton *m_quickBtn = nullptr;
    QLabel *m_resultLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
};
