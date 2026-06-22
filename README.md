# AwesomeQt — Qt 6 中文深度教程

> 一套聚焦 C++ / QtWidgets 的 Qt 6 中文深度教程：三层递进（入门 → 进阶 → 读源码）+ 可复用实例库，把「学会 Qt」送到「做出东西」。隶属 [Awesome-Embedded-Learning-Studio](https://github.com/Awesome-Embedded-Learning-Studio)。

---

## 这是什么

**AwesomeQt** 是一套 Qt 6 中文深度教程，隶属 [Awesome-Embedded-Learning-Studio](https://github.com/Awesome-Embedded-Learning-Studio)。

采用三层分级结构：

```
入门层 → 能跑起来，理解核心概念，知其然
进阶层 → 懂原理，写出工程级稳健代码
专家层 → 读懂 Qt 源码，每条结论带 文件:行号 证据
```

---

## 快速开始

### 前置检查

| 你需要... | 版本要求 |
|----------|---------|
| C++ 基础 | 熟悉类、继承、指针、模板 |
| CMake | ≥ 3.26 |
| 编译器 | MSVC 2019/2022 / MinGW 11.2+ / GCC 11+ |
| 操作系统 | Windows 10/11 / Linux / WSL2 |

### 三步跑起来

```bash
# 1. 克隆仓库
git clone https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt.git
cd Tutorial_AwesomeQt

# 2. 阅读教程索引
# 打开 tutorial/index.md 选择你的起点

# 3. 从环境搭建开始
# tutorial/beginner/00-environment-setup/
```

如果你是零基础，建议按顺序从「环境搭建」开始。如果你已经有 Qt 经验，可以直接跳到感兴趣的模块。

---

## 当前进度

```
入门层    ██████████  137 / 137 篇教程 · 134 个代码示例
进阶层    ██████████  134 / 134 篇教程 · 134 个代码示例
专家层    ░░░░░░░░░░  0 / ~101-108 篇（主力规划中，精确清单待定）
合计      ██████░░░░  271 / ~372-379 篇
```

🚀🚀🚀 更加详细的进度：[tutorial/index.md](tutorial/index.md)

---

## 更新日志

见 [changelogs/](changelogs/) 目录。

---

## 学习路径

### 路径 A：零基础完整学习

```
环境搭建 → QtBase → QtGui → QtWidgets → 项目实战 → QtNetwork → 扩展模块 → QML
```

适合：从没接触过 Qt，想系统学习的开发者。

### 路径 B：Qt 5 老用户速成

```
环境搭建（重点 CMake）→ Qt 6 新变化 → 直接跳到感兴趣的模块
```

适合：有 Qt 5 经验，想快速迁移到 Qt 6 的开发者。

### 路径 C：源码深度钻研

```
完成入门层 → 专家层源码解析章节（MOC 原理、信号槽底层、事件循环...）
```

适合：想理解 Qt 底层实现的设计模式爱好者。

---

## 教程特色

### 📝 不是 API 手册

文档里不会列出所有函数签名，那些东西官方文档写得更好。我们讲的是：**什么时候用什么、为什么这样用、哪里会踩坑**。

### 🚧 踩坑实录

每个知识点至少三个真实踩过的坑：

```markdown
> ⚠️ 坑 #N：[坑的名字]
> ❌ 错误做法：[描述或伪代码]
> ✅ 正确做法：[描述或伪代码]
> 💥 后果：[这样写会崩溃 / 内存泄漏 / 信号永远不触发...]
> 💡 一句话记住：[总结]
```

### 🧪 随堂测验

穿插在讲解中的互动题目，不只是读完就算：

- 📝 口述回答：用自己的话说说概念
- 🔲 代码填空：补全关键代码
- 🐛 调试挑战：找出有 bug 的代码
- 📖 源码阅读题（专家层）

### 🎯 练习项目

每篇配套一个动手项目，不做永远学不会。

### 🔗 验证过的官方文档链接

每篇末尾附上官方文档链接，全部经过验证可达。遇到疑问，官方文档是最终权威。

---

## AwesomeQt Widgets — 通用控件库

项目包含一套可复用的自定义控件库，位于 [`widget/`](widget/) 目录。每个控件都是独立的、可编译的库组件，统一使用 `AwesomeQt::` 命名空间。

**当前已实现：**

| 控件 | 说明 |
|------|------|
| [`StatusLED`](widget/status-led/) | 状态指示灯，支持 4 种状态（Normal/Warning/Error/Offline）+ 闪烁动画 |

**构建方式：** 每个控件是 STATIC 库 + 独立 demo，根 [`widget/CMakeLists.txt`](widget/CMakeLists.txt) 统一配置（C++17 / AUTOMOC / find_package Qt6）。`cd widget && cmake -B build && cmake --build build` 即可构建。

---

## 致谢

本项目的代码实例规划大量参考了以下开源项目和社区资源，在此致以诚挚的感谢：

### 核心参考仓库

| 项目 | 作者 | 说明 |
|------|------|------|
| [QWidgetDemo](https://github.com/feiyangqingyun/QWidgetDemo) | [feiyangqingyun (Qt实战)](https://qtchina.blog.csdn.net) | 5800+ Stars，涵盖 100+ 个 Qt 自定义控件、工具和界面 Demo，是本项目最核心的参考来源 |
| [MyTestCode](https://github.com/gongjianbo/MyTestCode) | [gongjianbo1992](https://blog.csdn.net/gongjianbo1992) | 丰富的 Qt/QML 博客示例代码，涵盖网络、多线程、OpenGL、插件、QML/C++ 互操作等 |
| [TTKWidgetTools](https://github.com/Greedysky/TTKWidgetTools) | [Greedysky Studio](https://github.com/Greedysky) | 86 个精美自定义控件，包含按钮、标签、进度条、仪表盘、滑块等，支持 Qt 4/5/6 全版本 |

### 灵感来源与参考项目

| 项目 | 说明 |
|------|------|
| [Qt-Advanced-Docking-System](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System) | 高级停靠窗口框架，IDE 风格 Dock 管理器 |
| [qt-material-widgets](https://github.com/laserpants/qt-material-widgets) | Material Design 风格的 Qt Widgets 控件库 |
| [Qt-ShowyWidgets](https://github.com/iwxyi/Qt-ShowyWidgets) | 花式/创意 Qt 控件集（心形开关、线条开关等） |
| [QCodeEditor](https://github.com/Megaxela/QCodeEditor) | Qt 代码编辑器控件（行号、语法高亮、代码折叠） |
| [JKQTPlotter](https://github.com/jkriege2/JKQtPlotter) | 功能丰富的 Qt 2D 绘图库 |
| [Chart-Qt](https://github.com/fair-acc/chart-qt) | 高性能 Qt 图表库 |
| [QDoubleRangeSlider](https://github.com/robert1207/QDoubleRangeSlider) | 双端范围滑块控件 |
| [Qt-FacileMenu](https://github.com/nicholasgasior/Qt-FacileMenu) | 动画右键菜单控件 |
| [QGeoView](https://github.com/nicholasgasior/QGeoView) | Qt 地图数据可视化控件 |
| [prison (QR/DataMatrix)](https://github.com/nicholasgasior/prison) | Qt 条形码与二维码生成库 |
| [SARibbon](https://github.com/czyt1988/SARibbon) | Microsoft Office 风格 Ribbon 工具栏控件 |
| [KDDockWidgets](https://github.com/KDAB/KDDockWidgets) | KDAB 出品的停靠窗口框架 |
| [QLineEditExt](https://github.com/divideconcept/QLineEditExt) | 混合型输入控件（滑块+文本+进度条） |
| [QWidget-FancyUI](https://github.com/nicholasgasior/QWidget-FancyUI) | Qt6 现代化 UI 组件库 |
| [QCustomPlot](https://www.qcustomplot.com/) | 高质量 2D 绘图控件 |
| [Qwt](https://qwt.sourceforge.io/) | 科学与技术绘图库 |

> 如果您是以上项目的作者，且对本项目的引用方式有任何疑问或希望调整，欢迎通过 [Issue](https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt/issues) 联系我们。

---

## 技术栈

| 项目 | 说明 |
|------|------|
| Qt 版本 | 6.9.1（编写时的 LTS） |
| 构建系统 | CMake 3.26+ |
| C++ 标准 | C++17/20 |
| 测试平台 | Windows 10/11, Ubuntu 22.04+, WSL2 |
| 编译器 | MSVC 2019/2022, MinGW 11.2+, GCC 11+ |

---

## 贡献与反馈

这个教程持续更新中。

如果你发现：
- 错误或不准确的内容
- 链接失效
- 可以改进的表达方式
- 想要补充的知识点

欢迎提交 Issue 或 Pull Request。

---

## 联系方式

- 作者：CharlieChen
- 邮箱：725610365@qq.com
- 组织：[Awesome-Embedded-Learning-Studio](https://github.com/Awesome-Embedded-Learning-Studio)

---

**现在你已经知道这是什么了，准备好了吗？从 [教程索引](tutorial/index.md) 开始你的 Qt 之旅吧！**
