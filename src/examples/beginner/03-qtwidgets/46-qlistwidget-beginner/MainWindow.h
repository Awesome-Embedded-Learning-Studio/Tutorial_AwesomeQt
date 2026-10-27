// QtWidgets 入门示例 46: QListWidget 便捷列表控件
// 演示：addItem / addItems / insertItem 添加条目
//       currentItem / selectedItems 获取选中
//       QListWidgetItem 图标/复选框/自定义数据
//       itemDoubleClicked / itemChanged 信号

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMainWindow>

/// @brief 自定义数据角色：任务优先级
constexpr int kPriorityRole = Qt::UserRole;

// ============================================================================
// MainWindow: QListWidget 综合演示（待办任务管理器）
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 添加一条任务到列表
    void addTask(const QString &name, int priority = 3);

    /// @brief 根据优先级设置条目样式
    void applyPriorityStyle(QListWidgetItem *item, int priority);

    /// @brief 点击"添加任务"按钮或回车
    void onAddTask();

    /// @brief 删除选中的任务条目
    void onDeleteSelected();

    /// @brief 清除所有已完成的任务
    void onClearDone();

    /// @brief 条目数据变化（复选框切换等）—— 更新完成样式
    void onItemChanged(QListWidgetItem *item);

    /// @brief 双击条目—— 弹出对话框修改任务名称
    void onItemDoubleClicked(QListWidgetItem *item);

    /// @brief 右键上下文菜单
    void onContextMenu(const QPoint &pos);

    /// @brief 更新底部状态文字
    void updateStatusText();

private:
    QListWidget *m_taskList = nullptr;
    QLineEdit *m_taskInput = nullptr;
    QLabel *m_statusLabel = nullptr;
};
