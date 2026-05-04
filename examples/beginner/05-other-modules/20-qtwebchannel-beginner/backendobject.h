/**
 * QtWebChannel 交互面板示例
 *
 * 本示例演示 QtWebChannel 模块的核心功能：
 * - QWebChannel 将 C++ 对象暴露给 JavaScript
 * - qwebchannel.js 前端侧配置
 * - 双向通信：JS 调用 Qt 方法 / Qt 发信号到 JS
 * - 与 QWebEngineView 集成
 *
 * 启动后弹出一个 WebEngineView 窗口，显示交互面板：
 *   - 获取时间：JS 调用 C++ getCurrentTime() 方法
 *   - 获取主机名：JS 调用 C++ getHostName() 方法
 *   - 系统信息：JS 调用 C++ getSystemInfo() 方法
 *   - 推送通知：JS 触发 C++ emit signal 回传 JS
 */

#include <QObject>
#include <QString>

/// @brief 暴露给 JavaScript 的后端交互对象
/// 演示 Q_PROPERTY 属性暴露、Q_INVOKABLE 方法调用、信号推送
class BackendObject : public QObject
{
    Q_OBJECT

    // Q_PROPERTY: JS 可以读取属性，监听 NOTIFY 信号
    Q_PROPERTY(QString platformInfo READ platformInfo CONSTANT)
    Q_PROPERTY(int callCount READ callCount NOTIFY callCountChanged)

public:
    explicit BackendObject(QObject *parent = nullptr);

    QString platformInfo() const;
    int callCount() const;

    /// @brief 获取当前时间字符串（ISO 格式）
    Q_INVOKABLE QString getCurrentTime();

    /// @brief 获取本机主机名
    Q_INVOKABLE QString getHostName();

    /// @brief 获取系统信息（JSON 格式）
    Q_INVOKABLE QString getSystemInfo();

    /// @brief 从 C++ 侧推送通知到 JS（演示信号回调）
    Q_INVOKABLE void sendNotification(const QString &text);

signals:
    // 属性变化信号——QWebChannel 自动推送给 JS
    void callCountChanged(int count);
    // 自定义信号——JS 可通过 .connect() 注册回调
    void notificationPushed(const QString &message);

private:
    void incrementCount();

    int m_callCount;
};
