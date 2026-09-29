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

#include "Demo.h"

#include <QApplication>
#include <QMainWindow>
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QPieSeries>
#include <QPieSlice>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QStringList>

Demo::Demo(QObject *parent)
    : QObject(parent)
{
}

void Demo::run()
{
    QMainWindow window;
    window.setWindowTitle("QtCharts 图表基础示例");
    window.resize(1200, 500);

    // 水平布局：三个图表并排显示
    auto *centralWidget = new QWidget(&window);
    auto *layout = new QHBoxLayout(centralWidget);

    // 创建三种图表
    auto *lineView = createLineChart();
    auto *barView = createBarChart();
    auto *pieView = createPieChart();

    layout->addWidget(lineView, 2);   // 折线图占比更大
    layout->addWidget(barView, 2);
    layout->addWidget(pieView, 1);

    window.setCentralWidget(centralWidget);
    window.show();

    QApplication::exec();
}

// ============================================================================
// 第一部分：折线图——月度销售趋势
// ============================================================================
QChartView* Demo::createLineChart()
{
    // 创建数据系列
    auto *series2024 = new QLineSeries();
    series2024->setName("2024");
    *series2024 << QPointF(0, 120) << QPointF(1, 135) << QPointF(2, 148)
                << QPointF(3, 162) << QPointF(4, 155) << QPointF(5, 178)
                << QPointF(6, 190) << QPointF(7, 185) << QPointF(8, 210)
                << QPointF(9, 198) << QPointF(10, 220) << QPointF(11, 245);

    auto *series2025 = new QLineSeries();
    series2025->setName("2025");
    *series2025 << QPointF(0, 140) << QPointF(1, 158) << QPointF(2, 165)
                << QPointF(3, 180) << QPointF(4, 172) << QPointF(5, 195)
                << QPointF(6, 210) << QPointF(7, 205) << QPointF(8, 235)
                << QPointF(9, 225) << QPointF(10, 248) << QPointF(11, 270);

    // 创建图表
    auto *chart = new QChart();
    chart->addSeries(series2024);
    chart->addSeries(series2025);
    chart->setTitle("月度销售趋势（万元）");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // 手动配置 X 轴
    auto *axisX = new QValueAxis();
    axisX->setRange(0, 11);
    axisX->setTickCount(12);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("月份");

    // 手动配置 Y 轴
    auto *axisY = new QValueAxis();
    axisY->setRange(0, 300);
    axisY->setTickCount(7);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("销售额（万元）");

    // 添加坐标轴并关联系列
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series2024->attachAxis(axisX);
    series2024->attachAxis(axisY);
    series2025->attachAxis(axisX);
    series2025->attachAxis(axisY);

    // 图例设置
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // 创建视图
    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    return view;
}

// ============================================================================
// 第二部分：柱状图——部门季度营收
// ============================================================================
QChartView* Demo::createBarChart()
{
    // 创建数据集（每组代表一个部门）
    auto *engineering = new QBarSet("Engineering");
    auto *marketing = new QBarSet("Marketing");
    auto *sales = new QBarSet("Sales");

    *engineering << 85 << 92 << 98 << 105;
    *marketing   << 55 << 60 << 58 << 65;
    *sales       << 70 << 78 << 82 << 88;

    // 创建柱状图系列
    auto *series = new QBarSeries();
    series->append(engineering);
    series->append(marketing);
    series->append(sales);

    // 创建图表
    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("部门季度营收（万元）");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // X 轴：分类标签
    QStringList categories;
    categories << "Q1" << "Q2" << "Q3" << "Q4";
    auto *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("季度");

    // Y 轴：数值
    auto *axisY = new QValueAxis();
    axisY->setRange(0, 120);
    axisY->setTickCount(7);
    axisY->setLabelFormat("%d");
    axisY->setTitleText("营收（万元）");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    return view;
}

// ============================================================================
// 第三部分：饼图——市场份额
// ============================================================================
QChartView* Demo::createPieChart()
{
    auto *series = new QPieSeries();
    series->setHoleSize(0.35);  // 环形图效果（内圆空洞）

    // 添加切片
    auto *slice1 = series->append("Product A", 35);
    auto *slice2 = series->append("Product B", 28);
    auto *slice3 = series->append("Product C", 22);
    auto *slice4 = series->append("Product D", 15);

    // 突出显示最大切片
    slice1->setExploded(true);
    slice1->setExplodeDistanceFactor(0.08);

    // 设置标签格式：名称 + 百分比
    for (auto *slice : series->slices()) {
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: %2%")
                            .arg(slice->label())
                            .arg(slice->percentage() * 100, 0, 'f', 1));
    }

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("产品市场份额");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignRight);

    auto *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    return view;
}
