---
title: "0.2 第一个 CMake Qt6 工程"
description: "从零跑通一个 Qt 6 的 CMake 工程。CMakeLists.txt 逐行拆：最低版本 3.16 的出处、C++17 是硬要求、AUTOMOC 三件套在安排什么、find_package 的 COMPONENTS、target_link_libraries 的 PRIVATE。另含 out-of-source 构建的好处、资源文件 .qrc 的 :/ 路径坑、.ui 文件与 AUTOUIC、多模块结构与 PUBLIC/PRIVATE 的分工。附一个找错测验和一个改标准看报错的实验。"
---

# 现代Qt开发教程（新手篇）0.2——第一个 CMake Qt6 工程

qmake 的 `.pro` 文件语法亲切，网上教程成山，为什么 Qt 6 的世界非要换 CMake？因为官方把重心整个挪了过去：Qt 6 新增的构建命令，`qt_add_executable`、`qt_add_resources` 这一族，只在 CMake 侧存在，文档示例也全线 CMake 化。qmake 还能用，但新项目再从它起步，等于逆着官方的方向游。这一篇咱们从零建一个能跑的工程，CMakeLists.txt 每一行为什么在那，讲清楚。

## 十行代码，三道工序

代码本身十行不到：

```cpp
// main.cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLabel label("Hello, Qt 6!");
    label.resize(400, 300);
    label.show();

    return app.exec();
}
```

但这十行背后，构建系统要干三件普通 C++ 工程不用干的活。类头文件里出现 Q_OBJECT，就得先过 MOC 生成元对象代码，信号槽才有得连。图片、图标这类资源要过 RCC 编译进二进制。.ui 界面文件要过 UIC 转成 C++ 头文件。咱们看 CMakeLists.txt 时那些陌生的配置，大半就是在安排这三道工序。

## CMakeLists.txt 逐行拆

一份完整的配置长这样，咱们顺着往下过：

```cmake
cmake_minimum_required(VERSION 3.16)
project(HelloQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(HelloQt
    main.cpp
)

target_link_libraries(HelloQt PRIVATE
    Qt6::Widgets
)
```

开头两行是每个 CMake 工程的标配。`cmake_minimum_required` 的版本咱们跟着官方写 3.16：入门示例和 qtbase 源码顶层都是这个数，机器上的 CMake 比这新当然没问题。`project()` 声明项目名、版本、语言，VERSION 会在 CMake 里生成 `PROJECT_VERSION_MAJOR` 一族变量，代码里要用版本号时直接取。

`CMAKE_CXX_STANDARD` 给 17 是 Qt 6 的硬要求——它的头文件用上了 C++17 特性，标准给低，报错直接从 Qt 头文件深处冒出来，位置根本不在咱们自己的代码里。旁边的 `CMAKE_CXX_STANDARD_REQUIRED ON` 是让它硬得彻底：编译器不支持，配置阶段就报错拦下，而不是拖到编译期再炸。

接着三行 `CMAKE_AUTO*` 就是给开头那三道工序派活的：AUTOMOC 盯 Q_OBJECT，AUTORCC 盯资源，AUTOUIC 盯界面文件。开关一开，哪个文件变了、要重跑哪个工具，CMake 自己盯着，不用咱们操心。漏了 AUTOMOC 的后果很典型：编译期风平浪静，链接期报上一条 `undefined reference to vtable for XXX`——类里写了 Q_OBJECT，moc 的代码却没跟上，吓人的报错背后就这么个根因。补上开关，把含 Q_OBJECT 的文件列进目标，就平了。

`find_package(Qt6 REQUIRED COMPONENTS Widgets)` 是整份文件的灵魂，灵魂在 COMPONENTS。Qt 6 是模块化的，用到哪个模块才列哪个：Widgets 是桌面控件一族，几乎总在。Quick 归 QML，教程 Part 6 才登场。Network 和 Sql 在各自篇章出现。咱们全勾上去也不会报错，只是拖慢编译、撑大程序。REQUIRED 的意思是找不到就报错停下，绝不静默继续。

这个报错咱们在 0.1 篇见过面：`Could not find Qt6`。多半是 CMake 不知道 Qt 装在哪，也就是 CMAKE_PREFIX_PATH 没传进去，命令行下的给法：

```bash
cmake -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64
# Linux 换成 ~/Qt/6.9.1/gcc_64
```

最后两行收尾：咱们用 `add_executable` 建目标、列源文件，`target_link_libraries` 把 Qt6::Widgets 链进来，PRIVATE 修饰的意思是这层链接关系只归 HelloQt 自己，不外传。单目标的工程里 PRIVATE 看不出差别，等多模块一节 PUBLIC 登场，它的意义才显出来。

## 跑起来：out-of-source 的规矩

咱们把工程目录建好，构建目录单独开一个，人待在 build 里配置：

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/home/您的用户名/Qt/6.9.1/gcc_64
cmake --build .
./HelloQt          # MSVC 多配置生成器下在 Debug/HelloQt.exe
```

咱们看到末尾三行 `Configuring done`、`Generating done`、`Build files have been written to` 再编译，最后窗口弹出，"Hello, Qt 6!"。

特意单开 build 目录不是洁癖。CMake 生成的缓存、中间文件全关在里面，源码目录一个不沾。想推倒重来，删掉目录了事。版本控制也只需忽略 build/ 一项。咱们要是在源码目录里直接 `cmake .`，生成物撒一地，混进 git 提交后再清理就是体力活了。

## 拿一份有毛病的配置练手

下面这份配置能过配置阶段，但构建一定出问题，毛病在哪，咱们来当一回排查者：

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(MyApp
    main.cpp
    widget.cpp
)

target_link_libraries(MyApp PRIVATE Qt6::Widgets)
```

答案两处。`CMAKE_CXX_STANDARD` 给了 14，报错会从 Qt 头文件深处冒出来；AUTOMOC 没开，而 widget.cpp 里有带 Q_OBJECT 的类，链接期等着的 vtable 报错。对照上一节的逐行拆解，咱们两处都能对上号。

还想再狠一点，拿能跑的工程做实验：把 `CMAKE_CXX_STANDARD` 改成 14，重新配置构建，亲眼看编译器报的第一个错长什么样、落在哪个文件。见过一次，以后咱们在别的项目撞上同样的报错，就不会先怀疑自己的代码了。

## 资源文件：路径前那个冒号

咱们要把图片、图标弄进 Qt 工程，走的是 .qrc 这条路。`resources.qrc` 里登记文件：

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource>
        <file>images/icon.png</file>
    </qresource>
</RCC>
```

把它加进 `add_executable` 的源列表，AUTORCC 就接手了。咱们用的时候有个细节容易栽跟头：路径前面有个冒号，`QPixmap(":/images/icon.png")`。这个 `:/` 是资源系统的入口标记——资源已经编进二进制，只能从这条路走。写成 `QPixmap("images/icon.png")`，编译照样过、运行不报错，图片默默出不来，发布后才发现图标全丢。这种静默失败没有报错可看，排查起来最磨人，头一回写资源路径就把它记牢。

## .ui 文件：界面描述接进 CMake

咱们在 Qt Creator 里拖出来的界面存成 .ui 文件，本质是一份 XML，手写一个最小的也行：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>MainWindow</class>
 <widget class="QMainWindow" name="MainWindow">
  <property name="windowTitle">
   <string>Hello Qt</string>
  </property>
 </widget>
 <resources/>
 <connections/>
</ui>
```

咱们同样把它加进 `add_executable`。AUTOUIC 把它转成 `ui_mainwindow.h` 放进构建目录，代码里 include 进来用：

```cpp
#include <QApplication>
#include "ui_mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Ui::MainWindow ui;
    QMainWindow window;
    ui.setupUi(&window);

    window.show();
    return app.exec();
}
```

`ui.setupUi(&window)` 一执行，XML 里描述的控件就在 window 上建了出来，咱们拖出来的界面和手写的逻辑就这样接上了。

## 项目大了：抽库，让 PUBLIC 上场

项目一大，所有源文件挤一个目录就不好过了。咱们常见的做法是核心逻辑抽成库、主程序去链它：

```text
MyApp/
├── CMakeLists.txt
├── app/
│   ├── CMakeLists.txt
│   └── main.cpp
├── core/
│   ├── CMakeLists.txt
│   ├── core.h
│   └── core.cpp
└── resources/
    └── app.qrc
```

根 CMakeLists.txt 管全局，咱们用 `add_subdirectory(core)` 和 `add_subdirectory(app)` 把两个子模块挂进来。core 声明成库：

```cmake
# core/CMakeLists.txt
add_library(core
    core.h
    core.cpp
)

target_link_libraries(core PUBLIC
    Qt6::Widgets
)
```

app 是可执行文件，链接 core：

```cmake
# app/CMakeLists.txt
add_executable(MyApp
    main.cpp
    ../resources/app.qrc
)

target_link_libraries(MyApp PRIVATE
    core
)
```

值得咱们盯的是两处链接关键字。core 链 Widgets 用 PUBLIC：core 的头文件里出现 Qt 类型，依赖 core 的 app 编译时也得见到 Qt 的头文件和库，这层“顺带传下去”就是 PUBLIC 的意思。app 链 core 用 PRIVATE，因为 app 是链条终点，这层依赖到它为止。往外传播用 PUBLIC，自己消费用 PRIVATE，项目分层越复杂，这条规则越值钱。

## 官方文档参考

本篇基于 Qt 6.9.1 与 CMake 3.16+；文中工程在 Windows（MinGW/MSVC）与 Linux（GCC）行为一致。

[Qt 文档 · Build with CMake](https://doc.qt.io/qt-6/cmake-manual.html) · Qt 官方 CMake 手册，目标与命令的全量说明

[CMake · Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html) · CMake 官方循序渐进教程

[CMake · cmake-commands(7)](https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html) · 全部 CMake 命令的参考手册

---

环境搭建到此收官：Qt 装好、IDE 接上、第一个工程跑通。咱们想再练手的话，给 .ui 那份 XML 里塞一个 QPushButton 重新构建，看 AUTOUIC 自动重跑；下一篇 [1.1 QObject 与元对象系统](../01-qtbase/01-qobject-meta-system-beginner.md) 进 QtBase 正题，Q_OBJECT 这个到处出现的宏，就该拆开看看里面是什么了。
