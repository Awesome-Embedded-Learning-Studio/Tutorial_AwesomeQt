/// @file    unit_spin_box.cpp
/// @brief   UnitSpinBox 类实现——动态单位整数微调框。
///
/// 对应教程：进阶层 03-QtWidgets/29-QSpinBox 进阶。

#include "unit_spin_box.h"

#include <QLineEdit>
#include <QRegularExpression>

// 常量：单位换算基数
static constexpr int kBytesPerKB = 1024;
static constexpr int kBytesPerMB = 1024 * 1024;

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

UnitSpinBox::UnitSpinBox(QWidget* parent)
    : QSpinBox(parent)
{
    // 不使用 prefix/suffix——所有格式化逻辑在 textFromValue/valueFromText 中完成
    setRange(0, 100 * kBytesPerMB);  // 上限 100 MB
    setSingleStep(kBytesPerKB);      // 默认步进 1 KB
    setWrapping(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// textFromValue —— 内部值 → 显示文本
// ─────────────────────────────────────────────────────────────────────────────

QString UnitSpinBox::textFromValue(int value) const
{
    if (value >= kBytesPerMB) {
        // 显示为 MB，保留 1 位小数
        const double mb = static_cast<double>(value) / kBytesPerMB;
        return QStringLiteral("%1 MB").arg(mb, 0, 'f', 1);
    }
    if (value >= kBytesPerKB) {
        // 显示为 KB，保留 1 位小数
        const double kb = static_cast<double>(value) / kBytesPerKB;
        return QStringLiteral("%1 KB").arg(kb, 0, 'f', 1);
    }
    // 小于 1 KB，直接显示字节数
    return QStringLiteral("%1 B").arg(value);
}

// ─────────────────────────────────────────────────────────────────────────────
// valueFromText —— 显示文本 → 内部值
// ─────────────────────────────────────────────────────────────────────────────

int UnitSpinBox::valueFromText(const QString& text) const
{
    // 去掉 prefix/suffix（虽然我们没有设置，但防御性编程）
    QString cleaned = text.trimmed();
    if (!prefix().isEmpty() && cleaned.startsWith(prefix())) {
        cleaned.remove(0, prefix().length());
    }
    if (!suffix().isEmpty() && cleaned.endsWith(suffix())) {
        cleaned.chop(suffix().length());
    }
    cleaned = cleaned.trimmed();

    // 尝试匹配 MB
    if (cleaned.endsWith(QStringLiteral("MB"), Qt::CaseInsensitive)) {
        QString numPart = cleaned;
        numPart.chop(2);
        bool ok = false;
        const double val = numPart.trimmed().toDouble(&ok);
        if (ok) {
            return qRound(val * kBytesPerMB);
        }
    }

    // 尝试匹配 KB
    if (cleaned.endsWith(QStringLiteral("KB"), Qt::CaseInsensitive)) {
        QString numPart = cleaned;
        numPart.chop(2);
        bool ok = false;
        const double val = numPart.trimmed().toDouble(&ok);
        if (ok) {
            return qRound(val * kBytesPerKB);
        }
    }

    // 尝试匹配 B
    if (cleaned.endsWith(QStringLiteral("B"), Qt::CaseInsensitive)) {
        QString numPart = cleaned;
        numPart.chop(1);
        bool ok = false;
        const int val = numPart.trimmed().toInt(&ok);
        if (ok) {
            return val;
        }
    }

    // 最后尝试纯数字（当作字节）
    bool ok = false;
    const int val = cleaned.toInt(&ok);
    return ok ? val : value();
}

// ─────────────────────────────────────────────────────────────────────────────
// validate —— 输入验证
// ─────────────────────────────────────────────────────────────────────────────

QValidator::State UnitSpinBox::validate(QString& text, int& pos) const
{
    Q_UNUSED(pos)

    // 匹配格式：<数字> [空白] <可选单位 B/KB/MB>
    static const QRegularExpression kPattern(
        QStringLiteral("^\\s*(\\d+\\.?\\d*)\\s*(B|KB|MB)?\\s*$"),
        QRegularExpression::CaseInsensitiveOption);

    const auto match = kPattern.match(text);
    if (!match.hasMatch()) {
        // 完全不匹配格式，检查是否是正在输入的中间态
        static const QRegularExpression kPartialPattern(
            QStringLiteral("^\\s*\\d+\\.?\\d*$"),
            QRegularExpression::CaseInsensitiveOption);
        if (kPartialPattern.match(text).hasMatch()) {
            return QValidator::Intermediate;
        }
        return QValidator::Invalid;
    }

    // 提取数值部分并检查范围
    bool ok = false;
    const double numVal = match.captured(1).toDouble(&ok);
    if (!ok) {
        return QValidator::Invalid;
    }

    // 根据单位换算为字节数
    const QString unit = match.captured(2).toUpper();
    int bytes = 0;
    if (unit == QStringLiteral("MB")) {
        bytes = qRound(numVal * kBytesPerMB);
    } else if (unit == QStringLiteral("KB")) {
        bytes = qRound(numVal * kBytesPerKB);
    } else {
        bytes = static_cast<int>(numVal);
    }

    if (bytes >= minimum() && bytes <= maximum()) {
        return QValidator::Acceptable;
    }
    return QValidator::Intermediate;
}
