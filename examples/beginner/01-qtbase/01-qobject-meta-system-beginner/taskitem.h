#ifndef TASKITEM_H
#define TASKITEM_H

#include <QObject>
#include <QString>
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>

class TaskItem : public QObject
{
    Q_OBJECT

public:
    explicit TaskItem(const QString &title, int priority, QObject *parent = nullptr)
        : QObject(parent), m_title(title), m_priority(priority), m_completed(false)
    {
        qDebug() << "创建 TaskItem:" << m_title << "(优先级:" << m_priority << ")";
    }

    ~TaskItem() override
    {
        qDebug() << "销毁 TaskItem:" << m_title;
    }

    // 声明多个属性，演示完整的属性系统
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(bool completed READ completed WRITE setCompleted NOTIFY completedChanged)

    // Getter 和 Setter 函数
    QString title() const { return m_title; }
    void setTitle(const QString &title)
    {
        if (m_title != title) {
            m_title = title;
            emit titleChanged();
        }
    }

    int priority() const { return m_priority; }
    void setPriority(int priority)
    {
        if (m_priority != priority) {
            m_priority = priority;
            emit priorityChanged();
        }
    }

    bool completed() const { return m_completed; }
    void setCompleted(bool completed)
    {
        if (m_completed != completed) {
            m_completed = completed;
            emit completedChanged();
        }
    }

    // 演示如何通过元对象系统打印属性信息
    void printProperties() const
    {
        const QMetaObject *meta = metaObject();  // 获取元对象
        qDebug() << "=== " << m_title << " 的属性 ===";
        // 遍历所有属性
        for (int i = 0; i < meta->propertyCount(); ++i) {
            QMetaProperty prop = meta->property(i);
            if (prop.isReadable()) {  // 跳过不可读属性
                qDebug() << prop.name() << "=" << prop.read(this);
            }
        }
        qDebug() << "====================";
    }

signals:
    void titleChanged();
    void priorityChanged();
    void completedChanged();

private:
    QString m_title;
    int m_priority;
    bool m_completed;
};

#endif // TASKITEM_H
