/**
 * @file password_edit_window.cpp
 * @brief PasswordEdit 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "password_edit_window.h"

#include "password_edit.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

PasswordEditWindow::PasswordEditWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("PasswordEdit 演示 —— 显隐切换 + 实时强度"));

    auto* central = new QWidget(this);
    setCentralWidget(central);

    edit_ = new AwesomeQt::PasswordEdit(central);
    echo_label_ = new QLabel(QStringLiteral("(空)"), central);
    echo_label_->setWordWrap(true);
    strength_label_ = new QLabel(QStringLiteral("弱"), central);
    visible_check_ = new QCheckBox(QStringLiteral("显示密码"), central);

    auto* demo_layout = new QVBoxLayout;
    demo_layout->setSpacing(6);
    demo_layout->addWidget(new QLabel(QStringLiteral("密码：")));
    demo_layout->addWidget(edit_);
    demo_layout->addSpacing(8);
    demo_layout->addWidget(new QLabel(QStringLiteral("实时回显：")));
    demo_layout->addWidget(echo_label_);
    demo_layout->addWidget(new QLabel(QStringLiteral("当前强度：")));
    demo_layout->addWidget(strength_label_);
    demo_layout->addWidget(visible_check_);
    demo_layout->addStretch();

    auto* demo_box = new QGroupBox(QStringLiteral("①②③④  演示"), central);
    demo_box->setLayout(demo_layout);

    auto* hint_box = new QGroupBox(QStringLiteral("强度提示"), central);
    auto* hint_layout = new QVBoxLayout(hint_box);
    hint_layout->addWidget(new QLabel(QStringLiteral("输入 < 6 位 / 纯字母 → 弱（红 1 块）\n"
                                                     "种类 = 2（如字母+数字）→ 中（黄 2 块）\n"
                                                     "种类 >= 3 且 >= 8 位 → 强（绿 3 块）"),
                                      hint_box));

    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);
    root->addWidget(demo_box, 1);
    root->addWidget(hint_box, 1);

    resize(520, 280);

    // 信号绑定：函数指针语法
    connect(edit_, &AwesomeQt::PasswordEdit::textVisibleChanged, visible_check_,
            &QCheckBox::setChecked);
    connect(visible_check_, &QCheckBox::toggled, edit_, &AwesomeQt::PasswordEdit::setTextVisible);
    connect(edit_, &AwesomeQt::PasswordEdit::strengthChanged, this,
            [this](AwesomeQt::PasswordEdit::Strength s) {
                QString name;
                switch (s) {
                    case AwesomeQt::PasswordEdit::Strength::kWeak:
                        name = QStringLiteral("弱");
                        break;
                    case AwesomeQt::PasswordEdit::Strength::kMedium:
                        name = QStringLiteral("中");
                        break;
                    case AwesomeQt::PasswordEdit::Strength::kStrong:
                        name = QStringLiteral("强");
                        break;
                }
                strength_label_->setText(name);
            });
    // 文本变化实时回显（密文时也显示真实内容，验证 text() 拿得到）
    connect(edit_, &AwesomeQt::PasswordEdit::textChanged, this, [this](const QString& text) {
        echo_label_->setText(text.isEmpty() ? QStringLiteral("(空)") : text);
    });
}
