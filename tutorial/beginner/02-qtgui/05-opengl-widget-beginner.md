# 现代Qt开发教程（新手篇）2.5——QOpenGLWidget 嵌入 OpenGL 基础

## 1. 前言 / 为什么要在 Qt 里用 OpenGL

说实话，我一开始对在 Qt 里写 OpenGL 是有抵触心理的。QPainter 画各种图形不好吗，为什么要折腾 GPU？直到有一次我需要实时显示一个不断旋转的 3D 模型，QPainter 每帧都要重绘几千个多边形，帧率直接掉到个位数，这才老老实实去学 OpenGL。后来的事实也证明了：但凡涉及 3D 渲染、大量图形的实时绘制、或者需要利用 GPU 并行计算能力的场景，OpenGL（或者 Vulkan、Metal）是你唯一的出路。

Qt 提供了 QOpenGLWidget 这个类，让你可以在普通的 QWidget 界面里嵌入一个 OpenGL 渲染区域。它的使用方式跟普通 Widget 不太一样——你不是重写 paintEvent，而是重写三个生命周期函数：`initializeGL` 在 OpenGL 上下文创建后调用一次，用于初始化资源；`resizeGL` 在窗口大小改变时调用，用于设置视口和投影矩阵；`paintGL` 每帧调用，用于执行实际的绘制命令。这三个函数构成了 QOpenGLWidget 的完整渲染循环。

这篇文章我们一起来从零开始：搭建 QOpenGLWidget 的基本框架、理解 OpenGL 上下文的生命周期、用 VAO 和 VBO 画出一个三角形。别小看这个三角形——它是所有 3D 渲染的基石，搞懂了顶点数据的提交方式和着色器的基本用法，后面学更复杂的几何体就有基础了。

## 2. 环境说明

本篇代码适用于 Qt 6.5+ 版本（示例基于 Qt 6.9.1 验证），CMake 3.26+，C++17 标准。示例代码依赖 QtGui 和 QtWidgets 模块——QOpenGLWidget 在 QtWidgets 中，QOpenGLFunctions 在 QtGui 中。关于 OpenGL 版本：我们的示例使用 OpenGL 3.3 Core Profile，这需要你的显卡和驱动支持至少 OpenGL 3.3（2010 年以后的显卡基本都支持）。如果你的环境只有 OpenGL 2.1 或者 OpenGL ES 2.0/3.0（比如某些嵌入式设备），代码需要做相应调整——主要是着色器版本号的差异。在 Linux 上，你可能需要安装 Mesa 库（`sudo apt install libgl1-mesa-dev`）才能编译 OpenGL 相关的代码。所有代码在 Linux、Windows、macOS 上都可以编译运行。

## 3. 核心概念讲解

### 3.1 QOpenGLWidget 的三个生命周期函数

QOpenGLWidget 的使用方式跟普通 QWidget 有本质区别。普通 Widget 重写 `paintEvent` 用 QPainter 画，QOpenGLWidget 则重写三个以 `GL` 结尾的函数，每个函数在特定的时机被调用。理解这三个函数的调用时机和职责，是用好 QOpenGLWidget 的前提。

`initializeGL()` 在 Widget 第一次显示之前调用一次，此时 OpenGL 上下文已经创建好了，你可以安全地调用任何 OpenGL 函数。这个函数是你初始化 OpenGL 资源的地方——创建着色器程序、生成 VAO 和 VBO、加载纹理、设置 OpenGL 状态。注意这个函数只会被调用一次，如果你需要重新初始化资源，通常的做法是设置一个标志位，然后在 `paintGL` 里检查并执行。

`resizeGL(int w, int h)` 在窗口大小改变时调用（第一次显示时也会调用一次）。这个函数的职责是设置 OpenGL 的视口（viewport）和投影矩阵。视口决定了 OpenGL 渲染结果映射到窗口的哪个区域，通常直接设置成整个窗口大小。

`paintGL()` 是每帧绘制的地方，相当于普通 Widget 的 paintEvent。系统会在需要重绘的时候自动调用这个函数——你可以通过 `update()` 来主动请求重绘。在 `paintGL` 里你需要清除颜色缓冲区、绑定着色器和顶点数据、发出绘制命令。

```cpp
class GLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr) : QOpenGLWidget(parent) {}

protected:
    void initializeGL() override
    {
        // 此时 OpenGL 上下文已经就绪
        // 初始化 OpenGL 函数指针
        initializeOpenGLFunctions();
        // 设置背景色
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        // 创建着色器、VAO、VBO 等……
    }

    void resizeGL(int w, int h) override
    {
        // 设置视口覆盖整个窗口
        glViewport(0, 0, w, h);
        // 如果有投影矩阵，在这里更新……
    }

    void paintGL() override
    {
        // 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);
        // 绑定着色器和顶点数据，发出绘制命令……
    }
};
```

这里有一个非常重要的细节：`initializeOpenGLFunctions()` 这个调用。Qt 通过 `QOpenGLFunctions` 类封装了跨平台的 OpenGL 函数调用。但 OpenGL 的函数指针在运行时需要从显卡驱动中获取，`initializeOpenGLFunctions()` 就是做这件事的。你必须在 `initializeGL` 里先调用它，之后才能使用 `glClear`、`glGenBuffers` 等函数。如果你在其他地方（比如构造函数里）调用 OpenGL 函数，函数指针还没有初始化，程序会崩溃。

### 3.2 QOpenGLFunctions 平台无关调用方式

不同操作系统上的 OpenGL 函数加载机制完全不一样——Windows 用 wgl，Linux 用 glx，macOS 有自己的 CGL。Qt 的 `QOpenGLFunctions` 把这些差异全部封装了，让你用同一套代码在所有平台上调用 OpenGL 函数。

使用方式很简单：让你的 QOpenGLWidget 同时继承 `QOpenGLFunctions`（或者使用 `QOpenGLFunctions_3_3_Core` 等版本特定的类），然后在 `initializeGL` 中调用 `initializeOpenGLFunctions()`。之后你就可以直接使用 `glClear`、`glGenBuffers`、`glBindBuffer` 等标准的 OpenGL 函数了：

```cpp
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    // ...
};
```

如果你使用版本特定的类（比如 `QOpenGLFunctions_3_3_Core`），你可以确保使用的是 OpenGL 3.3 Core Profile 的函数集，不会意外调用到已废弃的旧 API。对于入门来说，直接用 `QOpenGLFunctions` 就够了，它默认提供 OpenGL ES 2.0 兼容的函数集，同时也包含了桌面 OpenGL 的大部分常用函数。

你可能会问：为什么不直接 `#include <GL/gl.h>` 然后调 `glClear()`？在 Windows 上，`GL/gl.h` 只提供 OpenGL 1.1 的函数，更高级的函数（VBO、着色器等）需要手动用 `wglGetProcAddress` 加载。Qt 帮你把这些脏活累活全干了，而且还能正确处理不同平台之间的差异。所以请使用 `QOpenGLFunctions`，不要自己去折腾平台相关的函数加载。

### 3.3 VAO、VBO 与绘制三角形

现在我们来做一件正事：在屏幕上画一个三角形。在 OpenGL 3.3 Core Profile 中，绘制任何东西都需要经过这几个步骤——准备顶点数据、创建并填充缓冲区（VBO）、设置顶点属性绑定（VAO）、编写着色器程序、发出绘制命令。这些概念初看起来很绕，我们一步步来。

VBO（Vertex Buffer Object）是 GPU 端的内存缓冲区，用来存储顶点数据。你把顶点的位置、颜色、纹理坐标等数据放进 VBO 里，GPU 就能高效地读取这些数据进行渲染。

VAO（Vertex Array Object）记录了顶点属性的绑定状态——也就是告诉 OpenGL "从哪个 VBO 的哪个偏移位置开始读取数据、每个顶点有几个分量、数据类型是什么"。有了 VAO，你每次绘制的时候只需要绑定对应的 VAO，OpenGL 就知道该怎么解释顶点数据了。

着色器（Shader）是运行在 GPU 上的小程序。顶点着色器（Vertex Shader）处理每个顶点的位置变换，片段着色器（Fragment Shader）决定每个像素的颜色。在 Core Profile 下，你必须自己写这两个着色器——OpenGL 不再提供任何默认的着色器。

我们来看具体的代码流程：

首先是着色器源代码。我们用 GLSL（OpenGL Shading Language）来编写，顶点着色器接收顶点位置并直接输出，片段着色器输出一个固定的颜色：

```cpp
// 顶点着色器：接收位置数据，直接传给 OpenGL
const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    void main() {
        gl_Position = vec4(aPos, 1.0);
    }
)";

// 片段着色器：输出橙色
const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0);  // 橙色
    }
)";
```

然后在 `initializeGL` 里创建着色器程序、VBO 和 VAO：

```cpp
void initializeGL() override
{
    initializeOpenGLFunctions();
    glClearColor(0.12f, 0.12f, 0.18f, 1.0f);

    // ---- 编译着色器 ----
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    // 检查编译是否成功……（完整代码见示例）

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // ---- 链接着色器程序 ----
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    // 着色器对象已经链接到程序中了，可以删除
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ---- 顶点数据 ----
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  // 左下
         0.5f, -0.5f, 0.0f,  // 右下
         0.0f,  0.5f, 0.0f   // 顶部
    };

    // ---- 创建 VAO 和 VBO ----
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    // 绑定 VAO（记录后续的顶点属性设置）
    glBindVertexArray(m_vao);

    // 绑定 VBO 并填充数据
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性：location=0，3 个 float，不归一化，步长 3*sizeof(float)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // 解绑 VBO 和 VAO（VAO 已经记录了绑定信息）
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
```

这段代码的执行流程是这样的：先编译顶点和片段着色器，把两者链接成一个着色器程序；然后定义三角形的三个顶点坐标（注意 OpenGL 的坐标系原点在屏幕中心，x 向右，y 向上，范围是 -1 到 1）；接着创建 VAO 和 VBO，把顶点数据上传到 GPU；最后通过 `glVertexAttribPointer` 告诉 OpenGL 如何解析 VBO 中的数据——从 location 0 开始，每个顶点 3 个 float 分量（x, y, z）。

绘制的时候就很简洁了：

```cpp
void paintGL() override
{
    glClear(GL_COLOR_BUFFER_BIT);

    // 使用着色器程序
    glUseProgram(m_shaderProgram);
    // 绑定 VAO（自动恢复了之前记录的顶点属性设置）
    glBindVertexArray(m_vao);
    // 发出绘制命令：画 3 个顶点组成的三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
```

你可能会觉得为了画一个三角形写这么多代码有点夸张——确实，OpenGL 的初始化代码量比较大，这是它跟 QPainter 最大的区别。但好处是一旦初始化完成，绘制任何复杂的几何体只需要修改顶点数据和着色器，核心代码结构不变。画一个三角形和画一百万个三角形的绘制代码几乎一样，区别只在于顶点数据的数量。

### 3.4 OpenGL 上下文生命周期与资源释放

OpenGL 资源（着色器程序、VBO、VAO、纹理等）是跟 OpenGL 上下文绑定的。如果上下文被销毁了，这些资源也会跟着失效。QOpenGLWidget 的上下文在 Widget 被销毁时自动释放，但我们需要确保在正确的时机释放我们手动创建的资源。

最佳实践是重写 `cleanupGL()` 函数（Qt 6 中新增）或者在析构函数中确保上下文是 current 状态后再释放：

```cpp
~GLWidget()
{
    // 确保上下文是当前的，然后释放 OpenGL 资源
    makeCurrent();
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteProgram(m_shaderProgram);
    doneCurrent();
}
```

这里有两个关键函数：`makeCurrent()` 把 OpenGL 上下文切换到当前线程，这样你才能调用 OpenGL 函数；`doneCurrent()` 释放上下文。在析构函数里你必须先 `makeCurrent()`，因为 OpenGL 的删除函数（`glDeleteBuffers` 等）要求当前线程有一个活跃的 OpenGL 上下文。

还有一个你可能遇到的坑：在构造函数中创建 OpenGL 资源。构造函数执行时 Widget 还没有显示，OpenGL 上下文还没有创建。如果你在构造函数里调 `glGenBuffers()`，要么函数指针为空直接崩溃，要么上下文不存在创建失败。所有的 OpenGL 资源创建都必须在 `initializeGL()` 中进行，这是一条铁律。

如果你需要在运行时更新 OpenGL 资源（比如重新加载着色器或者更新顶点数据），也要确保在正确的上下文中操作。最安全的方式是在 `paintGL` 或者通过 `makeCurrent()` / `doneCurrent()` 包裹的代码块中进行。

到这里你可以想一想：如果你要让这个三角形旋转起来，应该怎么实现？提示：你可以通过一个定时器不断调用 `update()` 触发 `paintGL`，然后在每帧中更新一个旋转角度的 uniform 变量传给着色器，在顶点着色器里用旋转矩阵变换顶点位置。这就是所有 OpenGL 动画的基本原理——每帧更新数据，重新绘制。

## 4. 踩坑预防

第一个坑是着色器编译失败但没有任何提示。GLSL 着色器代码是在运行时编译的，如果你的着色器代码有语法错误，`glCompileShader` 不会抛异常，只是编译失败。着色器编译失败后 `glDrawArrays` 画不出任何东西，画面一片空白，你完全不知道哪里出了问题。所以每次编译着色器之后一定要检查编译状态：

```cpp
GLint success;
glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    qDebug() << "着色器编译失败:" << infoLog;
}
```

着色器程序的链接也一样，需要检查 `GL_LINK_STATUS`。建议你把着色器编译和链接的检查封装成辅助函数，每次都用，别偷懒省掉——等到着色器代码复杂了之后，出问题排查起来会让你怀疑人生。

第二个坑是在错误的线程中调用 OpenGL 函数。QOpenGLWidget 的 OpenGL 上下文是跟 GUI 线程绑定的，你只能在 GUI 线程中调用 OpenGL 函数。如果你在后台线程中调 `glClear()` 或者 `glDrawArrays()`，要么直接崩溃，要么画出来的东西是空的。如果确实需要在后台线程做 OpenGL 渲染，你需要把 QOpenGLContext 移到后台线程，并且自己管理上下文的切换——这已经超出入门范围了，初学者先把所有 OpenGL 操作放在 GUI 线程中就好。

第三个坑是 OpenGL 版本兼容性。我们的示例使用 `#version 330 core`，需要 OpenGL 3.3 支持。如果你的系统只支持 OpenGL 2.1（比如老旧的集成显卡），着色器编译会直接失败。macOS 从某个版本开始只支持 Core Profile，如果你用了 `glBegin/glEnd` 这种旧式 API 会直接报错。建议在程序启动时检查 OpenGL 版本：

```cpp
// 在 initializeGL 中
const char *version = (const char *)glGetString(GL_VERSION);
qDebug() << "OpenGL 版本:" << version;
```

第四个坑是忘记调用 `initializeOpenGLFunctions()`。这个函数必须在 `initializeGL` 中调用，且只能调用一次。忘了它的话，所有 `glXxx` 函数调用都会崩溃——因为函数指针根本没有被初始化。如果你的程序在 `initializeGL` 的第一行 OpenGL 调用就崩溃了，99% 的概率是忘记调这个函数了。

## 5. 练习项目

我们来做一个实战练习：实现一个彩色旋转三角形。在基础三角形的基础上增加以下功能——每个顶点使用不同的颜色（红、绿、蓝），通过顶点颜色插值实现三角形表面的渐变色效果；使用 QTimer 驱动动画，每帧让三角形旋转一个小角度；通过一个旋转矩阵 uniform 变量把旋转角度传给顶点着色器；在 Widget 上添加两个按钮控制旋转的暂停和继续。

完成标准是：继承 QOpenGLWidget 和 QOpenGLFunctions，在 initializeGL 中创建着色器程序和 VAO/VBO，顶点数据包含位置和颜色两个属性；顶点着色器接收一个 `mat4` 类型的 uniform 旋转矩阵，用 `mat4 * vec4` 变换顶点位置；片段着色器接收从顶点着色器插值传来的颜色，直接输出；使用 `QTimer` 以约 60fps 的频率调用 `update()` 触发重绘，每帧更新旋转角度；在 resizeGL 中正确设置视口。几个提示：旋转矩阵可以用三角函数手动构造，也可以后续用 QMatrix4x4；顶点颜色插值是 OpenGL 的默认行为——只要顶点着色器中用 `out` 输出颜色变量、片段着色器中用对应的 `in` 接收，GPU 会自动在三个顶点之间做线性插值。

## 6. 官方文档参考链接

[Qt 文档 · QOpenGLWidget Class](https://doc.qt.io/qt-6/qopenglwidget.html) -- QOpenGLWidget 的完整 API，三个生命周期函数的详细说明

[Qt 文档 · QOpenGLFunctions Class](https://doc.qt.io/qt-6/qopenglfunctions.html) -- 跨平台 OpenGL 函数封装，包含所有常用 OpenGL 函数

[Qt 文档 · OpenGL Examples](https://doc.qt.io/qt-6/examples-widgets-opengl.html) -- Qt 官方 OpenGL 示例集合，包含 2D 和 3D 渲染的各种案例

[OpenGL 参考文档 · LearnOpenGL](https://learnopengl.com/) -- 学习 OpenGL 最好的在线教程，虽然不是 Qt 相关的，但 OpenGL 概念是通用的

[Qt 文档 · QOpenGLShaderProgram Class](https://doc.qt.io/qt-6/qopenglshaderprogram.html) -- Qt 封装的着色器程序类，可以简化着色器编译和链接的代码

---

到这里，QOpenGLWidget 的基本用法你已经掌握了：三个生命周期函数各自的职责、QOpenGLFunctions 的跨平台调用方式、VAO/VBO 的创建和绑定流程、着色器的编译和链接。记住最核心的一条：所有 OpenGL 资源的创建必须在 `initializeGL` 中进行，绘制操作在 `paintGL` 中执行，析构时先 `makeCurrent()` 再释放资源。这只是 OpenGL 的起点，但这个起点你已经站得很稳了。
