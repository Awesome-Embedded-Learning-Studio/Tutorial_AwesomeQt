// QtWidgets 入门示例 43: QToolBox 工具箱折叠面板
// 演示：addItem / insertItem 添加面板
//       currentChanged 信号响应当前面板切换
//       setItemEnabled 禁用某个面板
//       侧边栏导航的典型应用场景

#include <QMainWindow>

class QToolBox;
class QTextEdit;

// ============================================================================
// MainWindow: QToolBox 综合演示主窗口
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 创建"显示设置"面板
    QWidget *createDisplayPage();

    /// @brief 创建"网络配置"面板
    QWidget *createNetworkPage();

    /// @brief 创建"声音设置"面板
    QWidget *createSoundPage();

    /// @brief 创建"高级选项"面板
    QWidget *createAdvancedPage();

    /// @brief 响应 QToolBox 面板切换
    void onToolBoxCurrentChanged(int index);

    /// @brief 响应"启用高级选项"复选框
    void onAdvancedCheckToggled(bool checked);

    /// @brief 保存 QToolBox 当前展开的面板索引
    void saveToolBoxState();

    /// @brief 从 QSettings 恢复面板索引
    void restoreToolBoxState();

    /// @brief 在日志区域追加一行
    void log(const QString &message);

private:
    QToolBox *m_toolbox = nullptr;
    QTextEdit *m_logArea = nullptr;
};
