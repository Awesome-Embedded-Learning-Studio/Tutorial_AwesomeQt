#include <QMainWindow>

class QLabel;
class QStandardItem;
class QStandardItemModel;

class ProductColumnView;

// ============================================================================
// MainWindow: 产品分类浏览器
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 构建三层产品分类树并填充数据
    void populateProductData();

    /// @brief 向分类节点添加一个带详情的产品条目
    void addProduct(QStandardItem *parent,
                    const QString &name,
                    const QString &category,
                    double price,
                    const QString &brand,
                    const QString &description);

    /// @brief 点击条目时更新面包屑导航
    void onItemClicked(const QModelIndex &index);

private:
    QStandardItemModel *m_model = nullptr;
    ProductColumnView *m_columnView = nullptr;
    QLabel *m_breadcrumbLabel = nullptr;
};
