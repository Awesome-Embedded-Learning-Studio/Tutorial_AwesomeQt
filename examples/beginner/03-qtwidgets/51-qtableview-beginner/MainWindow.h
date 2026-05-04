// QtWidgets 入门示例 51: QTableView Model 驱动表格视图
// 演示：QStandardItemModel 配合 QTableView
//       horizontalHeader / verticalHeader 表头控制
//       setSpan 合并单元格
//       resizeColumnsToContents 自动列宽 + QSortFilterProxyModel 过滤排序

#include <QMainWindow>

class QComboBox;
class QLineEdit;
class QLabel;
class QSortFilterProxyModel;
class QStandardItemModel;
class QTableView;

// ============================================================================
// MainWindow: QTableView 综合演示（员工管理表）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 填充模拟的员工数据
    void populateEmployeeData();

    /// @brief 对同一部门的行做合并展示
    void applySpan();

    /// @brief 搜索/筛选变更时更新 proxy model 过滤条件
    void onFilterChanged();

    /// @brief 更新底部统计数据
    void updateStatistics();

private:
    QStandardItemModel *m_model = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
    QTableView *m_tableView = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_deptCombo = nullptr;
    QLabel *m_statsLabel = nullptr;
};
