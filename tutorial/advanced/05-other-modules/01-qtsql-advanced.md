---
title: "5.1 QtSql 进阶：事务、连接池、ORM 封装"
description: "入门篇我们把 QSqlDatabase 的基本连接和 QSqlQuery 的增删改查跑通了。写个单线程、单连接的小工具确实够用。但工程项目和 Demo 之间的差距，就藏在「并发」和「可靠性」这两个词里。"
---

# 现代Qt开发教程（进阶篇）5.1——QtSql 进阶：事务、连接池、ORM 封装

## 1. 前言 / 从「能查数据」到「扛住并发」

入门篇我们把 QSqlDatabase 的基本连接和 QSqlQuery 的增删改查跑通了。写个单线程、单连接的小工具确实够用。但工程项目和 Demo 之间的差距，就藏在「并发」和「可靠性」这两个词里。

比如事务。入门篇提过一嘴 `transaction()` / `commit()` / `rollback()`，但没展开。在实际项目中，事务是数据一致性的最后一道防线——银行转账时「A 扣钱」和「B 加钱」必须在同一个事务里，要么都成功要么都回滚。如果你的代码里到处是裸的 `INSERT` 和 `UPDATE` 而没有事务保护，数据迟早会出问题。

然后是连接池。QSqlDatabase 的连接不是无代价的——每次 `addDatabase()` + `open()` 都要建立 TCP 连接（远程数据库）或者初始化引擎（SQLite）。在多线程环境下，你不能多线程共享同一个连接（Qt 文档明确说不安全），但给每个操作都创建新连接又太浪费。你需要一个连接池：提前创建若干连接放在池子里，谁要用就从池子里取，用完还回去。

这篇我们就一起来把事务的正确使用姿势、线程安全的连接池实现、以及轻量级 ORM 封装模式这三个工程必备技能拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。默认你已经掌握入门篇中 QSqlDatabase 和 QSqlQuery 的基本用法。本篇依赖 Qt6::Sql 模块，CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Sql)` 引入。示例使用 SQLite 作为数据库引擎（Qt 内置驱动），事务和连接池的概念对 MySQL、PostgreSQL 等远程数据库同样适用。

## 3. 核心概念讲解

### 3.1 事务——从「多条语句」到「原子操作」

事务的 ACID 属性（原子性、一致性、隔离性、持久性）是数据库理论的经典话题，我们不讲理论，只讲 Qt 中怎么正确使用。

最常见的事务场景是「一组相关的写操作必须同时成功」。比如用户注册时同时创建用户记录和初始化配置记录，这两步要么一起完成要么都不做。

```cpp
bool createUserWithConfig(QSqlDatabase &db, const QString &username,
                           const QString &email)
{
    if (!db.transaction()) {
        qDebug() << "Failed to start transaction:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    // 第一步：创建用户
    query.prepare("INSERT INTO users (username, email) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(email);
    if (!query.exec()) {
        qDebug() << "Insert user failed:" << query.lastError().text();
        db.rollback();
        return false;
    }

    // 获取新用户的 ID
    qint64 userId = query.lastInsertId().toLongLong();

    // 第二步：初始化配置
    query.prepare("INSERT INTO user_config (user_id, theme, language) "
                   "VALUES (?, 'light', 'zh_CN')");
    query.addBindValue(userId);
    if (!query.exec()) {
        qDebug() << "Insert config failed:" << query.lastError().text();
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        qDebug() << "Commit failed:" << db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}
```

这里有一个非常重要的细节：`QSqlQuery query(db)` 构造时传入了数据库连接。如果你不传，`QSqlQuery` 会使用默认连接。在多连接的环境下（比如使用了连接池），你必须显式指定连接，否则事务的 `commit()` / `rollback()` 和 `QSqlQuery` 操作可能不在同一个连接上，导致事务保护形同虚设。

还有一个容易踩的坑是事务的嵌套。SQLite 不支持嵌套事务——在 `transaction()` 未 `commit()` 之前再次调用 `transaction()` 会失败。如果你有一个通用的事务包装函数被嵌套调用，就会出问题。解决方案是使用 `SAVEPOINT`（SQL 标准的保存点）来模拟嵌套事务：

```cpp
// 模拟嵌套事务
query.exec("SAVEPOINT sp1");
// ... 操作 ...
// 提交保存点
query.exec("RELEASE SAVEPOINT sp1");
// 或者回滚到保存点
query.exec("ROLLBACK TO SAVEPOINT sp1");
```

### 3.2 连接池——多线程下的数据库连接管理

Qt 文档明确说了：QSqlDatabase 的连接只能在创建它的线程中使用。这意味着多线程环境下每个线程都需要自己的数据库连接。你不能在主线程创建一个 `QSqlDatabase` 然后在工作线程里用——结果是不可预测的，可能崩溃，可能数据错乱，可能看起来正常但在高压下出问题。

连接池的核心思路是：启动时预创建若干连接，每个线程需要数据库操作时从池子取一个，用完还回去。这样避免了频繁创建/销毁连接的开销，又保证了线程安全。

```cpp
class ConnectionPool : public QObject
{
    Q_OBJECT
public:
    static ConnectionPool &instance()
    {
        static ConnectionPool pool;
        return pool;
    }

    /// 获取一个数据库连接（线程安全）
    QSqlDatabase getConnection()
    {
        QMutexLocker locker(&mutex_);

        // 先尝试从空闲池取
        if (!freeConnections_.isEmpty()) {
            QString name = freeConnections_.dequeue();
            return QSqlDatabase::database(name);
        }

        // 空闲池为空，创建新连接
        if (totalConnections_ < kMaxConnections) {
            QString name = QString("conn_%1").arg(totalConnections_++);
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", name);
            db.setDatabaseName(dbPath_);
            if (!db.open()) {
                qDebug() << "Failed to open connection:" << db.lastError();
                return QSqlDatabase();
            }
            return db;
        }

        // 连接数已达上限，等待
        qDebug() << "Connection pool exhausted, waiting...";
        poolCondition_.wait(&mutex_, kWaitTimeoutMs);

        // 被唤醒后再试一次
        if (!freeConnections_.isEmpty()) {
            QString name = freeConnections_.dequeue();
            return QSqlDatabase::database(name);
        }

        qDebug() << "Connection pool timeout";
        return QSqlDatabase();
    }

    /// 归还连接（线程安全）
    void returnConnection(const QSqlDatabase &db)
    {
        QMutexLocker locker(&mutex_);
        freeConnections_.enqueue(db.connectionName());
        poolCondition_.wakeOne();
    }

    void setDatabasePath(const QString &path) { dbPath_ = path; }

private:
    ConnectionPool() = default;

    QMutex mutex_;
    QQueue<QString> freeConnections_;
    QWaitCondition poolCondition_;
    int totalConnections_ = 0;
    QString dbPath_;

    static constexpr int kMaxConnections = 10;
    static constexpr int kWaitTimeoutMs = 5000;
};

// RAII 包装器：自动归还连接
class ScopedConnection
{
public:
    ScopedConnection() : db_(ConnectionPool::instance().getConnection()) {}
    ~ScopedConnection()
    {
        if (db_.isOpen()) {
            ConnectionPool::instance().returnConnection(db_);
        }
    }
    QSqlDatabase &db() { return db_; }
    ScopedConnection(const ScopedConnection &) = delete;
    ScopedConnection &operator=(const ScopedConnection &) = delete;

private:
    QSqlDatabase db_;
};
```

`ScopedConnection` 是一个 RAII 包装器——构造时从池子里取连接，析构时自动归还。这比手动 `getConnection()` / `returnConnection()` 安全得多，因为函数提前 return 或异常退出时连接不会被遗漏。

使用方式：

```cpp
void doSomeWork()
{
    ScopedConnection conn;
    QSqlDatabase &db = conn.db();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.exec("SELECT * FROM users");
    // ... 处理结果 ...
    // 函数结束时 conn 析构，连接自动归还
}
```

### 3.3 轻量级 ORM 封装——减少重复的 SQL 手工劳动

QtSql 没有内置 ORM（对象关系映射），所有的 SQL 都需要手写。对于简单项目这不是问题，但当你的应用有几十张表、上百个字段，手写 `INSERT` / `UPDATE` / `SELECT` 就变成了一件又臭又长的苦差事。

我们不需要引入重量级的第三方 ORM 库——一个轻量级的封装就够了。核心思路是：用 `QMap<QString, QVariant>` 表示一行数据（字段名 → 值），然后根据操作类型自动生成 SQL。

```cpp
class TableGateway
{
public:
    TableGateway(const QString &tableName, QSqlDatabase &db)
        : table_(tableName), db_(db)
    {}

    /// 插入一行
    bool insert(const QMap<QString, QVariant> &row)
    {
        if (row.isEmpty()) return false;

        QStringList columns = row.keys();
        QStringList placeholders;
        for (int i = 0; i < columns.size(); ++i) {
            placeholders << "?";
        }

        QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
            .arg(table_, columns.join(", "), placeholders.join(", "));

        QSqlQuery query(db_);
        query.prepare(sql);
        for (const auto &col : columns) {
            query.addBindValue(row[col]);
        }

        if (!query.exec()) {
            qDebug() << "Insert failed:" << query.lastError().text();
            return false;
        }
        return true;
    }

    /// 按条件查询
    QList<QMap<QString, QVariant>> select(
        const QString &whereClause = QString(),
        const QList<QVariant> &bindValues = {})
    {
        QString sql = QString("SELECT * FROM %1").arg(table_);
        if (!whereClause.isEmpty()) {
            sql += " WHERE " + whereClause;
        }

        QSqlQuery query(db_);
        query.prepare(sql);
        for (const auto &val : bindValues) {
            query.addBindValue(val);
        }

        QList<QMap<QString, QVariant>> results;
        if (query.exec()) {
            while (query.next()) {
                QMap<QString, QVariant> row;
                QSqlRecord record = query.record();
                for (int i = 0; i < record.count(); ++i) {
                    row[record.fieldName(i)] = record.value(i);
                }
                results.append(row);
            }
        }
        return results;
    }

    /// 按条件更新
    bool update(const QMap<QString, QVariant> &data,
                const QString &whereClause,
                const QList<QVariant> &whereValues)
    {
        if (data.isEmpty()) return false;

        QStringList setClauses;
        for (const auto &col : data.keys()) {
            setClauses << QString("%1 = ?").arg(col);
        }

        QString sql = QString("UPDATE %1 SET %2 WHERE %3")
            .arg(table_, setClauses.join(", "), whereClause);

        QSqlQuery query(db_);
        query.prepare(sql);
        for (const auto &col : data.keys()) {
            query.addBindValue(data[col]);
        }
        for (const auto &val : whereValues) {
            query.addBindValue(val);
        }

        return query.exec();
    }

    /// 按条件删除
    bool remove(const QString &whereClause,
                const QList<QVariant> &bindValues)
    {
        QString sql = QString("DELETE FROM %1 WHERE %2")
            .arg(table_, whereClause);

        QSqlQuery query(db_);
        query.prepare(sql);
        for (const auto &val : bindValues) {
            query.addBindValue(val);
        }

        return query.exec();
    }

private:
    QString table_;
    QSqlDatabase &db_;
};
```

使用方式非常直观：

```cpp
ScopedConnection conn;
TableGateway users("users", conn.db());

// 插入
users.insert({{"username", "alice"}, {"email", "alice@example.com"}});

// 查询
auto results = users.select("username = ?", {"alice"});

// 更新
users.update({{"email", "new@example.com"}}, "id = ?", {1});

// 删除
users.remove("id = ?", {1});
```

这种封装覆盖了 80% 的日常 CRUD 操作，代码量极少，没有外部依赖。它的局限在于不支持关联查询（JOIN）、复杂聚合和子查询——这些场景你还是得手写 SQL。但对于单表操作来说，这种封装能帮你省下大量重复代码。

现在有一道调试题。你的应用在多线程环境中偶尔出现「database is locked」错误。代码中已经使用了连接池，每个线程都有独立连接。问题出在哪里？

最可能的原因是 SQLite 的文件锁。SQLite 在写操作时会锁定整个数据库文件——即使你有 10 个连接，同时只有一个连接能写。如果你的多个线程同时尝试写操作，后面的会拿到 `SQLITE_BUSY` 错误。解决方案有两个：第一是在连接时设置 `db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=5000")`，让 SQLite 在遇到锁时等待最多 5 秒而不是立刻报错；第二是减少事务的持有时间——不要在事务里做耗时操作。

## 4. 踩坑预防

第一个坑是 QSqlQuery 不指定连接名。在多连接环境下（连接池），`QSqlQuery` 默认使用默认连接（名字为空的那个）。如果你创建了命名连接但没有在 `QSqlQuery` 构造时传入，事务的 `commit()` 操作的是命名连接，而 `QSqlQuery` 的 `exec()` 操作的是默认连接——事务保护形同虚设。养成习惯：只要有多个连接，所有 `QSqlQuery` 构造时都显式传入 `db`。

第二个坑是连接池耗尽导致死等。`getConnection()` 中用了 `QWaitCondition::wait()`，如果所有连接都被占着不还（比如某个线程拿到了连接但卡住了），后续线程就会一直等。解决方案是设置超时（上面代码中 `kWaitTimeoutMs = 5000`），超时后返回无效连接并记录日志。

第三个坑是 SQLite 的 WAL 模式。默认的日志模式（rollback journal）在写操作时会阻塞读操作。如果你的应用有大量并发读写，应该在连接后立即执行 `PRAGMA journal_mode=WAL`，启用 Write-Ahead Logging 模式。WAL 模式下读操作不会被写操作阻塞，并发性能大幅提升。

## 5. 练习项目

练习项目：多线程日志入库系统。我们要实现一个日志收集器，多个工作线程产生日志数据，通过连接池写入 SQLite 数据库。

具体要求：5 个工作线程每秒各产生 1 条日志记录（时间戳、线程 ID、级别、消息），通过 ConnectionPool 获取连接后批量插入（每 10 条一次事务）。日志表有索引加速查询。查询接口支持按时间范围和级别过滤。完成标准：5 个线程持续写入 60 秒无错误、数据量准确（300 条）、查询接口响应正常、数据库文件未损坏。

提示几个关键点：SQLite 使用 WAL 模式 + busy timeout 配置。批量插入用事务包裹 10 条 INSERT，而不是一条一个事务。`QSqlDatabase::database(name)` 可以在任意线程获取已存在的命名连接。

## 6. 官方文档参考链接

[Qt 文档 · QSqlDatabase](https://doc.qt.io/qt-6/qsqldatabase.html) -- 数据库连接管理，包含事务和驱动配置

[Qt 文档 · QSqlQuery](https://doc.qt.io/qt-6/qsqlquery.html) -- SQL 查询执行，包含预编译语句和参数绑定

[Qt 文档 · Executing SQL Statements](https://doc.qt.io/qt-6/sql-sqlstatements.html) -- SQL 执行完整指南，包含事务和批处理示例

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。事务保证数据一致性、连接池解决多线程并发、轻量 ORM 减少重复劳动——有了这三件武器，QtSql 的工程应用就不在话下了。
