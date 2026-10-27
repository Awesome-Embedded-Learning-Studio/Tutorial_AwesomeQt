/// @file    gl_widget.cpp
/// @brief   GlWidget 类实现——QOpenGLWidget 着色器渲染彩色三角形。
///
/// 对应教程：进阶层 02-QtGui/05-OpenGL 集成基础。
/// 关键知识点：
/// - QOpenGLShaderProgram 管理着色器编译与链接
/// - VAO（Vertex Array Object）封装顶点属性配置
/// - VBO（Vertex Buffer Object）存储 GPU 侧顶点数据
/// - initializeGL / paintGL / resizeGL 三阶段生命周期

#include "gl_widget.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

// 三角形顶点数据：x, y, z, r, g, b（位置 + 颜色交织存储）
static constexpr float kTriangleVertices[] = {
    // position (x, y, z)    // color (r, g, b)
     0.0f,  0.5f,  0.0f,     1.0f, 0.0f, 0.0f,  // 顶部 — 红色
    -0.5f, -0.5f,  0.0f,     0.0f, 1.0f, 0.0f,  // 左下 — 绿色
     0.5f, -0.5f,  0.0f,     0.0f, 0.0f, 1.0f,  // 右下 — 蓝色
};

GlWidget::GlWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_program(nullptr)
    , m_vao(0)
    , m_vbo(0)
    , m_initialized(false)
{
}

GlWidget::~GlWidget()
{
    // @note OpenGL 资源必须在拥有正确 GL 上下文的线程中释放。
    // makeCurrent() 确保析构时上下文绑定到当前线程。
    makeCurrent();

    // @note QOpenGLExtraFunctions 提供 OpenGL ES 3.0 / OpenGL 3.0 级别的
    // 函数（如 VAO 操作），而 QOpenGLFunctions 仅覆盖 OpenGL ES 2.0
    auto* gl = QOpenGLContext::currentContext()->extraFunctions();
    if (m_vao != 0) {
        gl->glDeleteVertexArrays(1, reinterpret_cast<GLuint*>(&m_vao));
    }
    if (m_vbo != 0) {
        gl->glDeleteBuffers(1, reinterpret_cast<GLuint*>(&m_vbo));
    }

    doneCurrent();
}

void GlWidget::initializeGL()
{
    auto* gl = QOpenGLContext::currentContext()->extraFunctions();

    // 设置清除颜色为深灰色背景
    gl->glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    // 编译并链接着色器
    if (!setupShaders()) {
        return; // 着色器编译失败，paintGL 中会跳过绘制
    }

    // 创建并填充 VAO/VBO
    setupVertexData();

    m_initialized = true;
}

void GlWidget::paintGL()
{
    if (!m_initialized) {
        return; // 着色器或缓冲区初始化失败，不绘制
    }

    auto* gl = QOpenGLContext::currentContext()->extraFunctions();

    gl->glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    // 绑定 VAO — 它记录了顶点属性与 VBO 的绑定关系，
    // 无需每次重新配置属性指针
    gl->glBindVertexArray(static_cast<GLuint>(m_vao));

    gl->glDrawArrays(GL_TRIANGLES, 0, 3);

    gl->glBindVertexArray(0);
    m_program->release();
}

void GlWidget::resizeGL(int w, int h)
{
    auto* gl = QOpenGLContext::currentContext()->extraFunctions();

    // 视口设置为整个控件区域；宽高比变化时此处可加入投影矩阵更新
    gl->glViewport(0, 0, w, h);
}

auto GlWidget::setupShaders() -> bool
{
    m_program = new QOpenGLShaderProgram(this);

    // 顶点着色器：将位置传递到 gl_Position，颜色直接传递到片段着色器
    const char* vertexSrc = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;
        out vec3 vertexColor;
        void main() {
            gl_Position = vec4(aPos, 1.0);
            vertexColor = aColor;
        }
    )glsl";

    // 片段着色器：直接使用从顶点着色器插值得到的颜色
    const char* fragmentSrc = R"glsl(
        #version 330 core
        in vec3 vertexColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vertexColor, 1.0);
        }
    )glsl";

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSrc)) {
        return false;
    }
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSrc)) {
        return false;
    }
    if (!m_program->link()) {
        return false;
    }

    return true;
}

void GlWidget::setupVertexData()
{
    auto* gl = QOpenGLContext::currentContext()->extraFunctions();

    // 创建 VAO 并绑定 — 后续的 VBO 和属性配置会被记录在此 VAO 中
    gl->glGenVertexArrays(1, reinterpret_cast<GLuint*>(&m_vao));
    gl->glBindVertexArray(static_cast<GLuint>(m_vao));

    // 创建 VBO 并上传顶点数据
    gl->glGenBuffers(1, reinterpret_cast<GLuint*>(&m_vbo));
    gl->glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(m_vbo));
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(kTriangleVertices),
                     kTriangleVertices, GL_STATIC_DRAW);

    // 位置属性：location=0，3 个 float，stride=6 个 float，偏移=0
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              6 * sizeof(float), nullptr);
    gl->glEnableVertexAttribArray(0);

    // 颜色属性：location=1，3 个 float，stride=6 个 float，偏移=3 个 float
    gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              6 * sizeof(float),
                              reinterpret_cast<void*>(3 * sizeof(float)));
    gl->glEnableVertexAttribArray(1);

    // 解绑 VAO，防止后续操作意外修改状态
    gl->glBindVertexArray(0);
}
