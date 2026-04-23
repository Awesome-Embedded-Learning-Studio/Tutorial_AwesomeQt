# 现代Qt开发教程（新手篇）0.2——第一个 CMake Qt6 工程从零跑通

## 1. 前言：为什么非得用 CMake

说实话，我第一次接触 CMake 的时候，内心是拒绝的。qmake 用得好好的，为什么要换？结果配了一天 CMake，最后生成的还是一堆乱七八糟的文件。

但后来我发现，Qt 6 时代，CMake 已经是唯一的选择了。qmake 虽然还能用，但已经被官方标记为 "legacy"。而且 CMake 的威力是真的强大——跨平台构建、依赖管理、IDE 集成，这些都不是 qmake 能比的。

所以这一篇，我们不搞虚的，直接从零创建一个 Qt 6 的 CMake 工程。我会把每个字段、每个命令都解释清楚，让你不仅知其然，更知其所以然。

## 2. 最小可运行的 Qt 程序

在写 CMake 之前，先让我们看看一个最小的 Qt 程序长什么样：

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

就这么简单。但这背后，Qt 的构建系统需要做很多事情：MOC（Meta-Object Compiler）处理 Q_OBJECT 宏，RCC（Resource Compiler）把资源文件编译成 C++ 代码，UIC（User Interface Compiler）把 .ui 文件转成 C++ 代码。CMake 会自动调用这些工具，前提是你的 CMakeLists.txt 写对了。

## 3. CMakeLists.txt 逐行解析

下面是一个标准的 Qt 6 项目的 CMakeLists.txt，我会逐行解释：

```cmake
# 1. 指定 CMake 最低版本
cmake_minimum_required(VERSION 3.26)

# 2. 项目名称和语言
project(HelloQt VERSION 1.0 LANGUAGES CXX)

# 3. 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. 自动处理 Qt 的 MOC、RCC、UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 5. 查找 Qt6 包
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# 6. 创建可执行文件
add_executable(HelloQt
    main.cpp
)

# 7. 链接 Qt 库
target_link_libraries(HelloQt PRIVATE
    Qt6::Widgets
)
```

我们从上往下走。`cmake_minimum_required(VERSION 3.26)` 告诉 CMake 这个项目至少需要 3.26 版本，因为 Qt 6.9.1 就要求这个版本起步。`project(HelloQt VERSION 1.0 LANGUAGES CXX)` 声明了项目名称是 HelloQt，版本号 1.0，使用 C++ 语言——这里的 VERSION 会自动生成版本宏，后面可以用。`CMAKE_CXX_STANDARD` 设为 17 是因为 Qt 6 需要 C++17 或更高，`CMAKE_CXX_STANDARD_REQUIRED ON` 表示编译器必须支持这个标准，不支持就报错。

然后是三件套：`CMAKE_AUTOMOC` 让 CMake 自动运行 MOC 处理信号槽机制，`CMAKE_AUTORCC` 自动处理资源文件（图片、图标之类），`CMAKE_AUTOUIC` 自动把 .ui 界面文件转成 C++ 代码。这三个开关开了之后，你基本不用手动管 Qt 的代码生成工具了。

`find_package(Qt6 REQUIRED COMPONENTS Widgets)` 是整个配置的灵魂——它告诉 CMake 去找 Qt6 的 Widgets 模块，`REQUIRED` 意味着找不到就直接报错，不会静默跳过。`add_executable(HelloQt main.cpp)` 创建一个叫 HelloQt 的可执行文件，后面的 main.cpp 是源文件。最后 `target_link_libraries(HelloQt PRIVATE Qt6::Widgets)` 把 Qt 的 Widgets 库链接进来，`PRIVATE` 表示这个链接只对 HelloQt 自己生效，不会传染给依赖 HelloQt 的其他目标。

这里有个大坑——如果你忘了设 CMAKE_PREFIX_PATH，`find_package(Qt6)` 会直接告诉你 "Could not find Qt6"。CMake 需要知道 Qt 安装在哪里，CMAKE_PREFIX_PATH 就是那张地图。配置的时候加上 `-DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.1/gcc_64` 就行了。

### 3.2 COMPONENTS 参数详解

`find_package(Qt6 REQUIRED COMPONENTS Widgets)` 里的 `COMPONENTS` 是什么意思？Qt 6 是模块化的，不同的功能在不同的模块里。Widgets 是传统桌面控件（QPushButton、QLabel），几乎总是需要；Quick 是 QML/Qt Quick 现代界面，写 QML 时才需要；Network 是网络编程（QTcpSocket、QHttp），做网络请求时才需要；Sql 是数据库（QSqlDatabase、QSqlQuery），操作数据库时才需要。你只需要引入你用到的模块，不需要的模块不要加，否则会增加编译时间和最终程序大小。

## 4. 从零创建项目

### 4.1 创建目录结构

```bash
# 创建项目目录
mkdir HelloQt && cd HelloQt

# 创建文件
touch main.cpp CMakeLists.txt

# 目录结构应该是这样：
# HelloQt/
# ├── CMakeLists.txt
# └── main.cpp
```

### 4.2 写入代码

把上面的 main.cpp 和 CMakeLists.txt 内容分别填入对应的文件。

### 4.3 配置项目

```bash
# 创建构建目录（推荐 out-of-source 构建）
mkdir build && cd build

# 配置项目（记得替换成你的 Qt 路径）
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64

# Linux 下：
cmake .. -DCMAKE_PREFIX_PATH=/home/你的用户名/Qt/6.9.1/gcc_64
```

如果配置成功，你会看到：

```
-- Configuring done
-- Generating done
-- Build files have been written to: /path/to/HelloQt/build
```

你可能注意到我们是在 build 目录里编译的，而不是直接在源码目录里。这种 out-of-source 构建方式有几个好处：源码目录保持干净，CMake 生成的文件都在 build 里；想清理构建产物的时候直接删 build 目录就行；而且 .gitignore 只需要忽略 build/ 目录，不会污染 Git 仓库。

### 4.4 编译运行

```bash
# 编译
cmake --build .

# 运行（Windows）
./Debug/HelloQt.exe

# 运行（Linux/WSL2）
./HelloQt
```

如果一切顺利，你会看到一个显示 "Hello, Qt 6!" 的窗口。

## 5. 常见编译错误及解决

### 5.1 "Could not find Qt6"

CMake 找不到 Qt 安装位置。解决方法就是设置 CMAKE_PREFIX_PATH：

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.1/gcc_64
```

### 5.2 "The C++ compiler does not support C++17"

编译器太老了。升级编译器，或者确保 `CMAKE_CXX_STANDARD` 设为 17。

### 5.3 "moc_xxx.cpp not found"

AUTOMOC 没开。确保 CMakeLists.txt 里有：

```cmake
set(CMAKE_AUTOMOC ON)
```

### 5.4 "undefined reference to vtable for XXX"

这个报错看着吓人，实际上就是类里写了 Q_OBJECT 但没有 moc 生成的代码被链接。确保 AUTOMOC 开启，并且把包含 Q_OBJECT 的头文件加到 add_executable 里。

我们来试一个调试练习。下面这段 CMakeLists.txt 有好几处问题，看看你能找出多少：

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt6 REQUIRED Widgets)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp Qt6::Widgets)
```

问题不少：VERSION 3.16 太低了，Qt 6.9.1 需要 3.26 以上；C++14 也不够，Qt 6 要求 C++17；`find_package` 缺少 `COMPONENTS` 关键字；`target_link_libraries` 缺少 `PRIVATE`；更别说还缺少整个 AUTOMOC/AUTORCC/AUTOUIC 三件套。这六处问题不改完，构建一定失败。

## 6. 进阶：添加资源文件

Qt 项目常常需要添加图片、图标等资源。这时候需要用 .qrc 资源文件。

### 6.1 创建资源文件

创建一个 `resources.qrc` 文件：

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource>
        <file>images/icon.png</file>
    </qresource>
</RCC>
```

### 6.2 更新 CMakeLists.txt

```cmake
add_executable(HelloQt
    main.cpp
    resources.qrc
)
```

因为我们之前已经开了 AUTORCC，CMake 会自动调用 RCC 把资源编译进可执行文件。

### 6.3 在代码中使用

```cpp
#include <QApplication>
#include <QLabel>
#include <QPixmap>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLabel label;
    label.setPixmap(QPixmap(":/images/icon.png"));
    label.show();

    return app.exec();
}
```

这里有个很容易栽的跟头——注意资源路径前面的冒号。`QPixmap(":/images/icon.png")` 是对的，`QPixmap("images/icon.png")` 是错的。冒号前缀 `:/` 是 Qt 资源系统的标识，不用它的话程序运行时找不到图片，或者发布之后图片就丢了。因为资源文件已经被编译进可执行文件里，只能通过 `:/` 路径访问，不能当普通文件路径用。

## 7. 进阶：使用 .ui 文件

### 7.1 创建 .ui 文件

在 Qt Creator 里设计界面，会自动生成 .ui 文件。或者手动创建一个简单的：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>MainWindow</class>
 <widget class="QMainWindow" name="MainWindow">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>400</width>
    <height>300</height>
   </rect>
  </property>
  <property name="windowTitle">
   <string>Hello Qt</string>
  </property>
  <widget class="QWidget" name="centralwidget">
   <widget class="QPushButton" name="pushButton">
    <property name="geometry">
     <rect>
      <x>150</x>
      <y>120</y>
      <width>93</width>
      <height>29</height>
     </rect>
    </property>
    <property name="text">
     <string>Click Me</string>
    </property>
   </widget>
  </widget>
 </widget>
 <resources/>
 <connections/>
</ui>
```

### 7.2 更新 CMakeLists.txt

```cmake
add_executable(HelloQt
    main.cpp
    mainwindow.ui
)
```

### 7.3 在代码中使用

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

整合一下上面学的所有内容，一个包含 .ui 文件的完整 CMakeLists.txt 应该是这样：`cmake_minimum_required(VERSION 3.26)`，`project(MyApp VERSION 1.0 LANGUAGES CXX)`，C++ 标准设 17，AUTOMOC 和 AUTOUIC 都开，`find_package(Qt6 REQUIRED COMPONENTS Widgets)`，add_executable 里加上 main.cpp 和 mainwindow.ui，最后 `target_link_libraries(MyApp PRIVATE Qt6::Widgets)`。缺任何一个都会出问题。

## 8. 多模块项目结构

实际项目中，代码通常会分成多个模块：

```
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

根目录的 CMakeLists.txt：

```cmake
cmake_minimum_required(VERSION 3.26)
project(MyApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_subdirectory(core)
add_subdirectory(app)
```

core/CMakeLists.txt：

```cmake
add_library(core
    core.h
    core.cpp
)

target_link_libraries(core PUBLIC
    Qt6::Widgets
)
```

app/CMakeLists.txt：

```cmake
add_executable(MyApp
    main.cpp
    ../resources/app.qrc
)

target_link_libraries(MyApp PRIVATE
    core
)
```

你会发现这里用了 `add_subdirectory` 来引入子模块，core 作为一个库被 app 链接。core 里用的是 `PUBLIC` 链接 Qt6::Widgets，这样依赖 core 的目标也会自动获得 Qt 的头文件和库路径；而 app 里链接 core 用的是 `PRIVATE`，只对 MyApp 自己可见。这种分层结构在项目变大之后会非常有用。

## 9. 练习项目

**练习项目：Qt 计算器**

创建一个简单的计算器程序，支持加减乘除四种运算。

完成标准：使用 CMake 构建系统，界面用 .ui 文件设计（有数字按钮和运算符按钮），有一个显示结果的 QLineEdit，点击等号按钮能计算结果，能处理除零错误。

CMakeLists.txt 需要链接 QtWidgets 模块，.ui 文件需要加到 add_executable 里。逻辑上用 QLineEdit::text() 获取输入，转成数字进行运算，除零时显示 "Error"。

## 10. 官方文档参考

[CMake 手册 - Qt 6](https://doc.qt.io/qt-6/cmake-manual.html) · Qt 官方的 CMake 使用指南
[CMake 命令参考](https://doc.qt.io/qt-6/cmake-commands-api.html) · CMake 命令参考
[CMake 教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html) · CMake 官方教程

*（链接已验证，2026-03-17 可访问）*

---

**到这里你的第一个 Qt 6 项目就跑通了！** 掌握了 CMake 基本用法，后面的项目构建都不在话下。下一节我们会深入 Qt 的核心——信号槽机制。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
