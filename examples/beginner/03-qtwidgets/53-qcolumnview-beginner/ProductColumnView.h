// QtWidgets 入门示例 53: QColumnView 多列级联视图
// 演示：macOS Finder 列视图风格
//       setModel 与层级数据绑定（三层产品分类树）
//       updatePreviewWidget 预览面板集成
//       面包屑路径导航

#include <QColumnView>

class QLabel;
class QModelIndex;

// ============================================================================
// ProductColumnView: 带预览面板的多列级联视图
// ============================================================================
class ProductColumnView : public QColumnView
{
    Q_OBJECT

public:
    explicit ProductColumnView(QWidget *parent = nullptr);

private Q_SLOTS:
    /// @brief 选中条目变化时更新预览面板
    void handlePreviewUpdate(const QModelIndex &index);

private:
    /// @brief 显示产品详细信息
    void showProductDetail(const QString &name,
                           const QVariantMap &info);

    /// @brief 显示分类基本信息
    void showCategoryInfo(const QString &name,
                          const QModelIndex &index);

    /// @brief 计算条目在树中的深度
    int getDepth(const QModelIndex &index) const;

private:
    QLabel *m_previewTitle = nullptr;
    QLabel *m_previewBody = nullptr;
};
