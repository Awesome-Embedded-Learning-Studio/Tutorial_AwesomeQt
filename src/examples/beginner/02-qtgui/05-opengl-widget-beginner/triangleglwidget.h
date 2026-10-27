#pragma once

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QTimer>

// ============================================================================
// 核心控件: OpenGL 三角形渲染 Widget
// ============================================================================
class TriangleGLWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    explicit TriangleGLWidget(QWidget *parent = nullptr);
    ~TriangleGLWidget() override;

    void setRotating(bool rotating);
    void resetRotation();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    GLuint m_vao = 0;                             ///< 顶点数组对象
    GLuint m_vbo = 0;                             ///< 顶点缓冲对象
    QOpenGLShaderProgram *m_program = nullptr;    ///< 着色器程序
    int m_rotationLoc = -1;                       ///< rotation uniform 变量位置
    float m_rotation = 0.0f;                      ///< 当前旋转角度（度）
    bool m_rotating = true;                       ///< 是否正在旋转
    QTimer *m_timer = nullptr;                    ///< 动画定时器
};
