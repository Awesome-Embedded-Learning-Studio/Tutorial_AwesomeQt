#pragma once

#include <QWidget>

// 基本图形绘制 —— 画一个"图形大全"展示面板
class ShapeGalleryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShapeGalleryWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};
