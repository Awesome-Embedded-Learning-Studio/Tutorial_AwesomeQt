#ifndef PRINTEXAMPLEWINDOW_H
#define PRINTEXAMPLEWINDOW_H

#include <QMainWindow>
#include <QPixmap>

class QTextEdit;
class QLabel;

class PrintExampleWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PrintExampleWindow(QWidget *parent = nullptr);

private:
    QTextEdit *title_edit_;
    QTextEdit *body_edit_;
    QLabel *img_path_label_;
    QLabel *status_label_;
    QPixmap current_image_;
};

#endif // PRINTEXAMPLEWINDOW_H
