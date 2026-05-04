// app_controller.cpp — AppController 实现
#include "app_controller.h"

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

QString AppController::userName() const
{
    return m_userName;
}

void AppController::setUserName(const QString &name)
{
    if (m_userName != name) {
        m_userName = name;
        emit userNameChanged();
    }
}

int AppController::counter() const
{
    return m_counter;
}

void AppController::setCounter(int value)
{
    if (m_counter != value) {
        m_counter = value;
        emit counterChanged();
    }
}

QString AppController::themeColor() const
{
    return m_themeColor;
}

void AppController::setThemeColor(const QString &color)
{
    if (m_themeColor != color) {
        m_themeColor = color;
        emit themeColorChanged();
    }
}

void AppController::increment()
{
    setCounter(m_counter + 1);
}

void AppController::reset()
{
    setCounter(0);
}

QString AppController::greeting() const
{
    if (m_userName.isEmpty()) {
        return "Hello, stranger!";
    }
    return "Hello, " + m_userName + "! Count: " + QString::number(m_counter);
}
