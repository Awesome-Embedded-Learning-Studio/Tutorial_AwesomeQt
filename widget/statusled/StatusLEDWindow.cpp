#include "StatusLEDWindow.h"
#include "status_led.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using Status = AwesomeQt::StatusLED::Status;

StatusLEDWindow::StatusLEDWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
}

QWidget* StatusLEDWindow::setup_static_layout() {
    auto* group = new QGroupBox("Static Status LEDs");
    auto* grid = new QGridLayout(group);

    const struct {
        Status status;
        QString name;
    } items[] = {
        {Status::NORMAL, "Normal"}, {Status::WARNING, "Warning"},
        {Status::ERROR, "Error"},   {Status::OFFLINE, "Offline"},
    };

    for (int i = 0; i < 4; ++i) {
        auto* led = new AwesomeQt::StatusLED(items[i].status, group);
        auto* label = new QLabel(items[i].name, group);
        label->setAlignment(Qt::AlignCenter);
        grid->addWidget(led, 0, i, Qt::AlignCenter);
        grid->addWidget(label, 1, i, Qt::AlignCenter);
    }

    return group;
}

QWidget* StatusLEDWindow::setup_dynamic_layout() {
    auto* group = new QGroupBox("Interactive LED");
    auto* layout = new QHBoxLayout(group);

    auto* led = new AwesomeQt::StatusLED(Status::NORMAL, group);

    auto* status_label = new QLabel("Normal", group);

    auto* cycle_btn = new QPushButton("Cycle Status", group);
    connect(cycle_btn, &QPushButton::clicked, this, [led]() {
        Status s = static_cast<Status>((static_cast<int>(led->status()) + 1) %
                                       (static_cast<int>(Status::OFFLINE) + 1));
        led->setStatus(s);
    });

    auto* blink_btn = new QPushButton("Toggle Blinking", group);
    connect(blink_btn, &QPushButton::clicked, this, [led, blink_btn]() {
        led->setBlinking(!led->isBlinking());
        blink_btn->setText(led->isBlinking() ? "Stop Blinking" : "Toggle Blinking");
    });

    connect(led, &AwesomeQt::StatusLED::statusChanged, this, [status_label](Status s) {
        static const char* names[] = {"Normal", "Warning", "Error", "Offline"};
        status_label->setText(names[static_cast<int>(s)]);
    });

    layout->addWidget(led);
    layout->addWidget(status_label);
    layout->addWidget(cycle_btn);
    layout->addWidget(blink_btn);

    return group;
}

QWidget* StatusLEDWindow::setup_sizes_layout() {
    auto* group = new QGroupBox("Different Sizes");
    auto* layout = new QHBoxLayout(group);

    const int sizes[] = {16, 24, 32};
    for (int sz : sizes) {
        auto* led = new AwesomeQt::StatusLED(Status::NORMAL, group);
        led->setLedSize(sz);
        auto* label = new QLabel(QString("%1px").arg(sz), group);
        label->setAlignment(Qt::AlignCenter);

        auto* col = new QVBoxLayout();
        col->addWidget(led, 0, Qt::AlignCenter);
        col->addWidget(label, 0, Qt::AlignCenter);
        layout->addLayout(col);
    }

    return group;
}

void StatusLEDWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(setup_static_layout());
    layout->addWidget(setup_dynamic_layout());
    layout->addWidget(setup_sizes_layout());

    setWindowTitle("StatusLED Widget Demo");
    resize(500, 400);
}
