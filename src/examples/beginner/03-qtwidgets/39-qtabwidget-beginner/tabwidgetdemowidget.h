// QtWidgets 入门示例 39: QTabWidget 标签页控件
// 演示：addTab / insertTab / removeTab 动态管理标签页
//       setTabPosition 上/下/左/右 与 setTabShape
//       currentChanged 信号响应标签切换
//       setTabIcon / setTabToolTip 标签美化

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QTabWidget>
#include <QWidget>

// ============================================================================
// TabWidgetDemoWidget: QTabWidget 综合演示窗口
// ============================================================================
class TabWidgetDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabWidgetDemoWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 创建"常规设置"页面
    QWidget *createGeneralPage();

    /// @brief 创建"网络设置"页面
    QWidget *createNetworkPage();

    /// @brief 创建"外观设置"页面
    QWidget *createAppearancePage();

    /// @brief 创建"高级设置"页面
    QWidget *createAdvancedPage();

private:
    QTabWidget *m_tabWidget = nullptr;
    QComboBox *m_positionCombo = nullptr;
    QComboBox *m_shapeCombo = nullptr;
    QCheckBox *m_closableCheck = nullptr;
    QCheckBox *m_movableCheck = nullptr;
    QCheckBox *m_unlockCheck = nullptr;
    QLabel *m_statusLabel = nullptr;
};
