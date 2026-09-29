// QtWidgets 入门示例 16: QAbstractSpinBox 数字输入基类
// DemoWindow: 主演示窗口

#ifndef DEMO_WINDOW_H
#define DEMO_WINDOW_H

#include <QWidget>

class QDoubleSpinBox;
class QLabel;

class DemoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DemoWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 更新 16 进制信息标签
    void updateHexInfo(int value);

private:
    QDoubleSpinBox *m_lockedSpin = nullptr;
    QLabel *m_hexInfoLabel = nullptr;
    QLabel *m_vcLabel = nullptr;
    QLabel *m_efLabel = nullptr;
    int m_valueChangedCount = 0;
    int m_editingFinishedCount = 0;
};

#endif // DEMO_WINDOW_H
