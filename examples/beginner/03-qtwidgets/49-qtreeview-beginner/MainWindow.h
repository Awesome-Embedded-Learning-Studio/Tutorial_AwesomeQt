// QtWidgets 入门示例 49: QTreeView Model 驱动树视图
// 演示：与 QStandardItemModel 配合树结构展示
//       QFileSystemModel + QTreeView 文件树示例
//       setRootIndex 设置显示根节点
//       expand / collapse / expandAll 节点展开控制

#include <QMainWindow>

class QFileSystemModel;
class QTreeView;
class QLineEdit;
class QLabel;

// ============================================================================
// MainWindow: QTreeView 综合演示（文件浏览器）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 点击文件/文件夹时更新信息面板
    void onItemClicked(const QModelIndex &index);

    /// @brief 路径导航——跳转到用户输入的目录
    void onNavigate();

private:
    QTreeView *m_treeView = nullptr;
    QFileSystemModel *m_fsModel = nullptr;
    QLineEdit *m_pathInput = nullptr;
    QLabel *m_infoLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
};
