#pragma once

#include <QWidget>

class CircularGauge;
class QSlider;
class QLabel;

// ============================================================================
// 主窗口：两个仪表盘 + 滑块控制
// ============================================================================
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    CircularGauge *m_cpuGauge = nullptr;
    CircularGauge *m_memGauge = nullptr;
    QSlider *m_cpuSlider = nullptr;
    QSlider *m_memSlider = nullptr;
    QLabel *m_cpuValueLabel = nullptr;
    QLabel *m_memValueLabel = nullptr;
};
