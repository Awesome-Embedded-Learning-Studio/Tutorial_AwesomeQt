#pragma once

#include <QList>
#include <QStringList>
#include <QWidget>

// 简易柱状图控件
class BarChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BarChartWidget(QWidget *parent = nullptr);

    void setData(const QList<int> &data);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QList<int> m_data;
    QStringList m_labels;
};
