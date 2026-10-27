// QtWidgets 入门示例 29: QSpinBox / QDoubleSpinBox 数字步进框
// 演示：setRange / setSingleStep / setPrefix / setSuffix
//       setValue / value() 取值与设值
//       QDoubleSpinBox::setDecimals() 控制小数位
//       valueChanged(int/double) 信号响应

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// ImageExportPanel: 图像导出设置面板
// 覆盖 QSpinBox 和 QDoubleSpinBox 的核心用法
// ============================================================================
class ImageExportPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ImageExportPanel(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 宽度变化时的联动处理
    void onWidthChanged(int width);

    /// @brief 高度变化时的联动处理
    void onHeightChanged(int height);

    /// @brief 锁定宽高比状态切换
    void onLockRatioToggled(bool checked);

    /// @brief 其他设置变化
    void onSettingsChanged();

    /// @brief 更新摘要显示
    void updateSummary();

    /// @brief 追加一条操作日志
    void appendLog(const QString &message);

private:
    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
    QCheckBox *m_lockRatioCheck = nullptr;

    QSpinBox *m_dpiSpin = nullptr;
    QDoubleSpinBox *m_scaleSpin = nullptr;
    QSpinBox *m_qualitySpin = nullptr;

    QLabel *m_summaryLabel = nullptr;
    QPlainTextEdit *m_logEdit = nullptr;

    double m_aspectRatio = 16.0 / 9.0;
};
