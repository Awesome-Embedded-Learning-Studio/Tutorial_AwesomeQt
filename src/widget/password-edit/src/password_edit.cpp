/**
 * @file password_edit.cpp
 * @brief PasswordEdit 控件实现——显隐切换 + 实时强度指示
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "password_edit.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QVBoxLayout>

namespace AwesomeQt {

// 强度色块样式：亮=对应档色，未亮=中性灰。固定字号色块，保持组合控件本色。
static constexpr const char* kStyleOff =
    "QLabel { background-color: #c0c0c0; border-radius: 2px; }";
static constexpr const char* kStyleWeak =
    "QLabel { background-color: #e53935; border-radius: 2px; }"; // 红
static constexpr const char* kStyleMedium =
    "QLabel { background-color: #fdd835; border-radius: 2px; }"; // 黄
static constexpr const char* kStyleStrong =
    "QLabel { background-color: #43a047; border-radius: 2px; }"; // 绿

PasswordEdit::PasswordEdit(QWidget* parent) : QWidget(parent) {
    // —— 编辑行：密码框 + 显隐切换按钮 ——
    edit_ = new QLineEdit(this);
    edit_->setEchoMode(text_visible_ ? QLineEdit::Normal : QLineEdit::Password);
    edit_->setPlaceholderText(QStringLiteral("请输入密码"));

    toggle_btn_ = new QToolButton(this);
    toggle_btn_->setText(QStringLiteral("显")); // 默认密文 → 按钮提示「显」
    toggle_btn_->setFocusPolicy(Qt::NoFocus);
    toggle_btn_->setCursor(Qt::PointingHandCursor);

    auto* edit_row = new QHBoxLayout;
    edit_row->setContentsMargins(0, 0, 0, 0);
    edit_row->addWidget(edit_);
    edit_row->addWidget(toggle_btn_);

    // —— 强度行：3 个色块 ——
    auto* strength_row = new QHBoxLayout;
    strength_row->setContentsMargins(0, 0, 0, 0);
    strength_row->setSpacing(4);
    for (int i = 0; i < 3; ++i) {
        strength_labels_[i] = new QLabel(this);
        strength_labels_[i]->setFixedHeight(6);
        strength_labels_[i]->setStyleSheet(kStyleOff);
        strength_row->addWidget(strength_labels_[i]);
    }
    strength_row->addStretch(); // 色块靠左，右侧弹性留白

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(4);
    root->addLayout(edit_row);
    root->addLayout(strength_row);

    // 函数指针语法，禁 SIGNAL/SLOT 宏
    connect(edit_, &QLineEdit::textChanged, this, &PasswordEdit::onTextChanged);
    // 透传文本变化信号，供外部回显用（demo 无需 findChild 内部控件）
    connect(edit_, &QLineEdit::textChanged, this, &PasswordEdit::textChanged);
    connect(toggle_btn_, &QToolButton::clicked, this, [this] { setTextVisible(!text_visible_); });

    updateStrengthIndicator(); // 初始空文本 → kWeak，3 块全灰
}

QString PasswordEdit::text() const {
    return edit_->text();
}

void PasswordEdit::setText(const QString& text) {
    // setText 会触发 textChanged → onTextChanged 重算强度，无需在此重复
    edit_->setText(text);
}

bool PasswordEdit::textVisible() const {
    return text_visible_;
}

void PasswordEdit::setTextVisible(bool visible) {
    if (text_visible_ == visible) {
        return; // 无变化，避免重复发信号
    }
    text_visible_ = visible;
    edit_->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    syncToggleText();
    emit textVisibleChanged(visible);
}

PasswordEdit::Strength PasswordEdit::strength() const {
    return strength_;
}

QString PasswordEdit::placeholderText() const {
    return edit_->placeholderText();
}

void PasswordEdit::setPlaceholderText(const QString& text) {
    if (edit_->placeholderText() == text) {
        return;
    }
    edit_->setPlaceholderText(text);
    emit placeholderTextChanged(text);
}

PasswordEdit::Strength PasswordEdit::computeStrength(const QString& text) {
    const int length = text.length();
    if (length == 0) {
        return Strength::kWeak; // 空文本安全归弱
    }

    // 统计字符种类（小写/大写/数字/符号 各一类）
    int classes = 0;
    bool has_lower = false, has_upper = false, has_digit = false, has_symbol = false;
    for (const QChar& ch : text) {
        if (ch.isLower()) {
            has_lower = true;
        } else if (ch.isUpper()) {
            has_upper = true;
        } else if (ch.isDigit()) {
            has_digit = true;
        } else {
            has_symbol = true; // 其余一律算符号
        }
    }
    classes =
        (has_lower ? 1 : 0) + (has_upper ? 1 : 0) + (has_digit ? 1 : 0) + (has_symbol ? 1 : 0);

    // 长度 < 6 或 种类数 <= 1 → 弱
    if (length < 6 || classes <= 1) {
        return Strength::kWeak;
    }
    // 种类数 == 2 → 中
    if (classes == 2) {
        return Strength::kMedium;
    }
    // 种类数 >= 3 且 长度 >= 8 → 强；否则中
    if (classes >= 3 && length >= 8) {
        return Strength::kStrong;
    }
    return Strength::kMedium;
}

QSize PasswordEdit::sizeHint() const {
    return QSize(220, 56); // edit 行 + 强度行
}

void PasswordEdit::onTextChanged(const QString& text) {
    const Strength new_strength = computeStrength(text);
    if (new_strength != strength_) {
        strength_ = new_strength;
        updateStrengthIndicator();
        emit strengthChanged(strength_);
    }
}

void PasswordEdit::updateStrengthIndicator() {
    // 弱=红亮 1 块、中=黄亮 2 块、强=绿亮 3 块，未亮=灰
    const int lit = static_cast<int>(strength_) + 1; // kWeak=1, kMedium=2, kStrong=3
    const char* on_style = kStyleWeak;
    switch (strength_) {
        case Strength::kWeak:
            on_style = kStyleWeak;
            break;
        case Strength::kMedium:
            on_style = kStyleMedium;
            break;
        case Strength::kStrong:
            on_style = kStyleStrong;
            break;
    }
    for (int i = 0; i < 3; ++i) {
        strength_labels_[i]->setStyleSheet(i < lit ? on_style : kStyleOff);
    }
}

void PasswordEdit::syncToggleText() {
    // 明文时按钮提示「隐」（点了会隐藏），密文时提示「显」
    toggle_btn_->setText(text_visible_ ? QStringLiteral("隐") : QStringLiteral("显"));
}

} // namespace AwesomeQt
