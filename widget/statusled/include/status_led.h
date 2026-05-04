/**
 * @file status_led.h
 * @author Charliechen114514 (chengh1922@mails.jlu.edu.cn)
 * @brief
 * @version 0.1
 * @date 2026-04-21
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <QWidget>

class QTimer;

namespace AwesomeQt {
class StatusLED : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int ledSize READ ledSize WRITE setLedSize NOTIFY ledSizeChanged)

  public:
    enum class Status { NORMAL, WARNING, ERROR, OFFLINE };
    Q_ENUM(Status);

    explicit StatusLED(QWidget* parent = nullptr);
    StatusLED(const Status default_status, QWidget* parent = nullptr);

    void setStatus(const Status default_status);
    Status status() const;

    void setBlinking(bool enabled);
    bool isBlinking() const;

    void setLedSize(int diameter);
    int ledSize() const;

    QSize sizeHint() const override;

  signals:
    void statusChanged(Status newStatus);
    void ledSizeChanged(int newSize);

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    void initBlinkTimer();

    Status status_{Status::OFFLINE};
    int led_size_{20};
    QTimer* blink_timer_{nullptr};
    bool blink_visible_{true};
};
} // namespace AwesomeQt
