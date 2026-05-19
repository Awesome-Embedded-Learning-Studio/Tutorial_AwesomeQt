/// @file    ip_address_validator.cpp
/// @brief   IpAddressValidator 类实现——IPv4 逐段验证。
///
/// 对应教程：进阶层 03-QtWidgets/22-QLineEdit 进阶。

#include "ip_address_validator.h"

#include <QStringList>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

IpAddressValidator::IpAddressValidator(QObject* parent)
    : QValidator(parent)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// 验证逻辑
// ─────────────────────────────────────────────────────────────────────────────

QValidator::State IpAddressValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos)

    // 空字符串是合法的中间状态——用户还没开始输入
    if (input.isEmpty()) {
        return Intermediate;
    }

    // 按 '.' 分割为四段
    const QStringList segments = input.split(QLatin1Char('.'));

    // 超过四段必定非法
    if (segments.size() > 4) {
        return Invalid;
    }

    // 检查连续的点号（空段）
    // 输入 "192..1.1" 中间有空段，视为非法
    for (const auto& seg : segments) {
        if (seg.isEmpty()) {
            // 允许尾部空段（用户正在输入下一段），但中间空段非法
            // 段索引 == segments.size()-1 且为空说明刚按了点号准备输入
            continue;
        }
    }

    for (int i = 0; i < segments.size(); ++i) {
        const QString& seg = segments.at(i);

        // 空段：如果是最后一段说明用户刚按了 '.'，算 Intermediate
        if (seg.isEmpty()) {
            if (i == segments.size() - 1 && segments.size() > 1) {
                continue;
            }
            return Intermediate;
        }

        // 检查每段是否全为数字
        for (int j = 0; j < seg.size(); ++j) {
            const QChar ch = seg.at(j);
            if (!ch.isDigit()) {
                return Invalid;
            }
        }

        // 检查前导零：只有 "0" 本身合法，"01"、"00" 非法
        if (seg.size() > 1 && seg.startsWith(QLatin1Char('0'))) {
            return Invalid;
        }

        // 检查数值范围 0-255
        bool ok = false;
        const int value = seg.toInt(&ok);
        if (!ok || value < 0 || value > 255) {
            return Invalid;
        }
    }

    // 四段都合法且每段非空才算 Acceptable
    if (segments.size() == 4) {
        // 确保最后一段不为空
        if (!segments.last().isEmpty()) {
            return Acceptable;
        }
    }

    // 不足四段或最后一段为空，属于 Intermediate
    return Intermediate;
}

// ─────────────────────────────────────────────────────────────────────────────
// fixup
// ─────────────────────────────────────────────────────────────────────────────

void IpAddressValidator::fixup(QString& input) const
{
    Q_UNUSED(input)
    // 本示例不需要 fixup 逻辑，QValidator 基类已提供空实现
}
