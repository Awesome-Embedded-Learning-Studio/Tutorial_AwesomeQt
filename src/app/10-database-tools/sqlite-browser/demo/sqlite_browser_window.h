/**
 * @file sqlite_browser_window.h
 * @brief SQLite 浏览器主窗口——打开 db / 表列表 / 可编辑表格 / 任意 SQL 执行
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>
#include <QString>

class QPlainTextEdit;
class QSqlDatabase;
class QSqlTableModel;
class QSqlQueryModel;
class QTableView;
class QListWidget;
class QLabel;
class QAction;

/// @brief SQLite 数据库浏览器（app 栏整机范式）。
///
/// QMainWindow + 菜单/工具栏/状态栏，中央区左 QListWidget 列表（表名）/ 右 QTableView
/// （当前表内容，OnFieldChange 可编辑）/ 底 QPlainTextEdit（任意 SQL 输入 + Execute）。
///
/// 关键设计：①每个 db 用「命名连接」（QSqlDatabase::addDatabase("QSQLITE", name)），
/// 不污染默认连接；②切表前先把旧 model 释放（view 设 nullptr 再 delete）再 setTable，
/// 避免旧数据残留 / connection still in use；③自定义 SQL 走只读 QSqlQueryModel，与
/// OnFieldChange 可编辑的 QSqlTableModel 区分。
class SqliteBrowserWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit SqliteBrowserWindow(QWidget* parent = nullptr);
    ~SqliteBrowserWindow() override;

    /// 打开指定路径的 SQLite db（不走文件对话框——供程序化打开 / 测试 harness 复用）。
    /// 成功返回 true，失败返回 false 并把错误写进状态栏。
    bool openDatabase(const QString& filePath);

  private slots:
    void onOpenDatabase();
    void onCloseDatabase();
    void onTableSelected(int row);
    void onExecuteSql();

  private:
    void setupActions();
    void setupMenuBar();
    void setupToolBar();
    void setupCentral();
    void setupStatusBar();

    void refreshTableList();
    void loadTable(const QString& table);
    void clearTableModel(); // 切表/关 db 前：view model 设 nullptr 再 delete model
    void closeDatabase();   // 释放 model → db.close → removeDatabase(连接名)
    void updateStatus();

    static QString makeConnectionName(const QString& filePath);

    QLabel* status_label_{nullptr};
    QListWidget* table_list_{nullptr};
    QTableView* table_view_{nullptr}; // 表内容（OnFieldChange 可编辑）
    QTableView* query_view_{nullptr}; // 任意 SQL 结果（只读）
    QPlainTextEdit* sql_edit_{nullptr};

    QAction* open_action_{nullptr};
    QAction* close_action_{nullptr};
    QAction* execute_action_{nullptr};

    QString connection_name_; // 命名连接——关 db 时按名 remove
    QString db_file_path_;    // 当前 db 文件（状态栏显示）
    QSqlTableModel* table_model_{nullptr};
    QSqlQueryModel* query_model_{nullptr};
};
