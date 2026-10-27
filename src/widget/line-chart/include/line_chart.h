/**
 * @file line_chart.h
 * @brief 纯 QPainter 折线图控件 LineChart——Y 轴自动缩放、可选网格/数据点/线下填充
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QColor>
#include <QVector>
#include <QWidget>

namespace AwesomeQt {

/// 纯 QPainter 折线图（不依赖 QtCharts）。
///
/// 设计要点（详见成品导览 index.md）：
/// - 数据为 `QVector<qreal>`，X 轴按索引均匀分布；Y 轴取数据 min..max 自动缩放；
/// - `appendPoint` / `setData` / `clear` 均触发 update()，无强制动画（静态即可）；
/// - 属性（lineColor/axisColor/gridColor/showGrid/showDots/showArea）全是 Q_PROPERTY，
///   配 NOTIFY，可被 Designer / State machine 直接驱动；
/// - 边界全防护：空数据直接 return、单点或 min==max 给 ±padding 防除零。
class LineChart : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：外观与开关项，配 READ/WRITE/NOTIFY，可被 Designer/State machine 驱动 ——
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
    Q_PROPERTY(QColor axisColor READ axisColor WRITE setAxisColor NOTIFY axisColorChanged)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor NOTIFY gridColorChanged)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
    Q_PROPERTY(bool showDots READ showDots WRITE setShowDots NOTIFY showDotsChanged)
    Q_PROPERTY(bool showArea READ showArea WRITE setShowArea NOTIFY showAreaChanged)

  public:
    explicit LineChart(QWidget* parent = nullptr);

    /// @brief 追加一个数据点，触发重绘（Y 轴 auto-scale 随之更新）
    void appendPoint(qreal value);

    /// @brief 整体替换数据序列，触发重绘
    /// @param values 新的数据序列（可空，清空图表）
    void setData(const QVector<qreal>& values);

    /// @brief 清空所有数据
    void clear();

    QVector<qreal> data() const;

    void setLineColor(const QColor& color);
    QColor lineColor() const;

    void setAxisColor(const QColor& color);
    QColor axisColor() const;

    void setGridColor(const QColor& color);
    QColor gridColor() const;

    void setShowGrid(bool on);
    bool showGrid() const;

    void setShowDots(bool on);
    bool showDots() const;

    void setShowArea(bool on);
    bool showArea() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  signals:
    void lineColorChanged(const QColor& color);
    void axisColorChanged(const QColor& color);
    void gridColorChanged(const QColor& color);
    void showGridChanged(bool on);
    void showDotsChanged(bool on);
    void showAreaChanged(bool on);

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    QVector<qreal> values_;

    QColor line_color_{QColor(30, 120, 220)};  // 折线主色（蓝）
    QColor axis_color_{QColor(120, 120, 120)}; // 坐标轴
    QColor grid_color_{QColor(220, 220, 220)}; // 网格（淡灰）

    bool show_grid_{true};
    bool show_dots_{true};
    bool show_area_{false};
};

} // namespace AwesomeQt
