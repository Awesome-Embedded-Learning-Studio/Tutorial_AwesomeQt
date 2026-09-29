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

#ifndef DEMO_H
#define DEMO_H

#include <QObject>

class Demo : public QObject
{
    Q_OBJECT

public:
    explicit Demo(QObject *parent = nullptr);
    void run();

private:
    void demonstrateConnectionAndSchema();
    void demonstrateBasicQueries();
    void demonstratePreparedStatements();
    void demonstrateErrorHandling();
    void demonstrateTransaction();
    void demonstrateSummary();
};

#endif // DEMO_H
