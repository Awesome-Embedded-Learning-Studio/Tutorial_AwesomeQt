/**
 * @file ip_edit.h
 * @brief IPv4 地址输入控件 IpEdit——4 个八位段 QLineEdit + 点分隔 + 自动跳焦 + 0-255 校验
 * @copyright Copyright (c) 2026
 */
#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QWidget>

namespace AwesomeQt {

/// @brief IPv4 地址输入控件：4 个八位段用点分隔，自动跳焦 + 0-255 校验。
///
/// 设计要点：
/// - 4 个 QLineEdit（octets_[0..3]）+ 3 个 QLabel(".") 分隔，QHBoxLayout 排开；
/// - 每段 maxLength(3) + 居中对齐 + QIntValidator(0,255) 双保险；
/// - 焦点流转三规则：满 3 位合法自动跳下段、按 '.' 跳下段（消费掉点）、段首退格跳上段；
/// - text() 拼成 "a.b.c.d"，空段补 0；setText 越界段夹到 0-255、缺段补 0、空串全清。
class IpEdit : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：placeholderHint 可被 Designer 驱动 ——
    Q_PROPERTY(QString placeholderHint READ placeholderHint WRITE setPlaceholderHint NOTIFY
                   placeholderHintChanged)

  public:
    /// @brief 八位段数量（IPv4 固定 4 段）
    static constexpr int kOctetCount = 4;

    explicit IpEdit(QWidget* parent = nullptr);

    /// @brief 取当前地址（"a.b.c.d"，空段补 0）
    QString text() const;

    /// @brief 设置地址（按点拆分填入，越界段夹 0-255、缺段补 0、空串全清）
    /// @param ip 形如 "192.168.1.1" 的 IPv4 字符串
    void setText(const QString& ip);

    /// @brief 4 段都 0-255 且非全空才算合法
    bool isValid() const;

    /// @brief 清空所有 4 段
    void clear();

    /// @brief 占位提示文字（同步下发到 4 个子 QLineEdit）
    QString placeholderHint() const;

    /// @brief 设置占位提示文字
    void setPlaceholderHint(const QString& hint);

    QSize sizeHint() const override;

  signals:
    /// @brief 任意段文本变化时发出（外部回显用）
    void textChanged(const QString& fullAddress);
    /// @brief 末段回车或整体失焦时发出（同 QLineEdit::editingFinished 语义）
    void editingFinished();
    /// @brief 占位提示文字变化时发出
    void placeholderHintChanged(const QString& hint);

  protected:
    /// @brief 拦截 '.' / BackSpace 做跨段跳焦（消费掉点号、段首退格回上段）
    void keyPressEvent(QKeyEvent* event) override;

  private:
    /// @brief 段失焦：若是末段则发 editingFinished（扩展位）
    void onOctetEditingFinished();
    /// @brief 聚焦到下一段（存在则 setFocus，返回是否成功）
    bool focusNextOctet(int from_index);
    /// @brief 聚焦到上一段并把光标移到段末（用于段首退格回跳）
    bool focusPrevOctet(int from_index);
    /// @brief 取段索引对应 QLineEdit 指针（带边界保护，越界返回 nullptr）
    QLineEdit* octet(int index) const;

    QLineEdit* octets_[kOctetCount]{}; // 4 个八位段（对象树托管，parent=this）
    QLabel* dots_[kOctetCount - 1]{};  // 3 个点分隔符
    QString placeholder_hint_;
};

} // namespace AwesomeQt
