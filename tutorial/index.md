# AwesomeQt 教程索引

> **一份陪你从零到精通 Qt 6 的踩坑实录**
>
> 这不是官方文档的翻译，也不是 API 手册的堆砌——这是一位在 Qt 底层摸爬滚打多年的工程师（好吧，其实就3年（逃）），把所有踩过的坑、熬过的夜、调试过的 segfault，写成了能陪你走完整条路的教程。

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
│                         知其然              ✅ 已完成        │
├─────────────────────────────────────────────────────────────┤
│  进阶层 (Advanced)   →  掌握高级用法，写出工程级代码          │
│                         知其然且知所以然（在写了在写了）      │
├─────────────────────────────────────────────────────────────┤
│  专家层 (Expert)     →  读懂 Qt 源码，理解设计模式           │
│                         知其所以然并能举一反三 （在写了在写了）│
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

### 入门层 (137 / 137 篇 · 134 个代码示例)

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

#### ✅ 02 · QtGui (已完成 6/6)

- [x] [QPainter 绘图基础](beginner/02-qtgui/01-qpainter-basic-beginner.md)
- [x] [坐标系与 QTransform 变换基础](beginner/02-qtgui/02-coordinate-transform-beginner.md)
- [x] [QImage、QPixmap、QIcon 图像处理基础](beginner/02-qtgui/03-image-pixmap-beginner.md)
- [x] [QFont 与文本渲染基础](beginner/02-qtgui/04-font-text-rendering-beginner.md)
- [x] [QOpenGLWidget 嵌入 OpenGL 基础](beginner/02-qtgui/05-opengl-widget-beginner.md)
- [x] [拖放系统基础](beginner/02-qtgui/06-drag-drop-beginner.md)

#### ✅ 03 · QtWidgets — 主题能力篇 (已完成 10/10)

- [x] [布局系统基础](beginner/03-qtwidgets/01-layout-system-beginner.md)
- [x] [事件处理与传播基础](beginner/03-qtwidgets/02-event-handling-beginner.md)
- [x] [Model/View 架构入门](beginner/03-qtwidgets/03-model-view-beginner.md)
- [x] [样式表 QSS 基础](beginner/03-qtwidgets/04-qss-stylesheet-beginner.md)
- [x] [自定义绘制 Widget 基础](beginner/03-qtwidgets/05-custom-widget-paint-beginner.md)
- [x] [对话框体系基础](beginner/03-qtwidgets/06-dialog-system-beginner.md)
- [x] [QMainWindow 主窗口体系基础](beginner/03-qtwidgets/07-main-window-system-beginner.md)
- [x] [图形视图框架基础](beginner/03-qtwidgets/08-graphics-view-beginner.md)
- [x] [属性动画框架基础](beginner/03-qtwidgets/09-animation-framework-beginner.md)
- [x] [QMdiArea 多文档界面基础](beginner/03-qtwidgets/10-mdi-beginner.md)

#### ✅ 03 · QtWidgets — 控件速查篇 (已完成 64/64)

**基类与抽象类：**

- [x] [QWidget 基类](beginner/03-qtwidgets/11-qwidget-base-beginner.md)
- [x] [QAbstractButton 抽象按钮](beginner/03-qtwidgets/12-qabstractbutton-base-beginner.md)
- [x] [QFrame 帧控件](beginner/03-qtwidgets/13-qframe-base-beginner.md)
- [x] [QAbstractScrollArea 滚动区域](beginner/03-qtwidgets/14-qabstractscrollarea-base-beginner.md)
- [x] [QAbstractItemView 视图基类](beginner/03-qtwidgets/15-qabstractitemview-base-beginner.md)
- [x] [QAbstractSpinBox 微调框基类](beginner/03-qtwidgets/16-qabstractspinbox-base-beginner.md)

**按钮类：**

- [x] [QPushButton](beginner/03-qtwidgets/17-qpushbutton-beginner.md)
- [x] [QToolButton](beginner/03-qtwidgets/18-qtoolbutton-beginner.md)
- [x] [QRadioButton](beginner/03-qtwidgets/19-qradiobutton-beginner.md)
- [x] [QCheckBox](beginner/03-qtwidgets/20-qcheckbox-beginner.md)
- [x] [QCommandLinkButton](beginner/03-qtwidgets/21-qcommandlinkbutton-beginner.md)

**输入类：**

- [x] [QLineEdit](beginner/03-qtwidgets/22-qlineedit-beginner.md)
- [x] [QTextEdit](beginner/03-qtwidgets/23-qtextedit-beginner.md)
- [x] [QPlainTextEdit](beginner/03-qtwidgets/24-qplaintextedit-beginner.md)
- [x] [QTextBrowser](beginner/03-qtwidgets/25-qtextbrowser-beginner.md)
- [x] [QKeySequenceEdit](beginner/03-qtwidgets/26-qkeysequenceedit-beginner.md)
- [x] [QComboBox](beginner/03-qtwidgets/27-qcombobox-beginner.md)
- [x] [QFontComboBox](beginner/03-qtwidgets/28-qfontcombobox-beginner.md)
- [x] [QSpinBox / QDoubleSpinBox](beginner/03-qtwidgets/29-qspinbox-doublespinbox-beginner.md)
- [x] [QDateTimeEdit / QDateEdit / QTimeEdit](beginner/03-qtwidgets/30-qdatetimeedit-dateedit-timeedit-beginner.md)
- [x] [QSlider](beginner/03-qtwidgets/31-qslider-beginner.md)
- [x] [QScrollBar](beginner/03-qtwidgets/32-qscrollbar-beginner.md)
- [x] [QDial](beginner/03-qtwidgets/33-qdial-beginner.md)

**显示类：**

- [x] [QLabel](beginner/03-qtwidgets/34-qlabel-beginner.md)
- [x] [QProgressBar](beginner/03-qtwidgets/35-qprogressbar-beginner.md)
- [x] [QLCDNumber](beginner/03-qtwidgets/36-qlcdnumber-beginner.md)
- [x] [QCalendarWidget](beginner/03-qtwidgets/37-qcalendarwidget-beginner.md)

**容器类：**

- [x] [QGroupBox](beginner/03-qtwidgets/38-qgroupbox-beginner.md)
- [x] [QTabWidget](beginner/03-qtwidgets/39-qtabwidget-beginner.md)
- [x] [QTabBar](beginner/03-qtwidgets/40-qtabbar-beginner.md)
- [x] [QStackedWidget](beginner/03-qtwidgets/41-qstackedwidget-beginner.md)
- [x] [QSplitter](beginner/03-qtwidgets/42-qsplitter-beginner.md)
- [x] [QToolBox](beginner/03-qtwidgets/43-qtoolbox-beginner.md)
- [x] [QScrollArea](beginner/03-qtwidgets/44-qscrollarea-beginner.md)
- [x] [QFrame 分隔线](beginner/03-qtwidgets/45-qframe-separator-beginner.md)

**列表 / 树 / 表格：**

- [x] [QListWidget](beginner/03-qtwidgets/46-qlistwidget-beginner.md)
- [x] [QListView](beginner/03-qtwidgets/47-qlistview-beginner.md)
- [x] [QTreeWidget](beginner/03-qtwidgets/48-qtreewidget-beginner.md)
- [x] [QTreeView](beginner/03-qtwidgets/49-qtreeview-beginner.md)
- [x] [QTableWidget](beginner/03-qtwidgets/50-qtablewidget-beginner.md)
- [x] [QTableView](beginner/03-qtwidgets/51-qtableview-beginner.md)
- [x] [QHeaderView](beginner/03-qtwidgets/52-qheaderview-beginner.md)
- [x] [QColumnView](beginner/03-qtwidgets/53-qcolumnview-beginner.md)
- [x] [QUndoView](beginner/03-qtwidgets/54-qundoview-beginner.md)

**主窗口与对话框：**

- [x] [QMainWindow](beginner/03-qtwidgets/55-qmainwindow-beginner.md)
- [x] [QMenuBar / QAction](beginner/03-qtwidgets/56-qmenubar-menu-action-beginner.md)
- [x] [QToolBar](beginner/03-qtwidgets/57-qtoolbar-beginner.md)
- [x] [QStatusBar](beginner/03-qtwidgets/58-qstatusbar-beginner.md)
- [x] [QDockWidget](beginner/03-qtwidgets/59-qdockwidget-beginner.md)
- [x] [QDialog](beginner/03-qtwidgets/60-qdialog-beginner.md)
- [x] [QDialogButtonBox](beginner/03-qtwidgets/61-qdialogbuttonbox-beginner.md)
- [x] [QMessageBox](beginner/03-qtwidgets/62-qmessagebox-beginner.md)
- [x] [QInputDialog](beginner/03-qtwidgets/63-qinputdialog-beginner.md)
- [x] [QColorDialog](beginner/03-qtwidgets/64-qcolordialog-beginner.md)
- [x] [QFontDialog](beginner/03-qtwidgets/65-qfontdialog-beginner.md)
- [x] [QFileDialog](beginner/03-qtwidgets/66-qfiledialog-beginner.md)
- [x] [QProgressDialog](beginner/03-qtwidgets/67-qprogressdialog-beginner.md)
- [x] [QErrorMessage](beginner/03-qtwidgets/68-qerrormessage-beginner.md)
- [x] [QWizard](beginner/03-qtwidgets/69-qwizard-beginner.md)
- [x] [QSplashScreen](beginner/03-qtwidgets/70-qsplashscreen-beginner.md)
- [x] [QMdiArea / QMdiSubWindow](beginner/03-qtwidgets/71-qmdiarea-mdisubwindow-beginner.md)
- [x] [QPrinter](beginner/03-qtwidgets/72-qprinter-beginner.md)
- [x] [QPrintDialog](beginner/03-qtwidgets/73-qprintdialog-beginner.md)
- [x] [QPrintPreviewDialog](beginner/03-qtwidgets/74-qprintpreviewdialog-beginner.md)

#### ✅ 04 · QtNetwork (已完成 6/6)

- [x] [TCP 通信](beginner/04-qtnetwork/01-tcp-socket-beginner.md)
- [x] [UDP 通信](beginner/04-qtnetwork/02-udp-socket-beginner.md)
- [x] [HTTP 请求 (QNetworkAccessManager)](beginner/04-qtnetwork/03-network-access-manager-beginner.md)
- [x] [WebSocket](beginner/04-qtnetwork/04-websocket-beginner.md)
- [x] [SSL/TLS](beginner/04-qtnetwork/05-ssl-tls-beginner.md)
- [x] [串口通信 QtSerialPort](beginner/04-qtnetwork/06-serial-port-beginner.md)

#### ✅ 05 · 其他扩展模块 (已完成 25/25)

- [x] [QtSql 数据库基础](beginner/05-other-modules/01-qtsql-database-beginner.md)
- [x] [QtSql 表格模型](beginner/05-other-modules/02-qtsql-tablemodel-beginner.md)
- [x] [QtCharts 图表基础](beginner/05-other-modules/03-qtcharts-basic-beginner.md)
- [x] [QtMultimedia 播放器](beginner/05-other-modules/04-qtmultimedia-player-beginner.md)
- [x] [QtMultimedia 相机](beginner/05-other-modules/05-qtmultimedia-camera-beginner.md)
- [x] [QtSVG 矢量图形](beginner/05-other-modules/06-qtsvg-beginner.md)
- [x] [QtPrintSupport 打印支持](beginner/05-other-modules/07-qtprintsupport-overview-beginner.md)
- [x] [QtSerialBus / Modbus](beginner/05-other-modules/08-qtserialbus-modbus-beginner.md)
- [x] [QtMQTT](beginner/05-other-modules/09-qtmqtt-beginner.md)
- [x] [QtBluetooth](beginner/05-other-modules/10-qtbluetooth-beginner.md)
- [x] [QtNFC](beginner/05-other-modules/11-qtnfc-beginner.md)
- [x] [QtStateMachine 状态机](beginner/05-other-modules/12-qtstatemachine-beginner.md)
- [x] [QtSCXML 状态图表](beginner/05-other-modules/13-qtscxml-beginner.md)
- [x] [Qt3D 基础](beginner/05-other-modules/14-qt3d-basic-beginner.md)
- [x] [QtQuick3D](beginner/05-other-modules/15-qtquick3d-beginner.md)
- [x] [QtQuick3D Physics](beginner/05-other-modules/16-qtquick3d-physics-beginner.md)
- [x] [QtPdf](beginner/05-other-modules/17-qtpdf-beginner.md)
- [x] [QtHttpServer](beginner/05-other-modules/18-qthttpserver-beginner.md)
- [x] [QtWebSockets Server](beginner/05-other-modules/19-qtwebsockets-server-beginner.md)
- [x] [QtWebChannel](beginner/05-other-modules/20-qtwebchannel-beginner.md)
- [x] [QtWebEngine](beginner/05-other-modules/21-qtwebengine-beginner.md)
- [x] [QtRemoteObjects](beginner/05-other-modules/22-qtremoteobjects-beginner.md)
- [x] [QtSpatialAudio 空间音频](beginner/05-other-modules/23-qtspatial-audio-beginner.md)
- [x] [QtTextToSpeech 语音合成](beginner/05-other-modules/24-qttexttospeech-beginner.md)
- [x] [Qt5Compat 迁移指南](beginner/05-other-modules/25-qt5compat-migration-beginner.md)

#### ✅ 06 · QML 现代界面 (已完成 7/7)

前置要求：已学完 QtWidgets。

- [x] [QML 语法基础](beginner/06-qml/01-qml-syntax-basics-beginner.md)
- [x] [属性绑定与信号处理](beginner/06-qml/02-property-binding-beginner.md)
- [x] [QtQuick Controls 控件集](beginner/06-qml/03-qtquick-controls-beginner.md)
- [x] [C++ 与 QML 互操作](beginner/06-qml/04-cpp-qml-interop-beginner.md)
- [x] [QML 动画与状态](beginner/06-qml/05-qml-animation-states-beginner.md)
- [x] [QML Model/Delegate 模式](beginner/06-qml/06-qml-model-delegate-beginner.md)
- [x] [QML Canvas 与粒子系统](beginner/06-qml/07-qml-canvas-particles-beginner.md)

### 🎯 项目实战 · Session 1 (9 篇)

- [x] [P1: 状态指示灯](engineering/session1/P1-status-led-practice.md)
- [x] [P2: 搜索编辑框](engineering/session1/P2-search-edit-practice.md)
- [x] [P3: 表单布局](engineering/session1/P3-form-layout-practice.md)
- [x] [P4: 秒表应用](engineering/session1/P4-stopwatch-app-practice.md)
- [x] [P5: 确认对话框](engineering/session1/P5-confirm-dialog-practice.md)
- [x] [P6: 拨动开关](engineering/session1/P6-toggle-switch-practice.md)
- [x] [P7: 向导页](engineering/session1/P7-wizard-page-practice.md)
- [x] [P8: Base64 编解码工具](engineering/session1/P8-base64-helper-practice.md)
- [x] [P9: 哈希计算器](engineering/session1/P9-hash-calculator-practice.md)

### 进阶层 (0 / 预估 137 篇)

待开始...

### 专家层 (0 / 预估 142 篇)

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

这个教程是**动态生成**的，有部分内容AI参与（笔者工作会有些忙，其他开源项目笔者也有在负责），如果你发现：

- 错误或不准确的内容
- 链接失效
- 可以改进的表达方式

直接提出 Issue 或 Pull Request开喷就好，我第一时间响应之！

PS：这里采取的License也是MIT协议。你懂的，随便改，随便用，随意fork!

---

**这么有耐心！好，让我们开始吧！**

建议从 [00 · Qt6 安装踩坑指南](beginner/00-environment-setup/00-qt6-install-beginner.md) 开始你的 Qt 之旅。
