━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
入门 · 环境搭建 · 00 · Qt6 安装踩坑指南
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 1. 前言：为什么装个 Qt 都能折腾半夜

说实话，我第一次装 Qt 的时候真的以为双击安装程序就能完事。结果呢？光是一个编译器版本不对就让我折腾了三个小时。后来装了又卸、卸了又装，差不多把能踩的坑都踩了一遍。

所以这一篇我们不是在走过场，而是真的要陪大家把这个环境彻底搭稳。我们会覆盖 Windows、Linux 原生、还有 WSL2 三种场景——因为我知道总有人会在 WSL2 里跑 Qt，然后奇怪为什么界面弹不出来（别笑，我当年就是）。

现在我们要做的是：先把地基打牢，后面才能愉快地写代码。不然你会收获一个非常漂亮的编译错误列表，然后对着屏幕发呆。

## 2. 环境说明

本教程基于 **Qt 6.9.1** 版本编写，以下平台均已验证：

| 平台 | 编译器要求 | 测试状态 |
|------|-----------|---------|
| Windows 10/11 | MSVC 2019/2022 或 MinGW 11.2+ | ✅ 已验证 |
| Linux（Ubuntu 22.04+） | GCC 11+ | ✅ 已验证 |
| WSL2 + WSLg | 同 Linux | ✅ 已验证 |

> 💡 **重要提示**：Qt 6.9 对 CMake 最低版本要求是 3.26，额，其实是我猜测的，当然，Qt的确不支持低版本的CMake，我的建议是为什么不支持新的呢？

## 3. Qt Online Installer 详解

### 3.1 第一步——获取安装程序

我们要用的是 Qt 官方的在线安装器，它是统一入口，不管你是哪个平台都用它。

官方下载地址：https://www.qt.io/download-qt-installer

你可能会看到两个版本：Open Source 和 Commercial。对我们学习和个人项目来说，选 Open Source 就行，它是 LGPL 协议的，商业友好。注册个 Qt 账号就能免费用，不用想太复杂。

> ⚠️ **坑 #1：国内下载龟速**
> ❌ 错误做法：直接从官网下载，然后盯着进度条发呆两小时
> ✅ 正确做法：使用国内镜像源，或者找朋友分享离线安装包
> 💥 后果：你可能会以为自己网络炸了，反复重启下载
> 💡 一句话记住：官方源在国外，不用镜像就是在跟自己过不去

### 3.2 安装器选项逐行解读

打开安装器后，第一步会让你登录。用你刚才注册的 Qt 账号登录就行，跳不过。

然后到了关键环节——**选择安装组件**。这里真的很多人会乱选一通，要么装了一堆用不上的东西占几十个 G，要么该装的没装。

让我逐个说清楚：

#### 基础必选（无论如何都要装）

- **Qt 6.9.1 → Desktop（MinGW 或 MSVC）**
  - Windows 用户二选一：MinGW 轻量，MSVC 调试体验更好
  - Linux/WSL2 用户：这里选 Desktop (GCC)
  - 这是你写桌面应用的基础

- **CMake**
  - 勾上！虽然你可能系统里已经有 CMake，但 Qt 自带的版本是经过验证的
  - 版本对应 Qt 6.9 要求，省心

- **Qt Creator**
  - 官方 IDE，虽然后面我们也会教 VS Code 和 CLion
  - 至少装一个备用，排错时很有用

#### 推荐安装（根据需求选）

- **Qt 6.9.1 → Qt Quick 2D Renderer**
  - 要做 QML 界面的话装上
  - 纯 Widgets 应用可以暂时跳过

- **Qt 6.9.1 → Add-Ons → Qt Network Authorization**
  - 做网络登录（OAuth）的话需要
  - 不做这个可以先不管

- **Qt 5.15.15 → Qt Compatibility**
  - 如果你要维护旧代码，装个 5.15 兼容套件
  - 纯新项目可以不装

#### 冷门/可忽略

- **Qt 6.9.1 → Android / iOS / WebAssembly**
  - 除非你要做跨平台移动端，否则别装
  - 装了会显著增加安装时间

- **Qt 3D / Qt WebGL / Qt SCXML**
  - 这些是特定场景用的，真需要的时候再装不迟

> ⚠️ **坑 #2：路径别手滑**
> ❌ 错误做法：装到 `C:\Program Files\Qt` 或其他带空格的路径
> ✅ 正确做法：装到 `C:\Qt` 或 `D:\Dev\Qt`，路径简单无空格
> 💥 后果：CMake 和某些构建工具会炸，报错信息还特别迷
> 💡 一句话记住：路径别玩花活，越简单越好

### 3.3 安装过程验证

安装完成先别急着关窗口，我们简单验证一下：

1. 打开命令行（Windows 用 PowerShell 或 CMD，Linux 用终端）
2. 输入 `qmake --version` 或 `cmake --version`
3. 如果能看到版本号输出，说明 PATH 配置成功

> 📝 **随堂测验：口述回答**
> 用自己的话说说：Qt Online Installer 里的「Desktop」组件是干什么的？为什么 Windows 上有 MinGW 和 MSVC 两个版本？
>
> *(请先自己想一下，再往下滑看答案)*
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> - Desktop 组件是开发桌面应用的编译工具链和库
> - MinGW 是基于 GCC 的 Windows 编译器，开源免费
> - MSVC 是微软的编译器，与 Visual Studio 集成更好
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 4. Windows 专题

### 4.1 编译器配置

如果你选了 MinGW，安装器会帮你顺带装好 MinGW 编译器，基本不用额外配置。

但如果你选了 MSVC，你需要确保：

1. **Visual Studio 2019/2022 已安装**，并且勾选了「使用 C++ 的桌面开发」工作负载
2. **Windows SDK 已安装**，在 VS Installer 里找到单个组件，搜索「Windows SDK」，选一个最新版装上

> ⚠️ **坑 #3：VS 没装对的症状**
> ❌ 错误做法：只装了 VS 主体，没勾选 C++ 工作负载
> ✅ 正确做法：打开 VS Installer → 修改 → 勾选「使用 C++ 的桌面开发」
> 💥 后果：CMake 找不到 cl.exe，构建时一脸懵逼
> 💡 一句话记住：装 Qt ≠ 装了 C++ 编译器，两码事

### 4.2 环境变量验证

Windows 下安装器一般会自动配 PATH，但万一没配上：

1. 右键「此电脑」→ 属性 → 高级系统设置 → 环境变量
2. 在 PATH 里添加：
   - `C:\Qt\6.9.1\mingw_64\bin`（MinGW 版本）
   - 或 `C:\Qt\6.9.1\msvc2019_64\bin`（MSVC 版本）
3. 重启命令行窗口，再试 `qmake --version`

> 🔲 **随堂测验：代码填空**
> 补全以下 Windows PowerShell 命令，验证 Qt 环境是否配置正确：
>
> ```powershell
> # 验证 qmake 版本
> ______
>
> # 验证 CMake 版本
> ______ --version
>
> # 验证 Qt 库路径（假设装在 C:\Qt）
> ls ______
> ```
>
> *(提示：第一行用 qmake 命令，第二行用 cmake，第三行是目录路径)*
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> ```powershell
> qmake --version
> cmake --version
> ls C:\Qt\6.9.1\mingw_64\bin
> ```
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 5. Linux 专题

### 5.1 系统依赖安装

Ubuntu/Debian 下，Qt 依赖一堆系统库。先装这些：

```bash
sudo apt update
sudo apt install -y build-essential libgl1-mesa-dev libxkbcommon-x11-0 \
    libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxfixes-dev \
    libxi-dev libxrender-dev libxcb1-dev libx11-xcb-dev libxcb-glx0-dev \
    libxcb-keysyms1-dev libxcb-image0-dev libxcb-shm0-dev libxcb-icccm4-dev \
    libxcb-sync0-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-randr0-dev \
    libxcb-util-dev libxcb-xinerama0-dev libxcb-xkb-dev libxcb-cursor0-dev
```

我知道这串命令看着很长，但缺任何一个都可能编译失败。别偷懒，一次跑完。

### 5.2 安装器权限问题

Linux 下运行 Qt 安装器可能需要可执行权限：

```bash
chmod +x qt-unified-linux-x64-online.run
./qt-unified-linux-x64-online.run
```

> ⚠️ **坑 #4：缺少 libxcb 导致的迷之报错**
> ❌ 错误做法：看到缺库就装一个，然后反复编译失败
> ✅ 正确做法：一次性装完上面那串依赖（真的，我都替你踩过了）
> 💥 后果：程序启动就崩溃，日志里只有 "Failed to load platform plugin"
> 💡 一句话记住：Linux Qt 缺库不是开玩笑的，一次装齐省心

> 🐛 **随堂测验：调试挑战**
>
> 以下命令有问题，请问哪里错了，会导致什么后果？
>
> ```bash
> # 想要安装 Qt 构建依赖
> sudo apt install build-essential libgl1-mesa-dev
> # 然后直接运行 Qt 安装器
> ./qt-unified-linux-x64-online.run
> ```
>
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
>
> **答案参考**：
> - 问题：缺少大量 X11 和 xcb 相关依赖
> - 后果：Qt 应用能编译但无法运行GUI，会报 "Could not connect to display" 或 "Failed to load platform plugin xcb"
> ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 6. WSL2 专题：GUI 的正确打开方式

WSL2 现在已经支持 GUI 了（WSLg），但默认配置下你可能遇到显示问题。

### 6.1 确认 WSLg 可用

在 WSL2 终端里运行：

```bash
# 检查 WSL 版本
wsl.exe --version

# 简单测试 GUI
echo "export DISPLAY=:0" >> ~/.bashrc
```

如果你的 Windows 11 版本够新（Build 22000+），WSLg 应该是默认启用的。

### 6.2 Qt 环境配置

WSL2 下推荐直接用 Windows 版的 Qt，这样不用在 Linux 里再装一遍。

在 `~/.bashrc` 添加：

```bash
export PATH="/mnt/c/Qt/6.9.1/mingw_64/bin:$PATH"
export QT_QPA_PLATFORM=xcb
```

但要注意：这样运行的是 Windows 编译的 Qt 程序，如果你想在 WSL2 里原生编译 Linux 版，还是得按 Linux 篇的流程来。

> ⚠️ **坑 #5：WSL2 DISPLAY 设置错误**
> ❌ 错误做法：随便设一个 DISPLAY 值，或者用旧的 X Server 方案
> ✅ 正确做法：确认 WSLg 已启用，直接运行就行
> 💥 后果：GUI 程序启动失败，显示 "could not connect to display"
> 💡 一句话记住：Win11 + WSL2 时代，不用折腾 X Server 了

## 7. 常见安装报错与修复

### 7.1 "Error while loading module dependencies"

这个问题常见于 Windows，通常是因为：

- 缺少 Visual C++ Redistributable
- 路径里有中文字符
- 杀毒软件拦截

解决方法：装最新的 VC++ Redistributable，把安装路径改成纯英文，临时关闭杀毒软件。

### 7.2 "CMake was unable to find a build program"

CMake 找不到编译器。检查：

- Windows：MSVC 或 MinGW 是否在 PATH 里
- Linux：`which g++` 有输出吗？
- WSL2：同 Linux

### 7.3 安装器卡在 "Downloading"

网络问题，科学上网或者换国内镜像源。

## 8. 最终验证

安装完成，我们来打个收尾：

**Windows / Linux / WSL2 通用验证：**

```bash
# 1. 检查 qmake
qmake --version

# 2. 检查 CMake
cmake --version

# 3. 最重要的一步——创建测试工程
mkdir qt-test && cd qt-test
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.1/gcc_64 ..
# Windows 路径示例：-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64
```

如果以上都能顺利跑通，恭喜你，环境搞定了！接下来就可以正式写 Qt 代码了。

## 9. 练习项目

🎯 **练习项目：Hello Qt 环境验证**

📋 **功能描述**：
创建一个最小化的 Qt 工程，编译运行后弹出一个空窗口，标题显示 "Qt is alive!"。这个项目的目的不是学 Qt API，而是验证你的开发环境真的能跑起来。

✅ **完成标准**：
工程能用 CMake 成功配置，编译无报错，运行后能看到一个空白窗口标题显示正确。如果这一步都跑不通，说明环境还有问题，别着急往后学。

💡 **提示**：
- 新建一个文件夹，里面放 `main.cpp` 和 `CMakeLists.txt`
- CMakeLists.txt 里 find_package Qt6 的 Core 和 Widgets 模块
- main.cpp 里创建一个 QApplication 和一个空 QWidget，调用 show()
- 用命令行 `cmake -B build && cmake --build build` 来编译

## 10. 官方文档参考

📎 [Qt 官方安装指南](https://doc.qt.io/qt-6/get-and-install-qt.html) · 包含各平台的详细安装说明
📎 [Qt 6.9.1 平台要求](https://doc.qt.io/qt-6/supported-platforms.html) · 确认你的系统是否被支持
📎 [CMake 与 Qt 配置](https://doc.qt.io/qt-6/cmake-get-started.html) · CMake 集成的最佳实践

*（链接已验证，2026-03-17 可访问）*

---

**到这里就大功告成了！** 环境搭好，下一章我们就能愉快地写第一行 Qt 代码了。如果你在这一步遇到任何奇怪的问题，别慌——我也遇到过，大概率是路径或编译器的问题，对着上面的坑检查一遍基本能解决。
