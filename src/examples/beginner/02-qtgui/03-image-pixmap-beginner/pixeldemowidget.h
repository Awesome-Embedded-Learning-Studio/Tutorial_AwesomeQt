// QtGui 入门示例 03: QImage 像素级操作
#pragma once

#include <QWidget>

class PixelDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PixelDemoWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    /// @brief 生成演示用的 QImage，做像素级操作后转为 QPixmap 显示
    void generateDemoImage();

    QPixmap m_pixmap;
};
