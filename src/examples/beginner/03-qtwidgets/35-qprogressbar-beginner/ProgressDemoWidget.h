#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "Worker.h"

// ============================================================================
// ProgressDemoWidget: QProgressBar 综合演示窗口
// ============================================================================
class ProgressDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressDemoWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 启动工作线程
    void startWork();

    /// @brief 取消工作线程
    void cancelWork();

    /// @brief 清理线程后的 UI 恢复
    void cleanupThread();

private:
    QProgressBar *m_percentBar = nullptr;
    QProgressBar *m_detailBar = nullptr;
    QProgressBar *m_busyBar = nullptr;
    QLabel *m_detailLabel = nullptr;
    QLabel *m_busyLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_startBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;

    Worker *m_worker = nullptr;
    QThread *m_thread = nullptr;
};
