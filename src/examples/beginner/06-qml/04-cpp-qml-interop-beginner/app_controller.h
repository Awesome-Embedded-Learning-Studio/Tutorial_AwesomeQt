// app_controller.h — 可暴露给 QML 的 C++ 控制器类
#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QQmlEngine>

class AppController : public QObject
{
    Q_OBJECT
    // 将 C++ 属性暴露给 QML 引擎
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY counterChanged)
    Q_PROPERTY(QString themeColor READ themeColor WRITE setThemeColor NOTIFY themeColorChanged)

    // 允许该类在 QML 中通过类型名直接实例化
    QML_ELEMENT

public:
    explicit AppController(QObject *parent = nullptr);

    // 属性读取方法
    QString userName() const;
    int counter() const;
    QString themeColor() const;

    // 属性写入方法（内部发射对应的 NOTIFY 信号）
    void setUserName(const QString &name);
    void setCounter(int value);
    void setThemeColor(const QString &color);

    // Q_INVOKABLE 使该方法可从 QML 中调用
    Q_INVOKABLE void increment();
    Q_INVOKABLE void reset();
    Q_INVOKABLE QString greeting() const;

signals:
    void userNameChanged();
    void counterChanged();
    void themeColorChanged();
    // 自定义信号也可以从 C++ 发射到 QML
    void notificationRequested(const QString &message);

private:
    QString m_userName;
    int m_counter = 0;
    QString m_themeColor = "#3498db";
};

#endif // APP_CONTROLLER_H
