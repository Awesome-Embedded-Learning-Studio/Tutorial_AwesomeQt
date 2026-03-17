━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
入门 · 环境搭建 · 02 · 第一个 CMake Qt6 工程从零跑通
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

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

就这么简单。但这背后，Qt 的构建系统需要做很多事情：
- MOC（Meta-Object Compiler）：处理 Q_OBJECT 宏
- RCC（Resource Compiler）：把资源文件编译成 C++ 代码
- UIC（User Interface Compiler）：把 .ui 文件转成 C++ 代码

CMake 会自动调用这些工具，前提是你的 CMakeLists.txt 写对了。

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

### 3.1 逐字段解释

| 字段 | 作用 | 为什么这样写 |
|------|------|-------------|
| `cmake_minimum_required(VERSION 3.26)` | CMake 最低版本要求 | Qt 6.9.1 需要 CMake ≥ 3.26 |
| `project(HelloQt VERSION 1.0 LANGUAGES CXX)` | 项目名称和语言 | VERSION 会自动生成版本宏 |
| `CMAKE_CXX_STANDARD 17` | C++ 标准 | Qt 6 需要 C++17 或更高 |
| `CMAKE_AUTOMOC ON` | 自动运行 MOC | Qt 的信号槽机制需要 |
| `CMAKE_AUTORCC ON` | 自动运行 RCC | 资源文件（图片、图标）需要 |
| `CMAKE_AUTOUIC ON` | 自动运行 UIC | .ui 界面文件需要 |
| `find_package(Qt6 REQUIRED COMPONENTS Widgets)` | 查找 Qt6 的 Widgets 模块 | REQUIRED 表示必须找到，找不到就报错 |
| `add_executable(HelloQt main.cpp)` | 创建可执行文件 | 第一个参数是名字，后面是源文件 |
| `target_link_libraries(HelloQt PRIVATE Qt6::Widgets)` | 链接 Qt 库 | PRIVATE 表示这个链接只对 HelloQt 生效 |

> ⚠️ **坑 #1：CMAKE_PREFIX_PATH 没设置**
> ❌ 错误做法：find_package(Qt6) 找不到，不知道问题在哪
> ✅ 正确做法：配置时加上 `-DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.1/gcc_64`
> 💥 后果：CMake 报错 "Could not find Qt6"
> 💡 一句话记住：CMake 需要知道 Qt 安装在哪里，CMAKE_PREFIX_PATH 就是地图

### 3.2 COMPONENTS 参数详解

`find_package(Qt6 REQUIRED COMPONENTS Widgets)` 里的 `COMPONENTS` 是什么意思？

Qt 6 是模块化的，不同的功能在不同的模块里：

| 模块 | 功能 | 何时需要 |
|------|------|----------|
| Widgets | 传统桌面控件（QPushButton、QLabel） | 几乎总是需要 |
| Quick | QML/Qt Quick 现代界面 | 写 QML 时需要 |
| Network | 网络编程（QTcpSocket、QHttp） | 做网络请求时需要 |
| Sql | 数据库（QSqlDatabase、QSqlQuery） | 操作数据库时需要 |
| Widgets | GUI 界面 | 传统桌面程序 |

你只需要引入你用到的模块。不需要的模块不要加，否则会增加编译时间和最终程序大小。

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

> 📝 **随堂测验：口述回答**
> 用自己的话说说：为什么推荐 out-of-source 构建（在 build 目录里编译）？
>
> *(请先自己想一下，再往下滑看答案)*
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> - 源码目录保持干净，CMake 生成的文件都在 build 里
> - 方便清理构建产物：直接删 build 目录就行
> - 避免污染 Git 仓库，.gitignore 只需要忽略 build/
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 5. 常见编译错误及解决

### 5.1 "Could not find Qt6"

**原因**：CMake 找不到 Qt 安装位置

**解决**：设置 CMAKE_PREFIX_PATH

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.1/gcc_64
```

### 5.2 "The C++ compiler does not support C++17"

**原因**：编译器太老

**解决**：升级编译器，或者确保 `CMAKE_CXX_STANDARD` 设为 17

### 5.3 "moc_xxx.cpp not found"

**原因**：AUTOMOC 没开

**解决**：确保 CMakeLists.txt 里有：

```cmake
set(CMAKE_AUTOMOC ON)
```

### 5.4 "undefined reference to vtable for XXX"

**原因**：类里有 Q_OBJECT 但没有 moc 生成的代码被链接

**解决**：
- 确保 AUTOMOC 开启
- 把包含 Q_OBJECT 的头文件加到 add_executable 里

> 🐛 **随堂测验：调试挑战**
>
> 以下 CMakeLists.txt 有问题，请问哪里错了，会导致什么后果？
>
> ```cmake
> cmake_minimum_required(VERSION 3.16)
> project(MyApp VERSION 1.0 LANGUAGES CXX)
>
> set(CMAKE_CXX_STANDARD 14)
>
> find_package(Qt6 REQUIRED Widgets)
>
> add_executable(MyApp main.cpp)
> target_link_libraries(MyApp Qt6::Widgets)
> ```
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> - VERSION 3.16 太低，Qt 6.9.1 需要 ≥ 3.26
> - C++14 不够，Qt 6 需要 C++17
> - find_package 缺少 COMPONENTS
> - target_link_libraries 缺少 PRIVATE
> - 缺少 AUTOMOC/AUTORCC/AUTOUIC
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

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

CMake 会自动调用 RCC 把资源编译进可执行文件。

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

> ⚠️ **坑 #2：资源路径冒号**
> ❌ 错误做法：`QPixmap("images/icon.png")` // 直接用文件路径
> ✅ 正确做法：`QPixmap(":/images/icon.png")` // 用资源路径，前面加冒号
> 💥 后果：程序运行时找不到图片，或者发布后图片丢失
> 💡 一句话记住：资源路径用冒号前缀 `:/`，这是 Qt 资源系统的标识

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

> 🔲 **随堂测验：代码填空**
> 补全以下 CMakeLists.txt，使其能正确编译一个包含 .ui 文件的 Qt 项目：
>
> ```cmake
> cmake_minimum_required(VERSION ______)
> project(MyApp VERSION 1.0 LANGUAGES CXX)
>
> set(CMAKE_CXX_STANDARD __)
> set(CMAKE_AUTOMOC __)
> set(CMAKE_AUTOUIC ON)
>
> find_package(Qt6 ______ COMPONENTS Widgets)
>
> add_executable(MyApp
>     main.cpp
>     mainwindow.____
> )
>
> target_link_libraries(MyApp ______ Qt6::Widgets)
> ```
>
> *(提示：3.26、17、ON、REQUIRED、.ui、PRIVATE)*
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> ```cmake
> cmake_minimum_required(VERSION 3.26)
> project(MyApp VERSION 1.0 LANGUAGES CXX)
>
> set(CMAKE_CXX_STANDARD 17)
> set(CMAKE_AUTOMOC ON)
> set(CMAKE_AUTOUIC ON)
>
> find_package(Qt6 REQUIRED COMPONENTS Widgets)
>
> add_executable(MyApp
>     main.cpp
>     mainwindow.ui
> )
>
> target_link_libraries(MyApp PRIVATE Qt6::Widgets)
> ```
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

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

## 9. 练习项目

🎯 **练习项目：Qt 计算器**

📋 **功能描述**：
创建一个简单的计算器程序，支持加减乘除四种运算。

✅ **完成标准**：
- 使用 CMake 构建系统
- 界面用 .ui 文件设计（有数字按钮和运算符按钮）
- 有一个显示结果的 QLineEdit
- 点击等号按钮能计算结果
- 能处理除零错误

💡 **提示**：
- CMakeLists.txt 需要链接 QtWidgets 模块
- .ui 文件需要加到 add_executable 里
- 用 QLineEdit::text() 获取输入，用 QDoubleLineEdit 转数字
- 除零时显示 "Error"

## 10. 官方文档参考

📎 [CMake 手册 - Qt 6](https://doc.qt.io/qt-6/cmake-manual.html) · Qt 官方的 CMake 使用指南
📎 [qt-cmake-standalone 测试](https://doc.qt.io/qt-6/cmake-commands-api.html) · CMake 命令参考
📎 [CMake 教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html) · CMake 官方教程

*（链接已验证，2026-03-17 可访问）*

---

**到这里你的第一个 Qt 6 项目就跑通了！** 掌握了 CMake 基本用法，后面的项目构建都不在话下。下一节我们会深入 Qt 的核心——信号槽机制。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
