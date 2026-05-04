/**
 * QtSql 数据库连接与查询基础示例
 *
 * 本示例演示 QtSql 模块的核心用法：
 * 1. QSqlDatabase 连接 SQLite 数据库
 * 2. QSqlQuery 执行 DDL 和 DML 语句
 * 3. prepare + bindValue 预编译防 SQL 注入
 * 4. QSqlError 错误处理
 * 5. 事务管理（transaction / commit / rollback）
 */

#include "Demo.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QDebug>
#include <QStringList>

Demo::Demo(QObject *parent)
    : QObject(parent)
{
}

void Demo::run()
{
    qDebug() << "========================================";
    qDebug() << "QtSql 数据库连接与查询基础示例";
    qDebug() << "========================================";

    demonstrateConnectionAndSchema();
    demonstrateBasicQueries();
    demonstratePreparedStatements();
    demonstrateErrorHandling();
    demonstrateTransaction();
    demonstrateSummary();

    // 关闭数据库连接
    QSqlDatabase db = QSqlDatabase::database();
    db.close();
    qDebug() << "\n数据库连接已关闭";

    qDebug() << "\n========================================";
    qDebug() << "要点回顾：";
    qDebug() << "  1. QSqlDatabase::addDatabase() 创建连接";
    qDebug() << "  2. QSqlQuery::exec() 执行 SQL";
    qDebug() << "  3. prepare + bindValue 防注入";
    qDebug() << "  4. lastError() 做错误处理";
    qDebug() << "  5. transaction/commit/rollback 保证原子性";
    qDebug() << "========================================";
}

// ============================================================================
// 第一部分：数据库连接与建表
// ============================================================================
void Demo::demonstrateConnectionAndSchema()
{
    qDebug() << "\n=== 1. 数据库连接与建表 ===";

    // 添加 SQLite 数据库连接（使用默认连接名）
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("company.db");

    if (!db.open()) {
        qWarning() << "数据库打开失败:" << db.lastError().text();
        return;
    }
    qDebug() << "数据库连接成功";

    // 检查 SQLite 驱动是否支持事务
    qDebug() << "驱动名称:" << db.driverName();
    qDebug() << "支持事务:" << db.driver()->hasFeature(QSqlDriver::Transactions);

    // 建表
    QSqlQuery query;
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS employees ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL,"
        "  department TEXT,"
        "  salary REAL,"
        "  hire_date TEXT"
        ")"
    );

    if (!ok) {
        qWarning() << "建表失败:" << query.lastError().text();
    } else {
        qDebug() << "employees 表就绪";
    }

    // 清空旧数据（方便重复运行示例）
    query.exec("DELETE FROM employees");
}

// ============================================================================
// 第二部分：直接执行 SQL（基础 INSERT / SELECT）
// ============================================================================
void Demo::demonstrateBasicQueries()
{
    qDebug() << "\n=== 2. 基础 SQL 操作 ===";

    QSqlQuery query;

    // 直接执行 INSERT
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Alice', 'Engineering', 85000.0, '2024-03-15')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Bob', 'Marketing', 65000.0, '2024-06-01')");
    query.exec("INSERT INTO employees (name, department, salary, hire_date) "
               "VALUES ('Carol', 'Engineering', 92000.0, '2023-11-20')");

    qDebug() << "插入 3 条记录";

    // 查询全部
    qDebug() << "\n--- 所有员工 ---";
    query.exec("SELECT id, name, department, salary FROM employees ORDER BY salary DESC");

    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        QString dept = query.value(2).toString();
        double salary = query.value(3).toDouble();
        qDebug() << "  #" << id << name << dept << salary;
    }

    // UPDATE + numRowsAffected
    query.exec("UPDATE employees SET salary = 88000.0 WHERE name = 'Alice'");
    qDebug() << "\n更新 Alice 薪资，受影响行数:" << query.numRowsAffected();

    // DELETE
    query.exec("DELETE FROM employees WHERE name = 'Bob'");
    qDebug() << "删除 Bob，受影响行数:" << query.numRowsAffected();
}

// ============================================================================
// 第三部分：预编译与参数绑定
// ============================================================================
void Demo::demonstratePreparedStatements()
{
    qDebug() << "\n=== 3. 预编译与参数绑定 ===";

    QSqlQuery query;

    // 命名占位符方式
    query.prepare(
        "INSERT INTO employees (name, department, salary, hire_date) "
        "VALUES (:name, :dept, :salary, :hire_date)"
    );

    // 绑定参数并执行（安全：自动处理转义，防止 SQL 注入）
    query.bindValue(":name", "O'Brien");   // 含单引号，bindValue 安全处理
    query.bindValue(":dept", "Engineering");
    query.bindValue(":salary", 78000.0);
    query.bindValue(":hire_date", "2024-08-10");

    if (!query.exec()) {
        qWarning() << "插入失败:" << query.lastError().text();
    } else {
        qDebug() << "命名占位符插入成功（含特殊字符 O'Brien）";
    }

    // 位置占位符方式
    query.prepare(
        "INSERT INTO employees (name, department, salary, hire_date) "
        "VALUES (?, ?, ?, ?)"
    );

    query.addBindValue("Diana");
    query.addBindValue("Marketing");
    query.addBindValue(62000.0);
    query.addBindValue("2025-01-15");

    if (!query.exec()) {
        qWarning() << "插入失败:" << query.lastError().text();
    } else {
        qDebug() << "位置占位符插入成功";
    }

    // 预编译查询
    query.prepare("SELECT name, salary FROM employees WHERE department = :dept AND salary > :min_salary");
    query.bindValue(":dept", "Engineering");
    query.bindValue(":min_salary", 75000.0);

    if (query.exec()) {
        qDebug() << "\n--- Engineering 部门薪资 > 75000 ---";
        while (query.next()) {
            qDebug() << "  " << query.value("name").toString()
                     << "salary:" << query.value("salary").toDouble();
        }
    }
}

// ============================================================================
// 第四部分：错误处理
// ============================================================================
void Demo::demonstrateErrorHandling()
{
    qDebug() << "\n=== 4. 错误处理 ===";

    QSqlQuery query;

    // 故意执行一条引用不存在表的 SQL
    if (!query.exec("SELECT * FROM nonexistent_table")) {
        QSqlError error = query.lastError();

        qDebug() << "错误类型:" << error.type();           // StatementError
        qDebug() << "错误编号:" << error.nativeErrorCode();
        qDebug() << "错误描述:" << error.text();
    }

    // 故意违反 NOT NULL 约束
    if (!query.exec("INSERT INTO employees (name) VALUES (NULL)")) {
        qDebug() << "\n约束冲突:" << query.lastError().text();
    }
}

// ============================================================================
// 第五部分：事务管理
// ============================================================================
void Demo::demonstrateTransaction()
{
    qDebug() << "\n=== 5. 事务管理（批量插入） ===";

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query;

    // 启动事务
    if (!db.transaction()) {
        qWarning() << "事务启动失败:" << db.lastError().text();
        return;
    }

    query.prepare("INSERT INTO employees (name, department, salary, hire_date) "
                  "VALUES (:name, :dept, :salary, :hire_date)");

    const int kBatchSize = 50;
    bool success = true;

    for (int i = 0; i < kBatchSize; ++i) {
        query.bindValue(":name", QString("BatchEmp_%1").arg(i, 3, 10, QChar('0')));
        query.bindValue(":dept", i % 2 == 0 ? "Sales" : "Support");
        query.bindValue(":salary", 45000.0 + i * 200);
        query.bindValue(":hire_date", "2025-02-01");

        if (!query.exec()) {
            qWarning() << "批量插入失败，第" << i << "条:" << query.lastError().text();
            success = false;
            break;
        }
    }

    if (success) {
        db.commit();
        qDebug() << "事务提交成功，批量插入" << kBatchSize << "条记录";
    } else {
        db.rollback();
        qDebug() << "事务回滚，所有插入已撤销";
    }
}

// ============================================================================
// 第六部分：综合查询演示
// ============================================================================
void Demo::demonstrateSummary()
{
    qDebug() << "\n=== 6. 综合查询 ===";

    QSqlQuery query;

    // 各部门人数统计
    query.exec("SELECT department, COUNT(*) AS cnt, AVG(salary) AS avg_salary "
               "FROM employees GROUP BY department ORDER BY avg_salary DESC");

    qDebug() << "--- 部门统计 ---";
    while (query.next()) {
        qDebug() << "  " << query.value("department").toString()
                 << "人数:" << query.value("cnt").toInt()
                 << "平均薪资:" << query.value("avg_salary").toDouble();
    }

    // 总记录数
    query.exec("SELECT COUNT(*) FROM employees");
    if (query.next()) {
        qDebug() << "\n总记录数:" << query.value(0).toInt();
    }
}
