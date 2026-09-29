#include "richtextcardwidget.h"

#include <QPainter>
#include <QTextDocument>

RichTextCardWidget::RichTextCardWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QTextDocument 富文本卡片");
    resize(480, 380);

    // 初始化 QTextDocument（成员变量，不在 paintEvent 中反复创建）
    m_doc = new QTextDocument(this);
    m_doc->setDocumentMargin(0);
    updateCardContent();
}

void RichTextCardWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 整体背景
    painter.fillRect(rect(), QColor(245, 247, 250));

    // 卡片参数
    int cardX = 20;
    int cardY = 20;
    int cardW = width() - 40;

    // 先设定文档宽度，计算文档高度
    m_doc->setTextWidth(cardW - 40);  // 卡片内左右各 20px 内边距
    qreal docHeight = m_doc->size().height();
    int cardH = static_cast<int>(docHeight) + 40;  // 上下各 20px

    // 画卡片背景（白色圆角矩形 + 阴影效果）
    // 简易阴影：先画一个偏移的深色矩形
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(200, 200, 210, 80));
    painter.drawRoundedRect(cardX + 3, cardY + 3, cardW, cardH, 10, 10);

    // 卡片主体
    painter.setBrush(QColor(255, 255, 255));
    painter.setPen(QPen(QColor(220, 225, 230), 1));
    painter.drawRoundedRect(cardX, cardY, cardW, cardH, 10, 10);

    // 画文档内容（在卡片内部，留出内边距）
    painter.translate(cardX + 20, cardY + 20);
    m_doc->drawContents(&painter);
}

void RichTextCardWidget::updateCardContent()
{
    QString html = R"(
        <div style="font-family: Sans;">
            <h2 style="color: #2C3E50; margin: 0 0 8px 0;">
                QTextDocument 富文本渲染
            </h2>
            <p style="color: #7F8C8D; font-size: 11pt; margin: 0 0 12px 0;">
                <i>AwesomeQt 教程</i> | 2025-04-22
            </p>
            <hr style="border: none; border-top: 1px solid #EAECEE; margin: 0 0 12px 0;">
            <p style="color: #34495E; font-size: 11pt; line-height: 1.6; margin: 0 0 8px 0;">
                QTextDocument 支持 <b>HTML 子集</b>的标记语法，可以在
                QPainter 上渲染带格式的富文本内容。
            </p>
            <p style="color: #34495E; font-size: 11pt; line-height: 1.6; margin: 0 0 8px 0;">
                支持的功能包括：
                <b>加粗</b>、<i>斜体</i>、<u>下划线</u>、
                <span style="color: #E74C3C;">彩色文字</span>、
                <span style="color: #27AE60;">各种颜色</span>。
            </p>
            <p style="color: #34495E; font-size: 11pt; line-height: 1.6; margin: 0 0 12px 0;">
                通过 <tt>setHtml()</tt> 设置内容，
                <tt>setTextWidth()</tt> 控制宽度，
                <tt>size()</tt> 获取实际渲染尺寸。
            </p>
            <hr style="border: none; border-top: 1px solid #EAECEE; margin: 0 0 8px 0;">
            <p style="color: #95A5A6; font-size: 9pt; margin: 0; text-align: right;">
                Qt 6.9.1 | C++17 | CMake 3.26+
            </p>
        </div>
    )";
    m_doc->setHtml(html);
}
