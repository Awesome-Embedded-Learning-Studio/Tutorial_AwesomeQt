// QtWidgets 入门示例 35: QProgressBar 进度条
// 演示：setRange + setValue 更新进度
//       无限进度 setRange(0, 0) 滚动动画
//       setFormat 自定义显示文字
//       跨线程安全更新进度条（Worker + moveToThread）

#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include <atomic>

// ============================================================================
// Worker: 模拟耗时操作的工作类，在工作线程中运行
// ============================================================================
class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(int totalCount, QObject *parent = nullptr)
        : QObject(parent)
        , m_totalCount(totalCount)
    {
    }

    /// @brief 请求取消操作
    void requestCancel()
    {
        m_cancelRequested.store(true);
    }

public slots:
    /// @brief 执行耗时操作（在 QThread 中运行）
    void doWork();

signals:
    /// @brief 进度百分比变化
    void progressChanged(int percent);

    /// @brief 详细进度变化（当前值, 总值）
    void detailChanged(int current, int total);

    /// @brief 操作完成
    void finished();

    /// @brief 操作被取消
    void canceled(int atPosition);

private:
    int m_totalCount = 100;
    std::atomic<bool> m_cancelRequested{false};
};

#endif // WORKER_H
