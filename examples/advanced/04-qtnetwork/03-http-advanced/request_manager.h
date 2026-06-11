/// @file    request_manager.h
/// @brief   HTTP request manager with priority queue, concurrency limiting, and
///          request interceptor pattern.
///
/// @details 对应教程：进阶层 04-QtNetwork/03-HTTP 高级用法。
///          演示请求优先级队列、最大并发控制、拦截器自动注入请求头。

#pragma once

#include <QByteArray>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QQueue>
#include <QUrl>

#include <functional>
#include <queue>
#include <vector>

/// @brief 管理 HTTP 请求的优先级队列，控制最大并发数，并支持请求拦截器。
class RequestManager : public QObject
{
    Q_OBJECT

public:
    /// @brief 请求优先级枚举，数值越小优先级越高。
    enum class Priority
    {
        kHigh   = 0,  ///< 高优先级，优先出队
        kNormal = 1,  ///< 普通优先级，默认
        kLow    = 2   ///< 低优先级，最后出队
    };
    Q_ENUM(Priority)

    /// @brief 拦截器回调类型，可在发送前修改 QNetworkRequest（如注入请求头）。
    using Interceptor = std::function<void(QNetworkRequest&)>;

    /// @brief 构造函数，初始化网络管理器和并发槽位。
    /// @param[in] maxConcurrent 最大并发请求数，默认 4。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    /// @note maxConcurrent 设为 1 可实现严格串行请求；设得过大可能触发服务端限流。
    explicit RequestManager(int maxConcurrent = 4, QObject* parent = nullptr);

    /// @brief 析构函数，中止所有活跃请求。
    ~RequestManager() override;

    // 禁止拷贝和移动，QObject 派生类不支持
    RequestManager(const RequestManager&) = delete;
    RequestManager& operator=(const RequestManager&) = delete;
    RequestManager(RequestManager&&) = delete;
    RequestManager& operator=(RequestManager&&) = delete;

    /// @brief 设置请求拦截器，每个请求在发送前都会经过该回调。
    /// @param[in] interceptor 拦截器函数对象。
    /// @note 拦截器可用于统一注入 User-Agent、Authorization 等请求头。
    void setInterceptor(Interceptor interceptor);

    /// @brief 将一个 GET 请求加入优先级队列。
    /// @param[in] url    请求的目标 URL。
    /// @param[in] priority 请求优先级，默认 kNormal。
    /// @note 入队后若当前活跃数未达上限，会立即派发请求。
    void enqueue(const QUrl& url, Priority priority = Priority::kNormal);

    /// @brief 返回当前排队等待的请求数量。
    [[nodiscard]] int pendingCount() const;

    /// @brief 返回当前正在执行的请求数量。
    [[nodiscard]] int activeCount() const;

signals:
    /// @brief 某个请求完成时发射。
    /// @param url      请求的 URL。
    /// @param httpStatus HTTP 状态码（0 表示网络错误，无有效响应）。
    /// @param body     响应体数据。
    void requestFinished(const QUrl& url, int httpStatus, const QByteArray& body);

    /// @brief 所有请求（排队 + 活跃）均已完成时发射。
    void allFinished();

private:
    /// @brief 内部请求条目，保存 URL 和优先级。
    struct RequestEntry
    {
        QUrl url;
        Priority priority;
    };

    /// @brief 从优先级队列中取出下一个请求并派发。
    /// @note 仅在活跃数小于最大并发时才会实际发送请求。
    void dequeueNext();

    /// @brief 处理单个请求完成的回调。
    /// @param[in] reply 完成的网络回复对象。
    /// @note 回复对象在此函数结束后由 Qt 自动删除（QNetworkAccessManager 所有权）。
    void handleReplyFinished(QNetworkReply* reply);

    /// @brief 优先级队列比较器：priority 值小的排前面。
    struct PriorityCompare
    {
        bool operator()(const RequestEntry& a, const RequestEntry& b) const
        {
            return static_cast<int>(a.priority) > static_cast<int>(b.priority);
        }
    };

    QNetworkAccessManager* m_nam;              ///< Qt 网络访问管理器
    Interceptor m_interceptor;                 ///< 请求拦截器回调
    int m_maxConcurrent;                       ///< 最大并发请求数
    int m_activeCount;                         ///< 当前活跃请求数
    int m_totalEnqueued;                       ///< 已入队的总请求数（用于追踪完成状态）
    int m_totalFinished;                       ///< 已完成的总请求数

    /// @note 使用 std::priority_queue 实现 O(log n) 的优先级出队。
    ///       QList + std::make_heap 也可，但 std::priority_queue 更直观。
    std::priority_queue<RequestEntry, std::vector<RequestEntry>, PriorityCompare>
        m_queue;
};
