// QtWidgets 入门示例 15: QAbstractItemView 视图基类
// DemoWindow: 主演示窗口

#ifndef DEMO_WINDOW_H
#define DEMO_WINDOW_H

#include <QAbstractItemView>
#include <QWidget>

class QStandardItemModel;
class QTableView;
class QListView;
class QLabel;
class QTextEdit;

class DemoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DemoWindow(QWidget *parent = nullptr);

private:
    /// @brief 初始化数据模型
    void initModel();

    /// @brief 初始化界面
    void initUi();

    /// @brief 同时设置两个视图的选择模式
    void setSelectionModeForViews(QAbstractItemView::SelectionMode mode);

private:
    QStandardItemModel *m_model = nullptr;
    QTableView *m_tableView = nullptr;
    QListView *m_listView = nullptr;
    QLabel *m_currentLabel = nullptr;
    QLabel *m_modeInfoLabel = nullptr;
    QTextEdit *m_selectedInfo = nullptr;
};

#endif // DEMO_WINDOW_H
