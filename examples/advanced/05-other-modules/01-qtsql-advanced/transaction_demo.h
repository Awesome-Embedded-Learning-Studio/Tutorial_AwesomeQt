/// @file    transaction_demo.h
/// @brief   数据库事务提交与回滚演示。
///
/// 对应教程：进阶层 05-其他模块/01-QtSql 高级。
/// 展示 QSqlDatabase::transaction() / commit() / rollback() 的正确用法，
/// 以及在异常路径下通过回滚保证数据一致性。

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>

/// @brief 事务演示类，展示数据库事务的提交与回滚。
///
/// 创建一张 employees 表，通过事务批量插入记录。
/// Demo 1: 正常提交——插入 5 条记录后 commit。
/// Demo 2: 故意回滚——插入 3 条记录后模拟错误并 rollback，验证数据未持久化。
class TransactionDemo : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] db 已打开的 QSqlDatabase 连接。
    /// @param[in] parent 父对象指针。
    /// @note 持有数据库引用但不拥有所有权，连接由 ConnectionPool 管理。
    explicit TransactionDemo(const QSqlDatabase& db, QObject* parent = nullptr);

    /// @brief 创建 employees 表（若不存在）。
    /// @return true 表示建表成功，false 表示执行出错。
    bool createTable();

    /// @brief 演示事务成功提交：插入 5 条记录后 commit。
    /// @return true 表示事务提交成功，false 表示中途出错。
    bool demoCommit();

    /// @brief 演示事务回滚：插入 3 条记录后故意触发错误并 rollback。
    /// @return true 表示回滚成功执行（说明数据未持久化），false 表示回滚失败。
    /// @note 回滚成功意味着数据库中不包含本次插入的记录，这正是期望行为。
    bool demoRollback();

    /// @brief 查询并打印当前 employees 表中的全部记录。
    void printAllRecords();

private:
    /// @brief 向 employees 表插入一条记录（不在事务内调用）。
    /// @param[in] name   员工姓名。
    /// @param[in] dept   部门名称。
    /// @param[in] salary 薪水。
    /// @return true 表示执行成功。
    bool insertRecord(const QString& name, const QString& dept, int salary);

    QSqlDatabase m_db;  ///< 数据库连接引用（不拥有）
};
