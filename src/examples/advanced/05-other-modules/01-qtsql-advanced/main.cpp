/// @file    main.cpp
/// @brief   QtSql 高级示例程序入口。
///
/// 对应教程：进阶层 05-其他模块/01-QtSql 高级。
/// 演示连接池获取/归还、事务提交、事务回滚三种场景，最后打印结果并退出。

#include "connection_pool.h"
#include "transaction_demo.h"

#include <QCoreApplication>
#include <QSqlDatabase>

#include <cstdio>

/// @brief 演示连接池的基本获取与归还操作。
/// @param[in] pool 已初始化的连接池指针。
/// @note 连接池使用 :memory: 数据库时，所有连接共享同一个内存数据库实例，
///       这简化了演示——事务回滚后的状态对所有连接立即可见。
static void demoConnectionPool(ConnectionPool* pool)
{
    std::fprintf(stderr, "\n========================================\n");
    std::fprintf(stderr, "=== Demo 1: Connection Pool          ===\n");
    std::fprintf(stderr, "========================================\n");

    // 从池中借出两个连接
    QString conn1 = pool->getConnection();
    QString conn2 = pool->getConnection();

    std::fprintf(stderr, "  Acquired: %s and %s\n",
                 conn1.toUtf8().constData(), conn2.toUtf8().constData());
    std::fprintf(stderr, "  Free connections remaining: %d\n",
                 pool->poolSize() - 2);

    // 归还连接，让其他线程可以复用
    pool->releaseConnection(conn1);
    pool->releaseConnection(conn2);

    std::fprintf(stderr, "  Both connections returned to pool.\n");
}

/// @brief 演示事务提交：插入 5 条记录后 commit。
/// @param[in] db 已打开的数据库连接。
static void demoTransactionCommit(const QSqlDatabase& db)
{
    std::fprintf(stderr, "\n========================================\n");
    std::fprintf(stderr, "=== Demo 2: Transaction COMMIT       ===\n");
    std::fprintf(stderr, "========================================\n");

    TransactionDemo demo(db);
    demo.createTable();
    demo.demoCommit();
    demo.printAllRecords();
}

/// @brief 演示事务回滚：插入 3 条记录后模拟错误并 rollback。
/// @param[in] db 已打开的数据库连接。
static void demoTransactionRollback(const QSqlDatabase& db)
{
    std::fprintf(stderr, "\n========================================\n");
    std::fprintf(stderr, "=== Demo 3: Transaction ROLLBACK     ===\n");
    std::fprintf(stderr, "========================================\n");

    TransactionDemo demo(db);
    demo.demoRollback();
    demo.printAllRecords();

    std::fprintf(stderr, "\n[Verification] After rollback, the table should still contain"
                         " only the 5 records from Demo 2.\n");
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 连接池使用 SQLite 内存数据库——所有连接共享同一实例
    ConnectionPool pool;

    // --- Demo 1: 连接池获取与归还 ---
    demoConnectionPool(&pool);

    // 从池中取出一个连接，用于后续事务演示
    QString connectionName = pool.getConnection();
    QSqlDatabase db = QSqlDatabase::database(connectionName);

    // --- Demo 2: 事务提交 ---
    demoTransactionCommit(db);

    // --- Demo 3: 事务回滚 ---
    demoTransactionRollback(db);

    // 归还连接
    pool.releaseConnection(connectionName);

    std::fprintf(stderr, "\n=== All demos completed successfully. ===\n");

    return 0;
}
