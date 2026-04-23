#pragma once

#include <QWidget>

// 演示 1: 变换基础展示 —— 画旋转的花瓣图案
class TransformDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransformDemoWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};
