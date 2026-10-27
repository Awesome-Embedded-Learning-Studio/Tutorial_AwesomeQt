// QtWidgets 入门示例 38: QGroupBox 分组框
// 演示：带标题边框的控件分组容器
//       setCheckable(true) 可勾选分组框（整组启用/禁用）
//       setAlignment 标题对齐
//       嵌套布局的正确姿势

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

// ============================================================================
// GroupBoxDemoWidget: QGroupBox 综合演示窗口
// ============================================================================
class GroupBoxDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GroupBoxDemoWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

private:
    // 基本设置
    QLineEdit *m_usernameEdit = nullptr;
    QComboBox *m_langCombo = nullptr;
    QCheckBox *m_autoStartCheck = nullptr;

    // 网络配置（可勾选分组框）
    QGroupBox *m_networkGroup = nullptr;
    QLineEdit *m_proxyEdit = nullptr;
    QSpinBox *m_portSpin = nullptr;

    // 界面偏好 - 主题
    QCheckBox *m_darkModeCheck = nullptr;
    QCheckBox *m_systemThemeCheck = nullptr;

    // 界面偏好 - 字体
    QSpinBox *m_fontSizeSpin = nullptr;
    QComboBox *m_fontCombo = nullptr;

    // 状态
    QLabel *m_statusLabel = nullptr;
};
