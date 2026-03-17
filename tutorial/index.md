# AwesomeQt 教程索引

> **一份陪你从零到精通 Qt 6 的踩坑实录**
>
> 这不是官方文档的翻译，也不是 API 手册的堆砌——这是一位在 Qt 底层摸爬滚打多年的工程师，把所有踩过的坑、熬过的夜、调试过的 segfault，写成了能陪你走完整条路的教程。

---

## 谁适合读这个教程

| 你可能是... | 这个教程能给你... |
|------------|-----------------|
| C++ 初学者 | 从零开始的 Qt 入门，第一行代码到完整应用 |
| 有其他 GUI 框架经验 | Qt 独特的信号槽、对象树、元对象系统详解 |
| Qt 5 老用户 | Qt 6 的新变化、CMake 构建系统、现代 C++ 写法 |
| 需要深入底层 | MOC 原理、源码解析、设计模式拆解 |
| 工程项目开发者 | 实战导向的最佳实践、性能优化、跨平台部署 |

**前置要求：**
- 熟悉 C++ 基础语法（类、继承、指针、模板）
- 有基本的命令行操作能力
- 不需要任何 Qt 或 GUI 框架经验

---

## 教程结构：三层分级

这个教程分为三个层级，逐层深入：

```
┌─────────────────────────────────────────────────────────────┐
│  入门层 (Beginner)  →  能跑起来，理解核心概念，初步使用 API  │
│                         知其然                               │
├─────────────────────────────────────────────────────────────┤
│  进阶层 (Advanced)   →  掌握高级用法，写出工程级代码          │
│                         知其然且知所以然                     │
├─────────────────────────────────────────────────────────────┤
│  专家层 (Expert)     →  读懂 Qt 源码，理解设计模式           │
│                         知其所以然并能举一反三               │
└─────────────────────────────────────────────────────────────┘
```

每个层级的文档按模块组织，章节顺序相同，内容深度递进。专家层包含专属章节（如 MOC 原理、信号槽底层实现），无需在其他层占位。

---

## 学习路径推荐

### 路径 A：零基础完整学习路线

```
00 环境搭建 (3篇)
    ↓
01 QtBase 核心模块 (16篇)
    ↓
02 QtGui 绘图与图像 (6篇)
    ↓
03 QtWidgets 传统界面 (26篇)
    ↓
[项目实战]
    ↓
04 QtNetwork 网络编程 (6篇)
    ↓
05 其他扩展模块 (按需学习)
    ↓
06 QML 现代界面 (需学完 QtWidgets)
```

### 路径 B：有经验者快速查漏补缺

- 快速浏览「环境搭建」，确保能编译运行
- 直接跳到感兴趣的模块，按需深入
- 遇到问题再回头查看基础概念的详细讲解

### 路径 C：深度钻研源码

建议在完成入门层后，直接进入专家层的源码解析章节：
- MOC 编译器原理
- 信号槽底层实现
- Qt 内存模型与隐式共享
- 事件循环源码

---

## 文档阅读指南

### 每篇文档的标准结构

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[层级] · [模块] · [序号] · [知识点]
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1. 前言/动机段 — 为什么学这个，实际意义
2. 环境说明 — 平台差异、版本依赖
3. 核心概念讲解 — 深度递进的讲解
4. 踩坑预防清单 — 至少 3 个常见坑
5. 随堂测验 — 穿插在讲解中
6. 练习项目 — 动手实践
7. 官方文档参考 — 验证过的可靠链接
```

### 阅读建议

1. **别跳过「踩坑预防」** —— 这些都是真实遇到的问题，跳过等于将来浪费时间
2. **随堂测验自己先想** —— 直接看答案没有意义，大脑需要主动思考
3. **练习项目一定要做** — Qt 是实战技能，光看不练永远学不会
4. **遇到问题先查官方文档** —— 链接在每篇末尾，都已验证可达

### 代码与示例

- **文档中的代码**：聚焦概念讲解的关键片段，不是完整可运行程序
- **完整示例工程**：在 `examples/` 目录，由 Example Agent 独立生成，每个都能编译运行

---

## 当前进度

### 入门层 (19 / 预估 130 篇)

#### ✅ 00 · 环境搭建 (已完成 3/3)

- [x] [Qt6 安装踩坑指南](beginner/00-environment-setup/00-qt6-install-beginner.md) — Windows / Linux / WSL2 完整安装流程
- [x] [IDE 配置全指南](beginner/00-environment-setup/01-ide-setup-beginner.md) — VS Code / CLion / Qt Creator 三端配置
- [x] [第一个 CMake Qt6 工程](beginner/00-environment-setup/02-cmake-first-project-beginner.md) — 从零跑通最小项目

#### ✅ 01 · QtBase 核心模块 (已完成 16/16)

- [x] [QObject 与元对象系统初识](beginner/01-qtbase/01-qobject-meta-system-beginner.md)
- [x] [Signal / Slot 基础用法与新式语法](beginner/01-qtbase/02-signal-slot-beginner.md)
- [x] [QString、QByteArray、字符串操作入门](beginner/01-qtbase/03-string-encoding-beginner.md)
- [x] [QList、QMap、QHash、QSet 基础用法](beginner/01-qtbase/04-container-beginner.md)
- [x] [QVariant 与类型系统基础](beginner/01-qtbase/05-variant-type-beginner.md)
- [x] [Qt 内存管理与对象树基础](beginner/01-qtbase/06-memory-management-beginner.md)
- [x] [Qt 事件系统与事件循环初识](beginner/01-qtbase/07-event-system-beginner.md)
- [x] [QFile、QDir、QTextStream 文件读写](beginner/01-qtbase/08-file-io-beginner.md)
- [x] [QThread 与线程安全基础](beginner/01-qtbase/09-multithreading-beginner.md)
- [x] [QProcess 启动外部程序基础](beginner/01-qtbase/10-qprocess-beginner.md)
- [x] [QTimer 定时器基础用法](beginner/01-qtbase/11-timer-beginner.md)
- [x] [QPluginLoader 插件系统初识](beginner/01-qtbase/12-plugin-beginner.md)
- [x] [国际化 tr() 与翻译流程基础](beginner/01-qtbase/13-i18n-beginner.md)
- [x] [qDebug / qWarning 日志基础](beginner/01-qtbase/14-logging-beginner.md)
- [x] [QRegularExpression 正则表达式基础](beginner/01-qtbase/15-regex-beginner.md)
- [x] [QJsonDocument 与 QXmlStreamReader 解析基础](beginner/01-qtbase/16-json-xml-beginner.md)

#### ⏳ 02 · QtGui (待完成 0/6)

- [ ] QPainter 绘图基础
- [ ] 坐标系与 QTransform 变换基础
- [ ] QImage、QPixmap、QIcon 图像处理基础
- [ ] QFont 与文本渲染基础
- [ ] QOpenGLWidget 嵌入 OpenGL 基础
- [ ] 拖放系统基础

#### ⏳ 03 · QtWidgets — 主题能力篇 (待完成 0/10)

- [ ] 布局系统基础（五大布局管理器）
- [ ] 事件处理与传播基础
- [ ] Model/View 架构入门
- [ ] 样式表 QSS 基础
- [ ] 自定义绘制 Widget 基础
- [ ] 对话框体系基础
- [ ] QMainWindow 主窗口体系基础
- [ ] 图形视图框架基础
- [ ] 属性动画框架基础
- [ ] QMdiArea 多文档界面基础

#### ⏳ 03 · QtWidgets — 控件速查篇 (待完成 0/16)

包括按钮类、输入类、显示类、容器类、列表/树/表格视图等所有标准 Widget 的详细讲解。

#### ⏳ 04 · QtNetwork (待完成 0/6)

- [ ] TCP 通信
- [ ] UDP 通信
- [ ] HTTP 请求
- [ ] WebSocket
- [ ] SSL/TLS
- [ ] 串口通信 QtSerialPort

#### ⏳ 05 · 其他扩展模块 (待完成 0/8)

QtSql、QtCharts、QtMultimedia、QtBluetooth、Qt3D、QtPdf、Modbus、MQTT。

#### ⏳ 06 · QML 独立教程 (待完成 0/9)

前置要求：已学完 QtWidgets。三层均有，独立成册。

### 进阶层 (0 / 预估 130 篇)

待开始...

### 专家层 (0 / 预估 145 篇)

待开始...（含 MOC 原理、信号槽源码、内存模型、事件循环源码等专属章节）

---

## 技术栈说明

| 项目 | 版本/说明 |
|------|----------|
| Qt 版本 | 6.9.1（编写时的最新 LTS） |
| 构建系统 | CMake 3.26+ |
| C++ 标准 | C++17/C++20 |
| 测试平台 | Windows 10/11, Ubuntu 22.04+, WSL2 |
| 编译器 | MSVC 2019/2022, MinGW 11.2+, GCC 11+ |

---

## 写作风格说明

这个教程有自己的"性格"：

- **第一人称叙事** —— 会说"我们"、"你会发现"，而不是"用户应当"
- **允许情绪表达** —— 会吐槽、会感叹踩坑经历，但保持专业
- **实战导向** —— 每个概念都结合实际应用场景，不只是理论
- **踩坑必写后果** —— 不只说错误做法，必须说明会出什么问题
- **禁止 ChatGPT 风格** —— 拒绝"值得注意的是"、"综上所述"等空话

---

## 与官方文档的关系

- 这个教程**不是**官方文档的替代品
- 建议的学习方式：**教程入门 → 官方文档深入 → 源码验证**
- 每篇末尾的官方文档链接都已验证可达
- 遇到疑问时，官方文档是最终权威

---

## 贡献与反馈

这个教程是**动态生成**的，由 AI 根据严格指南持续产出。

如果你发现：
- 错误或不准确的内容
- 链接失效
- 可以改进的表达方式

欢迎提出 Issue 或 Pull Request。

---

## 许可协议

本教程内容遵循 CC BY-NC-SA 4.0 协议：
- ✅ 可以分享、转载、演绎
- ✅ 必须署名原作者
- ❌ 不得商用
- ⚠️ 演绎作品需采用相同协议

---

**现在你已经知道了如何使用这个教程，让我们开始吧！**

建议从 [00 · Qt6 安装踩坑指南](beginner/00-environment-setup/00-qt6-install-beginner.md) 开始你的 Qt 之旅。
