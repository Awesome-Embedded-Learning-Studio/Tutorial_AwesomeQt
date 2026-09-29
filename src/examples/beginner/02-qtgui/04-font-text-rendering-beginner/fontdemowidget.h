#pragma once

#include <QWidget>

class FontDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FontDemoWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};
