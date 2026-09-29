#include "fontdemowidget.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>

FontDemoWidget::FontDemoWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QFont 与 drawText 演示");
    resize(650, 450);
}

void FontDemoWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 浅色背景
    painter.fillRect(rect(), QColor(250, 250, 252));

    int y = 20;

    // ---- 区域 1: 基本的 drawText(x, y, text) ----
    // y 是基线位置，不是文字顶部
    QFont titleFont("Sans", 18, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(QColor(44, 62, 80));
    QFontMetrics titleFm(titleFont);
    painter.drawText(15, y + titleFm.ascent(),
                     "drawText 的各种用法演示");
    y += titleFm.lineSpacing() + 15;

    // ---- 区域 2: 不同字体属性的对比 ----
    painter.setPen(Qt::black);
    QFont baseFont("Sans", 13);
    painter.setFont(baseFont);
    QFontMetrics baseFm(baseFont);
    painter.drawText(15, y + baseFm.ascent(), "普通文字 (Sans, 13pt)");
    y += baseFm.lineSpacing();

    QFont boldFont("Sans", 13, QFont::Bold);
    painter.setFont(boldFont);
    QFontMetrics boldFm(boldFont);
    painter.drawText(15, y + boldFm.ascent(), "加粗文字 (Sans, 13pt, Bold)");
    y += boldFm.lineSpacing();

    QFont italicFont("Sans", 13, -1, true);  // italic = true
    painter.setFont(italicFont);
    QFontMetrics italicFm(italicFont);
    painter.drawText(15, y + italicFm.ascent(), "斜体文字 (Sans, 13pt, Italic)");
    y += italicFm.lineSpacing();

    QFont underlineFont("Sans", 13);
    underlineFont.setUnderline(true);
    painter.setFont(underlineFont);
    QFontMetrics ulFm(underlineFont);
    painter.drawText(15, y + ulFm.ascent(), "下划线文字 (Sans, 13pt, Underline)");
    y += ulFm.lineSpacing() + 15;

    // ---- 区域 3: QRect 版本 drawText + 对齐方式 ----
    painter.setPen(QColor(100, 100, 100));
    painter.setFont(QFont("Sans", 9));
    painter.drawText(15, y, 150, 20, Qt::AlignLeft | Qt::AlignVCenter,
                     "对齐演示：");
    y += 25;

    // 画三个矩形展示不同对齐方式
    int boxW = 180;
    int boxH = 50;
    int gap = 20;

    struct AlignDemo {
        Qt::Alignment align;
        QString label;
    };
    AlignDemo demos[] = {
        {Qt::AlignLeft | Qt::AlignTop, "左上对齐"},
        {Qt::AlignCenter, "居中对齐"},
        {Qt::AlignRight | Qt::AlignBottom, "右下对齐"},
    };

    QFont demoFont("Sans", 11);
    painter.setFont(demoFont);

    for (int i = 0; i < 3; ++i) {
        int bx = 15 + i * (boxW + gap);
        // 画虚线矩形框
        painter.setPen(QPen(QColor(180, 180, 180), 1, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(bx, y, boxW, boxH);
        // 在框内画文字
        painter.setPen(QColor(44, 62, 80));
        painter.drawText(QRect(bx, y, boxW, boxH),
                         demos[i].align, demos[i].label);
    }
    y += boxH + 20;

    // ---- 区域 4: 自动换行演示 ----
    painter.setPen(QColor(80, 80, 80));
    painter.setFont(QFont("Sans", 10));

    QRect wrapRect(15, y, 600, 100);
    // 画一个淡蓝色背景框
    painter.fillRect(wrapRect, QColor(235, 245, 255));
    painter.setPen(QPen(QColor(180, 200, 220), 1));
    painter.drawRect(wrapRect);

    // Qt::TextWordWrap 实现自动换行
    painter.setPen(QColor(50, 50, 50));
    painter.drawText(wrapRect.adjusted(8, 8, -8, -8),
                     Qt::TextWordWrap | Qt::AlignTop,
                     "这是一段比较长的文字，用来演示 drawText 的自动换行功能。"
                     "当文字超出矩形宽度时，加上 Qt::TextWordWrap 标志后，"
                     "Qt 会自动在适当的位置插入换行。"
                     "这在显示不确定长度的文本内容时非常有用。");
}
