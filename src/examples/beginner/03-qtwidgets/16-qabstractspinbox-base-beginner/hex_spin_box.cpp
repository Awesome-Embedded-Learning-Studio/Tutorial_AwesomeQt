// QtWidgets 入门示例 16: QAbstractSpinBox 数字输入基类
// HexSpinBox: 16 进制输入框，重写 validate / textFromValue / valueFromText

#include "hex_spin_box.h"

HexSpinBox::HexSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    setRange(0x00, 0xFF);
    setPrefix("0x");
    setDisplayIntegerBase(16);
}

/// @brief 验证输入是否为合法的 16 进制字符串
QValidator::State HexSpinBox::validate(QString &text, int &pos) const
{
    Q_UNUSED(pos)

    QString hexText = text;
    if (hexText.startsWith("0x", Qt::CaseInsensitive)) {
        hexText = hexText.mid(2);
    }

    if (hexText.isEmpty()) {
        return QValidator::Intermediate;
    }

    for (const QChar &c : hexText) {
        if (!(c.isDigit()
              || (c >= 'A' && c <= 'F')
              || (c >= 'a' && c <= 'f'))) {
            return QValidator::Invalid;
        }
    }

    bool ok = false;
    int value = hexText.toInt(&ok, 16);
    if (!ok) {
        return QValidator::Invalid;
    }
    if (value >= minimum() && value <= maximum()) {
        return QValidator::Acceptable;
    }
    return QValidator::Intermediate;
}

/// @brief 把模型数值转换为显示文本
QString HexSpinBox::textFromValue(int val) const
{
    return "0x" + QString::number(val, 16).toUpper().rightJustified(2, '0');
}

/// @brief 把显示文本转换为模型数值
int HexSpinBox::valueFromText(const QString &text) const
{
    QString hexText = text;
    if (hexText.startsWith("0x", Qt::CaseInsensitive)) {
        hexText = hexText.mid(2);
    }
    return hexText.toInt(nullptr, 16);
}
