#include "status_led.h"

#include <QPainter>
#include <QTimer>
#include <QRadialGradient>

namespace AwesomeQt {

static const QColor status_colors[] = {
    QColor(0, 200, 0),    // NORMAL  - green
    QColor(255, 180, 0),  // WARNING - amber
    QColor(255, 40, 40),  // ERROR   - red
    QColor(160, 160, 160) // OFFLINE - gray
};

static const char* status_names[] = {
    "Normal",
    "Warning",
    "Error",
    "Offline"
};

StatusLED::StatusLED(QWidget* parent) : QWidget(parent) {
    initBlinkTimer();
}

StatusLED::StatusLED(const Status default_status, QWidget* parent)
    : QWidget(parent), status_(default_status) {
    initBlinkTimer();
    setToolTip(status_names[static_cast<int>(status_)]);
}

void StatusLED::initBlinkTimer() {
    blink_timer_ = new QTimer(this);
    blink_timer_->setInterval(500);
    connect(blink_timer_, &QTimer::timeout, this, [this]() {
        blink_visible_ = !blink_visible_;
        update();
    });
}

void StatusLED::setStatus(const Status default_status) {
    if (status_ != default_status) {
        status_ = default_status;
        setToolTip(status_names[static_cast<int>(status_)]);
        update();
        emit statusChanged(default_status);
    }
}

StatusLED::Status StatusLED::status() const {
    return status_;
}

void StatusLED::setBlinking(bool enabled) {
    if (enabled) {
        blink_visible_ = true;
        blink_timer_->start();
    } else {
        blink_timer_->stop();
        blink_visible_ = true;
        update();
    }
}

bool StatusLED::isBlinking() const {
    return blink_timer_->isActive();
}

void StatusLED::setLedSize(int diameter) {
    if (led_size_ != diameter && diameter > 0) {
        led_size_ = diameter;
        updateGeometry();
        update();
        emit ledSizeChanged(diameter);
    }
}

int StatusLED::ledSize() const {
    return led_size_;
}

QSize StatusLED::sizeHint() const {
    return QSize(led_size_, led_size_);
}

void StatusLED::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor c = status_colors[static_cast<int>(status_)];
    if (!blink_visible_) {
        c = c.darker(400);
    }

    int r = std::min(width(), height()) / 2 - 1;

    QRadialGradient g(rect().center(), r);
    g.setColorAt(0.0, c.lighter(160));
    g.setColorAt(0.6, c);
    g.setColorAt(1.0, c.darker(150));

    p.setPen(Qt::NoPen);
    p.setBrush(g);
    p.drawEllipse(rect().center(), r, r);
}

} // namespace AwesomeQt
