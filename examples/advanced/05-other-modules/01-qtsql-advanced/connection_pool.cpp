/// @file    connection_pool.cpp
/// @brief   ConnectionPool 类的实现。
///
/// 使用 QMutexLocker + QWaitCondition 实现线程安全的连接获取/归还。

#include "connection_pool.h"

#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

#include <cstdio>

ConnectionPool::ConnectionPool(int poolSize, const QString& dbPath, QObject* parent)
    : QObject(parent)
    , m_poolSize(poolSize)
    , m_dbPath(dbPath)
{
    // 若未指定路径，使用 SQLite 内存数据库
    QString effectivePath = m_dbPath.isEmpty() ? QStringLiteral(":memory:") : m_dbPath;

    for (int i = 0; i < m_poolSize; ++i) {
        createConnection(i, effectivePath);
    }

    std::fprintf(stderr, "[ConnectionPool] Initialized with %d connections. Database: %s\n",
                 m_poolSize, effectivePath.toUtf8().constData());
}

ConnectionPool::~ConnectionPool()
{
    // 逐个移除 QSqlDatabase 注册的连接，避免资源泄漏
    for (const QString& name : std::as_const(m_allNames)) {
        QSqlDatabase::removeDatabase(name);
    }
    std::fprintf(stderr, "[ConnectionPool] All connections closed.\n");
}

QString ConnectionPool::getConnection()
{
    QMutexLocker locker(&m_mutex);

    // 无空闲连接时阻塞等待，直到有连接被归还
    while (m_freeNames.isEmpty()) {
        m_cond.wait(&m_mutex);
    }

    // 从空闲列表头部取走一个连接名
    QString name = m_freeNames.takeFirst();
    std::fprintf(stderr, "[ConnectionPool] Connection %s checked out.\n",
                 name.toUtf8().constData());
    return name;
}

void ConnectionPool::releaseConnection(const QString& connectionName)
{
    QMutexLocker locker(&m_mutex);

    // 归还到空闲列表尾部，实现简单的 FIFO 复用
    m_freeNames.append(connectionName);
    std::fprintf(stderr, "[ConnectionPool] Connection %s returned.\n",
                 connectionName.toUtf8().constData());

    // 唤醒一个正在等待的线程
    m_cond.wakeOne();
}

int ConnectionPool::poolSize() const
{
    return m_poolSize;
}

void ConnectionPool::createConnection(int index, const QString& dbPath)
{
    // 每个连接必须拥有唯一名称，QSqlDatabase 内部以名称做注册索引
    QString connectionName = QStringLiteral("pool_conn_%1").arg(index);

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        std::fprintf(stderr, "[ConnectionPool] Failed to open connection %s: %s\n",
                     connectionName.toUtf8().constData(),
                     db.lastError().text().toUtf8().constData());
        return;
    }

    // 开启 WAL 模式以提升并发读性能（对 :memory: 数据库无副作用）
    QSqlQuery query(db);
    query.exec(QStringLiteral("PRAGMA journal_mode=WAL"));

    m_allNames.append(connectionName);
    m_freeNames.append(connectionName);
}
