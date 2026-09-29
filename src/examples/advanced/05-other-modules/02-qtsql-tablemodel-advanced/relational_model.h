/// @file    relational_model.h
/// @brief   封装 SQLite 数据库建表、填充样例数据与 QSqlRelationalTableModel 配置。
///
/// 对应教程：进阶层 05-其他模块/02-QSqlTableModel 高级用法。
/// 演示 QSqlRelationalTableModel + setRelation() 实现 外键映射，
/// 以及 QSqlRelationalDelegate 在 QTableView 中提供下拉选择。

#pragma once

#include <QObject>

class QSqlDatabase;
class QSqlRelationalTableModel;

/// @brief 封装关系型数据库的创建、样例数据填充与 RelationalModel 配置。
///
/// 内部使用 SQLite 内存数据库，创建三张表：
/// - departments(id, name)
/// - employees(id, name, dept_id, salary)
/// - titles(id, title)
///
/// 其中 dept_id 通过 setRelation() 映射到 departments.name，
/// 演示 Qt SQL 模块对外键关系的原生支持。
class RelationalModelSetup : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数，准备数据库环境。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit RelationalModelSetup(QObject* parent = nullptr);

    /// @brief 析构函数，关闭并移除数据库连接。
    ~RelationalModelSetup() override;

    // 禁止拷贝与赋值
    RelationalModelSetup(const RelationalModelSetup&) = delete;
    RelationalModelSetup& operator=(const RelationalModelSetup&) = delete;

    /// @brief 创建 SQLite 内存数据库及三张表，填充样例数据。
    /// @return true 表示全部成功；false 表示中途出错。
    /// @note 使用内存数据库（:memory:），进程退出后数据自动销毁，
    ///       适合教学演示，无需清理文件。
    bool createAndPopulateDatabase();

    /// @brief 配置 QSqlRelationalTableModel 并设置外键关系。
    /// @return 指向已配置好的 model 指针，调用者不拥有所有权。
    /// @note 必须在 createAndPopulateDatabase() 之后调用，
    ///       否则数据库中无表会导致 select() 失败。
    QSqlRelationalTableModel* setupRelationalModel();

    /// @brief 获取当前数据库连接引用，用于直接执行 SQL 查询。
    /// @return 数据库连接对象的引用。
    QSqlDatabase database() const;

    /// @brief 获取已配置的关系型模型指针。
    /// @return 模型指针，若尚未配置则返回 nullptr。
    QSqlRelationalTableModel* model() const;

private:
    /// @brief 创建 departments、employees、titles 三张表。
    /// @param[in] db 已打开的数据库连接。
    /// @return true 表示建表全部成功。
    /// @note 使用 IF NOT EXISTS 防止重复执行时报错。
    bool createTables(QSqlDatabase& db);

    /// @brief 向三张表中插入演示用样例数据。
    /// @param[in] db 已打开且已建表的数据库连接。
    /// @return true 表示插入全部成功。
    /// @note 3 个部门 + 6 名员工 + 4 个职级，覆盖足够的外键映射场景。
    bool insertSampleData(QSqlDatabase& db);

private:
    struct Private;              // Pimpl 惯用法，隐藏 QSqlDatabase 等实现细节
    Private* m_d;                ///< 私有实现指针
};
