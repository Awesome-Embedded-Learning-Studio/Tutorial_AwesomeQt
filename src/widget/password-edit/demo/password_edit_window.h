/**
 * @file password_edit_window.h
 * @brief PasswordEdit 演示主窗口
 * @copyright Copyright (c) 2026 AwesomeQt
 *
 * 展示：① 显隐切换；② 强度 3 色块实时变；③ 实时回显 text；
 * ④ 「显示密码」勾选框联动显隐。
 */
#pragma once

#include <QMainWindow>

class QCheckBox;
class QLabel;

namespace AwesomeQt {
class PasswordEdit;
}

class PasswordEditWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit PasswordEditWindow(QWidget* parent = nullptr);

  private:
    AwesomeQt::PasswordEdit* edit_{nullptr};
    QLabel* echo_label_{nullptr};
    QLabel* strength_label_{nullptr};
    QCheckBox* visible_check_{nullptr};
};
