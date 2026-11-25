---
title: "0.1 IDE 配置"
description: "VS Code、CLion、Qt Creator 三家 IDE 接上 Qt 工具链的方法。真正的配置只有两件事：告诉 IDE 用哪套编译器（VS Code 的 Kit / CLion 的 Toolchain / Qt Creator 的 Kit 三件套），再告诉它 Qt 装在哪（CMAKE_PREFIX_PATH，三家入口不同、填的是同一个值）。含智能提示自动注入、launch.json 调试配置、QString 的 pretty-printer、connect 日志开关与断点落点。"
---

# 现代Qt开发教程（新手篇）0.1——IDE 配置

同一个 Qt 工程，命令行里 `cmake -B build` 一把通过，进了 VS Code 满屏红色波浪线，`#include <QApplication>` 底下还画着"找不到头文件"。第一次见这场面的朋友多半会怀疑 IDE 坏了。IDE 没坏，它只是被蒙在鼓里：不知道 Qt 的头文件在哪，也不知道该用哪套编译器。把这两样告诉它，波浪线自己会退。这一篇咱们就干这件事，VS Code、CLion、Qt Creator 三家，各把工具链接上。

动 IDE 之前有个顺序要立住：命令行先通。上一篇装好的 Qt 6.9.1，`qmake --version` 有输出、`cmake --version` 可用，随便一个小工程能配置成功，再进 IDE。这么排序的道理很实际——命令行通了，IDE 里再出报错，咱们就能断定问题在配置；命令行没通就先折腾 IDE，两边的报错混在一起，谁也说不清。

## VS Code：两个插件，一条路径

插件装两个，都认 Microsoft 的：C/C++ 管智能提示、跳转和调试，CMake Tools 管 Kit 选择、CMake 配置和构建入口。市场里搜 CMake 会冒出一堆第三方插件，名字像、功能重叠，装混了互相打架，出了怪问题还容易赖到代码头上，装的时候认准发布者。另外 Qt 官方自己也在市场上架了 Qt C++ 扩展（2024 年起），CMake 集成、Qt 类型调试、Designer 界面编辑打包在一起，您想一站式可以试；本篇咱们仍走两个基础插件，出问题时排查面最小。

Kit 是 CMake Tools 给“一套编译器”起的名字。`Ctrl+Shift+P` 敲 `CMake: Select a Kit`，咱们按环境挑：MSVC 选带 amd64 字样的 Visual Studio 项，MinGW 选 GCC x86_64-w64-mingw32，Linux 和 WSL2 选 GCC x86_64-linux-gnu。

咱们接下来把 Qt 的位置告诉它。这步漏了的话，配置项目时会停在 "Could not find Qt6"——`find_package` 不知道 Qt 装在哪，IDE 的报错又不会直说，人就容易对着界面干着急。出路是给 CMake 传 CMAKE_PREFIX_PATH，VS Code 里落在 settings.json：

```json
// .vscode/settings.json
{
    "cmake.configureSettings": {
        "CMAKE_PREFIX_PATH": "C:/Qt/6.9.1/mingw_64"
    }
}
```

Linux 换成 `/home/您的用户名/Qt/6.9.1/gcc_64`。网上教程给这步的写法五花八门，Configure Args、variants、CMake Presets 都能到同一个地方，咱们认准目的就行：把 Qt 的安装前缀交给 CMake。

智能提示这一步，咱们不用照老教程一个个抄 include 路径。CMake Tools 配置完项目后，C/C++ 扩展可以直接从它手里拿每个文件的编译参数，Qt 头文件的位置自动就有了：

```json
// 同样在 .vscode/settings.json
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

咱们存盘，重新配置一次项目，波浪线应该退干净。真有残留，再用 `C/C++: Edit Configurations (UI)` 手动往 Include path 里补 Qt 的几个 include 目录，属于兜底手段。

调试按 F5，VS Code 这时读的是 `.vscode/launch.json`。咱们从 CMake Tools 生成的模板起步就够，关键是 program 一项指向构建产物：

```json
{
    "name": "Debug Qt App",
    "type": "cppdbg",
    "request": "launch",
    "program": "${command:cmake.launchTargetPath}",
    "args": [],
    "cwd": "${workspaceFolder}",
    "MIMode": "gdb",
    "setupCommands": [
        {
            "description": "Enable pretty-printing",
            "text": "-enable-pretty-printing",
            "ignoreFailures": true
        }
    ]
}
```

咱们把 setupCommands 里那行 enable-pretty-printing 先按下不表，调试一节回收。

## CLion：填对工具链，剩下它包了

CLion 对 CMake 是原生支持，咱们要填的东西少得多。Settings 的 Build, Execution, Deployment 里找 Toolchains：Windows MinGW 环境，CMake 用 bundled 或系统的都行，Build tool 在 Ninja 和 MinGW Makefiles 里挑一个，编译器指向 gcc 和 g++。MSVC 环境把编译器换成 VS 自带的 cl.exe。Linux 和 WSL2 几乎全自动，检测出来什么用什么。

装了 Visual Studio、CLion 却报找不到 cl.exe，是 MSVC 这边的老问题。CLion 要的是 cl.exe 的确切路径，装了 VS 不代表它能自己摸到。咱们用 VS 里的 Developer Command Prompt for VS 把 cl.exe 的位置找出来，回 Toolchains 手动填上，事情就了了。

Qt 的位置在 CMake profiles 里给：建一个 Profile，CMake options 填 `-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64`，Build type 选 Debug。写到这里您应该看出来了，这和 VS Code 里干的是同一件事，CMAKE_PREFIX_PATH 没变，换了个表格填而已。

调试是 CLion 的强项，QList、QHash 这类 Qt 容器展开直接看内容，咱们不用配任何东西。QML 高亮要另装插件，在 Settings 的 Plugins 页搜 QML，装上 QML Support 即可。

## Qt Creator：Kit 是三样凑一套

Qt Creator 界面朴素，胜在它是官方 IDE，对 Qt 的支持是内置的。.ui 文件的可视化编辑、信号槽的图形化连接，到今天还是它做得最顺手，哪怕咱们主力用别家，留着它画界面不亏。

它管配置的单位叫 Kit：编译器、Qt 版本、调试器，三样凑成一个可用的构建环境。首次启动它会自动检测已装的 Qt，检测不到就手动补——Edit 菜单进 Preferences，Kits 分类下的 Qt Versions 里填上 qmake 路径（比如 `C:/Qt/6.9.1/mingw_64/bin/qmake.exe`），再回 Kits 页确认有一个 Kit 挂着这个 Qt 版本。跳过这步直接编译，得到的是一句 "No valid kit found"，缺了哪样它不细说，得咱们自己回来补。

打开项目用 File 菜单的 Open File or Project 选中 `CMakeLists.txt`，配置自动跑完，Ctrl+R 运行。咱们双击 .ui 文件进设计器：左边控件面板拖布局，右边属性栏改字体颜色，底部信号槽编辑器图形化连线。画界面的效率比手写高出一截，新手期用它，熟练之后转手写也不迟。

## 三家怎么选

| 特性 | VS Code | CLion | Qt Creator |
|------|---------|-------|------------|
| 轻量程度 | 极轻量 | 中等 | 较轻量 |
| CMake 支持 | 良好 | 原生，最强 | 良好 |
| Qt 专用功能 | 一般 | 较弱 | 原生，最强 |
| 调试体验 | 一般 | 优秀 | 良好 |
| 授权 | 开源 | 商业授权（教育许可可申请） | 开源 |

怎么挑，给您一句实在话。预算敏感、愿意折腾，VS Code。CMake 重度用户、预算充足，CLion。界面设计多、专攻 Qt，Qt Creator。主力咱们定一个，另两个装着当备胎——主力 IDE 偶尔抽风的时候，有个能用的退路比什么都强。

## 调试：pretty-printer、connect 日志、断点落点

先说 QString。GDB 默认看不见它的内容——QString 里装的是指针加隐式共享的结构，不打 pretty-printer，变量窗口里全是 `d->data` 这类代理字段，没有可读的东西。Qt Creator 自带渲染，CLion 内置，VS Code 就用咱们 launch.json 里埋的那行 enable-pretty-printing，配合 GDB 的 Python 支持把 QString、QList 打成可读的样子。三家的差别只是要不要自己动手配这一下。

再说一个排查信号槽的开关。咱们怀疑“信号发了、槽没动”的时候，在 main.cpp 里加一行，connect 的匹配过程会全部打进日志：

```cpp
QLoggingCategory::setFilterRules("qt.core.qobject.connect=true");
```

它帮咱们分清两种情况：连接压根没建立，和连接建立了但槽没被调用。两种病的治法完全不同，先分清再动手，少走弯路。

断点也有讲究。MOC 生成的代码和 Qt 内部函数里下断点，命中位置常常漂到意想不到的地方；排查时把断点放在自己写的函数里，命中得最稳。咱们初学阶段守住这一条，能省不少疑惑。

## 官方文档参考

本篇基于 Qt 6.9.1；三家 IDE 的菜单细节随版本会有小幅挪动，找不到同名菜单时在设置里搜关键词最快。

[Qt Creator 手册](https://doc.qt.io/qtcreator/) · 官方 IDE 的完整文档

[VS Code · Using GCC with MinGW](https://code.visualstudio.com/docs/cpp/config-mingw) · 微软官方的 MinGW 工具链配置指南

[CLion · Quick CMake Tutorial](https://www.jetbrains.com/help/clion/quick-cmake-tutorial.html) · JetBrains 的 CMake 快速上手

---

配置是不是真在工作，做个破坏性实验最放心：把 settings.json 里的 CMAKE_PREFIX_PATH 故意改错，重新配置，"Could not find Qt6" 应声而出；改回去再配，报错消失。能亲手把错造出来再消掉，这套配置就归您了。下一篇 [0.2 第一个 CMake Qt6 工程](./02-cmake-first-project-beginner.md) 从零建第一个工程，CMakeLists.txt 每一行为什么在那，讲清楚。
