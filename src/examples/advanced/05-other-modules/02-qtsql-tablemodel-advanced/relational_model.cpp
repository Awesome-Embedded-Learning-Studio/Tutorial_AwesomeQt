/// @file    relational_model.cpp
/// @brief   RelationalModelSetup 类的实现，包含建表、填充数据与模型配置。
///
/// 对应教程：进阶层 05-其他模块/02-QSqlTableModel 高级用法。

#include "relational_model.h"

#include <memory>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRelationalTableModel>
#include <QDebug>

// ----------------------------------------------------------------------------
// Private 实现结构体，隐藏 QSqlDatabase 和 QSqlRelationalTableModel 细节
// ----------------------------------------------------------------------------
struct RelationalModelSetup::Private
{
    QSqlDatabase db;                            ///< SQLite 内存数据库连接
    QSqlRelationalTableModel* model = nullptr;  ///< 关系型表格模型
};

// ----------------------------------------------------------------------------
// 构造 / 析构
// ----------------------------------------------------------------------------
RelationalModelSetup::RelationalModelSetup(QObject* parent)
    : QObject(parent)
    , m_d(new Private())
{
    // 使用连接名 "relational_demo" 避免与其它连接冲突
    m_d->db = QSqlDatabase::addDatabase("QSQLITE", "relational_demo");
    m_d->db.setDatabaseName(":memory:");    // 内存数据库，进程退出即销毁
}

RelationalModelSetup::~RelationalModelSetup()
{
    // 必须先销毁 model，因为它持有基于该连接的 QSqlQuery
    if (m_d->model != nullptr) {
        delete m_d->model;
        m_d->model = nullptr;
    }

    // 先保存连接名，再将 QSqlDatabase 对象置为空（释放连接引用）
    const QString connName = m_d->db.connectionName();
    m_d->db = QSqlDatabase();  // 断开引用，否则 removeDatabase 会报警

    // 移除命名连接，防止同名连接冲突
    QSqlDatabase::removeDatabase(connName);

    delete m_d;
}

// ----------------------------------------------------------------------------
// 公有方法
// ----------------------------------------------------------------------------
bool RelationalModelSetup::createAndPopulateDatabase()
{
    if (!m_d->db.open()) {
        qWarning() << "[RelationalModelSetup] Failed to open database:"
                   << m_d->db.lastError().text();
        return false;
    }

    if (!createTables(m_d->db)) {
        qWarning() << "[RelationalModelSetup] Failed to create tables.";
        return false;
    }

    if (!insertSampleData(m_d->db)) {
        qWarning() << "[RelationalModelSetup] Failed to insert sample data.";
        return false;
    }

    qDebug() << "[RelationalModelSetup] Database created and populated successfully.";
    return true;
}

QSqlRelationalTableModel* RelationalModelSetup::setupRelationalModel()
{
    // 每次调用都重新创建，避免残留旧状态
    if (m_d->model != nullptr) {
        delete m_d->model;
    }

    // 使用命名连接创建 model，父子关系交给 QObject 管理
    m_d->model = new QSqlRelationalTableModel(this, m_d->db);

    // 指定操作的表为 employees
    m_d->model->setTable("employees");

    // 核心步骤：将 dept_id 列映射到 departments 表的 name 列
    // setRelation(列索引, QSqlRelation(外表名, 外表主键, 要显示的列))
    // 效果：查询时自动生成 LEFT JOIN，显示部门名称而非 ID
    m_d->model->setRelation(2, QSqlRelation("departments", "id", "name"));

    // OnManualSubmit 要求开发者显式调用 submitAll() 才会写入数据库
    // 防止用户在 QTableView 中编辑时自动提交不完整的数据
    m_d->model->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);

    if (!m_d->model->select()) {
        qWarning() << "[RelationalModelSetup] model->select() failed:"
                   << m_d->model->lastError().text();
        return nullptr;
    }

    return m_d->model;
}

QSqlDatabase RelationalModelSetup::database() const
{
    return m_d->db;
}

QSqlRelationalTableModel* RelationalModelSetup::model() const
{
    return m_d->model;
}

// ----------------------------------------------------------------------------
// 私有方法
// ----------------------------------------------------------------------------
bool RelationalModelSetup::createTables(QSqlDatabase& db)
{
    QSqlQuery query(db);

    // departments 表：部门信息，作为 employees.dept_id 的外键目标
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS departments ("
            "    id   INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    name TEXT    NOT NULL UNIQUE"
            ")")) {
        qWarning() << "[createTables] departments:" << query.lastError().text();
        return false;
    }

    // employees 表：员工信息，dept_id 引用 departments.id
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS employees ("
            "    id      INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    name    TEXT    NOT NULL,"
            "    dept_id INTEGER NOT NULL,"
            "    salary  REAL    NOT NULL,"
            "    FOREIGN KEY (dept_id) REFERENCES departments(id)"
            ")")) {
        qWarning() << "[createTables] employees:" << query.lastError().text();
        return false;
    }

    // titles 表：职级信息（本示例仅建表，不建立外键映射）
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS titles ("
            "    id    INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    title TEXT    NOT NULL UNIQUE"
            ")")) {
        qWarning() << "[createTables] titles:" << query.lastError().text();
        return false;
    }

    return true;
}

bool RelationalModelSetup::insertSampleData(QSqlDatabase& db)
{
    QSqlQuery query(db);

    // 开启事务，保证三张表数据要么全部写入，要么全部回滚
    if (!db.transaction()) {
        qWarning() << "[insertSampleData] begin transaction failed:"
                   << db.lastError().text();
        return false;
    }

    // ---- departments：3 个部门 ----
    if (!query.exec("INSERT INTO departments (name) VALUES ('Engineering')")
        || !query.exec("INSERT INTO departments (name) VALUES ('Marketing')")
        || !query.exec("INSERT INTO departments (name) VALUES ('Human Resources')")) {
        qWarning() << "[insertSampleData] departments insert failed:"
                   << query.lastError().text();
        db.rollback();
        return false;
    }

    // ---- employees：6 名员工，分布在不同部门 ----
    // dept_id: 1=Engineering, 2=Marketing, 3=Human Resources
    const QStringList employeeInserts = {
        "INSERT INTO employees (name, dept_id, salary) VALUES ('Alice',   1, 72000.0)",
        "INSERT INTO employees (name, dept_id, salary) VALUES ('Bob',     1, 65000.0)",
        "INSERT INTO employees (name, dept_id, salary) VALUES ('Charlie', 2, 58000.0)",
        "INSERT INTO employees (name, dept_id, salary) VALUES ('Diana',   2, 48000.0)",
        "INSERT INTO employees (name, dept_id, salary) VALUES ('Eve',     3, 55000.0)",
        "INSERT INTO employees (name, dept_id, salary) VALUES ('Frank',   3, 42000.0)",
    };

    for (const auto& sql : employeeInserts) {
        if (!query.exec(sql)) {
            qWarning() << "[insertSampleData] employee insert failed:"
                       << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    // ---- titles：4 个职级（本示例不参与外键映射，仅展示多表建表） ----
    const QStringList titleInserts = {
        "INSERT INTO titles (title) VALUES ('Junior Engineer')",
        "INSERT INTO titles (title) VALUES ('Senior Engineer')",
        "INSERT INTO titles (title) VALUES ('Manager')",
        "INSERT INTO titles (title) VALUES ('Director')",
    };

    for (const auto& sql : titleInserts) {
        if (!query.exec(sql)) {
            qWarning() << "[insertSampleData] title insert failed:"
                       << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        qWarning() << "[insertSampleData] commit failed:" << db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}
