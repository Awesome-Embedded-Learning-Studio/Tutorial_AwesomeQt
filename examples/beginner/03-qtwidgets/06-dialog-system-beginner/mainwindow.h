#pragma once

#include "userinfo.h"

#include <QWidget>

class QLabel;
class FindDialog;

// MainWindow: 主窗口
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    /// @brief 模态对话框：用 exec() 弹出，阻塞等待用户操作
    void onEditInfoClicked();

    /// @brief 非模态对话框：用 show() 弹出，不阻塞
    void onFindClicked();

private:
    void updateInfoDisplay();

    QLabel *m_infoDisplay = nullptr;
    UserInfo m_userInfo;
    FindDialog *m_findDialog = nullptr;
};
