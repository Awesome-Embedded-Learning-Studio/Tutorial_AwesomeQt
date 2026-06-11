/// @file    request_manager.cpp
/// @brief   RequestManager 类的实现：优先级队列、并发控制、拦截器。
///
/// @details 对应教程：进阶层 04-QtNetwork/03-HTTP 高级用法。

#include "request_manager.h"

#include <QNetworkReply>
#include <QTimer>

#include <queue>

// --------------------------------------------------------------------------
// 构造 / 析构
// --------------------------------------------------------------------------

RequestManager::RequestManager(int maxConcurrent, QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))   // 父对象为 this，自动销毁
    , m_maxConcurrent(maxConcurrent)
    , m_activeCount(0)
    , m_totalEnqueued(0)
    , m_totalFinished(0)
{
    // @note 不在此处设置默认拦截器，由调用方决定是否需要。
    //       保持管理器职责单一：调度 + 并发控制。
}

RequestManager::~RequestManager()
{
    // QNetworkAccessManager 的所有子对象（包括活跃 reply）由 Qt 对象树自动清理
}

// --------------------------------------------------------------------------
// 公有方法
// --------------------------------------------------------------------------

void RequestManager::setInterceptor(Interceptor interceptor)
{
    m_interceptor = std::move(interceptor);
}

void RequestManager::enqueue(const QUrl& url, Priority priority)
{
    m_queue.push({url, priority});
    ++m_totalEnqueued;
    dequeueNext();
}

int RequestManager::pendingCount() const
{
    return static_cast<int>(m_queue.size());
}

int RequestManager::activeCount() const
{
    return m_activeCount;
}

// --------------------------------------------------------------------------
// 私有方法
// --------------------------------------------------------------------------

void RequestManager::dequeueNext()
{
    // 没有空闲槽位或队列为空时直接返回
    if (m_activeCount >= m_maxConcurrent || m_queue.empty())
    {
        return;
    }

    RequestEntry entry = m_queue.top();
    m_queue.pop();

    QNetworkRequest request(entry.url);

    // 经过拦截器，让调用方有机会注入请求头等
    if (m_interceptor)
    {
        m_interceptor(request);
    }

    QNetworkReply* reply = m_nam->get(request);
    ++m_activeCount;

    // @note 使用 lambda 捕获 this 指针是安全的：reply 的生命周期由 m_nam 管理，
    //       而 m_nam 的生命周期不超过 RequestManager。
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReplyFinished(reply);
    });
}

void RequestManager::handleReplyFinished(QNetworkReply* reply)
{
    --m_activeCount;

    const QUrl url = reply->request().url();
    const int httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();

    // @note QNetworkReply::error() 为 NoError 时 httpStatus 才有实际意义；
    //       网络层失败时 httpStatus 为 0。
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning("Request failed: %s (error: %s)",
                 qPrintable(url.toString()),
                 qPrintable(reply->errorString()));
    }

    // 必须在发射信号前 deleteLater，避免 reply 在外部被二次访问时出现悬空指针
    reply->deleteLater();

    emit requestFinished(url, httpStatus, body);
    ++m_totalFinished;

    // 尝试派发下一个排队请求
    dequeueNext();

    // 全部请求完成时通知上层
    if (m_totalFinished >= m_totalEnqueued && m_queue.empty() && m_activeCount == 0)
    {
        emit allFinished();
    }
}
