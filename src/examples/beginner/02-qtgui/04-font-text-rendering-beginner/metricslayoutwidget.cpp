#include "metricslayoutwidget.h"

#include <QPainter>
#include <QFont>

MetricsLayoutWidget::MetricsLayoutWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QFontMetrics 精确布局演示");
    resize(550, 350);
}

void MetricsLayoutWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    // 模拟一个文章卡片布局
    int padding = 20;
    int contentWidth = width() - 2 * padding;
    int y = padding;

    // ---- 标题（大号加粗） ----
    QFont titleFont("Sans", 22, QFont::Bold);
    painter.setFont(titleFont);
    QFontMetrics titleFm(titleFont);
    painter.setPen(QColor(33, 37, 41));

    QString title = "QFontMetrics 布局计算";
    painter.drawText(padding, y + titleFm.ascent(), title);

    // 用 QFontMetrics 计算标题实际宽度
    int titleWidth = titleFm.horizontalAdvance(title);
    // 在标题下方画一条装饰线
    y += titleFm.height() + 4;
    painter.setPen(QPen(QColor(74, 144, 217), 2));
    painter.drawLine(padding, y, padding + titleWidth, y);
    y += 12;

    // ---- 副标题（中号，灰色） ----
    QFont subFont("Sans", 12);
    subFont.setItalic(true);
    painter.setFont(subFont);
    QFontMetrics subFm(subFont);
    painter.setPen(QColor(108, 117, 125));

    painter.drawText(padding, y + subFm.ascent(),
                     "作者：AwesomeQt | 2025-04-22 | Qt 6.9.1");
    y += subFm.lineSpacing() + 8;

    // ---- 分隔线 ----
    painter.setPen(QPen(QColor(222, 226, 230), 1));
    painter.drawLine(padding, y, padding + contentWidth, y);
    y += 12;

    // ---- 正文段落（普通大小，多行） ----
    QFont bodyFont("Sans", 11);
    painter.setFont(bodyFont);
    QFontMetrics bodyFm(bodyFont);
    painter.setPen(QColor(33, 37, 41));

    // 手动逐行绘制，精确控制行间距
    QStringList paragraphs = {
        "QFontMetrics 提供了文字尺寸的精确计算能力。通过 "
        "horizontalAdvance() 获取字符串宽度，通过 height() "
        "获取字体高度，通过 lineSpacing() 获取推荐的行间距。",
        "在自定义控件中做文字排版时，这些函数是不可或缺的。"
        "你可以根据文字的实际像素尺寸来精确安排每个元素的位置，"
        "而不是靠猜或者凑。"
    };

    for (const QString &para : paragraphs) {
        // 手动实现简单的自动换行
        QStringList lines = wrapText(bodyFm, para, contentWidth);
        for (const QString &line : lines) {
            painter.drawText(padding, y + bodyFm.ascent(), line);
            y += bodyFm.lineSpacing();
        }
        y += 6;  // 段落间距
    }

    // ---- 底部尺寸标注 ----
    y += 8;
    painter.setFont(QFont("Monospace", 9));
    QFontMetrics infoFm(painter.fontMetrics());
    painter.setPen(QColor(150, 150, 150));

    QString info = QString("titleFont.height()=%1  bodyFont.height()=%2  "
                           "bodyFont.lineSpacing()=%3")
                       .arg(titleFm.height())
                       .arg(bodyFm.height())
                       .arg(bodyFm.lineSpacing());
    painter.drawText(padding, y + infoFm.ascent(), info);
}

QStringList MetricsLayoutWidget::wrapText(const QFontMetrics &fm, const QString &text, int maxWidth)
{
    QStringList lines;
    QString currentLine;

    // 按字符逐个添加，超过宽度就换行
    for (const QChar &ch : text) {
        QString testLine = currentLine + ch;
        if (fm.horizontalAdvance(testLine) > maxWidth && !currentLine.isEmpty()) {
            lines.append(currentLine);
            currentLine = ch;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.isEmpty()) {
        lines.append(currentLine);
    }
    return lines;
}
