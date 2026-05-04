#ifndef TIMELINEBROWSER_H
#define TIMELINEBROWSER_H

#include <QLabel>
#include <QScrollBar>
#include <QWidget>

#include "TimelineWidget.h"

// ============================================================================
// TimelineBrowser: 时间轴浏览器主窗口
// ============================================================================
class TimelineBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineBrowser(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 事件过滤器：转发滚轮事件给水平滚动条
    bool eventFilter(QObject *watched, QEvent *event) override;

    /// @brief 缩放变化时更新
    void onZoomChanged(int value);

    /// @brief 根据内容宽度和视口宽度更新水平滚动条
    void updateHorizontalScrollBar();

    /// @brief 更新信息和状态栏
    void updateInfo();

    /// @brief 窗口大小变化时重新配置滚动条
    void resizeEvent(QResizeEvent *event) override;

private:
    TimelineWidget *m_timeline = nullptr;
    QScrollBar *m_hScrollBar = nullptr;
    QScrollBar *m_zoomScrollBar = nullptr;
    QLabel *m_infoLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
};

#endif // TIMELINEBROWSER_H
