/**
 * QSqlTableModel 数据库表格视图示例
 *
 * 本示例演示 QSqlTableModel + QTableView 的完整工作流：
 * 1. Model 创建与表绑定
 * 2. EditStrategy 三种策略（本例用 OnManualSubmit）
 * 3. setFilter / setSort 过滤与排序
 * 4. insertRow / removeRow 新增与删除
 * 5. submitAll / revertAll 提交与回滚
 * 6. 自定义表头与列可见性
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QSqlTableModel;
class QTableView;
class QLineEdit;
class QHBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupModel();
    void setupView();
    void setupToolbar();
    void setupFilterBar();

    // === 操作槽函数 ===
    void addRow();
    void deleteRow();
    void saveChanges();
    void revertChanges();
    void refreshData();
    void applyFilter();
    void clearFilter();

private:
    QSqlTableModel *m_model = nullptr;
    QTableView *m_tableView = nullptr;
    QLineEdit *m_nameFilterEdit = nullptr;
    QLineEdit *m_deptFilterEdit = nullptr;
    QHBoxLayout *m_filterLayout = nullptr;
};

#endif // MAINWINDOW_H
