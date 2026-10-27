#include "transformdemowidget.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

TransformDemoWidget::TransformDemoWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("坐标变换演示 —— 旋转花瓣");
    resize(500, 500);
}

void TransformDemoWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    // ------------------------------------------------------------------
    // 第一组：translate 的累加效果
    // ------------------------------------------------------------------
    painter.save();

    // 画 5 个矩形，每个在前一个基础上右移 60px
    QColor colors[] = {Qt::red, Qt::darkGreen, Qt::blue, Qt::darkCyan, Qt::darkMagenta};
    painter.translate(30, 30);  // 起始偏移

    for (int i = 0; i < 5; ++i) {
        painter.setPen(QPen(colors[i], 2));
        painter.setBrush(QBrush(QColor(colors[i].red(), colors[i].green(),
                                       colors[i].blue(), 80)));
        painter.drawRect(0, 0, 40, 40);
        painter.translate(60, 0);  // 累加平移
    }

    painter.restore();

    // ------------------------------------------------------------------
    // 第二组：rotate 的花瓣图案（save/restore 隔离每次旋转）
    // ------------------------------------------------------------------
    painter.save();
    painter.translate(width() / 2.0, height() / 2.0);  // 移到窗口中央

    // 画中心十字参考线
    painter.setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    painter.drawLine(-180, 0, 180, 0);
    painter.drawLine(0, -180, 0, 180);

    // 花瓣：12 个旋转的椭圆
    for (int i = 0; i < 12; ++i) {
        painter.save();  // 保存当前状态（原点在中心，无旋转）
        painter.rotate(i * 30);  // 旋转 i * 30 度

        // 设置花瓣颜色（色相渐变）
        QColor petalColor;
        petalColor.setHsv(i * 30, 200, 230, 150);
        painter.setPen(QPen(petalColor.darker(120), 1));
        painter.setBrush(QBrush(petalColor));

        // 画一个椭圆（花瓣形状），离原点有一定距离
        painter.drawEllipse(60, -15, 80, 30);

        painter.restore();  // 恢复到旋转前的状态
    }

    // 画中心圆
    painter.setPen(QPen(Qt::darkGray, 2));
    painter.setBrush(QBrush(QColor(255, 220, 100)));
    painter.drawEllipse(QPointF(0, 0), 20, 20);

    painter.restore();

    // ------------------------------------------------------------------
    // 第三组：scale 演示 —— 渐进缩放的同心矩形
    // ------------------------------------------------------------------
    painter.save();
    painter.translate(width() - 80, height() - 80);

    for (int i = 3; i >= 0; --i) {
        painter.save();
        double s = 0.4 + i * 0.2;
        painter.scale(s, s);
        int gray = 100 + i * 40;
        painter.setPen(QPen(QColor(gray, gray, gray), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(-50, -50, 100, 100);
        painter.restore();
    }

    painter.restore();
}
