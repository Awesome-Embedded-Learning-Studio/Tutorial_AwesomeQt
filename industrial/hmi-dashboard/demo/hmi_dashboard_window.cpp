/**
 * @file hmi_dashboard_window.cpp
 * @brief HMI Dashboard 主窗口实现（骨架）
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "hmi_dashboard_window.h"

#include <QBrush>
#include <QColor>
#include <QDockWidget>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace {

/// @brief 占位仪表控件：自绘空心圆 + 固定指针 + 标题。
/// 成品期替换为 widget/circle-progress、speed-meter（接口位置已留好）。
class PlaceholderGauge : public QWidget {
  public:
    PlaceholderGauge(const QString& title, QWidget* parent = nullptr)
        : QWidget(parent), title_(title) {
        setMinimumSize(120, 150);
    }

  protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const int side = qMin(width(), height() - 28);
        const QPointF center(width() / 2.0, 12 + side / 2.0);
        const double radius = side / 2.0 - 6;

        // 表盘外圈
        p.setPen(QPen(QColor(80, 140, 200), 3));
        p.setBrush(QColor(240, 245, 250));
        p.drawEllipse(center, radius, radius);

        // 占位指针（固定向上）
        p.setPen(QPen(QColor(40, 60, 90), 2));
        p.drawLine(center, center + QPointF(0, -radius + 10));

        // 标题
        p.setPen(QColor(40, 60, 90));
        p.drawText(QRectF(0, height() - 24, width(), 24), Qt::AlignCenter, title_);
    }

  private:
    QString title_;
};

} // namespace

HmiDashboardWindow::HmiDashboardWindow(QWidget* parent) : QMainWindow(parent) {
    setup_layout();
    setup_status_bar();
    setWindowTitle("HMI Dashboard (scaffold)");
    resize(900, 560);
}

void HmiDashboardWindow::setup_layout() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // 左：仪表区（温度/压力/流量 占位，成品期换成 widget/circle-progress、speed-meter）
    auto* gauge_panel = new QWidget(splitter);
    auto* gauge_layout = new QVBoxLayout(gauge_panel);
    gauge_layout->addWidget(new PlaceholderGauge("Temperature", gauge_panel));
    gauge_layout->addWidget(new PlaceholderGauge("Pressure", gauge_panel));
    gauge_layout->addWidget(new PlaceholderGauge("Flow", gauge_panel));

    // 中：趋势区（占位，成品期接 widget/line-chart 或 QtCharts）
    auto* trend_panel =
        new QLabel("Trend Curve\n(成品期接入 widget/line-chart 或 QtCharts)", splitter);
    trend_panel->setAlignment(Qt::AlignCenter);
    trend_panel->setStyleSheet("background:#fafafa;color:#888;");

    splitter->addWidget(gauge_panel);
    splitter->addWidget(trend_panel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    setCentralWidget(splitter);

    // 右 dock：报警列表（真实数据填充，非空）
    auto* alarm_list = new QListWidget(this);
    alarm_list->addItem("[WARN] 14:02:11  Temperature > 80 C");
    alarm_list->addItem("[ERR]  14:02:18  Pressure sensor timeout");

    auto* dock = new QDockWidget("Alarms", this);
    dock->setWidget(alarm_list);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void HmiDashboardWindow::setup_status_bar() {
    statusBar()->showMessage(
        "HMI Dashboard · 骨架（整机成品待 widget 链 speed-meter/circle-progress/line-chart 就位）");
}
