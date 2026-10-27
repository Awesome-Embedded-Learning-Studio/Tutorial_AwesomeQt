/// @file    gl_widget.h
/// @brief   演示 QOpenGLWidget + QOpenGLShaderProgram 绘制彩色三角形。
///
/// 对应教程：进阶层 02-QtGui/05-OpenGL 集成基础。
/// 演示 initializeGL / paintGL / resizeGL 三阶段生命周期，
/// VAO/VBO 创建与着色器编译链接流程。

#pragma once

#include <QOpenGLWidget>

class QOpenGLShaderProgram;

/// @brief OpenGL 渲染控件，使用着色器程序绘制彩色三角形。
///
/// 在 initializeGL() 中编译顶点/片段着色器并创建 VAO/VBO，
/// 在 paintGL() 中执行绘制调用，在 resizeGL() 中更新视口。
class GlWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化成员变量。
    /// @param[in] parent 父控件指针。
    explicit GlWidget(QWidget* parent = nullptr);

    /// @brief 析构函数，确保 OpenGL 资源在正确上下文中释放。
    ~GlWidget() override;

protected:
    /// @brief OpenGL 上下文创建后调用，用于初始化着色器和缓冲区。
    void initializeGL() override;

    /// @brief 每帧绘制调用，执行实际的渲染操作。
    void paintGL() override;

    /// @brief 控件尺寸变化时调用，更新视口和投影矩阵。
    /// @param[in] w 新宽度（像素）。
    /// @param[in] h 新高度（像素）。
    void resizeGL(int w, int h) override;

private:
    /// @brief 编译顶点着色器和片段着色器，链接为着色器程序。
    /// @return 编译链接成功返回 true。
    auto setupShaders() -> bool;

    /// @brief 创建 VAO/VBO 并上传三角形顶点数据到 GPU。
    void setupVertexData();

    QOpenGLShaderProgram* m_program;  ///< 着色器程序对象
    int   m_vao;                      ///< 顶点数组对象 ID
    int   m_vbo;                      ///< 顶点缓冲区对象 ID
    bool  m_initialized;              ///< 标记 OpenGL 资源是否已初始化
};
