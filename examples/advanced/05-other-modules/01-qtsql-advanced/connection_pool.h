/// @file    connection_pool.h
/// @brief   线程安全的 SQLite 数据库连接池。
///
/// 对应教程：进阶层 05-其他模块/01-QtSql 高级。
/// 演示 QMutex + QWaitCondition 实现连接池的获取与归还。

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

#include <QMutex>
#include <QWaitCondition>

/// @brief 线程安全的数据库连接池，管理若干具名 QSqlDatabase 连接。
///
/// 连接池在初始化时预创建固定数量的 QSqlDatabase 连接（各自拥有唯一连接名），
/// 调用方通过 getConnection() 获取空闲连接，使用完毕后通过 releaseConnection() 归还。
/// 当池中无空闲连接时，getConnection() 会阻塞等待直到有连接被归还。
class ConnectionPool : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建指定数量的数据库连接。
    /// @param[in] poolSize 连接池容量，默认 5。
    /// @param[in] dbPath   SQLite 数据库文件路径，传空则使用 :memory: 模式。
    /// @param[in] parent   父对象指针，Qt 对象树管理生命周期。
    /// @note SQLite 单文件写锁特性决定了并发写入场景下池大小不宜过大。
    explicit ConnectionPool(int poolSize = kDefaultPoolSize,
                            const QString& dbPath = QString(),
                            QObject* parent = nullptr);

    /// @brief 析构函数，关闭所有数据库连接并清理资源。
    ~ConnectionPool() override;

    // 禁止拷贝和赋值——连接池是全局唯一的资源管理器
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    /// @brief 从池中获取一个空闲的数据库连接名。
    /// @return 空闲连接的名称字符串，可传给 QSqlDatabase::database()。
    /// @note 若池中暂无空闲连接，当前线程会阻塞直到其他线程归还连接。
    QString getConnection();

    /// @brief 将连接归还到池中，供其他线程复用。
    /// @param[in] connectionName 之前通过 getConnection() 获取的连接名。
    /// @note 归还后若有线程正在等待，会自动唤醒其中一个。
    void releaseConnection(const QString& connectionName);

    /// @brief 获取连接池的总容量。
    /// @return 池中连接总数（含已借出与空闲）。
    int poolSize() const;

private:
    /// @brief 创建单个 QSqlDatabase 连接并加入空闲列表。
    /// @param[in] index 连接序号，用于构造唯一连接名。
    /// @param[in] dbPath 数据库文件路径。
    void createConnection(int index, const QString& dbPath);

    int m_poolSize;              ///< 连接池总容量
    QStringList m_freeNames;     ///< 当前空闲的连接名列表
    QStringList m_allNames;      ///< 所有已创建的连接名（析构时逐个关闭）
    QMutex m_mutex;              ///< 保护 m_freeNames 的互斥锁
    QWaitCondition m_cond;       ///< 空闲连接可用时的条件变量
    QString m_dbPath;            ///< SQLite 数据库路径

    static constexpr int kDefaultPoolSize = 5;  ///< 默认连接池大小
};
