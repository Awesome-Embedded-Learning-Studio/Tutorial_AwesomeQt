// QtWidgets 入门示例 45: QFrame 作为分隔线的用法
// 演示：水平分隔线 HLine+Sunken
//       垂直分隔线在工具栏中使用
//       QFrame 作为有边框容器配置
//       QFrame vs addSpacing 区别

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// 工具函数：创建一条水平分隔线 (HLine + Sunken)
// ============================================================================
QFrame *createHSeparator(int height = 2);

// ============================================================================
// 工具函数：创建一条垂直分隔线 (VLine + Sunken)
// ============================================================================
QFrame *createVSeparator(int width = 2);

// ============================================================================
// 工具函数：创建带标题的水平分隔线 (线 - 文字 - 线)
// ============================================================================
QLayout *createTitleSeparator(const QString &title);

// ============================================================================
// 工具函数：创建一个带边框的容器 QFrame
// ============================================================================
QFrame *createBorderedContainer();

// ============================================================================
// MainWindow: QFrame 分隔线综合演示
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化工具栏（含 VLine 分隔按钮组）
    void initToolbar();

    /// @brief 初始化中央控件（含 HLine 分隔区域、容器 QFrame、右侧 VLine）
    void initCentralWidget();
};
