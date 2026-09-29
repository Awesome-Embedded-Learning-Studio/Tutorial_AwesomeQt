// QtWidgets 入门示例 41: QStackedWidget 堆叠页面控件
// 演示：addWidget 添加页面 / setCurrentIndex 切换页面
//       与 QComboBox / QListWidget 组合做导航菜单
//       currentChanged 信号响应页面切换
//       区别于 QTabWidget（无标签头，适合自定义导航）

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// StackedWidgetDemoWidget: QStackedWidget 综合演示窗口
// ============================================================================
class StackedWidgetDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StackedWidgetDemoWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 创建"个人信息"页面
    QWidget *createPersonalPage();

    /// @brief 创建"网络设置"页面
    QWidget *createNetworkPage();

    /// @brief 创建"外观偏好"页面
    QWidget *createAppearancePage();

    /// @brief 创建"快捷键"页面
    QWidget *createShortcutPage();

    /// @brief 创建"关于"页面
    QWidget *createAboutPage();

    /// @brief 根据当前页面更新窗口标题
    void updateWindowTitle(int index);

    /// @brief 更新底部状态栏
    void updateStatus(int index);

private:
    QListWidget *m_navList = nullptr;
    QComboBox *m_jumpCombo = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    QLabel *m_statusLabel = nullptr;
};
