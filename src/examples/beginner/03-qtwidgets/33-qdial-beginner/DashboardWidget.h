// QtWidgets 入门示例 33: QDial 旋钮控件
// 演示：setWrapping 无限旋转 vs 有边界旋转
//       setNotchesVisible 显示刻度
//       valueChanged 信号与实时反馈
//       模拟汽车仪表盘 UI

#include <QApplication>
#include <QDial>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// DashboardWidget: 模拟汽车仪表盘面板
// ============================================================================
class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 创建通用仪表旋钮组
    QGroupBox *createGauge(const QString &name, const QString &unit,
                           int minVal, int maxVal, int step,
                           bool wrapping, const QColor &color);

    /// @brief 创建温度仪表旋钮组（带颜色变化）
    QGroupBox *createTempGauge();

    /// @brief 根据温度值更新标签颜色
    void updateTempColor(int temp);

private:
    QLabel *m_tempLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_angleLabel = nullptr;
    QDial *m_angleDial = nullptr;
};
