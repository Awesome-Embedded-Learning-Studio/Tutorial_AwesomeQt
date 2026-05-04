# AwesomeQt — 陪你从第一行代码到读懂源码

嘿！这里是Awesome Embedded Studio！我相信大家第一次接触 Qt 的时候，对着 `QObject::connect` 的四个参数发呆了整整一下午。后来又因为忘记加 `Q_OBJECT` 宏，收获了一个莫名其妙的 vtable 错误。再后来，在信号槽的跨线程调用上翻车，在对象树的内存管理上踩雷，在 MOC 生成的代码里迷失方向……

这是一份会陪着你走完整条路的教程——从第一个 `QApplication` 到读懂 MOC 生成的那一刻。我们会吐槽，会叹气，会一起熬夜调试，但你绝对不会断层。

---

## 这是什么

**AwesomeQt** 是一份面向 C++ 开发者的 Qt 6 全栈教程，采用三层分级结构：

```
入门层 → 能跑起来，理解核心概念，初步使用 API
进阶层 → 掌握高级用法，写出工程级代码
专家层 → 读懂 Qt 源码，理解设计模式与实现原理
```

这不是那种复制粘贴官方文档的教程。每一个知识点，都是实际工程项目中会用到的；每一个"坑"，都是真实遇到并花时间解决的。

> 本项目隶属于组织 [Awesome-Embedded-Learning-Studio](https://github.com/Awesome-Embedded-Learning-Studio) 的文档教程

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
入门层    ██████████  134 / 134 篇（代码示例） · 137 篇教程文章
进阶层    ░░░░░░░░░░  0 / 137 篇
专家层    ░░░░░░░░░░  0 / 142 篇
```

### ✅ 已完成模块

| 模块 | 代码示例 | 教程文章 | 说明 |
|------|----------|----------|------|
| 环境搭建 | — | 3 篇 | Qt6 安装、IDE 配置、CMake 入门 |
| QtBase 核心模块 | 16 个 ✅ | 16 篇 | QObject、信号槽、字符串、容器... |
| QtGui | 6 个 ✅ | 6 篇 | 绘图、图像、坐标变换... |
| QtWidgets | 74 个 ✅ | 74 篇 | 布局、控件、样式表... |
| QtNetwork | 6 个 ✅ | 6 篇 | TCP/UDP、HTTP、WebSocket... |
| 扩展模块 | 25 个 ✅ | 25 篇 | SQL、Charts、Multimedia、3D、SCXML... |
| QML | 7 个 ✅ | 7 篇 | 现代界面、C++ 互操作... |

查看详细进度：[tutorial/index.md](tutorial/index.md)

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

## 项目规划

除了教程文档之外，项目还规划了完整的 **代码实例库**，目标覆盖 1000+ 个 QtWidgets 项目。详见 [`todo/`](todo/) 目录下的规划清单：

| 清单 | 内容 | 条目数 |
|------|------|--------|
| [`todo/01-widget.md`](todo/01-widget.md) | 单个自定义控件（按钮/标签/输入/进度条/仪表盘/图表/表格/树/列表等） | 500+ |
| [`todo/02-app.md`](todo/02-app.md) | 完整应用 Demo（开发工具/网络工具/文件工具/系统工具/图像工具/多媒体/办公/游戏等） | 190+ |
| [`todo/03-model.md`](todo/03-model.md) | 控件组合与设计模式（窗口框架/导航布局/主题系统/表单/属性编辑器/拖拽/命令模式等） | 300+ |
| [`todo/04-qml.md`](todo/04-qml.md) | QML 专项项目 | 50+ |

代码实例统一使用 **Qt 6 + CMake** 构建，第三方依赖通过 CMake FetchContent 自动管理。

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

这个教程是**动态生成**的，持续更新中。

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
