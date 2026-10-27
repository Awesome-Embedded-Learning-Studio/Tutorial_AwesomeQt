#pragma once

#include <QWidget>

class QTextDocument;

class RichTextCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RichTextCardWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void updateCardContent();

    QTextDocument *m_doc = nullptr;
};
