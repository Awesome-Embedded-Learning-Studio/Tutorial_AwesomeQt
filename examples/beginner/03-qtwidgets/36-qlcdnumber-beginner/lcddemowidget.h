// QtWidgets 入门示例 36: QLCDNumber 液晶数字显示
// 演示：display(int/double/QString) 三种显示方式
//       setDigitCount 位数控制 + setSmallDecimalPoint 小数点策略
//       setMode 十进制/十六进制/八进制/二进制切换
//       QTimer + QLCDNumber 仪表盘计时器

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// LcdDemoWidget: QLCDNumber 综合演示窗口
// ============================================================================
class LcdDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LcdDemoWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 根据滑块值和当前进制更新 LCD 显示
    void updateLcdFromSlider(int value);

    /// @brief 启动计时器
    void startTimer();

    /// @brief 停止计时器
    void stopTimer();

    /// @brief 清除 overflow 状态标签
    void clearOverflowStatus();

private:
    QLCDNumber *m_mainLcd = nullptr;
    QSlider *m_slider = nullptr;
    QLabel *m_sliderValueLabel = nullptr;
    QComboBox *m_modeCombo = nullptr;
    QDoubleSpinBox *m_doubleSpin = nullptr;
    QPushButton *m_smallDecimalBtn = nullptr;
    QPushButton *m_timerBtn = nullptr;
    QPushButton *m_resetBtn = nullptr;
    QLabel *m_statusLabel = nullptr;
    QTimer *m_timer = nullptr;

    int m_elapsedSeconds = 0;
    bool m_timerRunning = false;
};
