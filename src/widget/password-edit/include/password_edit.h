/**
 * @file password_edit.h
 * @brief 密码输入控件 PasswordEdit——显隐切换 + 实时密码强度指示
 * @copyright Copyright (c) 2026 AwesomeQt
 *
 * 组合 QLineEdit（密码模式）+ QToolButton（显隐切换）+ 3 个 QLabel
 * 色块（强度指示），非自绘。强度按「长度 + 字符种类数」实时计算，
 * textChanged 触发重算并染色 + 发 strengthChanged。
 */
#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QToolButton;

namespace AwesomeQt {

/// @brief 密码输入控件：显隐切换 + 实时强度（弱/中/强）。
///
/// 内部组合 QLineEdit（EchoMode::Password）做密码框，右侧 QToolButton
/// 一键显隐，下方 3 个色块按强度染色（弱=红亮 1 块、中=黄亮 2 块、
/// 强=绿亮 3 块，未亮=灰）。强度算法见 computeStrength()。
///
/// 边界：空文本安全（算 kWeak，不崩）；显隐切换只改 echoMode 不动内容。
class PasswordEdit : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：可在 Designer / 外部直接驱动 ——
    Q_PROPERTY(bool textVisible READ textVisible WRITE setTextVisible NOTIFY textVisibleChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY
                   placeholderTextChanged)
    Q_PROPERTY(Strength strength READ strength NOTIFY strengthChanged)

  public:
    /// @brief 密码强度档位
    enum class Strength { kWeak, kMedium, kStrong };
    Q_ENUM(Strength)

    explicit PasswordEdit(QWidget* parent = nullptr);

    /// @brief 取当前输入文本（含显隐切换后的真实内容）。
    QString text() const;

    /// @brief 设置输入文本（会触发 textChanged → 重算强度）。
    void setText(const QString& text);

    /// @brief 当前是否明文显示。
    bool textVisible() const;

    /// @brief 切换明文/密文显示（同时更新切换按钮文案）。
    void setTextVisible(bool visible);

    /// @brief 当前密码强度。
    Strength strength() const;

    /// @brief 占位提示文字。
    QString placeholderText() const;

    /// @brief 设置占位提示文字。
    void setPlaceholderText(const QString& text);

    /// @brief 按长度 + 字符种类数算强度（静态，可独立测试）。
    ///
    /// 字符种类：小写 / 大写 / 数字 / 符号 各算一类。
    /// - 长度 < 6 或 种类数 <= 1 → kWeak
    /// - 种类数 == 2 → kMedium
    /// - 种类数 >= 3 且 长度 >= 8 → kStrong；否则 kMedium
    static Strength computeStrength(const QString& text);

    QSize sizeHint() const override;

  signals:
    /// @brief 输入文本变化（透传内部 QLineEdit::textChanged）。
    void textChanged(const QString& text);
    /// @brief 显隐状态变化。
    void textVisibleChanged(bool visible);
    /// @brief 占位文字变化。
    void placeholderTextChanged(const QString& text);
    /// @brief 强度变化（输入或 setText 触发）。
    void strengthChanged(Strength strength);

  private:
    /// @brief textChanged → 重算 strength_ 并刷新色块 + 发信号。
    void onTextChanged(const QString& text);

    /// @brief 按 strength_ 给 3 个色块染色（弱红/中黄/强绿，未亮灰）。
    void updateStrengthIndicator();

    /// @brief 同步切换按钮文案（显/隐）到当前可见状态。
    void syncToggleText();

    QLineEdit* edit_{nullptr};
    QToolButton* toggle_btn_{nullptr};
    QLabel* strength_labels_[3]{nullptr, nullptr, nullptr}; // 强度 3 色块
    Strength strength_{Strength::kWeak};
    bool text_visible_{false}; // 默认密文
};

} // namespace AwesomeQt
