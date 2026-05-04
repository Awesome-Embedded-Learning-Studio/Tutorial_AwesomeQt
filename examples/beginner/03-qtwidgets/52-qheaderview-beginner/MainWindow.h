#include <QHeaderView>
#include <QMainWindow>

class QComboBox;
class QLabel;
class QSortFilterProxyModel;
class QStandardItemModel;
class QTableView;

// ============================================================================
// MainWindow: QHeaderView 综合演示（商品清单）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 填充模拟的商品数据
    void populateProductData();

    /// @brief 应用统一的列宽策略到所有列
    void applyResizeMode(QHeaderView::ResizeMode mode);

private:
    QStandardItemModel *m_model = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
    QTableView *m_tableView = nullptr;
};
