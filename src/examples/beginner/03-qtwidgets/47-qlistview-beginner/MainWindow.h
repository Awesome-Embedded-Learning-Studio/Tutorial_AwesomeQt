// QtWidgets 入门示例 47: QListView Model 驱动列表视图
// 演示：与 QStringListModel 配合
//       setViewMode 列表/图标模式
//       setSpacing / setGridSize 图标布局
//       自定义 ItemDelegate 改变显示样式

#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QStringListModel>

#include "ColorItemDelegate.h"

// ============================================================================
// MainWindow: QListView 综合演示
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 添加新颜色到列表
    void onAddColor();

    /// @brief 选中变化时更新颜色预览
    void onSelectionChanged(const QModelIndex &current);

    /// @brief 更新信息标签
    void updateInfoLabel();

private:
    QListView *m_listView = nullptr;
    QStringListModel *m_model = nullptr;
    ColorItemDelegate *m_delegate = nullptr;
    QLineEdit *m_colorInput = nullptr;
    QLabel *m_previewLabel = nullptr;
    QLabel *m_infoLabel = nullptr;
};
