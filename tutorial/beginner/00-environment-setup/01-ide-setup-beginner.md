# 现代Qt开发教程（新手篇）0.1——IDE 配置全指南

## 1. 前言：为什么 IDE 配置能折腾死人

老实说，我第一次用 VS Code 跑 Qt 项目的时候，对着满屏的红色波浪线差点砸键盘。智能提示不工作、找不到头文件、CMake 配置报错——这些问题每一个都够让人崩溃的。

后来我才发现，问题不是 IDE 不好用，而是我根本没配对。Qt 的开发环境确实有点特殊：有 MOC、有 RCC、有 UIC，再加上 CMake 的各种变量，配错一个地方整条链就断了。

所以这一篇我们不是走马观花，而是真的要把三个主流 IDE（VS Code、CLion、Qt Creator）的配置彻底搞明白。你可以选一个自己喜欢的主力，另外两个备用——毕竟总有那么些时候，主力 IDE 突然抽风，有个备用能救命。

现在我们要做的是：选好你的武器，把它调教到能顺滑地写 Qt 代码。

## 2. 环境说明

本篇假设你已经完成了上一篇的 Qt 安装，并且 Qt 6.9.1 已安装（`qmake --version` 能看到版本号）、CMake 可用（`cmake --version` 输出 3.26 以上）、编译器已就位（Windows 上是 MSVC 或 MinGW，Linux 上是 GCC）。

我们会分别配置三个 IDE，你可以按需跳跃到感兴趣的部分。

无论选哪个 IDE，确保先在命令行能成功编译一个 Qt 工程。IDE 只是包装，底层工具链才是核心。命令行跑不通，IDE 配再花也没用。

## 3. VS Code 配置

### 3.1 必装插件

VS Code 装好之后，你至少需要这几个插件。

首先是 C/C++，微软官方出的那个——认准发布者是 Microsoft，别看到名字像的就装，最后装了一堆重复功能的插件互相打架，你还以为是代码的问题。这个插件管智能提示、语法高亮、跳转定义，基本上没有它你就只能写纯文本。

然后是 CMake Tools，发布者是 twxs。这是个神器，有了它你才能在 VS Code 里管理 CMake 项目、一键构建调试，不用它你会累死。Qt C++ Helpers 可装可不装，但它能提供信号槽语法高亮和 .ui 文件预览，写 Qt 项目的时候确实贴心。

### 3.2 CMake 工具链配置

VS Code 的 CMake Tools 插件需要知道你的编译器和 Qt 在哪里。

打开任意 CMake 项目（或者新建一个），按下 `Ctrl+Shift+P`，输入 `CMake: Select a Kit`。第一次运行它会让你选择编译器：Windows MSVC 选 Visual Studio Community 2019 Release - amd64，Windows MinGW 选 GCC x86_64-w64-mingw32，Linux 和 WSL2 选 GCC x86_64-linux-gnu。

然后需要设置 Qt 的路径。同样是 `Ctrl+Shift+P`，输入 `CMake: Configure Args`，添加以下参数：

```cmake
-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64
# Linux 下改成：
-DCMAKE_PREFIX_PATH=/home/你的用户名/Qt/6.9.1/gcc_64
```

这个配置会保存到 `.vscode/cmake-kits.json` 里，不用每次都设。

### 3.3 智能提示配置（c_cpp_properties.json）

VS Code 的智能提示需要知道 Qt 的头文件在哪里。按 `Ctrl+Shift+P`，输入 `C/C++: Edit Configurations (UI)`，在 "Include path" 里添加：

```json
[
    "${workspaceFolder}/**",
    "C:/Qt/6.9.1/mingw_64/include",
    "C:/Qt/6.9.1/mingw_64/include/QtCore",
    "C:/Qt/6.9.1/mingw_64/include/QtWidgets",
    "C:/Qt/6.9.1/mingw_64/include/QtGui"
]
```

Linux 路径同理调整。配置完后，你会看到红色的波浪线消失，`#include <QApplication>` 不再报错。

你可能会问，为什么 VS Code 需要配置 CMAKE_PREFIX_PATH？不配会怎样？其实道理很简单——CMAKE_PREFIX_PATH 告诉 CMake 去哪里找 Qt 的库文件和 CMake 配置文件。不配的话 `find_package(Qt6)` 会找不到，整条构建链就断了，CMake 会直接报错 "Could not find Qt6"，然后你对着报错一脸茫然。

### 3.4 调试配置（launch.json）

按 F5 调试时，VS Code 会读取 `.vscode/launch.json`。CMake Tools 会自动生成一个，但你可能需要微调：

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Qt App",
            "type": "cppdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```

## 4. CLion 配置

### 4.1 工具链设置

CLion 的好处是它对 CMake 的支持是原生的，配置起来比 VS Code 简单一些。

打开 Settings → Build, Execution, Deployment → Toolchains，根据你的环境设置对应参数。Windows MinGW 下 CMake 用 bundled 或系统安装的都行，Build tool 选 Ninja 或 MinGW Makefiles，C Compiler 和 C++ Compiler 指向你的 gcc 和 g++ 路径，Debugger 用 gdb 或 lldb。Windows MSVC 下基本一样，只是 C/C++ Compiler 要指向 VS 自带的 cl.exe。Linux 和 WSL2 下最简单，CMake 用 bundled，Build tool 选 Unix Makefiles，编译器就是 `/usr/bin/gcc` 和 `/usr/bin/g++`。

如果你装了 VS 但 CLion 报错找不到 cl.exe，别慌。CLion 需要知道 VS 工具链的确切路径，不是你装了 Visual Studio 它就能自动找到的。用 VS Installer 里的 "Developer Command Prompt for VS" 路径来定位 cl.exe 的位置，然后在 CLion 里手动填上就行。

### 4.2 CMake Profile 配置

在同一设置页面，找到 CMake profiles。添加一个新的 Profile，配置如下：

```cmake
# Build directory
${workspaceFolder}/cmake-build-debug

# CMake options (关键部分)
-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64

# Build type
Debug

# Toolchain
默认即可（CLion 会自动检测）
```

保存后，CLion 会在右上方显示这个 Profile，切换过去就行。

### 4.3 运行与调试

CLion 的调试配置很直观：点击右上角的运行配置，选择 Edit… 在 "Program arguments" 里填参数（如果需要），"Working directory" 默认是项目根目录。按 Shift+F9 就能开始调试。CLion 的调试器体验比 VS Code 好一些，尤其是查看 Qt 容器内容的时候。

### 4.4 Qt Quick (.qml) 支持

CLion 默认对 QML 的支持有限，需要额外配置。安装插件：Settings → Plugins → 搜索 "QML"，安装 QML Support 插件。

到这里你可能已经注意到了——不管哪个 IDE，CMake 配置的核心就是那个 CMAKE_PREFIX_PATH。CLion 里它藏在 CMake Profile 的 options 里，VS Code 里它藏在 CMake Tools 的 Configure Args 里，形式不同但本质一样。如果你的 CMake options 里要找 Qt 6.9.1，就是 `-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64`，Build type 写 Debug，要启用 Qt 的 MOC、RCC、UIC 自动处理就在 CMakeLists.txt 里写上 `set(CMAKE_AUTOMOC ON)`、`set(CMAKE_AUTORCC ON)`、`set(CMAKE_AUTOUIC ON)`。

## 5. Qt Creator 配置

### 5.1 为什么还要说 Qt Creator

我知道有些人可能觉得 Qt Creator 界面"老旧"，但说实话，它是官方 IDE，对 Qt 的支持是原生的。有些功能——比如 .ui 文件的可视化编辑、信号槽的图形化连接——只有 Qt Creator 做得最好。

所以建议是：主力可以选别的，但 Qt Creator 留着备用，特别是做 UI 设计的时候。

### 5.2 首次启动配置

Qt Creator 首次启动时会自动检测 Qt 安装，如果检测不到，打开 Edit → Preferences → Kits → Qt Versions，手动添加：

```
名称: Qt 6.9.1 (mingw_64)
qmake 路径: C:/Qt/6.9.1/mingw_64/bin/qmake.exe
```

然后在 Kits 页面，确保有一个 Kit 包含这个 Qt 版本。Qt Creator 的 Kit 是编译器 + Qt + 调试器的组合，缺一不可。如果你看到没 Kit 就不管直接编译，一定会得到一个 "No valid kit found" 的报错。

### 5.3 打开 CMake 项目

Qt Creator 打开 CMake 项目超简单：File → Open File or Project → 选择 `CMakeLists.txt`，它会自动配置，然后你就可以直接按 Ctrl+R 运行了。

### 5.4 .ui 文件可视化编辑

这是 Qt Creator 的杀手级功能。双击项目里的 .ui 文件，会打开一个可视化设计器：左侧是控件面板，拖拽就能加到界面上；右侧是属性编辑器，改字体、颜色、大小；底部是信号槽编辑器，图形化连接信号和槽。说实话，这个功能真的好用，特别是对新手。等你熟练了可以手写 UI，但刚开始用可视化工具能省很多时间。

我们来想一个场景——如果 VS Code 的 cmake-kits.json 里只配了编译器路径，但没有设置 CMAKE_PREFIX_PATH，同时 launch.json 里也没配 program 字段，会发生什么？CMake 找不到 Qt 的位置，无法解析任何 Qt 相关的 `find_package`，构建直接失败；就算构建通过了，调试器也不知道该启动哪个可执行文件。所以这两个文件的关键字段一定都要填上。

## 6. 三端对比与选择建议

| 特性 | VS Code | CLion | Qt Creator |
|------|---------|-------|------------|
| 轻量程度 | 极轻量 | 中等 | 较轻量 |
| CMake 支持 | 良好 | 原生支持，最强 | 良好 |
| Qt 专用功能 | 一般 | 较弱 | 原生支持，最强 |
| 调试体验 | 一般 | 优秀 | 良好 |
| 插件生态 | 最丰富 | 中等 | 较少 |
| 价格 | 免费 | 收费（学生免费） | 免费 |

选择建议很简单：预算敏感又喜欢折腾就选 VS Code，重度 CMake 用户且预算充足就选 CLion，Qt 专用且 UI 设计频繁就选 Qt Creator。

## 7. 通用调试技巧

无论你用哪个 IDE，调试 Qt 项目有几个通用的要点。

GDB 默认看不到 QString 的内部内容，需要使用 pretty-printer。Qt Creator 自带，VS Code 和 CLion 需要额外配置。如果你想在调试时看到信号槽的调用信息，在 `main.cpp` 里加这一行：

```cpp
QLoggingCategory::setFilterRules("qt.core.qobject.connect=true");
```

另外，Qt 的 MOC 会生成额外代码，有时候断点会打在奇怪的地方。建议断点打在你自己写的函数里，而不是 Qt 的内部函数。

## 8. 练习项目

**练习项目：三端 Hello Qt**

用三个 IDE 分别打开同一个 Qt Hello World 项目，配置好工具链，确保每个 IDE 都能成功编译运行。

完成标准是四个：VS Code 下 F5 能调试且智能提示正常，CLion 下右上角能选 Kit 且 Shift+F9 调试正常，Qt Creator 下能打开项目且 Ctrl+R 运行正常，最后三个 IDE 编译出的程序都能弹出 "Hello Qt" 窗口。

项目本身很简单——一个 QApplication 加一个显示 "Hello Qt" 的 QWidget。关键是先在命令行确认能编译，再用 IDE 打开。每个 IDE 里配置 CMAKE_PREFIX_PATH 是关键步骤，VS Code 需要手动配 include path，CLion 和 Qt Creator 会自动检测。

## 9. 官方文档参考

[Qt Creator 手册](https://doc.qt.io/qtcreator/) · 官方 IDE 的完整文档
[VS Code C++ 教程](https://code.visualstudio.com/docs/cpp/config-mingw) · 微软官方配置指南
[CLion CMake 教程](https://www.jetbrains.com/help/clion/cmake-support.html) · JetBrains CMake 支持

*（链接已验证，2026-03-17 可访问）*

---

**到这里就大功告成了！** 选一个你喜欢的 IDE，把它调教顺手。后面我们就要正式进入 Qt 代码的世界了。记住，IDE 只是工具，代码才是核心。别在配置上花太多时间，能用就行。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
