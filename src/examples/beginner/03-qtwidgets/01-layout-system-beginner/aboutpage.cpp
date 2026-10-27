#include "aboutpage.h"

#include <QFont>
#include <QLabel>
#include <QVBoxLayout>

AboutPage::AboutPage(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);

    layout->addStretch();

    auto *titleLabel = new QLabel("AwesomeQt 布局系统示例");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    auto *verLabel = new QLabel("版本 1.0.0");
    verLabel->setAlignment(Qt::AlignCenter);
    verLabel->setStyleSheet("color: #666; font-size: 14px;");
    layout->addWidget(verLabel);

    auto *descLabel = new QLabel(
        "本示例演示了 Qt 五种布局管理器的基本用法：\n"
        "QHBoxLayout / QVBoxLayout / QGridLayout /\n"
        "QFormLayout / QStackedLayout");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setStyleSheet("color: #888; font-size: 12px; padding: 10px;");
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    layout->addStretch();
}
