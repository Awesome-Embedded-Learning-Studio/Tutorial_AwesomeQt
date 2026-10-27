/**
 * @file sqlite_browser_window.cpp
 * @brief SqliteBrowserWindow 实现——打开 db / 表列表 / 可编辑表格 / 任意 SQL 执行
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "sqlite_browser_window.h"

#include <utility>

#include <QAction>
#include <QAtomicInt>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QStatusBar>
#include <QStyle>
#include <QTableView>
#include <QToolBar>

SqliteBrowserWindow::SqliteBrowserWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("SQLite Browser");
    resize(1000, 700);

    setupCentral(); // 先建控件——slots 触发时会解引用它们
    setupActions();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    updateStatus();
}

SqliteBrowserWindow::~SqliteBrowserWindow() {
    closeDatabase();
}

// ============================================================================
// 装配
// ============================================================================
void SqliteBrowserWindow::setupCentral() {
    table_list_ = new QListWidget(this);
    table_list_->setSelectionMode(QAbstractItemView::SingleSelection);

    table_view_ = new QTableView(this);
    table_view_->setObjectName("tableView");
    query_view_ = new QTableView(this);
    query_view_->setObjectName("queryView");
    sql_edit_ = new QPlainTextEdit(this);
    sql_edit_->setPlaceholderText("Type any SQL here, then press F5 or Execute…");

    // 上区：左表列表 | 右表内容；下区：SQL 输入 + 结果
    auto* top_splitter = new QSplitter(Qt::Horizontal, this);
    top_splitter->addWidget(table_list_);
    top_splitter->addWidget(table_view_);
    top_splitter->setStretchFactor(0, 1);
    top_splitter->setStretchFactor(1, 4);

    auto* outer = new QSplitter(Qt::Vertical, this);
    outer->addWidget(top_splitter);
    // SQL 输入 + 结果：上下叠
    outer->addWidget(query_view_);
    outer->addWidget(sql_edit_);
    outer->setStretchFactor(0, 4); // 表格区最大
    outer->setStretchFactor(1, 2);
    outer->setStretchFactor(2, 1);

    setCentralWidget(outer);

    connect(table_list_, &QListWidget::currentRowChanged, this,
            &SqliteBrowserWindow::onTableSelected);
}

void SqliteBrowserWindow::setupActions() {
    open_action_ = new QAction("&Open Database…", this);
    open_action_->setShortcut(QKeySequence::Open);
    open_action_->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    connect(open_action_, &QAction::triggered, this, &SqliteBrowserWindow::onOpenDatabase);

    close_action_ = new QAction("&Close Database", this);
    close_action_->setShortcut(QKeySequence::Close);
    close_action_->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    connect(close_action_, &QAction::triggered, this, &SqliteBrowserWindow::onCloseDatabase);

    execute_action_ = new QAction("&Execute SQL", this);
    execute_action_->setShortcut(Qt::Key_F5);
    execute_action_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(execute_action_, &QAction::triggered, this, &SqliteBrowserWindow::onExecuteSql);
}

void SqliteBrowserWindow::setupMenuBar() {
    auto* file_menu = menuBar()->addMenu("&File");
    file_menu->addAction(open_action_);
    file_menu->addAction(close_action_);
    file_menu->addSeparator();
    file_menu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

    auto* sql_menu = menuBar()->addMenu("&SQL");
    sql_menu->addAction(execute_action_);

    auto* help_menu = menuBar()->addMenu("&Help");
    help_menu->addAction("&About", this, [this]() {
        QMessageBox::about(this, "About",
                           "AwesomeQt SQLite Browser\n"
                           "app/ 栏整机成品：打开 db / 浏览表 / 可编辑表格 / 任意 SQL");
    });
}

void SqliteBrowserWindow::setupToolBar() {
    auto* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->addAction(open_action_);
    tb->addAction(close_action_);
    tb->addSeparator();
    tb->addAction(execute_action_);
}

void SqliteBrowserWindow::setupStatusBar() {
    status_label_ = new QLabel("No database open");
    statusBar()->addWidget(status_label_);
}

// ============================================================================
// 打开 / 关闭 db
// ============================================================================
QString SqliteBrowserWindow::makeConnectionName(const QString& filePath) {
    // 连接名 = 文件名 + mtime + 自增序号；QSqlDatabase 连接注册表是进程级，
    // 多窗口打开 basename 相同且 mtime 同秒的文件也不会撞名互相 invalidate
    static QAtomicInt counter{0};
    return "sqlite_" + QFileInfo(filePath).fileName() + "_" +
           QString::number(QFileInfo(filePath).lastModified().toSecsSinceEpoch()) + "_" +
           QString::number(counter.fetchAndAddAcquire(1));
}

void SqliteBrowserWindow::onOpenDatabase() {
    const QString path =
        QFileDialog::getOpenFileName(this, "Open SQLite Database", QDir::homePath(),
                                     "SQLite databases (*.db *.sqlite *.sqlite3);;All Files (*)");
    if (path.isEmpty()) {
        return;
    }
    openDatabase(path);
}

bool SqliteBrowserWindow::openDatabase(const QString& filePath) {
    // 先关掉当前 db，避免两个连接同时挂着
    closeDatabase();

    connection_name_ = makeConnectionName(filePath);
    bool ok = false;
    {
        // db 副本限制在内层作用域——失败 removeDatabase 前 ref 已回落到 1，无 warning
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connection_name_);
        db.setDatabaseName(filePath);
        ok = db.open();
        if (!ok) {
            const QString msg = db.lastError().text();
            if (msg.isEmpty()) {
                QMessageBox::warning(this, "Open Database", "Failed to open database.");
            } else {
                QMessageBox::warning(this, "Open Database", "Failed to open database:\n" + msg);
            }
        }
    }
    if (!ok) {
        QSqlDatabase::removeDatabase(connection_name_);
        connection_name_.clear();
        return false;
    }

    db_file_path_ = filePath;
    refreshTableList();
    updateStatus();
    return true;
}

void SqliteBrowserWindow::onCloseDatabase() {
    closeDatabase();
    refreshTableList();
    updateStatus();
}

void SqliteBrowserWindow::closeDatabase() {
    clearTableModel(); // 必须先释放所有引用 db 的 model，否则 removeDatabase 报 connection still in
                       // use
    if (query_model_ != nullptr) {
        query_view_->setModel(nullptr);
        delete query_model_;
        query_model_ = nullptr;
    }

    if (QSqlDatabase::contains(connection_name_)) {
        // db 副本必须在内层作用域先析构，再 removeDatabase——否则 ref!=1 触发
        // "connection still in use" warning（Qt 文档明令禁止的反模式）
        {
            QSqlDatabase db = QSqlDatabase::database(connection_name_, false);
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(connection_name_);
    }

    connection_name_.clear();
    db_file_path_.clear();
}

void SqliteBrowserWindow::clearTableModel() {
    if (table_model_ != nullptr) {
        // 先提交未落库的编辑（OnFieldChange 边界：编辑器仍持焦时切表/关库可能丢最后一条）
        table_model_->submitAll();
        table_view_->setModel(nullptr); // view 先松手，再 delete model——否则 Qt 打 warning
        delete table_model_;
        table_model_ = nullptr;
    }
}

// ============================================================================
// 表列表 + 选中表填表格
// ============================================================================
void SqliteBrowserWindow::refreshTableList() {
    table_list_->blockSignals(true);
    table_list_->clear();
    clearTableModel();

    if (!QSqlDatabase::contains(connection_name_)) {
        table_list_->blockSignals(false);
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(connection_name_, false);
    if (!db.isOpen()) {
        table_list_->blockSignals(false);
        return;
    }

    // 查 sqlite_master 取表名——WHERE type='table' 排除 view/index/trigger
    QSqlQuery query(db);
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name")) {
        status_label_->setText("List tables failed: " + query.lastError().text());
        table_list_->blockSignals(false);
        return;
    }
    while (query.next()) {
        table_list_->addItem(query.value(0).toString());
    }
    table_list_->blockSignals(false);
}

void SqliteBrowserWindow::onTableSelected(int row) {
    if (row < 0 || row >= table_list_->count()) {
        clearTableModel();
        updateStatus();
        return;
    }
    const QString table = table_list_->item(row)->text();
    loadTable(table);
    updateStatus();
}

void SqliteBrowserWindow::loadTable(const QString& table) {
    clearTableModel(); // 切表前先清旧 model——避免 setTable 撞到旧 db 引用

    QSqlDatabase db = QSqlDatabase::database(connection_name_, false);
    if (!db.isOpen()) {
        return;
    }

    table_model_ = new QSqlTableModel(this, db);
    table_model_->setTable(table); // 先 setTable，再 select——别在旧表残留上 select
    table_model_->setEditStrategy(QSqlTableModel::OnFieldChange); // 单元格失焦即提交
    if (!table_model_->select()) {
        status_label_->setText("Load table failed: " + table_model_->lastError().text());
        delete table_model_;
        table_model_ = nullptr;
        return;
    }
    // SQLite 驱动 QuerySize=false，QSqlTableModel 只 prefetch 255——fetchMore 拉全量，rowCount 才准
    while (table_model_->canFetchMore()) {
        table_model_->fetchMore();
    }
    table_view_->setModel(table_model_);
}

// ============================================================================
// 任意 SQL 执行——结果用只读 QSqlQueryModel 填
// ============================================================================
void SqliteBrowserWindow::onExecuteSql() {
    const QString sql = sql_edit_->toPlainText().trimmed();
    if (sql.isEmpty()) {
        return;
    }
    if (!QSqlDatabase::contains(connection_name_)) {
        status_label_->setText("No database open");
        return;
    }
    QSqlDatabase db = QSqlDatabase::database(connection_name_, false);
    if (!db.isOpen()) {
        status_label_->setText("Database not open");
        return;
    }

    // 释放上一个结果 model
    if (query_model_ != nullptr) {
        query_view_->setModel(nullptr);
        delete query_model_;
        query_model_ = nullptr;
    }

    // 先用 QSqlQuery exec——区分「有结果集」与「INSERT/UPDATE/DDL」
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        const QSqlError err = query.lastError();
        status_label_->setText("SQL error: " + err.text());
        return;
    }

    // 无结果集（INSERT/UPDATE/DELETE/CREATE…）：报影响行数，清空结果视图
    if (!query.isSelect()) {
        const int n = query.numRowsAffected();
        status_label_->setText(QString("OK — %1 row(s) affected").arg(n));
        return;
    }

    // 有结果集：交给只读 QSqlQueryModel 渲染（Qt6 setQuery 收右值——query 已 exec，move 进去）
    query_model_ = new QSqlQueryModel(this);
    query_model_->setQuery(std::move(query));
    query_view_->setModel(query_model_);
    // SQLite 驱动 QuerySize=false，QSqlQueryModel 只 prefetch 255——fetchMore 拉全量再报真实行数
    while (query_model_->canFetchMore()) {
        query_model_->fetchMore();
    }
    const int rows = query_model_->rowCount();
    const int cols = query_model_->columnCount();
    status_label_->setText(QString("OK — %1 row(s) × %2 col(s)").arg(rows).arg(cols));
}

// ============================================================================
// 状态栏
// ============================================================================
void SqliteBrowserWindow::updateStatus() {
    if (db_file_path_.isEmpty()) {
        status_label_->setText("No database open");
        return;
    }
    const QString file_name = QFileInfo(db_file_path_).fileName();
    if (table_model_ != nullptr) {
        status_label_->setText(QString("%1  ·  table: %2  ·  %3 row(s)")
                                   .arg(file_name)
                                   .arg(table_model_->tableName())
                                   .arg(table_model_->rowCount()));
    } else {
        status_label_->setText(file_name + "  ·  no table selected");
    }
}
