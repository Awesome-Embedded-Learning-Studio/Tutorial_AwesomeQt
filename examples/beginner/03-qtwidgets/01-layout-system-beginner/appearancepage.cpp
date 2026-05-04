#include "appearancepage.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

AppearancePage::AppearancePage(QWidget *parent) : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    auto *fontGroup = new QGroupBox("字体设置");
    auto *fontGrid = new QGridLayout(fontGroup);
    fontGrid->setSpacing(10);

    auto *sizeLabel = new QLabel("字体大小:");
    auto *sizeSpin = new QSpinBox;
    sizeSpin->setRange(8, 36);
    sizeSpin->setValue(12);

    auto *sizeSlider = new QSlider(Qt::Horizontal);
    sizeSlider->setRange(8, 36);
    sizeSlider->setValue(12);

    connect(sizeSlider, &QSlider::valueChanged,
            sizeSpin, &QSpinBox::setValue);
    connect(sizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            sizeSlider, &QSlider::setValue);

    auto *fontLabel = new QLabel("字体族:");
    auto *fontCombo = new QComboBox;
    fontCombo->addItems({"Arial", "Helvetica", "Times New Roman",
                         "Courier New", "Verdana"});

    fontGrid->addWidget(sizeLabel, 0, 0);
    fontGrid->addWidget(sizeSpin, 0, 1);
    fontGrid->addWidget(sizeSlider, 0, 2, 1, 2);
    fontGrid->addWidget(fontLabel, 1, 0);
    fontGrid->addWidget(fontCombo, 1, 1, 1, 3);

    fontGrid->setColumnStretch(2, 1);
    fontGrid->setColumnStretch(3, 1);

    mainLayout->addWidget(fontGroup);

    auto *colorGroup = new QGroupBox("颜色主题");
    auto *colorGrid = new QGridLayout(colorGroup);
    colorGrid->setSpacing(8);

    QStringList colors = {"#E74C3C", "#3498DB", "#2ECC71",
                          "#F39C12", "#9B59B6", "#1ABC9C"};
    QStringList names = {"红色", "蓝色", "绿色",
                         "橙色", "紫色", "青色"};

    for (int i = 0; i < colors.size(); ++i) {
        int row = i / 3;
        int col = i % 3;

        auto *btn = new QPushButton(names[i]);
        QString style = QString("QPushButton {"
                                "  background-color: %1;"
                                "  color: white;"
                                "  border: none;"
                                "  border-radius: 4px;"
                                "  padding: 8px;"
                                "  font-weight: bold;"
                                "}").arg(colors[i]);
        btn->setStyleSheet(style);
        colorGrid->addWidget(btn, row, col);
    }

    mainLayout->addWidget(colorGroup);
    mainLayout->addStretch();
}
