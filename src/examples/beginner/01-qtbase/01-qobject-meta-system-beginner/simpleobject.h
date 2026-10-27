#ifndef SIMPLEOBJECT_H
#define SIMPLEOBJECT_H

#include <QObject>
#include <QString>
#include <QDebug>

class SimpleObject : public QObject
{
    Q_OBJECT  // 必须的宏，启用元对象系统（信号槽、属性、反射等）

public:
    // 构造函数接受 parent 参数，用于建立对象树关系
    explicit SimpleObject(const QString &name, QObject *parent = nullptr)
        : QObject(parent), m_name(name)  // 初始化基类和成员变量
    {
        qDebug() << "创建 SimpleObject:" << m_name;
    }

    // 析构函数演示对象树销毁顺序
    ~SimpleObject() override
    {
        qDebug() << "销毁 SimpleObject:" << m_name;
    }

    // 使用 Q_PROPERTY 声明属性，让属性可被元对象系统识别
    // READ 指定读取函数，WRITE 指定写入函数，NOTIFY 指定变更信号
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    // name 属性的 getter
    QString name() const { return m_name; }

    // name 属性的 setter
    void setName(const QString &name)
    {
        if (m_name != name) {  // 只有值真正改变时才发出信号
            m_name = name;
            emit nameChanged();  // 发出属性变更信号
        }
    }

signals:  // signals: 关键字声明信号函数
    void nameChanged();  // 属性变更信号（Q_PROPERTY 的 NOTIFY）

private:
    QString m_name;  // 私有成员变量存储属性值
};

#endif // SIMPLEOBJECT_H
