// QtWidgets 入门示例 50: QTableWidget 便捷表格控件
// 演示：setRowCount / setColumnCount 设置行列数
//       setItem / item 单元格读写
//       setHorizontalHeaderLabels / setVerticalHeaderLabels 表头
//       cellChanged / cellClicked / currentCellChanged 信号

#include <QMainWindow>

class QTableWidget;
class QLabel;

// ============================================================================
// MainWindow: QTableWidget 综合演示（学生信息管理表）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 填充模拟的学生数据
    void populateStudentData();

    /// @brief 点击单元格时更新详情
    void onCellClicked(int row, int /*column*/);

    /// @brief 更新右侧详情面板
    void updateDetail(int row);

    /// @brief 更新底部统计数据
    void updateStatistics();

    /// @brief 添加学生
    void onAddStudent();

    /// @brief 删除选中学生
    void onDeleteStudent();

private:
    QTableWidget *m_tableWidget = nullptr;
    QLabel *m_detailLabel = nullptr;
    QLabel *m_statsLabel = nullptr;
    QLabel *m_editHintLabel = nullptr;
};
