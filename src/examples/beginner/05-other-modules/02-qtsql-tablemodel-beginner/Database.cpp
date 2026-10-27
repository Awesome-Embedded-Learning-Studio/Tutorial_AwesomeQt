#include "Database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ============================================================================
// 初始化数据库：建表 + 插入测试数据
// ============================================================================
bool initDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");  // 使用内存数据库，程序退出后自动释放

    if (!db.open()) {
        qWarning() << "数据库打开失败:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;
    query.exec(
        "CREATE TABLE employees ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL,"
        "  department TEXT,"
        "  salary REAL,"
        "  hire_date TEXT"
        ")"
    );

    // 插入测试数据
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Alice', 'Engineering', 85000.0, '2023-03-15')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Bob', 'Marketing', 65000.0, '2023-06-01')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Carol', 'Engineering', 92000.0, '2022-11-20')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Diana', 'Marketing', 71000.0, '2024-01-10')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Eve', 'Engineering', 78000.0, '2024-05-08')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Frank', 'Sales', 58000.0, '2023-09-22')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Grace', 'Sales', 63000.0, '2024-02-14')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Henry', 'Engineering', 95000.0, '2022-07-01')");

    qDebug() << "数据库初始化完成，插入 8 条测试数据";
    return true;
}
