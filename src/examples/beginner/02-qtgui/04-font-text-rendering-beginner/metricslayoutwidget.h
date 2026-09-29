#pragma once

#include <QFontMetrics>
#include <QStringList>
#include <QWidget>

class MetricsLayoutWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MetricsLayoutWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QStringList wrapText(const QFontMetrics &fm, const QString &text, int maxWidth);
};
