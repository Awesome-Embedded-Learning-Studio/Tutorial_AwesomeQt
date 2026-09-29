// QtWidgets 入门示例 48: QTreeWidget 便捷树形控件
// 演示：QTreeWidgetItem 构建层级树结构
//       addTopLevelItem / insertChild 增删节点
//       setColumnCount / setHeaderLabels 多列树表
//       itemExpanded / itemCollapsed / itemClicked 信号

#include <QMainWindow>

class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class QLineEdit;

// ============================================================================
// MainWindow: QTreeWidget 综合演示（项目文件浏览器）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 填充模拟的项目目录树
    void populateProjectTree();

    /// @brief 在父节点下创建子节点
    QTreeWidgetItem *createChild(QTreeWidgetItem *parent,
                                 const QString &name,
                                 const QString &type,
                                 const QString &size);

    /// @brief 获取节点的完整路径（从根到当前节点）
    QString getNodePath(QTreeWidgetItem *item) const;

    /// @brief 选中节点变化
    void onCurrentItemChanged(QTreeWidgetItem *current,
                               QTreeWidgetItem * /*previous*/);

    /// @brief 双击节点——弹出对话框修改名称
    void onItemDoubleClicked(QTreeWidgetItem *item, int /*column*/);

    /// @brief 添加节点（文件夹或文件）
    void onAddNode(const QString &nodeType);

    /// @brief 删除选中的节点
    void onDeleteNode();

private:
    QTreeWidget *m_treeWidget = nullptr;
    QLabel *m_pathLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLineEdit *m_nameInput = nullptr;
};
