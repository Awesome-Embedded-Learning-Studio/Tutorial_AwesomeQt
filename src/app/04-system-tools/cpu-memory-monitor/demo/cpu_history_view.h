/**
 * @file cpu_history_view.h
 * @brief CPU 占用率历史曲线——自绘 QWidget，paintEvent 画折线
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QWidget>
#include <vector>

/// @brief CPU 占用率历史曲线（自绘）。
///
/// 滚动窗口：push() 推一个 0..100 的采样点，超过容量丢最旧的，paintEvent 把整段画成折线。
/// 数据按容量归一化到画布宽度，y 轴 0..100 占用映射到画布高度。
class CpuHistoryView : public QWidget {
    Q_OBJECT
  public:
    explicit CpuHistoryView(int capacity = 60, QWidget* parent = nullptr);

    /// @brief 推一个采样点（占用率 0..100，-1 表示无效，画为缺口）。
    void push(int percent);

    QSize sizeHint() const override;

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    std::vector<int> samples_; // 滚动窗口，size 始终 ≤ capacity_
    int capacity_;
};
