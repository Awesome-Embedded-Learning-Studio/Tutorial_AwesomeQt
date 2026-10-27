/// @file    transaction_demo.cpp
/// @brief   TransactionDemo 类的实现。
///
/// 演示 QSqlDatabase 事务的 commit 与 rollback 流程。

#include "transaction_demo.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

#include <cstdio>

TransactionDemo::TransactionDemo(const QSqlDatabase& db, QObject* parent)
    : QObject(parent)
    , m_db(db)
{
}

bool TransactionDemo::createTable()
{
    // IF NOT EXISTS 保证重复调用不会报错
    QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS employees ("
        "    id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    name      TEXT    NOT NULL,"
        "    department TEXT   NOT NULL,"
        "    salary    INTEGER NOT NULL"
        ")");

    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        std::fprintf(stderr, "[TransactionDemo] createTable failed: %s\n",
                     query.lastError().text().toUtf8().constData());
        return false;
    }

    std::fprintf(stderr, "[TransactionDemo] Table 'employees' created (or already exists).\n");
    return true;
}

bool TransactionDemo::demoCommit()
{
    std::fprintf(stderr, "\n=== Demo: Transaction COMMIT ===\n");

    // 开启事务——此后所有写操作对其他连接不可见，直到 commit
    if (!m_db.transaction()) {
        std::fprintf(stderr, "[TransactionDemo] Failed to start transaction: %s\n",
                     m_db.lastError().text().toUtf8().constData());
        return false;
    }

    // 批量插入 5 条记录，均在此事务内
    insertRecord(QStringLiteral("Alice"),   QStringLiteral("Engineering"), 9500);
    insertRecord(QStringLiteral("Bob"),     QStringLiteral("Marketing"),   7800);
    insertRecord(QStringLiteral("Charlie"), QStringLiteral("Engineering"), 10200);
    insertRecord(QStringLiteral("Diana"),   QStringLiteral("HR"),           8100);
    insertRecord(QStringLiteral("Eve"),     QStringLiteral("Marketing"),   7200);

    // 提交事务——所有写入原子性生效
    if (!m_db.commit()) {
        std::fprintf(stderr, "[TransactionDemo] Commit failed: %s\n",
                     m_db.lastError().text().toUtf8().constData());
        m_db.rollback();  // 提交失败时回滚以保证一致性
        return false;
    }

    std::fprintf(stderr, "[TransactionDemo] Transaction COMMIT succeeded. 5 records saved.\n");
    return true;
}

bool TransactionDemo::demoRollback()
{
    std::fprintf(stderr, "\n=== Demo: Transaction ROLLBACK ===\n");

    if (!m_db.transaction()) {
        std::fprintf(stderr, "[TransactionDemo] Failed to start transaction: %s\n",
                     m_db.lastError().text().toUtf8().constData());
        return false;
    }

    // 在事务内插入 3 条记录
    insertRecord(QStringLiteral("Frank"), QStringLiteral("Sales"),      6500);
    insertRecord(QStringLiteral("Grace"), QStringLiteral("Sales"),      7100);
    insertRecord(QStringLiteral("Hank"),  QStringLiteral("Support"),    5800);

    // 模拟业务校验失败：发现 Hank 的薪水低于最低标准，整个批次应作废
    std::fprintf(stderr, "[TransactionDemo] Simulating business rule violation: "
                         "salary below threshold. Rolling back...\n");

    // 回滚事务——上述 3 条插入全部撤销
    if (!m_db.rollback()) {
        std::fprintf(stderr, "[TransactionDemo] Rollback failed: %s\n",
                     m_db.lastError().text().toUtf8().constData());
        return false;
    }

    std::fprintf(stderr, "[TransactionDemo] Transaction ROLLBACK succeeded. No records saved.\n");
    return true;
}

void TransactionDemo::printAllRecords()
{
    std::fprintf(stderr, "\n=== Current records in 'employees' ===\n");

    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("SELECT id, name, department, salary FROM employees ORDER BY id"))) {
        std::fprintf(stderr, "[TransactionDemo] SELECT failed: %s\n",
                     query.lastError().text().toUtf8().constData());
        return;
    }

    int count = 0;
    while (query.next()) {
        int id       = query.value(0).toInt();
        QString name = query.value(1).toString();
        QString dept = query.value(2).toString();
        int salary   = query.value(3).toInt();
        std::fprintf(stderr, "  [%d] %s | %s | $%d\n",
                     id, name.toUtf8().constData(),
                     dept.toUtf8().constData(), salary);
        ++count;
    }

    if (count == 0) {
        std::fprintf(stderr, "  (empty table)\n");
    } else {
        std::fprintf(stderr, "  Total: %d record(s).\n", count);
    }
}

bool TransactionDemo::insertRecord(const QString& name, const QString& dept, int salary)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO employees (name, department, salary) VALUES (?, ?, ?)"));
    query.addBindValue(name);
    query.addBindValue(dept);
    query.addBindValue(salary);

    if (!query.exec()) {
        std::fprintf(stderr, "[TransactionDemo] insertRecord failed for %s: %s\n",
                     name.toUtf8().constData(),
                     query.lastError().text().toUtf8().constData());
        return false;
    }
    return true;
}
