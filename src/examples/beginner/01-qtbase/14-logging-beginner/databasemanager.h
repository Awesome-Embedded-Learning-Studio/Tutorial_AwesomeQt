#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QDebug>
#include <QLoggingCategory>

// 数据库模块日志类别
Q_DECLARE_LOGGING_CATEGORY(databaseLog)

/**
 * 数据库管理器类：演示不同日志级别的使用
 */
class DatabaseManager : public QObject {
    Q_OBJECT

public:
    DatabaseManager(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * 执行数据库查询
     * @param query SQL 查询语句
     */
    void executeQuery(const QString &query) {
        qCDebug(databaseLog) << "执行查询:" << query;

        if (query.isEmpty()) {
            qCWarning(databaseLog) << "查询语句为空";
            return;
        }

        if (query.contains("DROP", Qt::CaseInsensitive)) {
            qCCritical(databaseLog) << "检测到危险操作: DROP 语句";
            return;
        }

        // 模拟查询执行
        qCDebug(databaseLog) << "查询执行成功，返回 0 行";
    }

    /**
     * 连接数据库
     */
    void connect() {
        qCInfo(databaseLog) << "正在连接数据库...";

        // 模拟连接
        bool success = true;

        if (success) {
            qCInfo(databaseLog) << "数据库连接成功";
        } else {
            qCCritical(databaseLog) << "数据库连接失败";
        }
    }
};

#endif // DATABASEMANAGER_H
