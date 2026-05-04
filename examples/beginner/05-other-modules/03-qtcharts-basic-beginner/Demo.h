/**
 * QtCharts 图表基础示例
 *
 * 本示例演示 QtCharts 模块的核心用法：
 * 1. QChart + QChartView 显示图表
 * 2. QLineSeries 折线图
 * 3. QBarSeries + QBarSet 柱状图
 * 4. QPieSeries 饼图
 * 5. QValueAxis / QBarCategoryAxis 坐标轴配置
 * 6. 图表主题与动画效果
 */

#ifndef DEMO_H
#define DEMO_H

#include <QObject>

class QChartView;

class Demo : public QObject
{
    Q_OBJECT

public:
    explicit Demo(QObject *parent = nullptr);
    void run();

private:
    QChartView* createLineChart();
    QChartView* createBarChart();
    QChartView* createPieChart();
};

#endif // DEMO_H
