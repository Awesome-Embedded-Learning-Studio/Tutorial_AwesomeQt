// QtWidgets 入门示例 16: QAbstractSpinBox 数字输入基类
// HexSpinBox: 16 进制输入框，重写 validate / textFromValue / valueFromText

#ifndef HEX_SPIN_BOX_H
#define HEX_SPIN_BOX_H

#include <QSpinBox>
#include <QValidator>

class HexSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    explicit HexSpinBox(QWidget *parent = nullptr);

protected:
    /// @brief 验证输入是否为合法的 16 进制字符串
    QValidator::State validate(QString &text, int &pos) const override;

    /// @brief 把模型数值转换为显示文本
    QString textFromValue(int val) const override;

    /// @brief 把显示文本转换为模型数值
    int valueFromText(const QString &text) const override;
};

#endif // HEX_SPIN_BOX_H
