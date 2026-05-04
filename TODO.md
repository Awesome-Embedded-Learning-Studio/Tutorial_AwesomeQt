# 📋 AwesomeQt · 教程生成进度追踪【百科全书版】

> **AI 使用说明**：每次启动前必须完整读取本文件，确认进度后再开始生成。完成一篇立刻将 `[ ]` 改为 `[x]` 并标注日期。
>
> **Badge 说明**：🔴 高频（工程项目必用）· 🟡 中频（按需使用）· ⚪ 低频（特定场景/冷门）

---

## 🔢 总体进度

```
入门层    ██████████  118 / 118 篇
进阶层    ░░░░░░░░░░  0 / 137 篇
专家层    ░░░░░░░░░░  0 / 142 篇
合计      ██░░░░░░░░  118 / 397 篇
```

---

## 🟢 入门层

全部完成（116 篇），归档于 [todo/archive/beginner-completed.md](todo/archive/beginner-completed.md)

---

## 🔵 进阶层

<details>
<summary><strong>00 · 环境搭建</strong>（3 篇）</summary>

- [ ] 🔴 `00-qt6-install-advanced.md` — 多平台进阶配置：多版本共存与 Qt 模块裁剪
  - Qt 多版本共存管理，`qtchooser` / 手动切换
  - 仅安装必要模块减少磁盘占用
  - 自定义安装路径与 `CMAKE_PREFIX_PATH` 配置
  - Windows 下 MSVC vs MinGW 工具链选择与陷阱

- [ ] 🔴 `01-ide-advanced.md` — IDE 进阶配置：调试器、静态分析、代码格式化
  - VS Code clangd 静态分析 + `compile_commands.json` 生成
  - CLion Sanitizer（ASan/UBSan）集成调试内存问题
  - Qt Creator 分析器（Valgrind / Heaptrack）使用
  - `.clang-format` 统一代码风格配置

- [ ] 🔴 `02-cmake-advanced.md` — CMake 进阶：多目标、共享库、CMakePresets
  - `add_library(SHARED)` 构建共享库并被主程序链接
  - `target_include_directories` / `target_compile_definitions` 精准传递配置
  - `CMakePresets.json` 多平台 Preset 统一构建命令
  - `FetchContent` 引入第三方依赖（如 spdlog / nlohmann-json）

</details>

<details>
<summary><strong>01 · QtBase</strong>（16 篇）</summary>

- [ ] 🔴 `01-qobject-property-system-advanced.md` — QObject 属性系统进阶：Q_PROPERTY 与动态属性
  - `Q_PROPERTY` 完整语法：READ/WRITE/NOTIFY/RESET/STORED
  - `setProperty()` / `property()` 动态属性（运行时添加）
  - 属性变化通知 NOTIFY 信号的最佳实践
  - `QMetaObject::propertyCount()` 反射枚举所有属性

- [ ] 🔴 `02-signal-slot-advanced.md` — 信号槽进阶：连接类型、Lambda 陷阱、跨线程
  - `Qt::BlockingQueuedConnection` 使用场景与死锁风险
  - Lambda 捕获对象指针的生命周期陷阱
  - `QObject::connect` 返回 `QMetaObject::Connection` 手动管理
  - `QSignalSpy` 单元测试中验证信号触发

- [ ] 🔴 `03-qstring-advanced.md` — QString 进阶：编码陷阱、QStringView、性能优化
  - Latin-1 / UTF-8 / UTF-16 隐式转换陷阱
  - `QStringView` 零拷贝字符串视图减少内存分配
  - `QString::arg()` 多参数替换的正确顺序
  - `QStringBuilder` (`%` 操作符) 拼接性能优化

- [ ] 🔴 `04-containers-advanced.md` — 容器进阶：隐式共享、算法、STL 互操作
  - COW（写时复制）何时触发拷贝，如何避免意外深拷贝
  - `<QtAlgorithms>` / `std::` 算法在 Qt 容器上的使用
  - `QList` ↔ `std::vector` 互转的零拷贝技巧
  - `QHash` 自定义键类型：重载 `operator==` 与 `qHash`

- [ ] 🟡 `05-qvariant-metatype-advanced.md` — QVariant 进阶：自定义类型注册
  - `Q_DECLARE_METATYPE(T)` + `qRegisterMetaType<T>()` 完整流程
  - 自定义类型在信号槽跨线程传递的注册要求
  - `QVariant::fromValue<T>` / `qvariant_cast<T>` 类型安全存取
  - `QMetaType` 反射能力：构造/析构/比较自定义类型

- [ ] 🔴 `06-memory-management-advanced.md` — 内存管理进阶：智能指针与循环引用
  - `QSharedPointer` + `QWeakPointer` 打破循环引用
  - `QPointer<T>` 弱指针自动置空于对象销毁（Qt 专属）
  - `QScopedPointer` vs `std::unique_ptr` 的互操作
  - 内存泄漏检测：Valgrind / AddressSanitizer 实战

- [ ] 🔴 `07-event-system-advanced.md` — 事件系统进阶：自定义事件与过滤器
  - 自定义事件类：继承 `QEvent` + `QEvent::registerEventType()`
  - `QCoreApplication::postEvent()` 跨线程安全投递自定义事件
  - `eventFilter()` 全局鼠标/键盘监听
  - `QAbstractNativeEventFilter` 截获原生系统消息

- [ ] 🔴 `08-file-io-advanced.md` — 文件 IO 进阶：序列化与文件监控
  - `QDataStream` 二进制序列化：版本兼容策略
  - `QFileSystemWatcher` 监控文件/目录变化
  - `QSaveFile` 原子写入防止数据损坏
  - 内存映射文件 `QFile::map()` 大文件处理

- [ ] 🔴 `09-qthread-advanced.md` — 多线程进阶：线程池与 QtConcurrent
  - `QThreadPool` + `QRunnable` 任务队列
  - `QtConcurrent::run()` 将函数异步提交线程池
  - `QFuture<T>` + `QFutureWatcher<T>` 异步结果监控
  - `QReadWriteLock` 读写锁优化多读少写场景

- [ ] 🟡 `10-qprocess-advanced.md` — QProcess 进阶：异步读写与进程间通信
  - `setProcessChannelMode(MergedChannels)` 合并 stdout/stderr
  - `readyReadStandardOutput` 异步流式读取大输出
  - 管道通信：将一个进程输出作为另一个的输入
  - 进程崩溃检测与退出码分析

- [ ] 🟡 `11-qtimer-advanced.md` — 定时器进阶：高精度计时与性能分析
  - `QTimer::timerType()` 三种精度级别选择
  - `QElapsedTimer` 纳秒级性能计时
  - 定时器聚合：避免大量小定时器的性能问题
  - `QDeadlineTimer` 超时控制在异步 API 中的应用

- [ ] 🟡 `12-plugin-advanced.md` — 插件系统进阶：版本管理与热加载
  - 插件接口版本控制防二进制不兼容
  - `QPluginLoader::unload()` 热卸载与资源释放顺序
  - `QDir::entryList()` 自动发现插件目录
  - 插件依赖链管理（插件 A 依赖插件 B）

- [ ] 🟡 `13-i18n-advanced.md` — 国际化进阶：复数规则与动态语言切换
  - 复数形式 `%n` 与各语言规则差异
  - 运行时动态切换语言（`QTranslator` 重新安装）
  - `QLocale` 数字/日期/货币的本地化格式
  - `lupdate` 扫描范围配置与过期字符串清理

- [ ] 🟡 `14-logging-advanced.md` — 日志进阶：自定义处理器与分类过滤
  - `qInstallMessageHandler` 自定义日志输出到文件
  - `QLoggingCategory` 多模块独立日志开关
  - Release 构建保留 qWarning 但禁用 qDebug
  - 结合 spdlog 构建工程级日志系统

- [ ] 🟡 `15-regex-advanced.md` — 正则进阶：命名捕获与性能分析
  - 命名捕获组 `(?P<name>...)` 提高可读性
  - `QRegularExpression::optimize()` JIT 预编译
  - 全局匹配 `QRegularExpression::globalMatch()` 迭代所有匹配
  - 灾难性回溯（Catastrophic Backtracking）预防

- [ ] 🟡 `16-json-xml-advanced.md` — JSON/XML 进阶：流式处理大文件
  - `QJsonDocument` 解析大 JSON 的内存开销分析
  - 流式 JSON 构建：手动拼接 vs `QJsonDocument` 性能对比
  - `QXmlStreamWriter` 生成格式化 XML
  - `QXmlStreamReader` 状态机解析嵌套 XML 结构

</details>

<details>
<summary><strong>02 · QtGui（进阶）</strong>（6 篇）</summary>

- [ ] 🔴 `01-qpainter-advanced.md` — QPainter 进阶：双缓冲、合成模式、抗锯齿
  - `setRenderHint(QPainter::Antialiasing)` 开启抗锯齿
  - `QPixmap` 离屏缓冲消除闪烁（双缓冲原理）
  - `setCompositionMode` 图层合成模式（正片叠底/滤色/叠加）
  - `QPainterPath` 复杂路径绘制与裁剪

- [ ] 🟡 `02-coordinate-transform-advanced.md` — 坐标变换进阶：矩阵组合与逆变换
  - `QTransform` 矩阵乘法组合多个变换
  - `QTransform::inverted()` 求逆变换（鼠标坐标映射回场景坐标）
  - 仿射变换 vs 投影变换的区别
  - `QPainter::worldTransform()` 在复杂绘制中保存/恢复

- [ ] 🔴 `03-image-processing-advanced.md` — 图像处理进阶：像素操作与格式转换
  - `QImage::pixel()` / `setPixel()` 逐像素操作
  - `QImage::Format` 格式转换与内存布局
  - `QImageReader` 支持的格式与流式加载大图
  - `Qt::SmoothTransformation` vs `FastTransformation` 缩放质量

- [ ] 🟡 `04-font-text-advanced.md` — 字体进阶：富文本与 QTextDocument
  - `QTextDocument` 完整文档模型（段落/表格/图片）
  - `QTextCursor` 程序化构建富文本内容
  - `QTextCharFormat` / `QTextBlockFormat` 格式控制
  - `QTextDocument::toHtml()` / `toMarkdown()` 格式导出

- [ ] 🟡 `05-opengl-advanced.md` — OpenGL 进阶：着色器与 VAO/VBO
  - `QOpenGLShaderProgram` 编译链接顶点/片段着色器
  - VAO / VBO 创建绑定与顶点属性布局
  - 纹理加载：`QOpenGLTexture` 封装
  - `QOpenGLFramebufferObject` 离屏渲染

- [ ] 🟡 `06-drag-drop-advanced.md` — 拖放进阶：自定义 MIME 与跨应用拖放
  - 自定义 MIME 类型定义与序列化
  - `dropMimeData()` 在 Model/View 中支持拖放重排
  - 跨应用文件拖放（从 Explorer/Finder 拖文件到 Qt 应用）
  - `Qt::DropAction` 区分复制/移动/链接操作

</details>

<details>
<summary><strong>03 · QtWidgets 进阶（篇幅等同入门层，此处只列主题能力篇，控件速查层内容与入门层相同）</strong>（10 篇）</summary>

- [ ] 🔴 `01-layout-system-advanced.md` — 布局进阶：尺寸策略与动态布局切换
  - `QSizePolicy` 六种策略（Fixed/Minimum/Maximum/Preferred/Expanding/Ignored）
  - 布局内插入/删除控件并刷新
  - `QStackedLayout` 实现动画页面切换
  - 嵌套布局性能优化与 `layout()` 调试

- [ ] 🔴 `02-event-handling-advanced.md` — 事件处理进阶：键盘修饰键与原生事件
  - `grabMouse()` / `grabKeyboard()` 强制捕获输入
  - `QApplication::keyboardModifiers()` 获取修饰键状态
  - `QWheelEvent::angleDelta()` 滚轮精细处理
  - `nativeEvent()` 处理平台原生窗口消息

- [ ] 🔴 `03-model-view-advanced.md` — Model/View 进阶：自定义 Model 与 Delegate
  - 继承 `QAbstractTableModel` 实现完整 Model
  - `QSortFilterProxyModel` 代理模型实现搜索过滤与排序
  - 自定义 `QStyledItemDelegate` 实现单元格编辑器
  - `QAbstractItemModel::beginInsertRows()` 数据增删的正确通知方式

- [ ] 🔴 `04-qss-advanced.md` — QSS 进阶：动态主题切换与复杂选择器
  - `setObjectName` 配合 `#id` 选择器精准定位
  - `QStyle::polish()` 结合 QSS 动态皮肤系统
  - `qproperty-*` 通过 QSS 设置 Q_PROPERTY 值
  - 高 DPI 下 QSS 像素值的缩放处理

- [ ] 🔴 `05-custom-widget-advanced.md` — 自定义控件进阶：子控件与 QStyle
  - `QStyleOption` + `QStylePainter` 跟随系统风格绘制
  - `QStyle::subControlRect()` 子控件矩形计算
  - `QSizePolicy::HeightForWidth` 保持宽高比
  - 发布自定义控件到 Qt Designer 插件

- [ ] 🔴 `06-dialog-advanced.md` — 对话框进阶：模态策略与数据验证
  - `Qt::ApplicationModal` vs `Qt::WindowModal` 模态范围
  - 输入验证失败阻止 `accept()` 的正确姿势
  - 多步骤向导对话框数据流管理
  - 对话框记忆上次位置与尺寸

- [ ] 🔴 `07-main-window-advanced.md` — 主窗口进阶：Dock 管理与状态持久化
  - `saveState()` / `restoreState()` 持久化整个主窗口布局
  - `QSettings` 保存/恢复窗口几何信息
  - 动态显示/隐藏 Dock 的菜单同步
  - `QMainWindow::setCorner()` 角落 Dock 区域归属

- [ ] 🟡 `08-graphics-view-advanced.md` — 图形视图进阶：自定义 Item 与碰撞检测
  - 继承 `QGraphicsItem` 实现完整自定义图元
  - `QGraphicsScene::collidingItems()` 碰撞检测
  - `itemChange()` 拦截位置/状态变化
  - `QGraphicsEffect` 给 Item 加模糊/阴影/颜色效果

- [ ] 🟡 `09-animation-advanced.md` — 动画进阶：状态机驱动与并行动画组
  - `QStateMachine` + 动画 `addTransition` 状态切换动画
  - `QParallelAnimationGroup` 多属性同时动画
  - `QAbstractAnimation::updateCurrentValue()` 自定义插值动画
  - 动画在 QML 与 Widgets 混合界面中的协调

- [ ] ⚪ `10-mdi-advanced.md` — MDI 进阶：子窗口策略与文档管理
  - 子窗口关闭前保存确认（拦截 `QCloseEvent`）
  - `QMdiArea::SubWindowActivated` 同步菜单状态
  - MDI vs 多标签（`QTabWidget`）的场景选择
  - 最大化子窗口时菜单栏合并处理

</details>

<details>
<summary><strong>03 · QtWidgets 进阶 — 控件速查篇</strong>（与入门层一一对应，共 60 篇）</summary>

> 文件命名规则：将入门层文件名中的 `-beginner` 替换为 `-advanced`，共 60 篇，内容深入至高级 API、性能优化、自定义扩展与工程实践。此处仅列出每篇的进阶重点方向，不再逐条展开知识点（与入门层结构对应）：

- [ ] 🔴 `11-qwidget-base-advanced.md` — 窗口属性进阶：WA_* 属性、透明背景、无边框窗口拖移
- [ ] 🔴 `12-qabstractbutton-base-advanced.md` — 自定义按钮状态机与三态按钮完整实现
- [ ] 🟡 `13-qframe-base-advanced.md` — QFrame 作为自定义带阴影容器的绘制实现
- [ ] 🟡 `14-qabstractscrollarea-base-advanced.md` — 手动同步双 ScrollArea 与视口坐标计算
- [ ] 🔴 `15-qabstractitemview-base-advanced.md` — 视图基类拖放重排、持久化编辑器、虚拟列表
- [ ] 🟡 `16-qabstractspinbox-base-advanced.md` — 自定义 SpinBox 子类实现非数字步进（如货币/角度）
- [ ] 🔴 `17-qpushbutton-advanced.md` — QPushButton 带动画的自定义 QSS 交互效果
- [ ] 🔴 `18-qtoolbutton-advanced.md` — QToolButton 动态切换图标与文字的工具栏适配
- [ ] 🔴 `19-qradiobutton-advanced.md` — QRadioButton 跨容器互斥与动态分组策略
- [ ] 🔴 `20-qcheckbox-advanced.md` — QCheckBox 树形全选/半选逻辑实现
- [ ] ⚪ `21-qcommandlinkbutton-advanced.md` — QCommandLinkButton 跨平台样式统一
- [ ] 🔴 `22-qlineedit-advanced.md` — QLineEdit 自定义内嵌图标按钮与实时补全
- [ ] 🔴 `23-qtextedit-advanced.md` — QTextEdit 语法高亮（QSyntaxHighlighter）实现
- [ ] 🟡 `24-qplaintextedit-advanced.md` — QPlainTextEdit 行号区域绘制与代码折叠
- [ ] 🟡 `25-qtextbrowser-advanced.md` — QTextBrowser 自定义资源加载（图片/CSS）
- [ ] ⚪ `26-qkeysequenceedit-advanced.md` — QKeySequenceEdit 冲突检测与全局热键注册
- [ ] 🔴 `27-qcombobox-advanced.md` — QComboBox 自定义 ItemDelegate 实现复杂下拉项
- [ ] 🟡 `28-qfontcombobox-advanced.md` — QFontComboBox 自定义预览与字体分类
- [ ] 🔴 `29-qspinbox-advanced.md` — QSpinBox 自定义 textFromValue / valueFromText
- [ ] 🟡 `30-qdatetimeedit-advanced.md` — QDateTimeEdit 时区感知与自定义日期范围显示
- [ ] 🔴 `31-qslider-advanced.md` — QSlider 多档位刻度标注与区间选择双滑块实现
- [ ] 🟡 `32-qscrollbar-advanced.md` — QScrollBar 驱动大画布局部视口同步
- [ ] 🟡 `33-qdial-advanced.md` — QDial 自定义刻度绘制与角度映射
- [ ] 🔴 `34-qlabel-advanced.md` — QLabel 动态 GIF 播放控制与高 DPI 图像适配
- [ ] 🔴 `35-qprogressbar-advanced.md` — QProgressBar 自定义绘制（圆形进度、渐变色）
- [ ] 🟡 `36-qlcdnumber-advanced.md` — QLCDNumber 自定义七段数码管外观
- [ ] 🟡 `37-qcalendarwidget-advanced.md` — QCalendarWidget 标记特殊日期与自定义单元格
- [ ] 🔴 `38-qgroupbox-advanced.md` — QGroupBox 动态折叠面板动画实现
- [ ] 🔴 `39-qtabwidget-advanced.md` — QTabWidget 可关闭/可拖动/自定义标签页实现
- [ ] 🟡 `40-qtabbar-advanced.md` — QTabBar 自定义绘制标签与角落按钮
- [ ] 🔴 `41-qstackedwidget-advanced.md` — QStackedWidget 滑动切换动画实现
- [ ] 🔴 `42-qsplitter-advanced.md` — QSplitter 自定义拖动手柄外观与最小宽度约束
- [ ] 🟡 `43-qtoolbox-advanced.md` — QToolBox 自定义标题栏样式与动画展开
- [ ] 🔴 `44-qscrollarea-advanced.md` — QScrollArea 平滑滚动动画与触控板手势支持
- [ ] 🔴 `45-qframe-advanced.md` — QFrame 自定义圆角阴影边框绘制
- [ ] 🔴 `46-qlistwidget-advanced.md` — QListWidget 拖放排序与自定义 ItemWidget
- [ ] 🔴 `47-qlistview-advanced.md` — QListView 大数据虚拟列表优化
- [ ] 🔴 `48-qtreewidget-advanced.md` — QTreeWidget 延迟加载子节点（懒加载）
- [ ] 🔴 `49-qtreeview-advanced.md` — QTreeView 自定义展开图标与整行选中
- [ ] 🔴 `50-qtablewidget-advanced.md` — QTableWidget 单元格合并与冻结首行首列
- [ ] 🔴 `51-qtableview-advanced.md` — QTableView 百万行数据虚拟滚动性能优化
- [ ] 🟡 `52-qheaderview-advanced.md` — QHeaderView 双级表头与自定义排序逻辑
- [ ] ⚪ `53-qcolumnview-advanced.md` — QColumnView 自定义列宽与预览组件
- [ ] ⚪ `54-qundoview-advanced.md` — QUndoView 与 QUndoStack 完整撤销重做系统
- [ ] 🔴 `55-qmainwindow-advanced.md` — QMainWindow 多显示器适配与全屏模式切换
- [ ] 🔴 `56-qmenubar-advanced.md` — QMenuBar 动态构建菜单与最近文件列表
- [ ] 🔴 `57-qtoolbar-advanced.md` — QToolBar 响应式工具栏（宽度不足时折叠）
- [ ] 🔴 `58-qstatusbar-advanced.md` — QStatusBar 多区域复杂状态显示
- [ ] 🔴 `59-qdockwidget-advanced.md` — QDockWidget 多文档编辑器布局持久化
- [ ] 🔴 `60-qdialog-advanced.md` — QDialog 异步对话框（非阻塞）与结果回调
- [ ] 🔴 `61-qdialogbuttonbox-advanced.md` — QDialogButtonBox 自定义帮助按钮与提示
- [ ] 🔴 `62-qmessagebox-advanced.md` — QMessageBox 自定义图标与详情区域
- [ ] 🟡 `63-qinputdialog-advanced.md` — QInputDialog 自定义验证器与输入范围
- [ ] 🟡 `64-qcolordialog-advanced.md` — QColorDialog 集成到自定义颜色选择器面板
- [ ] 🟡 `65-qfontdialog-advanced.md` — QFontDialog 过滤字体并预览效果
- [ ] 🔴 `66-qfiledialog-advanced.md` — QFileDialog 自定义预览面板与文件类型图标
- [ ] 🟡 `67-qprogressdialog-advanced.md` — QProgressDialog 异步任务取消与进度精确同步
- [ ] ⚪ `68-qerrormessage-advanced.md` — QErrorMessage 持久化抑制状态到 QSettings
- [ ] 🟡 `69-qwizard-advanced.md` — QWizard 非线性跳转与动态页面生成
- [ ] 🟡 `70-qsplashscreen-advanced.md` — QSplashScreen 渐变消隐动画与最小显示时长保证
- [ ] 🟡 `71-qmdiarea-advanced.md` — QMdiArea 标签页模式与子窗口菜单集成
- [ ] 🟡 `72-qprinter-advanced.md` — QPrinter 自定义页眉页脚与分页逻辑
- [ ] 🟡 `73-qprintdialog-advanced.md` — QPrintDialog 集成预览与打印范围选择
- [ ] ⚪ `74-qprintpreviewdialog-advanced.md` — QPrintPreviewDialog 自定义工具栏操作

</details>

<details>
<summary><strong>04 · QtNetwork（进阶）</strong>（6 篇）</summary>

- [ ] 🔴 `01-tcp-advanced.md` — TCP 进阶：多客户端、粘包处理、心跳机制
  - 多客户端连接管理（连接表 / ID 映射）
  - 自定义协议帧：包头长度字段解决粘包/拆包
  - 心跳包超时检测与断线重连自动机
  - `QTcpServer::setMaxPendingConnections` 控制连接队列

- [ ] 🟡 `02-udp-advanced.md` — UDP 进阶：多播、大数据分片重组
  - `QUdpSocket::joinMulticastGroup()` 加入多播组
  - UDP 大数据手动分片与序号重组
  - `QNetworkDatagram` 携带发送方信息
  - UDP 可靠传输的简单 ARQ 机制实现

- [ ] 🔴 `03-http-advanced.md` — HTTP 进阶：请求队列、拦截器、Cookie
  - 并发请求限流与优先级队列
  - `QNetworkAccessManager` 全局请求拦截（鉴权头注入）
  - `QNetworkCookieJar` 管理 Session Cookie
  - 断点续传：`Range` 头 + 本地进度记录

- [ ] 🟡 `04-websocket-advanced.md` — WebSocket 进阶：断线重连与心跳保活
  - 指数退避重连策略实现
  - Ping/Pong 心跳帧发送与超时检测
  - 大消息分帧发送（`sendBinaryMessage` 超大 payload）
  - WSS 证书配置与自签名证书信任

- [ ] 🟡 `05-ssl-advanced.md` — SSL 进阶：双向认证与证书链验证
  - 客户端证书（mTLS）配置流程
  - `QSslCertificate` 证书解析与有效期检查
  - `QSslError` 白名单忽略特定错误（仅开发调试）
  - Let's Encrypt 证书在 Qt 应用中的信任配置

- [ ] 🔴 `06-serialport-advanced.md` — 串口进阶：自定义协议封装与超时处理
  - 自定义帧格式：帧头/长度/数据/校验和解析状态机
  - 接收缓冲区管理：不完整帧的暂存策略
  - `QSerialPort::WaitForReadyRead` 同步等待 vs 异步 `readyRead`
  - 多串口同时管理与优先级调度

</details>

<details>
<summary><strong>05 · 其他模块（进阶，与入门层一一对应）</strong>（25 篇）</summary>

- [ ] 🔴 `01-qtsql-advanced.md` — QtSql 进阶：事务、连接池、ORM 封装
- [ ] 🟡 `02-qtsql-tablemodel-advanced.md` — QSqlRelationalTableModel 关联表视图
- [ ] 🔴 `03-qtcharts-advanced.md` — QtCharts/QtGraphs 进阶：实时数据更新与自定义 Axis
- [ ] 🔴 `04-qtmultimedia-player-advanced.md` — 媒体播放进阶：播放列表、媒体元数据、字幕
- [ ] 🟡 `05-qtmultimedia-camera-advanced.md` — 摄像头进阶：视频录制、帧处理、滤镜
- [ ] 🟡 `06-qtsvg-advanced.md` — QtSvg 进阶：动态修改 SVG 元素属性
- [ ] 🟡 `07-qtprintsupport-advanced.md` — 打印进阶：复杂报表生成与多页面布局
- [ ] 🔴 `08-modbus-advanced.md` — Modbus 进阶：RTU/TCP 切换、寄存器映射表管理
- [ ] 🟡 `09-mqtt-advanced.md` — MQTT 进阶：QoS 1/2、遗嘱消息、TLS 加密连接
- [ ] 🟡 `10-qtbluetooth-advanced.md` — 蓝牙进阶：GATT Profile 读写 Characteristic
- [ ] ⚪ `11-qtnfc-advanced.md` — NFC 进阶：NDEF 记录类型详解与写入标签
- [ ] 🟡 `12-qtstatemachine-advanced.md` — 状态机进阶：层次状态机、历史状态、并行状态
- [ ] ⚪ `13-qtscxml-advanced.md` — SCXML 进阶：数据模型与延迟事件
- [ ] 🟡 `14-qt3d-advanced.md` — Qt3D 进阶：自定义 Material、Framegraph 配置
- [ ] 🟡 `15-qtquick3d-advanced.md` — QtQuick3D 进阶：PBR 材质、环境光遮蔽、阴影
- [ ] ⚪ `16-qtquick3d-physics-advanced.md` — 物理进阶：关节约束、力与冲量、射线检测
- [ ] 🟡 `17-qtpdf-advanced.md` — QtPdf 进阶：文本搜索、选中复制、书签导航
- [ ] 🟡 `18-qthttpserver-advanced.md` — HttpServer 进阶：中间件链、静态文件服务、身份验证
- [ ] 🟡 `19-qtwebsockets-advanced.md` — WebSocket 服务端进阶：房间广播、消息队列
- [ ] ⚪ `20-qtwebchannel-advanced.md` — WebChannel 进阶：自定义传输层（非 WebEngine）
- [ ] ⚪ `21-qtwebengine-advanced.md` — WebEngine 进阶：自定义 URL Scheme、安全策略
- [ ] ⚪ `22-qtremoteobjects-advanced.md` — Remote Objects 进阶：自定义序列化与网络传输
- [ ] ⚪ `23-qtspatial-audio-advanced.md` — 空间音频进阶：混响、距离衰减、头部追踪
- [ ] ⚪ `24-qttexttospeech-advanced.md` — TTS 进阶：SSML 标记语言与语音合成控制
- [ ] 🟡 `25-qt5compat-advanced.md` — Qt5Compat 进阶：批量迁移策略与自动化检测工具

</details>

<details>
<summary><strong>06 · QML 独立教程（进阶）</strong>（7 篇）</summary>

- [ ] 🔴 `01-qml-syntax-advanced.md` — QML 语法进阶：绑定陷阱、`required` 属性、`readonly`
  - 绑定断裂（命令式赋值覆盖绑定）的排查与修复
  - `required property` 强制父级传值
  - `readonly property` 只读属性的正确用法
  - 延迟初始化与 `Component.onCompleted`

- [ ] 🔴 `02-property-binding-advanced.md` — 属性绑定进阶：`Binding` 元素与条件绑定
  - `Binding { target; property; value; when }` 条件绑定
  - `Qt.binding()` 在命令式代码中重建绑定
  - 双向绑定的正确实现（避免绑定循环）
  - 属性别名 `property alias` 与性能分析

- [ ] 🔴 `03-qtquick-controls-advanced.md` — Qt Quick Controls 进阶：自定义样式
  - `Material` / `Fusion` / `Universal` 样式切换
  - 自定义 Control 模板（`background` / `contentItem` / `indicator`）
  - `Palette` 主题色统一管理
  - `ToolTip` / `ToolTipAttached` 全局提示配置

- [ ] 🔴 `04-cpp-qml-interop-advanced.md` — C++/QML 进阶：类型系统与 QML 模块注册
  - `QML_ELEMENT` / `QML_SINGLETON` 宏的区别与用法
  - `qmlRegisterUncreatableType` 只暴露枚举/常量
  - C++ `QAbstractListModel` 完整实现供 QML ListView 驱动
  - Q_INVOKABLE 方法的线程安全注意事项

- [ ] 🔴 `05-qml-animation-advanced.md` — QML 动画进阶：路径动画与 Animator
  - `PathAnimation` 沿路径运动
  - `Animator`（在渲染线程运行，比 Animation 更流畅）
  - `SmoothedAnimation` / `SpringAnimation` 物理感动画
  - 动画性能分析：避免 JavaScript 在动画帧中执行

- [ ] 🔴 `06-qml-model-view-advanced.md` — QML 模型视图进阶：DelegateModel 与 section
  - `DelegateModel` 分组与排序
  - `ListView::section` 分节标题
  - `QSortFilterProxyModel` 在 QML 中的使用
  - 大数据列表性能：`cacheBuffer`、`displayMarginBeginning`

- [ ] 🟡 `07-qml-async-workerscript-advanced.md` — QML 异步进阶：WorkerScript 线程模型
  - `WorkerScript` 在后台线程执行 JS 计算
  - `sendMessage` / `onMessage` 线程间通信
  - `XMLHttpRequest` 在 QML 中的异步 HTTP 请求
  - `Loader` 异步组件加载（`asynchronous: true`）

</details>

---

## 🔴 专家层

> **注意**：专家层每篇文档均对应入门/进阶层的同名知识点，深入源码实现原理。此外包含专家专属章节（标注【专家专属】）。

<details>
<summary><strong>00 · 环境搭建（专家）</strong>（2 篇）</summary>

- [ ] 🟡 `00-qt6-build-from-source-expert.md` — 从源码编译 Qt6：配置裁剪与调试符号
  - `configure` 脚本各 `-feature-*` 开关含义
  - 仅编译特定模块的 `cmake --build` 目标指定
  - 调试版 Qt 编译（`-debug` / `-debug-and-release`）
  - Qt 源码目录结构导览：找到任意类的实现文件

- [ ] 🔴 `01-cmake-expert.md` — CMake 专家：生成器表达式、目标传播与自定义工具链
  - `$<TARGET_FILE:tgt>` 等生成器表达式深度解析
  - `INTERFACE` / `PUBLIC` / `PRIVATE` 属性传播语义
  - 自定义 `cmake` 工具链文件交叉编译
  - `cmake --graphviz` 依赖图可视化调试

</details>

<details>
<summary><strong>01 · QtBase（专家，含专属章节）</strong>（20 篇）</summary>

- [ ] 🔴 `01-qobject-meta-system-expert.md` — QObject 元对象系统源码拆解
  - `QObjectPrivate` d 指针模式（PIMPL）实现原理
  - `QMetaObject` 结构体：stringdata / data 数组布局
  - `qt_metacall` 函数的生成规则与分发逻辑
  - 对象树在 `QObjectPrivate::children` 中的存储实现

- [ ] 🔴 `02-signal-slot-internals-expert.md` — 信号槽底层：QMetaObject::activate 调用链源码
  - MOC 如何将 `emit signal()` 转换为 `activate()` 调用
  - `ConnectionData` 结构：连接列表的存储与锁
  - 跨线程投递：`QMetaCallEvent` 包装参数到事件队列
  - 信号槽性能对比：直接调用 / 函数指针 / Lambda 的开销

- [ ] 🔴 `03-qstring-memory-expert.md` — QString 内存模型源码：SSO 与 COW
  - `QStringPrivate` / `QArrayDataPointer` 内部数据结构
  - Short String Optimization（SSO）的触发条件与边界
  - `detach()` 写时复制触发时机的源码路径
  - `QString::fromRawData()` 零拷贝构造的内存安全边界

- [ ] 🔴 `04-containers-cow-expert.md` — Qt 容器隐式共享（COW）源码实现
  - `QSharedData` + `QSharedDataPointer` 引用计数机制
  - `QList<T>` 的 `QListData` 内部数组增长策略
  - `QHash` 开放地址法与再哈希触发条件
  - 容器线程安全：COW 与 `QMutex` 的配合边界

- [ ] 🟡 `05-qvariant-type-erasure-expert.md` — QVariant 类型擦除与 QMetaType 注册源码
  - `QVariant::Private` 联合体存储小对象优化
  - `QMetaType::construct` / `destroy` 函数指针表
  - 自定义类型注册的完整 vtable 填充过程
  - `QVariant::convert()` 类型转换链的查找机制

- [ ] 🔴 `06-memory-model-expert.md` — Qt 引用计数与内存模型源码全解
  - `QAtomicInt` 无锁引用计数在 `QSharedPointer` 中的实现
  - `QWeakPointer` 弱引用计数器（`ExternalRefCountData`）
  - 对象销毁顺序：`QObject::~QObject` 中子对象析构流程
  - `QtGlobalStatic` 全局对象的线程安全初始化

- [ ] 🔴 `07-event-loop-internals-expert.md` — 事件循环源码全解：QEventLoop 与平台抽象
  - `QAbstractEventDispatcher` 平台抽象层（epoll / IOCP / kqueue）
  - `QCoreApplication::processEvents()` 一次迭代做了什么
  - 定时器如何集成进事件循环（timerfd / SetTimer）
  - `QEventLoop::wakeUp()` 跨线程唤醒机制

- [ ] 🔴 `08-file-io-iodevice-expert.md` — QIODevice 抽象层与缓冲机制源码
  - `QIODevicePrivate::buffer` 读写缓冲区管理
  - `QFile` 平台 IO 后端（POSIX `read`/`write` vs Win32 `ReadFile`）
  - `readLine()` 的缓冲扫描实现与性能特征
  - `QBuffer`（内存 IO）的 `QByteArray` 引用机制

- [ ] 🔴 `09-qthread-internals-expert.md` — QThread 源码：平台线程封装与局部存储
  - `QThreadPrivate` 平台实现：`pthread_create` vs `CreateThread`
  - `QThreadStorage<T>`（线程局部存储）实现机制
  - `QThread::currentThread()` 如何在任意上下文定位当前线程对象
  - `moveToThread()` 的连接类型自动升级逻辑

- [ ] 🟡 `10-qprocess-platform-expert.md` — QProcess 源码：平台差异封装层
  - Unix 下 `fork()` + `execve()` 的使用与 `SIGCHLD` 监听
  - Windows 下 `CreateProcess()` 与 IO 重定向管道配置
  - 进程组管理与 `QProcess::terminate()` / `kill()` 的平台差异
  - `QProcessEnvironment` 环境变量的系统获取与合并

- [ ] 🟡 `11-qtimer-dispatch-expert.md` — QTimer 分发机制源码：timerEvent 与精度
  - `QObject::timerEvent()` 分发路径从 `startTimer()` 到回调
  - `Qt::CoarseTimer` / `PreciseTimer` 的精度保证机制
  - 定时器合并优化（Coarse Timer 的 5% 漂移窗口）
  - `QBasicTimer` vs `QTimer` 的开销对比

- [ ] 🟡 `12-plugin-loader-expert.md` — QPluginLoader 源码：QFactoryLoader 机制
  - `QFactoryLoader` 静态插件与动态插件的统一查找
  - 插件元数据 JSON 的嵌入与解析（`Q_PLUGIN_METADATA`）
  - `dlopen` / `LoadLibrary` 跨平台封装
  - 插件版本兼容性检查的二进制层实现

- [ ] 🟡 `13-i18n-translator-expert.md` — QTranslator 源码：消息查找算法
  - `.qm` 文件格式：魔数/偏移表/字符串池二进制布局
  - `QTranslatorPrivate::do_translate()` 哈希查找流程
  - 复数规则（Plural Forms）的运行时求值
  - `QCoreApplication::translate()` 全局翻译链查找顺序

- [ ] 🟡 `14-logging-message-handler-expert.md` — 日志系统源码：消息处理器机制
  - `qt_message_output()` 到 `qInstallMessageHandler` 的分发链
  - `QLoggingCategory` 规则解析与运行时过滤矩阵
  - `QMessageLogger` 上下文信息（文件/行号/函数）的传递
  - Release 构建 `QT_NO_DEBUG_OUTPUT` 宏展开后的零开销

- [ ] 🟡 `15-regex-pcre-expert.md` — QRegularExpression 源码：PCRE2 集成与 JIT
  - `QRegularExpressionPrivate` 持有 `pcre2_code` 编译结果
  - `optimize()` 触发 PCRE2 JIT 编译的内部调用
  - 匹配结果 `QRegularExpressionMatchPrivate` 的偏移数组解析
  - UTF-16 与 PCRE2 UTF 模式的字符偏移转换

- [ ] 🟡 `16-json-parser-expert.md` — QJsonDocument 源码：JSON 解析状态机
  - `QJsonParser` 递归下降解析器实现
  - `QJsonPrivate::Data` 二进制表示格式（非文本存储）
  - 写时复制在 `QJsonObject` / `QJsonArray` 修改时的触发
  - `QCborValue` 与 `QJsonValue` 的相互转换路径

- [ ] 🔴 `17-moc-compiler-expert.md` — 【专家专属】MOC 编译器原理：代码生成全流程
  - MOC 词法/语法分析阶段：识别 `signals` / `slots` / `Q_OBJECT`
  - 生成 `moc_xxx.cpp` 中 `qt_static_metacall` 的逻辑结构
  - `QMetaObject` 静态数据的内存布局（整数表 + 字符串池）
  - 为什么 MOC 是必要的：C++ 反射的当前局限性

- [ ] 🔴 `18-signal-slot-deep-dive-expert.md` — 【专家专属】信号槽实现深度拆解（续）
  - `QObjectPrivate::Connection` / `ConnectionList` 数据结构
  - 信号重入（signal re-entrancy）的处理机制
  - `QObject::blockSignals()` 的实现与 `signalsBlocked()` 原子性
  - 性能测量：信号槽 vs 虚函数 vs std::function 的基准对比

- [ ] 🔴 `19-cow-implicit-sharing-expert.md` — 【专家专属】Qt 隐式共享 COW 全解（续）
  - `QExplicitlySharedDataPointer` vs `QSharedDataPointer` 区别
  - 自定义隐式共享类的完整实现模板
  - COW 在多线程下的安全边界（读安全 / 写不安全）
  - Qt 内置共享类列表与使用建议

- [ ] 🔴 `20-event-loop-deep-dive-expert.md` — 【专家专属】事件循环深度源码全解（续）
  - `QSocketNotifier` 如何将 socket 事件集成进事件循环
  - `QAbstractEventDispatcher::processEvents()` 的 flags 语义
  - 嵌套事件循环（`QDialog::exec()` 内部）的重入安全
  - 事件循环空转检测与 CPU 占用分析工具

</details>

<details>
<summary><strong>02 · QtGui（专家）</strong>（6 篇）</summary>

- [ ] 🔴 `01-qpainter-backend-expert.md` — QPainter 源码：渲染后端与 QPaintEngine
  - `QPaintEngine` 抽象层：Raster / OpenGL / PDF / Print 后端
  - `QPainterPrivate::updateState()` 状态机与 dirty flag 优化
  - `QRasterPaintEngine` 软件光栅化实现（扫描线填充算法）
  - Qt Quick 的 Scene Graph 渲染路径与 QPainter 的关系

- [ ] 🟡 `02-qtransform-matrix-expert.md` — QTransform 矩阵运算源码
  - `QTransform` 内部 3x3 矩阵的类型分级（单位/平移/仿射/投影）
  - 矩阵乘法优化：根据类型跳过通用路径
  - `QTransform::map()` 对不同几何类型的分发
  - `QTransform::squaredNorm()` 在碰撞检测中的应用

- [ ] 🔴 `03-qimage-format-expert.md` — QImage 源码：像素格式与内存布局
  - `QImage::Format` 枚举的底层字节排列（ARGB32 vs RGBA8888）
  - `QImageData` 引用计数与 `detach()` 触发条件
  - `QImage::convertTo()` 格式转换的 lookup table 优化
  - 高 DPI 设备像素比（`devicePixelRatio`）在 QImage 中的处理

- [ ] 🟡 `04-font-engine-expert.md` — Qt 字体引擎源码：HarfBuzz 与平台字体后端
  - `QFontEngine` 抽象层与平台实现（FreeType / DirectWrite / CoreText）
  - HarfBuzz 文本整形（Shaping）与 OpenType 特性
  - `QFontCache` 字体缓存机制
  - 字体回退（Font Fallback）链查找算法

- [ ] 🟡 `05-opengl-context-expert.md` — Qt OpenGL 上下文管理源码
  - `QPA::QPlatformOpenGLContext` 平台抽象层
  - `QOpenGLContext::makeCurrent()` 上下文切换的线程约束
  - Qt 的 OpenGL 资源管理：`QOpenGLSharedResourceGuard`
  - `QSurface` 类型（Window / Offscreen / OpenGLWindow）

- [ ] 🟡 `06-drag-drop-platform-expert.md` — 拖放系统源码：平台 DnD 协议封装
  - `QDragManager` 全局拖放状态机
  - X11 XDND 协议与 Windows OLE IDropSource/IDropTarget
  - `QMimeData` 延迟数据提供（`retrieveData()` 懒求值）
  - `Qt::DropAction` 到平台 DnD effect 的映射

</details>

<details>
<summary><strong>03 · QtWidgets（专家，主题能力篇）</strong>（10 篇）</summary>

- [ ] 🔴 `01-layout-algorithm-expert.md` — 布局系统源码：尺寸分配算法
  - `QLayoutItem::expandingDirections()` 驱动空间分配决策
  - `QBoxLayout::setGeometry()` 中的空间分配循环
  - Stretch Factor 参与分配的权重计算数学
  - `QLayout::activate()` 触发时机与递归更新

- [ ] 🔴 `02-event-dispatch-expert.md` — 事件分发源码：QApplication::notify 全流程
  - `QApplication::notify()` 重写点与过滤器链执行顺序
  - 鼠标事件坐标转换：屏幕 → 窗口 → 控件本地坐标
  - 焦点系统：`QFocusEvent` 与 Tab 键焦点链遍历
  - `QShortcut` 快捷键事件的拦截优先级

- [ ] 🔴 `03-model-view-internals-expert.md` — Model/View 源码：视图与模型解耦机制
  - `QAbstractItemModelPrivate::notifyIndexObservers()` 变更通知传播
  - `QItemSelectionModel` 选择状态的高效存储（区间合并）
  - `QAbstractItemView::doItemsLayout()` 视图布局计算
  - `QPersistentModelIndex` 的索引追踪重映射机制

- [ ] 🔴 `04-qss-parser-expert.md` — QSS 源码：样式规则解析与应用
  - `QCss::Parser` 词法/语法分析生成样式规则集
  - `QStyleSheetStyle` 继承 `QWindowsStyle` 的叠加逻辑
  - 样式规则匹配：选择器特异性（Specificity）计算
  - `QStyleSheetStyle::polish()` 将规则应用于控件的流程

- [ ] 🔴 `05-widget-rendering-expert.md` — 控件渲染源码：脏矩形与 Backing Store
  - `QWidgetPrivate::drawWidget()` 绘制调用链
  - `QBackingStore` 与 `QPlatformBackingStore` 后端
  - 脏矩形（dirty region）的合并与最小重绘优化
  - `WA_OpaquePaintEvent` / `WA_NoSystemBackground` 属性影响

- [ ] 🔴 `06-dialog-event-loop-expert.md` — 对话框源码：模态事件循环实现
  - `QDialog::exec()` 内部的 `QEventLoop::exec()` 嵌套
  - 模态 Widget 对其他窗口事件的屏蔽机制
  - `QApplication::setActiveWindow()` 与模态窗口焦点管理
  - 嵌套模态对话框的安全调用约定

- [ ] 🔴 `07-main-window-layout-expert.md` — 主窗口源码：Dock/ToolBar 布局引擎
  - `QMainWindowLayout` 自定义布局引擎（非标准 QLayout）
  - Dock 区域的 BSP 树布局结构
  - `QDockWidget` 浮动时的顶层窗口转换机制
  - 工具栏溢出菜单的动态生成

- [ ] 🟡 `08-graphics-view-bsp-expert.md` — 图形视图源码：BSP 树与碰撞检测算法
  - `QGraphicsSceneBspTree` 二叉空间分割实现
  - `QGraphicsScene::collidingItems()` 的 BVH 近似碰撞查询
  - `QGraphicsItem::shape()` 精确碰撞路径 vs `boundingRect()` 近似
  - 大量 Item 场景的索引策略选择（BspTreeIndex vs NoIndex）

- [ ] 🟡 `09-animation-timer-expert.md` — 动画框架源码：时间轴驱动机制
  - `QAnimationTimer` 全局动画帧调度（16ms 帧率控制）
  - `QAbstractAnimationPrivate::setState()` 状态转换触发
  - `QVariantAnimation::updateCurrentValue()` 插值计算路径
  - 动画暂停时的时间补偿机制

- [ ] ⚪ `10-mdi-subwindow-expert.md` — MDI 源码：子窗口管理实现
  - `QMdiAreaPrivate` 子窗口 Z-order 栈管理
  - `QMdiSubWindowPrivate` 拖动/调整大小的鼠标区域判定
  - 最大化子窗口时菜单栏合并的实现细节
  - `QMdiArea::TabbedView` 模式的标签页集成

</details>

<details>
<summary><strong>03 · QtWidgets（专家，控件速查篇）</strong>（共 64 篇，与入门进阶对应）</summary>

> 专家控件速查篇深入每个控件的源码实现，包含：`QWidgetPrivate` 内部状态 / `paintEvent` 绘制实现 / 平台风格适配（`QStyle::drawControl`）/ 性能关键路径。文件命名：将 `-beginner` 替换为 `-expert`。重点标注如下：

- [ ] 🔴 `11-qwidget-base-expert.md` — QWidget 源码：窗口系统集成与 WA_* 属性实现
- [ ] 🔴 `12-qabstractbutton-base-expert.md` — QAbstractButton 源码：状态机与动画时序
- [ ] 🟡 `13-qframe-base-expert.md` — QFrame 源码：边框绘制的 QStyle 委托
- [ ] 🟡 `14-qabstractscrollarea-base-expert.md` — QAbstractScrollArea 源码：视口与滚动条同步
- [ ] 🔴 `15-qabstractitemview-base-expert.md` — QAbstractItemView 源码：布局计算与绘制优化
- [ ] 🟡 `16-qabstractspinbox-base-expert.md` — QAbstractSpinBox 源码：输入验证状态机
- [ ] 🔴 `17-qpushbutton-expert.md` — QPushButton 源码：Default/AutoDefault 键盘处理
- [ ] 🔴 `18-qtoolbutton-expert.md` — QToolButton 源码：菜单弹出定时器与动作绑定
- [ ] 🔴 `19-qradiobutton-expert.md` — QRadioButton 源码：互斥组自动查找算法
- [ ] 🔴 `20-qcheckbox-expert.md` — QCheckBox 源码：三态 checkState 存储与绘制
- [ ] ⚪ `21-qcommandlinkbutton-expert.md` — QCommandLinkButton 源码：平台样式适配
- [ ] 🔴 `22-qlineedit-expert.md` — QLineEdit 源码：文本布局引擎与光标绘制
- [ ] 🔴 `23-qtextedit-expert.md` — QTextEdit 源码：QTextDocument 与视图同步
- [ ] 🟡 `24-qplaintextedit-expert.md` — QPlainTextEdit 源码：块级布局与行数限制
- [ ] 🟡 `25-qtextbrowser-expert.md` — QTextBrowser 源码：资源加载与历史栈
- [ ] ⚪ `26-qkeysequenceedit-expert.md` — QKeySequenceEdit 源码：按键事件捕获机制
- [ ] 🔴 `27-qcombobox-expert.md` — QComboBox 源码：弹出窗口定位与 Model 管理
- [ ] 🟡 `28-qfontcombobox-expert.md` — QFontComboBox 源码：字体枚举与预览 Delegate
- [ ] 🔴 `29-qspinbox-expert.md` — QSpinBox 源码：输入验证与步进动画
- [ ] 🟡 `30-qdatetimeedit-expert.md` — QDateTimeEdit 源码：Section 编辑状态机
- [ ] 🔴 `31-qslider-expert.md` — QSlider 源码：鼠标拖动映射与刻度绘制
- [ ] 🟡 `32-qscrollbar-expert.md` — QScrollBar 源码：滑块尺寸计算公式
- [ ] 🟡 `33-qdial-expert.md` — QDial 源码：角度到值的非线性映射
- [ ] 🔴 `34-qlabel-expert.md` — QLabel 源码：富文本懒解析与 Buddy 快捷键注册
- [ ] 🔴 `35-qprogressbar-expert.md` — QProgressBar 源码：无限动画定时器集成
- [ ] 🟡 `36-qlcdnumber-expert.md` — QLCDNumber 源码：七段显示字形映射表
- [ ] 🟡 `37-qcalendarwidget-expert.md` — QCalendarWidget 源码：日期格式化与单元格绘制
- [ ] 🔴 `38-qgroupbox-expert.md` — QGroupBox 源码：标题绘制与 Checkable 子树禁用
- [ ] 🔴 `39-qtabwidget-expert.md` — QTabWidget 源码：QTabBar 与 QStackedWidget 协调
- [ ] 🟡 `40-qtabbar-expert.md` — QTabBar 源码：标签拖拽重排的鼠标事件处理
- [ ] 🔴 `41-qstackedwidget-expert.md` — QStackedWidget 源码：控件显隐与尺寸策略
- [ ] 🔴 `42-qsplitter-expert.md` — QSplitter 源码：分割比例存储与恢复
- [ ] 🟡 `43-qtoolbox-expert.md` — QToolBox 源码：按钮控件动态创建与布局
- [ ] 🔴 `44-qscrollarea-expert.md` — QScrollArea 源码：子 Widget 尺寸追踪
- [ ] 🔴 `45-qframe-expert.md` — QFrame 源码：QStyle::drawPrimitive 边框委托
- [ ] 🔴 `46-qlistwidget-expert.md` — QListWidget 源码：QListView + 内置 Model 封装层
- [ ] 🔴 `47-qlistview-expert.md` — QListView 源码：图标模式布局引擎
- [ ] 🔴 `48-qtreewidget-expert.md` — QTreeWidget 源码：内置 Model 的 checkState 传播
- [ ] 🔴 `49-qtreeview-expert.md` — QTreeView 源码：展开动画与节点延迟绘制
- [ ] 🔴 `50-qtablewidget-expert.md` — QTableWidget 源码：单元格 span 存储结构
- [ ] 🔴 `51-qtableview-expert.md` — QTableView 源码：虚拟滚动行列可见区域计算
- [ ] 🟡 `52-qheaderview-expert.md` — QHeaderView 源码：Section 尺寸的持久化存储
- [ ] ⚪ `53-qcolumnview-expert.md` — QColumnView 源码：列宽动画与根索引管理
- [ ] ⚪ `54-qundoview-expert.md` — QUndoView 源码：QUndoStack 信号与 Model 同步
- [ ] 🔴 `55-qmainwindow-expert.md` — QMainWindow 源码：QMainWindowLayout 自定义布局引擎
- [ ] 🔴 `56-qmenubar-expert.md` — QMenuBar 源码：平台原生菜单栏（macOS）集成
- [ ] 🔴 `57-qtoolbar-expert.md` — QToolBar 源码：溢出菜单动态生成算法
- [ ] 🔴 `58-qstatusbar-expert.md` — QStatusBar 源码：永久控件与临时消息的布局协调
- [ ] 🔴 `59-qdockwidget-expert.md` — QDockWidget 源码：浮动窗口创建与 re-dock 检测
- [ ] 🔴 `60-qdialog-expert.md` — QDialog 源码：exec() 嵌套事件循环的安全退出
- [ ] 🔴 `61-qdialogbuttonbox-expert.md` — QDialogButtonBox 源码：平台按钮顺序自动重排
- [ ] 🔴 `62-qmessagebox-expert.md` — QMessageBox 源码：图标资源懒加载与平台声音
- [ ] 🟡 `63-qinputdialog-expert.md` — QInputDialog 源码：动态控件切换（文字/数字/列表）
- [ ] 🟡 `64-qcolordialog-expert.md` — QColorDialog 源码：HSV 色轮与调色板存储
- [ ] 🟡 `65-qfontdialog-expert.md` — QFontDialog 源码：字体家族/样式/大小三级筛选
- [ ] 🔴 `66-qfiledialog-expert.md` — QFileDialog 源码：平台原生对话框与 Qt 实现的切换
- [ ] 🟡 `67-qprogressdialog-expert.md` — QProgressDialog 源码：最小显示时间保证算法
- [ ] ⚪ `68-qerrormessage-expert.md` — QErrorMessage 源码：消息抑制集合的持久化
- [ ] 🟡 `69-qwizard-expert.md` — QWizard 源码：页面栈管理与非线性跳转
- [ ] 🟡 `70-qsplashscreen-expert.md` — QSplashScreen 源码：无边框透明窗口实现
- [ ] 🟡 `71-qmdiarea-expert.md` — QMdiArea 源码：标签模式下的子窗口代理
- [ ] 🟡 `72-qprinter-expert.md` — QPrinter 源码：PDF 生成后端与页面坐标变换
- [ ] 🟡 `73-qprintdialog-expert.md` — QPrintDialog 源码：平台原生打印对话框封装
- [ ] ⚪ `74-qprintpreviewdialog-expert.md` — QPrintPreviewDialog 源码：预览页面缓存机制

</details>

<details>
<summary><strong>04 · QtNetwork（专家）</strong>（6 篇）</summary>

- [ ] 🔴 `01-tcp-socket-expert.md` — QAbstractSocket 源码：状态机实现
  - `QAbstractSocketPrivate` 十一态状态机转换图
  - 非阻塞 connect 的跨平台实现（SO_ERROR 检测）
  - `QSocketNotifier` 在 TCP 中驱动异步读写
  - `QAbstractSocket::waitForConnected()` 内部事件循环

- [ ] 🟡 `02-udp-socket-expert.md` — QUdpSocket 源码：数据报平台封装
  - `recvfrom` / `sendto` 系统调用的封装细节
  - 多播 `IP_ADD_MEMBERSHIP` setsockopt 平台差异
  - `QNetworkDatagram` 携带 TTL / HopLimit 的传递
  - UDP 套接字缓冲区满时的 `QAbstractSocket::SocketError` 处理

- [ ] 🔴 `03-network-access-expert.md` — QNetworkAccessManager 源码：请求队列与后端
  - `QNetworkAccessBackend` 可插拔后端设计
  - HTTP/2 多路复用在 `QNetworkAccessManager` 中的实现
  - 请求优先级队列与最大并发连接数控制
  - `QNetworkDiskCache` 磁盘缓存的 LRU 淘汰算法

- [ ] 🟡 `04-websocket-frame-expert.md` — QWebSocket 源码：帧解析与掩码处理
  - RFC 6455 帧结构：FIN/RSV/Opcode/Mask/Payload 字段解析
  - 客户端掩码随机数生成与 XOR 处理
  - 分片帧（Continuation Frame）的重组缓冲
  - 控制帧（Ping/Pong/Close）的优先处理

- [ ] 🟡 `05-ssl-openssl-expert.md` — Qt SSL 源码：OpenSSL / Schannel 抽象层
  - `QSslSocketBackendPrivate` 平台后端选择（OpenSSL / SecureTransport / Schannel）
  - TLS 握手过程在 Qt 异步 IO 模型中的状态管理
  - 证书链验证：`QSslCertificate::verify()` 调用 X.509 验证
  - ALPN 协议协商（HTTP/2 升级）的实现

- [ ] 🔴 `06-serialport-platform-expert.md` — QSerialPort 源码：POSIX/Win32 封装层
  - Unix 下 `termios` 结构体波特率/帧格式配置
  - Windows 下 `DCB` 结构体与 `SetCommState`
  - 异步 IO：`QWinOverlappedIoNotifier` vs `QSocketNotifier`
  - 硬件流控（RTS/CTS）的跨平台实现

</details>

<details>
<summary><strong>05 · 其他模块（专家）</strong>（25 篇）</summary>

- [ ] 🔴 `01-qtsql-driver-expert.md` — QtSql 源码：数据库驱动插件机制
- [ ] 🟡 `02-qtsql-model-expert.md` — QSqlQueryModel 源码：懒加载与行缓存
- [ ] 🔴 `03-qtcharts-render-expert.md` — QtCharts 源码：渲染流水线与坐标映射
- [ ] 🔴 `04-qtmultimedia-backend-expert.md` — QtMultimedia 源码：平台媒体后端抽象（GStreamer/AVFoundation/MFT）
- [ ] 🟡 `05-qtmultimedia-camera-expert.md` — 摄像头源码：视频帧捕获管道
- [ ] 🟡 `06-qtsvg-render-expert.md` — QtSvg 源码：SVG 解析器与 QPainter 渲染
- [ ] 🟡 `07-qtprintsupport-expert.md` — QtPrintSupport 源码：PDF 生成引擎
- [ ] 🔴 `08-modbus-protocol-expert.md` — Qt Serial Bus 源码：Modbus PDU 协议栈实现
- [ ] 🟡 `09-mqtt-protocol-expert.md` — Qt MQTT 源码：MQTT 协议帧编解码
- [ ] 🟡 `10-qtbluetooth-stack-expert.md` — QtBluetooth 源码：平台 BLE 栈封装（BlueZ/WinRT）
- [ ] ⚪ `11-qtnfc-platform-expert.md` — QtNFC 源码：NDEF 消息序列化与平台 API
- [ ] 🟡 `12-statemachine-internals-expert.md` — Qt StateMachine 源码：事件驱动执行引擎
- [ ] ⚪ `13-qtscxml-interpreter-expert.md` — Qt SCXML 源码：W3C SCXML 语义解释器
- [ ] 🟡 `14-qt3d-ecs-expert.md` — Qt3D 源码：ECS 架构与 Framegraph 调度
- [ ] 🟡 `15-qtquick3d-render-expert.md` — QtQuick3D 源码：与 Qt Quick SceneGraph 的集成
- [ ] ⚪ `16-qtquick3d-physics-expert.md` — QtQuick3D Physics 源码：PhysX SDK 封装
- [ ] 🟡 `17-qtpdf-pdfium-expert.md` — QtPdf 源码：PDFium 集成与渲染管道
- [ ] 🟡 `18-qthttpserver-routing-expert.md` — QtHttpServer 源码：路由匹配算法与请求解析
- [ ] 🟡 `19-qtwebsockets-server-expert.md` — QtWebSockets 服务端源码：握手升级流程
- [ ] ⚪ `20-qtwebchannel-transport-expert.md` — QtWebChannel 源码：可插拔传输层设计
- [ ] ⚪ `21-qtwebengine-chromium-expert.md` — QtWebEngine 源码：Chromium Content API 集成
- [ ] ⚪ `22-qtremoteobjects-repc-expert.md` — Qt Remote Objects 源码：REPC 编译器与通信协议
- [ ] ⚪ `23-qtspatial-audio-expert.md` — Qt Spatial Audio 源码：HRTF 音频渲染管道
- [ ] ⚪ `24-qttexttospeech-engine-expert.md` — Qt TTS 源码：引擎接口与异步合成流水线
- [ ] 🟡 `25-qt5compat-internals-expert.md` — Qt5Compat 源码：兼容 shim 实现与废弃 API 桥接

</details>

<details>
<summary><strong>06 · QML 独立教程（专家）</strong>（9 篇，含专属章节）</summary>

- [ ] 🔴 `01-qml-binding-engine-expert.md` — QML 绑定引擎源码：表达式求值与依赖追踪
  - `QQmlBinding` 表达式编译为字节码的过程
  - 依赖追踪：`QQmlNotifier` 观察者链
  - 绑定更新的延迟批处理（Deferred Evaluation）
  - `QQmlPropertyData` 属性元数据的存储结构

- [ ] 🔴 `02-qtquick-controls-style-expert.md` — Qt Quick Controls 源码：样式与模板实现
  - `QQuickStylePlugin` 样式插件加载机制
  - `Control::background` / `contentItem` 的 delegate 创建路径
  - `Theme` / `Palette` 的继承与覆盖规则
  - 自定义 Control 时正确继承 `T.Control` 的注意事项

- [ ] 🔴 `03-cpp-qml-type-system-expert.md` — C++/QML 类型系统源码：注册与元对象桥接
  - `QQmlTypeRegistry` 类型注册表的查找结构
  - `QML_ELEMENT` 宏展开后的 `qmlRegisterType` 等价代码
  - `QQmlContext` 上下文属性的查找链
  - `QQmlEngine::createQmlObject()` 动态实例化的执行路径

- [ ] 🔴 `04-qml-animation-engine-expert.md` — QML 动画引擎源码：时间轴与渲染线程
  - `QQuickAnimator` 在 Qt Quick 渲染线程执行的机制
  - `QUnifiedTimer` 全局动画时钟与帧同步
  - `QQuickTransition` 状态切换动画的触发与完成检测
  - `SmoothedAnimation` 弹簧-阻尼系统的数值积分

- [ ] 🔴 `05-qml-component-compiler-expert.md` — QML 自定义组件源码：编译单元与类型解析
  - `.qml` 文件编译为 `QQmlComponent` 的编译单元
  - 组件作用域（Component Scope）与文档作用域的隔离
  - `qmldir` 文件的解析与模块注册
  - Qt Quick Compiler（AOT 编译）原理与限制

- [ ] 🔴 `06-qml-model-view-expert.md` — QML 模型视图源码：虚拟化与回收池
  - `QQuickListView` 的 delegate 创建/回收池（`cacheBuffer`）
  - `QQmlDelegateModel` 分组与过滤的内部实现
  - `QQuickItemView::layout()` 可见区域增量计算
  - 大数据列表内存占用分析与 Profiler 使用

- [ ] 🟡 `07-workerscript-thread-expert.md` — WorkerScript 源码：独立 JS 引擎与通信
  - `QQuickWorkerScript` 持有独立 `QQmlEngine` 实例
  - `sendMessage` 跨线程序列化：`QVariant` 深拷贝约束
  - WorkerScript 与主线程 QML 的内存隔离边界
  - 错误传播与 `onError` 信号机制

- [ ] 🔴 `08-v4-engine-expert.md` — 【专家专属】V4 JavaScript 引擎原理：JIT 与 GC
  - V4 引擎架构：解释器 / Baseline JIT / Optimizing JIT 三层
  - `Heap::Base` 标记清除垃圾回收的触发条件
  - `Value` 类型：NaN boxing 的 64 位值表示
  - QML 对象与 V4 堆对象的双向引用管理

- [ ] 🔴 `09-qqmlengine-type-bridge-expert.md` — 【专家专属】QQmlEngine 类型系统与元对象桥接全解
  - `QQmlData`：附着在每个 `QObject` 上的 QML 扩展数据
  - C++ `QObject` 属性在 V4 引擎中的 accessor 函数生成
  - `QMetaObject` 与 `QQmlTypeData` 的双向查找
  - 信号跨 QML/C++ 边界触发时的参数类型转换

</details>

---

## 📊 快速状态查询

| 层级 | 总篇数 | 已完成 | 进行中 | 未开始 |
|------|--------|--------|--------|--------|
| 入门 | 118 | 72 | 0 | 46 |
| 进阶 | 137 | 0 | 0 | 137 |
| 专家 | 142 | 0 | 0 | 142 |
| **合计** | **397** | **72** | **0** | **325** |

---

*TODO 版本：v2.0（百科全书版）· 生成于 2026-03-17*
*每次完成一篇请立即更新对应条目（`[ ]` → `[x] 完成于 YYYY-MM-DD`）并更新上方统计表*