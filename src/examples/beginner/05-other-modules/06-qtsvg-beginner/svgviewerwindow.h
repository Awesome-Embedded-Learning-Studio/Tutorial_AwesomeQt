#ifndef SVGVIEWERWINDOW_H
#define SVGVIEWERWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <QByteArray>
#include <QStringList>
#include <QSize>

class QSvgWidget;
class QSlider;
class QLabel;

class SvgViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SvgViewerWindow(QWidget *parent = nullptr);

private:
    void loadSvgFromFile(const QString &path);
    void loadDemoSvg();
    void updateElementList();
    void applyZoom(int percent);
    void updateColorPreview(const QColor &color);

    QSvgWidget *svg_widget_;
    QSlider *zoom_slider_;
    QLabel *zoom_value_label_;
    QLabel *color_preview_;
    QLabel *element_list_;
    QLabel *extracted_icon_label_;
    QLabel *status_label_;

    QByteArray current_svg_data_;
    QStringList current_element_ids_;
    QColor current_color_ = QColor("#3498DB");
    QSize base_size_;
};

#endif // SVGVIEWERWINDOW_H
