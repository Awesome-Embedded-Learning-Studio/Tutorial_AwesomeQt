/// @file    hex_spin_box.cpp
/// @brief   HexSpinBox 类实现——十六进制微调框核心逻辑。
///
/// 对应教程：进阶层 03-QtWidgets/16-QAbstractSpinBox 基类进阶。

#include "hex_spin_box.h"

#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

HexSpinBox::HexSpinBox(QWidget* parent)
    : QAbstractSpinBox(parent)
{
    // 不使用 wrapping，靠 stepEnabled 控制箭头状态
    setWrapping(false);

    // 初始化显示文本
    lineEdit()->setText(valueToText(m_value));

    // 文本编辑完成时（按 Enter 或焦点离开）触发值的同步
    connect(lineEdit(), &QLineEdit::editingFinished, this, [this]() {
        const quintptr parsed = textToValue(lineEdit()->text());
        if (parsed != m_value) {
            m_value = parsed;
            lineEdit()->setText(valueToText(m_value));
            Q_EMIT hexValueChanged(m_value);
        }
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// 公共接口
// ─────────────────────────────────────────────────────────────────────────────

quintptr HexSpinBox::hexValue() const
{
    return m_value;
}

void HexSpinBox::setHexValue(quintptr value)
{
    const quintptr clamped = qBound(m_min, value, m_max);
    if (clamped != m_value) {
        m_value = clamped;
        lineEdit()->setText(valueToText(m_value));
        Q_EMIT hexValueChanged(m_value);
    }
}

void HexSpinBox::setMinimum(quintptr min)
{
    m_min = min;
    // 当前值可能需要钳位
    if (m_value < m_min) {
        m_value = m_min;
        lineEdit()->setText(valueToText(m_value));
    }
}

void HexSpinBox::setMaximum(quintptr max)
{
    m_max = max;
    if (m_value > m_max) {
        m_value = m_max;
        lineEdit()->setText(valueToText(m_value));
    }
}

void HexSpinBox::setRange(quintptr min, quintptr max)
{
    m_min = min;
    m_max = max;
    // 钳位当前值到新范围
    const quintptr clamped = qBound(m_min, m_value, m_max);
    if (clamped != m_value) {
        m_value = clamped;
        lineEdit()->setText(valueToText(m_value));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// validate + fixup —— 输入验证闭环
// ─────────────────────────────────────────────────────────────────────────────

QValidator::State HexSpinBox::validate(QString& text, int& pos) const
{
    Q_UNUSED(pos)

    QString cleaned = text;
    // 去掉可选的 "0x" 前缀方便后续判断
    if (cleaned.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
        cleaned = cleaned.mid(2);
    }

    // 空输入是中间态——用户正在清空准备重新输入，fixup 会补零
    if (cleaned.isEmpty()) {
        return QValidator::Intermediate;
    }

    // 只允许十六进制字符
    static const QRegularExpression kHexPattern(QStringLiteral("^[0-9a-fA-F]+$"));
    if (!kHexPattern.match(cleaned).hasMatch()) {
        // 含有非十六进制字符，直接拒绝
        return QValidator::Invalid;
    }

    // 解析数值并检查范围
    bool ok = false;
    const quintptr val = cleaned.toULongLong(&ok, 16);
    if (!ok) {
        // 数字过长溢出，属于中间态，fixup 会钳位
        return QValidator::Intermediate;
    }

    if (val >= m_min && val <= m_max) {
        return QValidator::Acceptable;
    }

    // 超出范围但可以修正——返回 Intermediate 让 fixup 有机会介入
    return QValidator::Intermediate;
}

void HexSpinBox::fixup(QString& text) const
{
    QString cleaned = text;
    if (cleaned.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
        cleaned = cleaned.mid(2);
    }

    // 去掉所有非十六进制字符
    static const QRegularExpression kNonHex(QStringLiteral("[^0-9a-fA-F]"));
    cleaned.remove(kNonHex);

    if (cleaned.isEmpty()) {
        // 空输入修正为最小值
        text = valueToText(m_min);
        return;
    }

    bool ok = false;
    quintptr val = cleaned.toULongLong(&ok, 16);
    if (!ok) {
        // 溢出时钳位到最大值
        val = m_max;
    }

    // 钳位到合法范围
    val = qBound(m_min, val, m_max);
    text = valueToText(val);
}

// ─────────────────────────────────────────────────────────────────────────────
// stepBy —— 基于光标位置的自定义步进
// ─────────────────────────────────────────────────────────────────────────────

void HexSpinBox::stepBy(int steps)
{
    // 步进单位取决于光标所在的十六进制数位
    const quintptr unit = stepUnitFromCursor();

    // 计算新值，处理无符号溢出
    quintptr newVal = m_value;
    if (steps > 0) {
        const quintptr increment = unit * static_cast<quintptr>(steps);
        // 防止溢出：如果加上增量会超过 m_max，钳位到 m_max
        if (m_max - newVal < increment) {
            newVal = m_max;
        } else {
            newVal += increment;
        }
    } else {
        const quintptr decrement = unit * static_cast<quintptr>(-steps);
        // 防止下溢：如果减去减量会低于 m_min，钳位到 m_min
        if (newVal - m_min < decrement) {
            newVal = m_min;
        } else {
            newVal -= decrement;
        }
    }

    newVal = qBound(m_min, newVal, m_max);
    if (newVal != m_value) {
        m_value = newVal;
        lineEdit()->setText(valueToText(m_value));
        Q_EMIT hexValueChanged(m_value);
    }
}

QAbstractSpinBox::StepEnabled HexSpinBox::stepEnabled() const
{
    StepEnabled flags;
    if (m_value > m_min) {
        flags |= StepDownEnabled;
    }
    if (m_value < m_max) {
        flags |= StepUpEnabled;
    }
    return flags;
}

// ─────────────────────────────────────────────────────────────────────────────
// 私有辅助
// ─────────────────────────────────────────────────────────────────────────────

quintptr HexSpinBox::textToValue(const QString& text) const
{
    QString cleaned = text;
    if (cleaned.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
        cleaned = cleaned.mid(2);
    }
    bool ok = false;
    quintptr val = cleaned.toULongLong(&ok, 16);
    if (!ok) {
        return m_value;  // 解析失败保持原值
    }
    return qBound(m_min, val, m_max);
}

QString HexSpinBox::valueToText(quintptr value) const
{
    // 使用 "0x" 前缀 + 大写十六进制，宽度根据最大值动态决定
    const int digits = (m_max > 0)
        ? qMax(1, static_cast<int>((sizeof(quintptr) * 8 - qCountLeadingZeroBits(m_max) + 3) / 4))
        : 1;
    return QStringLiteral("0x%1").arg(value, digits, 16, QLatin1Char('0')).toUpper();
}

quintptr HexSpinBox::stepUnitFromCursor() const
{
    // 计算光标位置对应的十六进制数位
    const QString text = lineEdit()->text();
    int cursorPos = lineEdit()->cursorPosition();

    // 去掉 "0x" 前缀的偏移量
    const int prefixLen = 2;  // "0x"
    if (cursorPos <= prefixLen) {
        // 光标在 "0x" 上，默认步进最高位
        const int hexDigits = text.length() - prefixLen;
        return static_cast<quintptr>(1) << (4 * qMax(0, hexDigits - 1));
    }

    // 光标在十六进制数位上，从右往左计算权重
    const int hexPos = cursorPos - prefixLen;  // 光标右边的十六进制位数
    const int hexDigits = text.length() - prefixLen;
    const int digitIndex = hexDigits - hexPos;  // 从右往左第几位（0-indexed）

    // digitIndex=0 时在最右边，步进单位=1（0x1）
    return static_cast<quintptr>(1) << (4 * qBound(0, digitIndex, static_cast<int>(sizeof(quintptr) * 2 - 1)));
}
