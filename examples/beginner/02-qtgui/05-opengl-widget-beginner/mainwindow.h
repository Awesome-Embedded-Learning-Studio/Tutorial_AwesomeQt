#pragma once

#include <QWidget>

class TriangleGLWidget;

// ============================================================================
// 主窗口：包含 OpenGL 渲染区域和控制按钮
// ============================================================================
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    TriangleGLWidget *m_glWidget = nullptr;
};
