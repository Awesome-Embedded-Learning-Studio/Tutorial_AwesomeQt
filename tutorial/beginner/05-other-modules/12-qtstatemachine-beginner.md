# 现代Qt开发教程（新手篇）5.12——Qt StateMachine 状态机框架

## 1. 前言：状态管理从面条代码到状态机

写 GUI 程序的时候，状态管理是一个绕不开的问题。一个登录界面就有"未登录"、"正在登录"、"登录成功"、"登录失败"四种状态，每种状态下按钮的启用/禁用、输入框的可编辑性、提示文字的内容都不一样。如果你用 bool 变量加 if-else 来管理，三个状态还算能维护，五个以上就开始乱了，十个以上基本就是面条代码——每次状态变化都要手动更新一堆 UI 属性，漏一个就是 bug。

Qt StateMachine 框架（Qt 6 中属于 QtStateMachine 模块）用有限状态机（Finite State Machine, FSM）的模型来解决这个问题。你只需要定义每个状态的属性配置和状态之间的转换条件，框架帮你管理状态切换时的属性变化和事件分发。这不是什么新概念——有限状态机在编译器、网络协议、游戏 AI 里用了几十年了——但 Qt 把它做成了一个类型安全、信号驱动、和 Qt 事件循环无缝集成的框架，用起来比手写状态管理代码优雅得多。

这篇我们要做的是从 QStateMachine + QState + QFinalState 的基础结构开始，逐步掌握 addTransition 信号/事件触发转换、assignProperty 状态进入时修改属性、层次状态机的构建方法。学完之后你会发现，很多之前用一堆 bool 和 switch-case 管理的 UI 状态逻辑，用状态机几行就能表达清楚。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 QtStateMachine 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core StateMachine Widgets)
```

QtStateMachine 在 Qt 6 中从 Qt 5 时代的 QtWidgets 子模块独立成了单独的 QtStateMachine 模块。在 Qt Installer 里需要确保勾选了这个组件。从源码编译时它在 `qtstatemachine` 仓库中。某些发行版的包管理器可能叫 `qt6-statemachine`。

QtStateMachine 的核心类都在 QState、QStateMachine、QFinalState 这些头文件中，不需要额外的 include 路径配置。它和 Qt 事件循环深度集成——状态机本身就在事件循环中驱动，不需要额外的线程或定时器。

编译工具链方面，MSVC 2019+、GCC 11+、Clang 14+ 均可，C++17 标准，CMake 3.26+ 构建系统。

## 3. 核心概念讲解

### 3.1 QStateMachine + QState + QFinalState——状态机的三件套

Qt 状态机的核心结构由三个类组成。QStateMachine 是状态机引擎，管理整个状态机的运行和事件分发。QState 表示一个具体的状态，你可以给它配置进入/离开时执行的动作，以及从它出发的转换条件。QFinalState 是一个特殊的终态——当状态机进入 QFinalState 时，状态机的 finished() 信号会触发，表示整个流程结束。

一个最简的状态机可以这样搭建：

```cpp
#include <QStateMachine>
#include <QState>
#include <QFinalState>

auto *machine = new QStateMachine(this);

// 创建状态
auto *idle = new QState();        // 空闲状态
auto *running = new QState();     // 运行中状态
auto *done = new QFinalState();   // 终态

// 给状态机设置初始状态
machine->addState(idle);
machine->addState(running);
machine->addState(done);
machine->setInitialState(idle);

// 启动状态机
machine->start();
```

状态机启动后自动进入初始状态（idle）。此时状态机就在事件循环中等待——它不消耗 CPU，只是在等待触发转换的事件或信号。

你可以连接状态机的 started()、stopped()、finished() 信号来监控状态机的生命周期。每个 QState 也有 entered() 和 exited() 信号，在状态进入和离开时触发——这对于调试和执行副作用非常有用：

```cpp
connect(idle, &QState::entered, this, []() {
    qDebug() << "进入空闲状态";
});

connect(running, &QState::entered, this, []() {
    qDebug() << "进入运行状态";
});
```

需要注意的一点：QState 的 entered() 信号在状态进入时触发，此时 assignProperty 设置的属性值已经生效。exited() 信号在状态离开时触发，此时属性还没有恢复——恢复发生在 exited() 信号之后。如果你需要在属性恢复前做些什么，用 exited() 信号；如果需要在属性设置后做些什么，用 entered() 信号。

### 3.2 addTransition——信号触发和事件触发的转换

状态之间的转换（Transition）是状态机的核心机制。Qt 提供了两种最常用的转换类型：信号触发转换和事件触发转换。

信号触发转换通过 addTransition() 方法添加，当指定的信号发射时自动执行状态转换：

```cpp
// 当 startButton 的 clicked 信号发射时，从 idle 转到 running
idle->addTransition(startButton, &QPushButton::clicked, running);

// 当 stopButton 的 clicked 信号发射时，从 running 转到 done
running->addTransition(stopButton, &QPushButton::clicked, done);
```

addTransition 的三个参数分别是：信号发送对象、信号（函数指针形式）、目标状态。信号触发转换是 Qt 状态机最常用的转换方式，因为它和 Qt 的信号/槽机制无缝对接——任何 QObject 的信号都可以作为触发条件。

你也可以用一个按钮的信号触发多个不同的转换（从不同状态出发），这在实际 UI 开发中很常见。比如同一个"提交"按钮，在"表单填写中"状态下触发"校验并提交"的转换，在"校验失败"状态下触发"重新校验"的转换。状态机根据当前状态自动决定走哪条转换路径。

如果你需要更复杂的转换条件，可以使用 QSignalTransition 或 QEventTransition 手动创建转换对象并设置条件守卫（guard condition）：

```cpp
auto *transition = new QSignalTransition(
    startButton, &QPushButton::clicked, idle);
transition->setTargetState(running);

// 设置守卫条件：只有当输入框不为空时才允许转换
connect(transition, &QSignalTransition::triggered, this,
        [this, transition]() {
    bool valid = !inputEdit->text().isEmpty();
    // 可以在这里做额外的逻辑
});

idle->addTransition(transition);
```

事件触发转换用于响应 QEvent。比如你想让某个状态在按键按下时转换：

```cpp
#include <QEventTransition>
#include <QKeyEventTransition>

// 当 widget 上按下 Escape 键时，从 running 转到 idle
auto *keyTransition = new QKeyEventTransition(
    widget, QEvent::KeyPress, Qt::Key_Escape, running);
keyTransition->setTargetState(idle);
running->addTransition(keyTransition);
```

QEventTransition 是通用的事件触发转换，QKeyEventTransition 是它的按键特化版本。事件触发转换在需要响应键盘、鼠标等底层输入事件时很有用，但在一般的 UI 状态管理中用得比较少——大部分场景用信号触发转换就够了。

### 3.3 assignProperty——状态进入时自动修改属性

状态机最实用的功能之一是 assignProperty——当进入某个状态时，自动修改指定对象的属性值。这比手动在 entered() 信号里写一堆 setEnabled()/setText() 干净得多：

```cpp
// 空闲状态下：启动按钮可用，停止按钮禁用
idle->assignProperty(startButton, "enabled", true);
idle->assignProperty(stopButton, "enabled", false);
idle->assignProperty(statusLabel, "text", "空闲");

// 运行状态下：启动按钮禁用，停止按钮可用
running->assignProperty(startButton, "enabled", false);
running->assignProperty(stopButton, "enabled", true);
running->assignProperty(statusLabel, "text", "运行中...");

// 完成状态下：两个按钮都禁用
done->assignProperty(startButton, "enabled", false);
done->assignProperty(stopButton, "enabled", false);
done->assignProperty(statusLabel, "text", "完成");
```

assignProperty 的三个参数分别是：目标 QObject、属性名（字符串）、属性值（QVariant）。它在状态进入时自动调用目标对象的 setProperty()。这意味着只要对象有 Q_PROPERTY 定义的属性（包括 Qt 内置控件的属性如 "enabled"、"text"、"visible" 等），都可以用 assignProperty 来设置。

状态切换时，assignProperty 的执行顺序是这样的：先执行离开状态的 exited() 信号，然后恢复之前状态 assignProperty 设置过的属性（如果新状态也设置了同一个属性，就覆盖为新的值），最后执行进入状态的 entered() 信号。属性的恢复机制依赖于 Qt 的属性系统——它会记住每个 assignProperty 之前的原始值，在状态离开时恢复。如果你不想自动恢复，可以使用 QState::RestoreProperties 标志来控制。

### 3.4 层次状态机——子状态与并行状态

当状态数量增多时，把所有状态平铺会导致状态机变得难以维护。Qt 支持层次状态机——一个 QState 可以包含子状态，形成树形层次结构。这在处理"大状态下有多个小状态"的场景时非常有用。

```cpp
// "运行"状态包含两个子状态："初始化"和"处理中"
auto *running = new QState();
auto *initializing = new QState(running);  // 子状态，父状态是 running
auto *processing = new QState(running);    // 子状态，父状态是 running

running->setInitialState(initializing);  // 进入 running 时先进入 initializing

// 子状态之间的转换
initializing->addTransition(initializeDoneSignal, processing);
```

层次状态机的核心规则是：当状态机处于一个子状态时，它同时也处于父状态。比如状态机处于 "processing" 时，它也处于 "running"。这意味着在 "running" 层面设置的 assignProperty 和转换条件对 "processing" 也有效。

父状态可以设置转换到其他同级状态——当这个转换触发时，所有子状态都会退出。比如在 "running"（不管当前是 initializing 还是 processing）上添加一个到 "idle" 的转换：

```cpp
// 从 running 的任意子状态都能转到 idle
running->addTransition(stopButton, &QPushButton::clicked, idle);
```

这种"父状态上的转换对所有子状态生效"的机制大大简化了状态机的复杂度。不需要在每个子状态上重复添加相同的转换条件。

Qt 还支持并行状态（QState::ParallelStates）——一个父状态下的多个子状态同时活跃。这在需要同时管理多个独立状态维度的场景中很有用，比如一个设备同时有"连接状态"和"认证状态"两个独立的维度。不过并行状态会增加状态组合的复杂性（两个各有 3 个子状态的并行状态会产生 9 种组合），使用时要注意控制规模。

## 4. 综合示例：状态机驱动的任务管理器

把前面学的串起来，我们写一个状态机驱动的任务管理器界面。界面有四个状态：空闲、运行中、暂停、完成。每个状态下按钮的启用/禁用和状态文字都由状态机自动管理。同时演示层次状态机的用法——"运行中"状态下包含"初始化"和"处理"两个子状态。完整代码见 `examples/beginner/05-other-modules/12-qtstatemachine-beginner/`，下面是关键部分的讲解。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core StateMachine Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::StateMachine Qt6::Widgets)
```

状态机构建：

```cpp
void setupStateMachine()
{
    machine_ = new QStateMachine(this);

    // 四个顶层状态
    auto *idle = new QState();
    auto *running = new QState();    // 包含子状态
    auto *paused = new QState();
    auto *done = new QFinalState();

    // "运行中"的子状态
    auto *init = new QState(running);
    auto *processing = new QState(running);
    running->setInitialState(init);

    // assignProperty：每个状态控制 UI 属性
    idle->assignProperty(start_btn_, "enabled", true);
    idle->assignProperty(pause_btn_, "enabled", false);
    idle->assignProperty(resume_btn_, "enabled", false);
    idle->assignProperty(status_label_, "text", "空闲");
    idle->assignProperty(progress_bar_, "value", 0);

    init->assignProperty(status_label_, "text",
                         "运行中 - 初始化...");
    processing->assignProperty(status_label_, "text",
                              "运行中 - 处理中...");

    paused->assignProperty(start_btn_, "enabled", false);
    paused->assignProperty(pause_btn_, "enabled", false);
    paused->assignProperty(resume_btn_, "enabled", true);
    paused->assignProperty(status_label_, "text", "已暂停");

    done->assignProperty(start_btn_, "enabled", true);
    done->assignProperty(pause_btn_, "enabled", false);
    done->assignProperty(resume_btn_, "enabled", false);
    done->assignProperty(status_label_, "text", "完成");
    done->assignProperty(progress_bar_, "value", 100);

    // 转换条件
    idle->addTransition(start_btn_, &QPushButton::clicked, running);
    running->addTransition(pause_btn_, &QPushButton::clicked, paused);
    paused->addTransition(resume_btn_, &QPushButton::clicked, running);
    running->addTransition(this, &TaskWindow::taskFinished, done);

    // 子状态转换
    init->addTransition(this, &TaskWindow::initFinished, processing);

    // 注册所有状态
    machine_->addState(idle);
    machine_->addState(running);
    machine_->addState(paused);
    machine_->addState(done);
    machine_->setInitialState(idle);

    // 监控状态切换（调试用）
    connect(idle, &QState::entered,
            this, []() { qDebug() << "进入空闲"; });
    connect(init, &QState::entered,
            this, []() { qDebug() << "进入初始化"; });
    connect(processing, &QState::entered,
            this, []() { qDebug() << "进入处理中"; });
    connect(paused, &QState::entered,
            this, []() { qDebug() << "进入暂停"; });
    connect(done, &QState::entered,
            this, []() { qDebug() << "进入完成"; });

    machine_->start();
}
```

运行程序后，你会看到按钮的启用/禁用和状态文字完全由状态机驱动——不需要手动写任何 setEnabled() 或 setText() 调用。点击"开始"按钮后状态从空闲进入运行中（先是初始化子状态，然后切换到处理中子状态），点击"暂停"按钮进入暂停状态（暂停到运行中的恢复会回到当前子状态），任务完成后进入终态。你会发现，对比用 if-else 手写同样逻辑的代码，状态机版本更清晰，也更容易扩展——新增一个状态只需要定义它的属性配置和转换条件，不会影响已有的状态逻辑。

## 5. 练习项目

练习项目：状态机驱动的登录流程。

我们要用状态机管理一个完整的登录流程，包含四个状态：表单填写、校验中、登录中、登录结果。表单填写状态下用户可以输入用户名和密码，校验中状态下检查输入合法性（非空检查），登录中状态下模拟网络请求（用 QTimer 延迟模拟），登录结果状态分两个子状态——成功和失败。

完成标准是这样的：使用 assignProperty 控制每个状态下输入框和按钮的启用/禁用状态；表单填写状态下用户名或密码为空时"登录"按钮禁用（可以用信号触发转换的守卫条件或者在状态内部动态控制）；校验失败时从校验中状态回到表单填写状态并显示错误提示；登录成功后显示成功页面，登录失败后允许重试回到表单填写状态；使用层次状态机把登录结果状态的"成功"和"失败"组织为子状态。

几个实现提示：用户名/密码的非空检查可以在转换触发前用 lambda 连接 QSignalTransition 的信号做拦截，也可以直接在按钮 clicked 信号的槽中做前置检查再发射自定义信号触发转换；登录模拟用 QTimer::singleShot 加延迟即可；assignProperty 设置 QLabel 的 "text" 属性来切换提示文字，设置 QWidget 的 "visible" 属性来控制成功/失败页面的显示。

## 6. 官方文档参考

[Qt 文档 · QtStateMachine 模块](https://doc.qt.io/qt-6/qtstatemachine-index.html) -- 状态机模块总览

[Qt 文档 · QStateMachine](https://doc.qt.io/qt-6/qstatemachine.html) -- 状态机引擎

[Qt 文档 · QState](https://doc.qt.io/qt-6/qstate.html) -- 状态类

[Qt 文档 · QFinalState](https://doc.qt.io/qt-6/qfinalstate.html) -- 终态类

[Qt 文档 · QSignalTransition](https://doc.qt.io/qt-6/qsignaltransition.html) -- 信号触发转换

[Qt 文档 · QEventTransition](https://doc.qt.io/qt-6/qeventtransition.html) -- 事件触发转换

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。Qt StateMachine 的核心价值在于把"状态-属性-转换"的三角关系用声明式的方式表达出来，取代命令式的 if-else 状态管理。QStateMachine + QState + QFinalState 定义状态骨架，addTransition 连接信号/事件触发转换，assignProperty 自动管理 UI 属性——这三件套组合起来，大部分 UI 状态管理场景都能覆盖。层次状态机在复杂场景下是必需品——没有它，状态数量会呈指数级膨胀。建议先从简单的两三个状态的状态机开始练习，熟练之后再尝试层次状态机和并行状态。状态机思维一旦建立起来，你会发现很多之前觉得"乱"的 UI 逻辑其实都可以用清晰的状态模型来描述。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
