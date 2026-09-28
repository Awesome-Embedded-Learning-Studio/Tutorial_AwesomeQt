#include "triangleglwidget.h"

#include <QDebug>

#include <cmath>

TriangleGLWidget::TriangleGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), m_rotation(0.0f), m_rotating(true)
{
    // 启动定时器，约 60fps 驱动重绘
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        if (m_rotating) {
            m_rotation += 1.0f;  // 每帧旋转 1 度
            if (m_rotation >= 360.0f) m_rotation -= 360.0f;
        }
        update();  // 请求重绘
    });
    m_timer->start(16);  // 约 60fps
}

TriangleGLWidget::~TriangleGLWidget()
{
    // 确保上下文是当前的，然后释放 OpenGL 资源
    makeCurrent();
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    // m_program 是 QOpenGLShaderProgram，会自动释放
    doneCurrent();
}

void TriangleGLWidget::setRotating(bool rotating) { m_rotating = rotating; }

void TriangleGLWidget::resetRotation()
{
    m_rotation = 0.0f;
    update();
}

void TriangleGLWidget::initializeGL()
{
    // 初始化 OpenGL 函数指针——这一步必须在所有 glXxx 调用之前
    initializeOpenGLFunctions();

    // 输出 OpenGL 版本信息，方便排查兼容性问题
    qDebug() << "OpenGL 版本:"
             << (const char *)glGetString(GL_VERSION);
    qDebug() << "GLSL 版本:"
             << (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    qDebug() << "渲染器:"
             << (const char *)glGetString(GL_RENDERER);

    // 设置深蓝色背景
    glClearColor(0.12f, 0.12f, 0.18f, 1.0f);

    // ---- 着色器源代码 ----
    // 顶点着色器：接收位置和颜色，应用旋转，输出变换后的位置和颜色
    const char *vertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;

        out vec3 vertexColor;

        uniform mat4 rotation;

        void main() {
            gl_Position = rotation * vec4(aPos, 1.0);
            vertexColor = aColor;
        }
    )glsl";

    // 片段着色器：接收插值后的颜色，直接输出
    const char *fragmentShaderSource = R"glsl(
        #version 330 core
        in vec3 vertexColor;
        out vec4 FragColor;

        void main() {
            FragColor = vec4(vertexColor, 1.0);
        }
    )glsl";

    // ---- 编译和链接着色器 ----
    m_program = new QOpenGLShaderProgram(this);

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                             vertexShaderSource)) {
        qDebug() << "顶点着色器编译失败:"
                 << m_program->log();
        return;
    }

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                             fragmentShaderSource)) {
        qDebug() << "片段着色器编译失败:"
                 << m_program->log();
        return;
    }

    if (!m_program->link()) {
        qDebug() << "着色器链接失败:" << m_program->log();
        return;
    }

    // 获取 rotation uniform 变量的位置
    m_rotationLoc = m_program->uniformLocation("rotation");

    // ---- 顶点数据：位置 (x,y,z) + 颜色 (r,g,b) ----
    // 每个顶点 6 个 float：前 3 个是位置，后 3 个是颜色
    float vertices[] = {
        // 位置              // 颜色（RGB）
         0.0f,  0.6f, 0.0f,  1.0f, 0.2f, 0.2f,  // 顶部 - 红色
        -0.5f, -0.4f, 0.0f,  0.2f, 1.0f, 0.2f,  // 左下 - 绿色
         0.5f, -0.4f, 0.0f,  0.2f, 0.2f, 1.0f,  // 右下 - 蓝色
    };

    // ---- 创建 VAO 和 VBO ----
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    // 绑定 VAO —— 从此刻起记录所有的顶点属性设置
    glBindVertexArray(m_vao);

    // 绑定 VBO 并上传顶点数据到 GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                 GL_STATIC_DRAW);

    // 设置顶点属性 0：位置（3 个 float，从偏移 0 开始）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 设置顶点属性 1：颜色（3 个 float，从偏移 3*sizeof(float) 开始）
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 解绑 VBO（VAO 已经记录了绑定信息）
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // 解绑 VAO
    glBindVertexArray(0);
}

void TriangleGLWidget::resizeGL(int w, int h)
{
    // 设置视口覆盖整个窗口
    glViewport(0, 0, w, h);
}

void TriangleGLWidget::paintGL()
{
    // 清除颜色缓冲区
    glClear(GL_COLOR_BUFFER_BIT);

    // 使用着色器程序
    m_program->bind();

    // 计算旋转矩阵（绕 Z 轴旋转）
    float angle = m_rotation * 3.14159265f / 180.0f;
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);

    // 4x4 旋转矩阵（列主序存储）
    // OpenGL 使用列主序，所以矩阵是转置后按列排列的
    float rotationMatrix[16] = {
        cosA,   sinA,  0.0f, 0.0f,  // 第一列
        -sinA,  cosA,  0.0f, 0.0f,  // 第二列
        0.0f,   0.0f,  1.0f, 0.0f,  // 第三列
        0.0f,   0.0f,  0.0f, 1.0f   // 第四列
    };

    // 将旋转矩阵传给着色器的 uniform 变量
    glUniformMatrix4fv(m_rotationLoc, 1, GL_FALSE, rotationMatrix);

    // 绑定 VAO（恢复之前记录的顶点属性设置）
    glBindVertexArray(m_vao);

    // 发出绘制命令：从顶点 0 开始，画 3 个顶点的三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 解绑
    glBindVertexArray(0);
    m_program->release();
}
