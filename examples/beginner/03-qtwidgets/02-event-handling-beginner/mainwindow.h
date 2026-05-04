#pragma once

#include <QWidget>

class DrawingCanvas;
class QLabel;

// ============================================================================
// 主窗口：事件过滤器 + sendEvent / postEvent 演示
// ============================================================================
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    /// @brief 事件过滤器：拦截画板的 Ctrl+Z
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void updateStatus();

    DrawingCanvas *m_canvas = nullptr;
    QLabel *m_statusLabel = nullptr;
};
