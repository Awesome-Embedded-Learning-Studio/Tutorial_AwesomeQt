/**
 * @file observer-pattern.h
 * @brief 观察者模式——Qt 信号槽实现 + 纯 C++ Observer 接口对照
 *
 * 教学要点（本件灵魂）：
 * - Qt 信号槽是编译期类型安全的观察者，对照裸 C++ observer list 的 void* 运行期类型擦除；
 * - 一个信号连多槽 = 一对多广播；
 * - Qt6 函数指针连接语法 connect(sender, &Cls::sig, receiver, &Cls::slot)；
 * - 连接类型 DirectConnection / QueuedConnection 对应同步 / 异步通知；
 * - 断开连接的几种方式（disconnect / lambda 连接的生命周期坑）。
 *
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QObject>
#include <QString>

#include <vector>

namespace AwesomeQt {

/// @brief 纯 C++ 观察者接口（对照实现）：不依赖 Qt 元系统，靠虚函数 + 运行期类型擦除。
///
/// 这是 GoF 经典写法：Subject 持一个 Observer* 列表，notify 时逐个回调。痛点：
/// - 通知签名写死（这里固定 double 温度），换数据类型就得改接口；
/// - Observer* 裸指针，Subject 不知道 observer 是否已被销毁（悬空回调风险）。
class Observer {
  public:
    virtual ~Observer() = default;

    /// @brief Subject 状态变化时回调。纯 C++ 做法只能把数据打包进固定签名。
    /// @param[in] temperature 新温度值（摄氏度）。
    virtual void onUpdate(double temperature) = 0;
};

/// @brief 纯 C++ Subject：维护 Observer* 列表，attach/detach/notify 三件套。
///
/// 对照 Qt 信号槽：这里「注册 / 注销 / 广播」全得手写，且 detach 之前 observer 被销毁
/// 就会悬空（notify 时崩溃）。Qt 信号槽用对象树 + 连接追踪免掉了这层手动管理。
class ClassicSubject {
  public:
    virtual ~ClassicSubject() = default;

    /// @brief 注册一个观察者。重复 attach 同一指针会被记录多次（GoF 原版语义）。
    /// @param[in] observer 观察者指针（不取得所有权）。
    void attach(Observer* observer);

    /// @brief 注销一个观察者（只移除第一个匹配项）。
    /// @param[in] observer 要移除的观察者指针。
    void detach(Observer* observer);

    /// @brief 向所有已注册观察者广播当前温度。
    /// @param[in] temperature 当前温度值。
    void notify(double temperature) const;

  private:
    std::vector<Observer*> observers_; // 裸指针列表，不持有所有权
};

/// @brief Qt 信号槽版 Subject：温度源，emit valueChanged 广播给任意数量的槽。
///
/// 这是 Qt 范式的观察者：Subject 只负责 emit 信号，「谁连、连几个、怎么响应」全是
/// 调用方用 connect 表达的。信号签名即通知契约（这里是 double），编译期类型检查，
/// 连错类型直接编译失败——对照 ClassicSubject::onUpdate 的运行期 void* 擦除。
class QtSubject : public QObject {
    Q_OBJECT

  public:
    /// @brief 连接类型枚举：演示 DirectConnection(同步) vs QueuedConnection(异步)。
    /// 用 Q_ENUM 让元系统认得，方便 demo 里打印当前连接类型名。
    enum class ConnectionFlavor {
        kSynchronous, // DirectConnection：emit 处直接调槽，同步
        kAsynchronous // QueuedConnection：投递事件到接收方线程，异步
    };
    Q_ENUM(ConnectionFlavor)

    explicit QtSubject(QObject* parent = nullptr);

    /// @brief 推入一个新温度值，触发 valueChanged 广播。
    /// @param[in] temperature 新温度（摄氏度）。
    /// @note emit 即「通知所有连接方」，Subject 不知道也不关心有几个 observer。
    void setValue(double temperature);

    /// @brief 当前温度。
    /// @return 最近一次 setValue 设入的值。
    double value() const;

  signals:
    /// @brief 温度变化信号——观察者模式的「通知」。一个信号可被任意数量的槽连接。
    /// @param temperature 新温度值。
    void valueChanged(double temperature);

    /// @brief 附带文字日志的信号（演示多信号、多 observer 各取所需）。
    /// @param message 日志文本。
    void logMessage(const QString& message);

  private:
    double value_{0.0};
};

} // namespace AwesomeQt
