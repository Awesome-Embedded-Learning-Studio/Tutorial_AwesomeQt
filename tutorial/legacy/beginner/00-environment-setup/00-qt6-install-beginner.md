---
title: "0.0 安装 Qt6"
description: "Qt 在线安装器与组件页的选择：Desktop 组件在三平台各自的工具链（MinGW / MSVC / GCC），CMake 与 Qt Creator 的取舍。坑的后果与根因：安装路径带空格、装了 Visual Studio 没勾 C++ 桌面开发工作负载、Linux 只装编译依赖不装 xcb 运行库、WSL2 里手写 DISPLAY。附清华 TUNA 镜像加速（--mirror 参数）与装完后的验证命令。"
---

# 而今从头迈步从头越：从Qt6开始安装

打开 Qt 安装器，登录账号，点完许可协议，迎面就是一屏组件树：Qt 6.9.1 底下挂着 Desktop、Android、WebAssembly 一串分支，工具栏里 CMake、Ninja、Qt Creator 各自打着勾。勾哪些？这一步选歪了，要么硬盘和时间搭进去几十 GB，要么装完打开 IDE 发现连编译器都没有。然后直接红温了。

## 网络与账号：安装器的两道门槛

国内网络直连官方源，下载慢是常态，进度条能卡到让人怀疑人生。咱们别跟进度条较劲，镜像站早就给出了正解，清华 TUNA 的帮助页写得明明白白：安装器本体从镜像的 `official_releases/online_installers/` 目录下载，启动时加一个 `--mirror` 参数，组件下载也走镜像：

```bash
qt-unified-windows-x64-online.exe --mirror https://mirrors.tuna.tsinghua.edu.cn/qt
```

Linux 同款，文件名换成 `qt-unified-linux-x64-online.run`，咱们跑之前记得 `chmod +x`。手头有稳定代理的话，直连加代理也能走通，两条路挑一条顺手的。

## 组件页：真正要做选择的地方

组件树里，Qt 6.9.1 节点下的 Desktop 是桌面开发的工具链与库本体，咱们直接勾上。Windows 在这里要做个选择：MinGW 还是 MSVC。MinGW 随安装器把编译器一并装好，装完就能用；MSVC 调试体验更好、与 Visual Studio 集成更顺，但要求机器上已经有可用的 VS。没装过 VS 的朋友选 MinGW 就好，不必犹豫。Linux 和 WSL2 没有这道选择题，Desktop (GCC 64-bit) 一项搞定。

工具栏里 CMake 和 Qt Creator，咱们都勾上。系统里也许已经有 CMake，但安装器带的版本与 Qt 6.9 配对验证过，能省掉版本对不上的排查；Qt Creator 在 0.1 篇虽不作为主力，.ui 界面文件的可视化编辑只有它做得最好，装上备用不亏。剩下的 Android、iOS、WebAssembly、Qt 3D，教程全程用不到，每一项都是实打实的下载时间，以后真要用了回来补装不迟。手上还有 Qt 5 老代码要维护的朋友，单独勾一份 Qt 5.15。

装到选择安装路径那一步留个神：路径别带空格，也别带中文。`C:\Program Files\Qt` 看着最正常，偏偏是它容易出事——CMake 和一批构建工具对带空格路径的处理并不完善，真炸起来，报错可能是一次莫名其妙的库查找失败，也可能停在链接器没头没尾的行为上，没人提示病根在路径。咱们装的时候选个 `C:\Qt` 这样的干净路径，这个坑就算绕过去了。

## Windows：装了 Qt 不等于装了编译器

咱们选了 MinGW 的话，到这里基本完事，安装器连编译器都安排好了。选了 MSVC 的还要再对一眼 VS Installer：Visual Studio 2019 或 2022 装了没？“使用 C++ 的桌面开发”这个工作负载勾了没？Windows SDK 随工作负载一起勾上。

这里最容易出的事故是装了 VS 主体、没勾 C++ 工作负载。这样装出来的 Qt 完全正常，但 CMake 探测不到 cl.exe，构建时报 "CMake was unable to find a build program"，报错里一个字不提工作负载的事。对着 Qt 排查半天查不到点子上，其实只要打开 VS Installer，点修改，把“使用 C++ 的桌面开发”勾上就解决了。您要是正被这类报错折磨，先去查这一项，八成就是它。

PATH 一般安装器会自动配好。万一咱们敲 `qmake --version` 得到“找不到命令”，就手动补：右键“此电脑”，依次进属性、高级系统设置、环境变量，把 Qt 的 bin 目录加进 PATH，MinGW 版是 `C:\Qt\6.9.1\mingw_64\bin`，MSVC 版是 `C:\Qt\6.9.1\msvc2019_64\bin`，改完重开命令行窗口再试。另有一个运行期的小插曲：工具报 "Error while loading module dependencies" 或提示缺 DLL，装一份微软 VC++ Redistributable 基本就解决；杀毒软件拦截安装目录的情况偶尔也有，临时放行即可。

## Linux：一长串依赖，各管各的时段

Ubuntu/Debian 下咱们先把系统依赖装齐，再跑安装器：

```bash
sudo apt update
sudo apt install -y build-essential libgl1-mesa-dev libxkbcommon-x11-0 \
    libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxfixes-dev \
    libxi-dev libxrender-dev libxcb1-dev libx11-xcb-dev libxcb-glx0-dev \
    libxcb-keysyms1-dev libxcb-image0-dev libxcb-shm0-dev libxcb-icccm4-dev \
    libxcb-sync0-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-randr0-dev \
    libxcb-util-dev libxcb-xinerama0-dev libxcb-xkb-dev libxcb-cursor0-dev
```

这串包名看着吓人，咱们拆开看就两拨：一拨是 `-dev` 结尾的头文件包，编译期要；一拨是 libxcb 系列的库，运行期 Qt 的 xcb 平台插件要链接它们。有人图省事只装 build-essential 和 mesa，结果编译确实能过，程序一启动就崩，终端里只有一句 `Failed to load platform plugin xcb`——头文件齐了所以编译过，xcb 插件依赖的系统库没跟上所以运行崩。编译过了、运行崩了，这种错位最迷惑人，装依赖这一步别省。

## WSL2：WSLg 把 X Server 那套送进了历史

Windows 11 的 WSL2 自带 WSLg，Linux 侧 GUI 程序的窗口直接出现在 Windows 桌面上，早年 VcXsrv、X410 那套手动搭 X Server 的方案可以整个退休了。确认它生效只要看环境变量：`echo $DISPLAY` 有值就是 WSLg 在工作，这个值由 WSL 自动注入，不用咱们动手。也正因为是自动注入，往 `.bashrc` 里手写 `export DISPLAY=:0` 反而多余，环境一变还可能连不上显示，报 "could not connect to display"。Windows 10 没有 WSLg，要跑 GUI 要么回 X Server 老方案，要么升级系统，本篇按 Win11 走。

咱们在 WSL2 里写 Qt，就按上面 Linux 篇装原生 Linux 版，行为与真机一致。另一条路是 Qt 装 Windows 侧、WSL 里只放代码和编辑器。两条路都通，但别把两边的 Qt 混进同一个 PATH——Windows 编译的程序和 Linux 编译的程序是两套二进制，混了之后出了报错，连从哪边查起都没头绪。

## 装完先验证，再撒手

```bash
qmake --version   # PATH 里的 Qt 工具就位
cmake --version   # 构建工具在
ls ~/Qt/6.9.1/gcc_64/bin   # Windows 换成 C:\Qt\6.9.1\mingw_64\bin，库文件都在
```

咱们看到三条命令都有正常输出，工具链层面就绪。至于让这套工具真正编译出一个窗口，那是 0.2 篇第一个工程的任务，那一步跑通了，环境才算彻底交卷。

## 官方文档参考

本篇基于 Qt 6.9.1；CMake 用 3.16 及以上皆可（qtbase 源码顶层与官方入门示例写的都是 3.16）。

[Qt 文档 · Get and Install Qt](https://doc.qt.io/qt-6/get-and-install-qt.html) · 官方安装总览，各平台安装方式的入口

[Qt 文档 · Supported Platforms](https://doc.qt.io/qt-6/supported-platforms.html) · 各平台支持的系统与编译器清单

[Qt 文档 · CMake Get Started](https://doc.qt.io/qt-6/cmake-get-started.html) · 用 CMake 建 Qt 工程的官方入门

[清华 TUNA 镜像 · Qt 使用帮助](https://mirrors.tuna.tsinghua.edu.cn/help/qt/) · 镜像下载地址与 --mirror 参数说明

---

环境就绪。下一篇 [0.1 IDE 配置](./01-ide-setup-beginner.md) 把 VS Code、CLion、Qt Creator 三家接上这套工具链。哪一步卡住的，咱们回头把这篇提过的坑再对一遍：路径空格、C++ 工作负载、xcb 依赖、DISPLAY 手写，安装期的事故大多落在这几处。第一个工程的完整构建验证，在 [0.2 第一个 CMake Qt6 工程](./02-cmake-first-project-beginner.md) 见。
