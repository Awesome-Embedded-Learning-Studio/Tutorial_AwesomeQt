// QtWidgets 入门示例 08: 图形视图框架基础
// MainWindow: 图形画板主窗口

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class InteractiveScene;
class GraphicsView;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void setupToolBar();
    void setupStatusBar();

private slots:
    void onMousePosChanged(const QPointF &scenePos);
    void onSelectionChanged(int count);

private:
    InteractiveScene *m_scene = nullptr;
    GraphicsView *m_view = nullptr;
    QLabel *m_posLabel = nullptr;
    QLabel *m_selectedLabel = nullptr;
    QLabel *m_itemCountLabel = nullptr;
};

#endif // MAINWINDOW_H
