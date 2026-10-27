// QtWidgets 入门示例 42: QSplitter 可拖动分割容器
// 演示：水平/垂直分割与嵌套
//       setSizes / sizes 程序化控制各区域宽度
//       setCollapsible 禁止折叠特定区域
//       saveState / restoreState 持久化分割比例

#include <QMainWindow>

class QSplitter;
class QTreeWidget;
class QTextEdit;

// ============================================================================
// MainWindow: QSplitter 综合演示主窗口（模拟 IDE 三区布局）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    /// @brief 初始化界面：外层水平分割 + 内层垂直分割
    void initUi();

    /// @brief 填充示例文件树
    void populateSampleTree();

    /// @brief 重置分割器到默认布局
    void resetToDefaultLayout();

    /// @brief 打印当前各分割器的 sizes() 值
    void showCurrentSizes();

    /// @brief 保存分割器状态到 QSettings
    void saveSplitterState();

    /// @brief 从 QSettings 恢复分割器状态
    void restoreSplitterState();

    /// @brief 在输出面板中追加一行日志
    void log(const QString &message);

private:
    QSplitter *m_outerSplitter = nullptr;
    QSplitter *m_innerSplitter = nullptr;
    QTreeWidget *m_treeWidget = nullptr;
    QTextEdit *m_editor = nullptr;
    QTextEdit *m_outputPanel = nullptr;
};
