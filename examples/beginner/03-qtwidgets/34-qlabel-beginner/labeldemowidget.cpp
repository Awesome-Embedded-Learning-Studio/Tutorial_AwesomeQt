// QtWidgets 入门示例 34: QLabel 文本与图像显示
// 演示：显示文本/HTML 富文本/图片
//       setAlignment 对齐与 setWordWrap 自动换行
//       setBuddy 关联快捷键到伙伴控件
//       linkActivated 信号处理超链接点击

#include "labeldemowidget.h"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMovie>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QDebug>
#include <QWidget>

// ============================================================================
// LabelDemoWidget: QLabel 综合演示窗口
// ============================================================================
LabelDemoWidget::LabelDemoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QLabel 综合演示 — 文本与图像显示");
    resize(600, 700);
    initUi();
}

/// @brief 初始化界面
void LabelDemoWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ================================================================
    // 区域 1: 纯文本与 HTML 富文本
    // ================================================================
    auto *textGroup = new QGroupBox("文本显示");
    auto *textLayout = new QVBoxLayout(textGroup);

    // 纯文本
    auto *plainLabel = new QLabel("纯文本：Hello, Qt! 当前版本 6.9.1");
    plainLabel->setTextFormat(Qt::PlainText);
    textLayout->addWidget(plainLabel);

    // 强制纯文本（避免 HTML 误判）
    auto *codeLabel = new QLabel(
        "代码片段：std::vector<int> vec = {1, 2, 3};");
    codeLabel->setTextFormat(Qt::PlainText);
    codeLabel->setStyleSheet(
        "background-color: #263238; color: #80CBC4;"
        "padding: 8px; border-radius: 4px; font-family: monospace;");
    textLayout->addWidget(codeLabel);

    // HTML 富文本
    auto *htmlLabel = new QLabel;
    htmlLabel->setTextFormat(Qt::RichText);
    htmlLabel->setText(
        "<h3 style='color: #1976D2;'>HTML 富文本演示</h3>"
        "<p>支持 <b>加粗</b>、<i>斜体</i>、<u>下划线</u> 等格式。</p>"
        "<p>也支持 <a href='https://doc.qt.io'>Qt 官方文档</a> "
        "这样的超链接。</p>"
        "<ul>"
        "<li>列表项 1</li>"
        "<li>列表项 2</li>"
        "</ul>");
    htmlLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    htmlLabel->setOpenExternalLinks(false);
    connect(htmlLabel, &QLabel::linkActivated, this,
            [this](const QString &url) {
        m_infoLabel->setText("点击了链接: " + url);
    });
    connect(htmlLabel, &QLabel::linkHovered, this,
            [this](const QString &url) {
        if (url.isEmpty()) {
            m_infoLabel->setText("就绪");
        } else {
            m_infoLabel->setText("悬停链接: " + url);
        }
    });
    textLayout->addWidget(htmlLabel);

    mainLayout->addWidget(textGroup);

    // ================================================================
    // 区域 2: 对齐与换行
    // ================================================================
    auto *alignGroup = new QGroupBox("对齐与换行");
    auto *alignLayout = new QGridLayout(alignGroup);

    // 左对齐
    auto *leftLabel = new QLabel("左对齐 (AlignLeft)");
    leftLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    leftLabel->setFixedHeight(40);
    leftLabel->setStyleSheet(
        "border: 1px solid #CCC; padding: 4px;");
    alignLayout->addWidget(leftLabel, 0, 0);

    // 居中
    auto *centerLabel = new QLabel("居中 (AlignCenter)");
    centerLabel->setAlignment(Qt::AlignCenter);
    centerLabel->setFixedHeight(40);
    centerLabel->setStyleSheet(
        "border: 1px solid #CCC; padding: 4px;");
    alignLayout->addWidget(centerLabel, 0, 1);

    // 右对齐
    auto *rightLabel = new QLabel("右对齐 (AlignRight)");
    rightLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rightLabel->setFixedHeight(40);
    rightLabel->setStyleSheet(
        "border: 1px solid #CCC; padding: 4px;");
    alignLayout->addWidget(rightLabel, 0, 2);

    // 自动换行演示
    auto *wrapLabel = new QLabel;
    wrapLabel->setWordWrap(true);
    wrapLabel->setFixedWidth(400);
    wrapLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    wrapLabel->setText(
        "这是一段很长的文本，用来演示 QLabel 的自动换行功能。"
        "当 wordWrap 设为 true 时，QLabel 会在单词边界处自动折行，"
        "保证文本始终在控件的可见宽度范围内显示。"
        "对于中文文本，Qt 会按字符进行换行，不会出现截断问题。"
        "这个标签的 fixedWidth 是 400px，你可以调整窗口大小观察效果。");
    wrapLabel->setStyleSheet(
        "border: 1px dashed #999; padding: 8px;"
        "background-color: #FFFDE7;");
    alignLayout->addWidget(wrapLabel, 1, 0, 1, 3);

    mainLayout->addWidget(alignGroup);

    // ================================================================
    // 区域 3: setBuddy 伙伴控件（快捷键跳转）
    // ================================================================
    auto *buddyGroup = new QGroupBox("setBuddy 伙伴控件 (Alt+字母跳转)");
    auto *buddyLayout = new QGridLayout(buddyGroup);

    // 姓名
    auto *nameLabel = new QLabel("姓名(&N):");
    auto *nameEdit = new QLineEdit;
    nameLabel->setBuddy(nameEdit);
    nameEdit->setPlaceholderText("按 Alt+N 跳到这里");
    buddyLayout->addWidget(nameLabel, 0, 0);
    buddyLayout->addWidget(nameEdit, 0, 1);

    // 邮箱
    auto *emailLabel = new QLabel("邮箱(&E):");
    auto *emailEdit = new QLineEdit;
    emailLabel->setBuddy(emailEdit);
    emailEdit->setPlaceholderText("按 Alt+E 跳到这里");
    buddyLayout->addWidget(emailLabel, 1, 0);
    buddyLayout->addWidget(emailEdit, 1, 1);

    // 年龄
    auto *ageLabel = new QLabel("年龄(&A):");
    auto *ageSpin = new QSpinBox;
    ageLabel->setBuddy(ageSpin);
    ageSpin->setRange(1, 150);
    ageSpin->setValue(25);
    buddyLayout->addWidget(ageLabel, 2, 0);
    buddyLayout->addWidget(ageSpin, 2, 1);

    auto *buddyHint = new QLabel(
        "提示：按 Alt+N / Alt+E / Alt+A 可以快速跳转到对应的输入框。"
        "QLabel 文本中的 &字母 会显示为带下划线的快捷键标记。");
    buddyHint->setWordWrap(true);
    buddyHint->setStyleSheet("color: #888; font-size: 11px;");
    buddyLayout->addWidget(buddyHint, 3, 0, 1, 2);

    mainLayout->addWidget(buddyGroup);

    // ================================================================
    // 区域 4: 图片显示（生成占位图）
    // ================================================================
    auto *imageGroup = new QGroupBox("图片显示");
    auto *imageLayout = new QHBoxLayout(imageGroup);

    // 生成一个带文字的占位图
    QPixmap placeholder(120, 120);
    placeholder.fill(QColor("#E3F2FD"));
    QPainter painter(&placeholder);
    painter.setPen(QColor("#1976D2"));
    painter.setFont(QFont("Arial", 12));
    painter.drawText(placeholder.rect(),
                     Qt::AlignCenter, "QPixmap\n120x120\n占位图");
    painter.end();

    auto *imageLabel = new QLabel;
    imageLabel->setPixmap(placeholder);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLayout->addWidget(imageLabel);

    // 缩放图片
    QPixmap scaled = placeholder.scaled(
        60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    auto *scaledLabel = new QLabel;
    scaledLabel->setPixmap(scaled);
    scaledLabel->setAlignment(Qt::AlignCenter);
    imageLayout->addWidget(scaledLabel);

    auto *imgInfo = new QLabel(
        "左: 原始 120x120\n右: 缩放到 60x60\n"
        "使用 QPixmap::scaled()\n保持宽高比");
    imgInfo->setWordWrap(true);
    imgInfo->setStyleSheet("color: #666; font-size: 12px;");
    imageLayout->addWidget(imgInfo);
    imageLayout->addStretch();

    mainLayout->addWidget(imageGroup);

    // ================================================================
    // 底部信息栏
    // ================================================================
    m_infoLabel = new QLabel("就绪");
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet(
        "color: #666; font-size: 12px;"
        "padding: 6px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    mainLayout->addWidget(m_infoLabel);
}
