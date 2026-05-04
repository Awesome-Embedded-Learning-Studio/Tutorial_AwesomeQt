#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>
#include <QDebug>

// 自定义任务类，用于演示智能指针的用法
class Task : public QObject
{
    Q_OBJECT

public:
    explicit Task(const QString& name, QObject* parent = nullptr)
        : QObject(parent), m_name(name)
    {
        qDebug() << "[Task] 创建任务:" << m_name;
    }

    ~Task()
    {
        qDebug() << "[Task] 销毁任务:" << m_name;
    }

    void execute()
    {
        qDebug() << "[Task] 执行任务:" << m_name;
    }

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

private:
    QString m_name;
};

#endif // TASK_H
