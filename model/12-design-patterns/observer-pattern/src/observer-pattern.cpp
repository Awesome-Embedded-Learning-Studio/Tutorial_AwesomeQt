/**
 * @file observer-pattern.cpp
 * @brief 观察者模式实现——纯 C++ ClassicSubject + Qt 信号槽 QtSubject
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "observer-pattern.h"

#include <algorithm>

namespace AwesomeQt {

// ---------------------------------------------------------------------------
// 纯 C++ 对照实现：ClassicSubject
// ---------------------------------------------------------------------------

void ClassicSubject::attach(Observer* observer) {
    if (observer == nullptr) {
        return; // 防御：不挂空指针，避免 notify 时空跳
    }
    observers_.push_back(observer);
}

void ClassicSubject::detach(Observer* observer) {
    // 只移除第一个匹配项（与 GoF 原版 disconnect 语义一致）
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

void ClassicSubject::notify(double temperature) const {
    // 裸指针广播：若某 observer 已被销毁但未 detach，这里就是悬空回调 → 崩溃。
    // 对照：Qt 信号槽在 receiver 被 delete 时连接自动失效，无此风险。
    for (Observer* observer : observers_) {
        observer->onUpdate(temperature);
    }
}

// ---------------------------------------------------------------------------
// Qt 信号槽版：QtSubject
// ---------------------------------------------------------------------------

QtSubject::QtSubject(QObject* parent) : QObject(parent) {}

void QtSubject::setValue(double temperature) {
    value_ = temperature;
    // emit 即「通知所有连接方」。连接数、连接方式由调用方用 connect 决定，
    // Subject 自己完全不感知——这是 Qt 信号槽相对 GoF observer list 的核心简化。
    emit valueChanged(temperature);
    emit logMessage(QString("temperature -> %1").arg(temperature, 0, 'f', 2));
}

double QtSubject::value() const {
    return value_;
}

} // namespace AwesomeQt
