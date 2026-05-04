> 以下入门层文章已全部完成，归档于 (2025-04-22)

## 00 · 环境搭建（3 篇）

全部完成

---

## 01 · QtBase（16 篇）

全部完成

---

## 02 · QtGui（6 篇）

- [x] 🔴 `01-qpainter-basic-beginner.md` — QPainter 绘图基础 (2025-04-22)
  - `paintEvent` 中获取 `QPainter` 的正确姿势
  - `QPen`（线条）/ `QBrush`（填充）/ `QColor` 基础设置
  - 绘制图形：直线、矩形、椭圆、多边形、文字
  - 坐标系原点与单位（逻辑坐标 vs 物理像素）

- [x] 🟡 `02-coordinate-transform-beginner.md` — 坐标系与 QTransform 变换基础 (2025-04-22)
  - `QPainter::translate` / `rotate` / `scale` 基础变换
  - `save()` / `restore()` 保存恢复画笔状态
  - 视口（viewport）与窗口（window）坐标映射
  - 局部坐标系与全局坐标系转换

- [x] 🔴 `03-image-pixmap-beginner.md` — QImage、QPixmap、QIcon 图像处理基础 (2025-04-22)
  - `QPixmap` vs `QImage`：显示用 vs 像素操作用的本质区别
  - 从文件/资源加载图像，`QRC` 资源系统基础
  - `QPixmap::scaled()` 缩放保持比例
  - `QIcon` 多尺寸图标与状态图标设置

- [x] 🟡 `04-font-text-rendering-beginner.md` — QFont 与文本渲染基础 (2025-04-22)
  - `QFont` 构造与属性设置（族名、大小、加粗、斜体）
  - `QPainter::drawText()` 绘制文字与对齐方式
  - `QFontMetrics` 计算文字尺寸（用于自定义布局）
  - 富文本渲染：`QTextDocument` 基础

- [x] 🟡 `05-opengl-widget-beginner.md` — QOpenGLWidget 嵌入 OpenGL 基础 (2025-04-22)
  - `QOpenGLWidget` 三个必须重写的函数：`initializeGL` / `resizeGL` / `paintGL`
  - `QOpenGLFunctions` 平台无关 OpenGL 调用方式
  - 绑定 VAO / VBO 绘制简单三角形
  - OpenGL 上下文生命周期与资源释放

- [x] 🟡 `06-drag-drop-beginner.md` — 拖放系统基础 (2025-04-22)
  - 在 Widget 上启用拖放：`setAcceptDrops(true)`
  - `dragEnterEvent` / `dropEvent` 处理拖放接收
  - `QDrag` 发起拖放操作
  - `QMimeData` 携带拖放数据（文本、文件路径、自定义）

---

## 03 · QtWidgets — 主题能力篇（10 篇）

- [x] 🔴 `01-layout-system-beginner.md` — 布局系统基础（五大布局管理器） (2025-04-22)
  - `QHBoxLayout` / `QVBoxLayout` 水平/垂直布局基础
  - `QGridLayout` 网格布局与行列 span
  - `QFormLayout` 表单布局（标签+输入对）
  - `QStackedLayout` 页面切换布局
  - `addStretch()` / `setSpacing()` / `setContentsMargins()` 调整间距

- [x] 🔴 `02-event-handling-beginner.md` — 事件处理与传播基础 (2025-04-22)
  - 重写 `mousePressEvent` / `keyPressEvent` / `resizeEvent`
  - `event->accept()` vs `event->ignore()` 控制传播链
  - `installEventFilter()` 拦截子控件事件
  - `QApplication::sendEvent()` vs `postEvent()` 区别

- [x] 🔴 `03-model-view-beginner.md` — Model/View 架构入门 (2025-04-22)
  - MVC/MVP 思想与 Qt Model/View 的对应关系
  - `QStringListModel` + `QListView` 最简示例
  - `QStandardItemModel` + `QTableView` 快速上手
  - `setData()` / `data()` / `rowCount()` 三个核心接口

- [x] 🔴 `04-qss-stylesheet-beginner.md` — 样式表 QSS 基础 (2025-04-22)
  - QSS 语法与 CSS 的异同
  - 类选择器 / ID 选择器 / 后代选择器
  - 常用伪状态：`:hover` / `:pressed` / `:checked` / `:disabled`
  - 从文件加载 QSS 与动态主题切换基础

- [x] 🔴 `05-custom-widget-paint-beginner.md` — 自定义绘制 Widget 基础 (2025-04-22)
  - `paintEvent` 完整流程与触发时机
  - `update()` vs `repaint()` 的区别与选用
  - 双缓冲绘制防闪烁原理
  - `sizeHint()` / `minimumSizeHint()` 告知布局系统尺寸

- [x] 🔴 `06-dialog-system-beginner.md` — 对话框体系基础 (2025-04-22)
  - `QDialog` 模态（`exec()`）与非模态（`show()`）的区别
  - `QDialogButtonBox` 标准按钮配置
  - 自定义对话框布局与数据返回
  - `QDialog::accept()` / `reject()` 关闭对话框

- [x] 🔴 `07-main-window-system-beginner.md` — QMainWindow 主窗口体系基础 (2025-04-22)
  - `QMainWindow` 区域划分：菜单栏/工具栏/状态栏/中央控件/Dock
  - `QMenuBar` / `QMenu` / `QAction` 菜单系统构建
  - `QToolBar` 工具栏添加按钮与分隔线
  - `QDockWidget` 可停靠面板基础

- [x] 🟡 `08-graphics-view-beginner.md` — 图形视图框架基础 (2025-04-22)
  - `QGraphicsScene` / `QGraphicsView` / `QGraphicsItem` 三角关系
  - 添加标准图元：矩形、椭圆、文字、像素图
  - 场景坐标 vs 视图坐标 vs 物品坐标的转换
  - 鼠标事件在 Scene 层拦截与响应

- [x] 🟡 `09-animation-framework-beginner.md` — 属性动画框架基础 (2025-04-22)
  - `QPropertyAnimation` 对任意 `Q_PROPERTY` 属性做动画
  - `setStartValue` / `setEndValue` / `setDuration` 基础配置
  - `QEasingCurve` 缓动函数效果对比
  - `QSequentialAnimationGroup` 串行动画组合

- [x] ⚪ `10-mdi-beginner.md` — QMdiArea 多文档界面基础 (2025-04-22)
  - `QMdiArea` 子窗口创建与管理
  - `QMdiSubWindow` 标题、图标与关闭行为
  - 子窗口排列模式（级联 / 平铺）
  - 激活子窗口与信号监听

---

## 03 · QtWidgets — 控件速查篇（抽象基类）（6 篇）

- [x] 🔴 `11-qwidget-base-beginner.md` — QWidget 基类：所有控件的根 (2025-04-22)
  - 窗口属性：`resize` / `move` / `setWindowTitle` / `setWindowIcon`
  - `show()` / `hide()` / `setVisible()` / `raise()` / `lower()`
  - 尺寸策略：`setSizePolicy` / `setFixedSize` / `setMinimumSize`
  - 窗口标志 `Qt::WindowFlags`（无边框、置顶、工具窗口）

- [x] 🔴 `12-qabstractbutton-base-beginner.md` — QAbstractButton：按钮基类机制 (2025-04-22)
  - `setCheckable` / `setChecked` / `setAutoRepeat` 核心属性
  - `clicked` / `toggled` / `pressed` / `released` 四个信号
  - `QButtonGroup` 实现单选互斥
  - 自定义按钮继承 `QAbstractButton` 必须重写的方法

- [x] 🟡 `13-qframe-base-beginner.md` — QFrame：可视框架基类 (2025-04-22)
  - `QFrame::Shape`（无框 / 盒形 / 面板 / 水平线 / 垂直线）
  - `QFrame::Shadow`（凸起 / 凹入 / 平面）
  - 作为分隔线的 `QFrame::HLine` / `VLine` 用法
  - `lineWidth` / `midLineWidth` 边框宽度控制

- [x] 🟡 `14-qabstractscrollarea-base-beginner.md` — QAbstractScrollArea：滚动区域基类 (2025-04-22)
  - `horizontalScrollBar()` / `verticalScrollBar()` 滚动条控制
  - `setHorizontalScrollBarPolicy` / `setVerticalScrollBarPolicy`
  - `scrollContentsBy()` 内容滚动响应
  - 视口（viewport）概念与绘制注意事项

- [x] 🔴 `15-qabstractitemview-base-beginner.md` — QAbstractItemView：视图基类 (2025-04-22)
  - `setModel()` / `setSelectionModel()` 绑定模型与选择模型
  - 选择模式：`SingleSelection` / `MultiSelection` / `ExtendedSelection`
  - `currentIndex()` / `selectedIndexes()` 获取选中项
  - `setItemDelegate()` 设置自定义委托

- [x] 🟡 `16-qabstractspinbox-base-beginner.md` — QAbstractSpinBox：数字输入基类 (2025-04-22)
  - `setReadOnly` / `setButtonSymbols` 控件外观
  - `editingFinished` 信号与输入验证
  - `stepBy()` 步进值控制
  - `validate()` 输入合法性检验机制

---

## 03 · QtWidgets — 控件速查篇（按钮类）（5 篇）

- [x] 🔴 `17-qpushbutton-beginner.md` — QPushButton：最常用的按钮 (2025-04-22)
  - `setDefault` / `setAutoDefault` 与对话框回车键关联
  - 按钮带菜单：`setMenu(QMenu*)` 下拉按钮
  - 图标按钮：`setIcon` + `setIconSize`
  - 扁平按钮 `setFlat(true)` 与 QSS 美化

- [x] 🔴 `18-qtoolbutton-beginner.md` — QToolButton：工具栏专用按钮 (2025-04-22)
  - `setToolButtonStyle`（图标/文字/两者）
  - `setPopupMode`（菜单显示模式：延迟/即时/只有箭头）
  - 与 `QAction` 关联：`setDefaultAction()`
  - 在工具栏中自动调整样式的机制

- [x] 🔴 `19-qradiobutton-beginner.md` — QRadioButton：单选按钮 (2025-04-22)
  - 自动互斥：同一 parent 下单选按钮天然互斥
  - `QButtonGroup` 跨 parent 实现互斥分组
  - `toggled(bool)` 信号监听状态变化
  - 自定义样式 QSS 圆形按钮美化

- [x] 🔴 `20-qcheckbox-beginner.md` — QCheckBox：复选框 (2025-04-22)
  - 三态复选框：`setTristate(true)` 与 `Qt::PartiallyChecked`
  - `checkStateChanged(Qt::CheckState)` vs `toggled(bool)` 信号区别
  - 复选框组实现"全选/全不选"逻辑
  - 与 `QTreeWidget` 结合的层级复选

- [x] ⚪ `21-qcommandlinkbutton-beginner.md` — QCommandLinkButton：命令链接按钮 (2025-04-22)
  - Windows Vista 风格命令链接按钮外观
  - `setDescription()` 设置副标题描述文字
  - 适用场景：向导对话框中的功能选项
  - 跨平台外观差异处理

---

## 03 · QtWidgets — 控件速查篇（输入类）（9 篇）

- [x] 🔴 `22-qlineedit-beginner.md` — QLineEdit：单行文本输入 (2025-04-22)
  - `setPlaceholderText` / `setMaxLength` / `setReadOnly`
  - `setEchoMode`（密码框、无回显）
  - `setValidator` + `QIntValidator` / `QRegularExpressionValidator`
  - `textChanged` vs `textEdited` vs `editingFinished` 信号区别

- [x] 🔴 `23-qtextedit-beginner.md` — QTextEdit：富文本多行编辑器 (2025-04-22)
  - 纯文本 vs 富文本模式切换
  - `setHtml` / `toHtml` / `toPlainText` 内容读写
  - 光标操作：`QTextCursor` 插入/选中/格式化
  - `document()->setModified()` 追踪修改状态

- [x] 🟡 `24-qplaintextedit-beginner.md` — QPlainTextEdit：纯文本高性能编辑器 (2025-04-22)
  - 与 `QTextEdit` 的核心区别（无富文本开销，适合大文本/日志）
  - `appendPlainText()` 追加日志的正确用法
  - `setMaximumBlockCount()` 限制行数防内存溢出
  - `highlightCurrentLine()` 实现行高亮效果

- [x] 🟡 `25-qtextbrowser-beginner.md` — QTextBrowser：只读富文本浏览器 (2025-04-22)
  - 显示 HTML / Markdown 格式文档
  - `setSource()` 加载本地 HTML 文件
  - `anchorClicked` 信号处理链接点击
  - 历史导航：`backward()` / `forward()` / `home()`

- [x] ⚪ `26-qkeysequenceedit-beginner.md` — QKeySequenceEdit：快捷键录入控件 (2025-04-22)
  - 用户自定义快捷键的录入场景
  - `keySequenceChanged` 信号获取录入结果
  - `setKeySequence()` 设置默认快捷键
  - 与 `QAction::setShortcut()` 结合的完整热键配置流程

- [x] 🔴 `27-qcombobox-beginner.md` — QComboBox：下拉选择框 (2025-04-22)
  - `addItem` / `addItems` / `insertItem` 添加选项
  - `currentIndex()` / `currentText()` / `currentData()` 获取当前值
  - `setEditable(true)` 可编辑组合框
  - `setModel()` 用自定义 Model 填充选项

- [x] 🟡 `28-qfontcombobox-beginner.md` — QFontComboBox：字体选择下拉框 (2025-04-22)
  - `setFontFilters()` 过滤字体类型（等宽/比例/全部）
  - `currentFont()` 获取选中字体
  - 在文字编辑器中实时预览字体变化
  - 字体名称的本地化显示

- [x] 🔴 `29-qspinbox-doublespinbox-beginner.md` — QSpinBox / QDoubleSpinBox：数字步进框 (2025-04-22)
  - `setRange` / `setSingleStep` / `setPrefix` / `setSuffix`
  - `setValue` / `value()` 取值与设值
  - `QDoubleSpinBox::setDecimals()` 控制小数位
  - `valueChanged(int/double)` 信号响应

- [x] 🟡 `30-qdatetimeedit-dateedit-timeedit-beginner.md` — 日期时间输入三件套 (2025-04-22)
  - `QDateEdit` / `QTimeEdit` / `QDateTimeEdit` 适用场景
  - `setMinimumDate` / `setMaximumDate` 限制范围
  - `setCalendarPopup(true)` 弹出日历选择器
  - `QDate` / `QTime` / `QDateTime` 数据类型互转

---

## 03 · QtWidgets — 控件速查篇（滑动/旋转类）（3 篇）

- [x] 🔴 `31-qslider-beginner.md` — QSlider：滑动条 (2025-04-22)
  - 水平/垂直方向设置 `Qt::Horizontal` / `Qt::Vertical`
  - `setRange` / `setValue` / `setSingleStep` / `setPageStep`
  - `valueChanged` / `sliderMoved` / `sliderReleased` 信号区别
  - QSS 自定义滑块外观（handle / groove）

- [x] 🟡 `32-qscrollbar-beginner.md` — QScrollBar：滚动条 (2025-04-22)
  - 独立使用滚动条驱动自定义控件滚动
  - `setRange` / `setPageStep` / `setSingleStep` 配置
  - `valueChanged` 信号驱动内容区域偏移
  - 与 `QScrollArea` 的关系（通常不需要直接操作）

- [x] 🟡 `33-qdial-beginner.md` — QDial：旋钮控件 (2025-04-22)
  - `setWrapping(true)` 无限旋转 vs 有边界旋转
  - `setNotchesVisible(true)` 显示刻度
  - `valueChanged` 信号与实时反馈
  - 模拟仪表盘 UI 中 QDial 的典型应用

---

## 03 · QtWidgets — 控件速查篇（显示类）（4 篇）

- [x] 🔴 `34-qlabel-beginner.md` — QLabel：文本与图像显示 (2025-04-22)
  - 显示文本、HTML 富文本、图片、GIF 动画
  - `setAlignment` 对齐与 `setWordWrap` 自动换行
  - `setBuddy` 关联快捷键到伙伴控件
  - `linkActivated` 信号处理超链接点击

- [x] 🔴 `35-qprogressbar-beginner.md` — QProgressBar：进度条 (2025-04-22)
  - `setRange(0, 100)` + `setValue(n)` 更新进度
  - 无限进度：`setRange(0, 0)` 滚动动画
  - `setFormat()` 自定义显示文字（`%p%` / `%v` / `%m`）
  - 在工作线程中安全更新进度条（跨线程信号槽）

- [x] 🟡 `36-qlcdnumber-beginner.md` — QLCDNumber：液晶数字显示 (2025-04-22)
  - `display(int/double/QString)` 显示数字
  - `setDigitCount` 位数与 `setSmallDecimalPoint` 小数点
  - `setMode`（十进制/十六进制/八进制/二进制）
  - 仪表盘数字显示的典型使用场景

- [x] 🟡 `37-qcalendarwidget-beginner.md` — QCalendarWidget：日历控件 (2025-04-22)
  - `setSelectedDate` / `selectedDate()` 日期选择
  - `selectionChanged` 信号响应用户选择
  - `setMinimumDate` / `setMaximumDate` 可选范围
  - `setHeaderTextFormat` / `setDateTextFormat` 自定义外观

---

## 03 · QtWidgets — 控件速查篇（容器类）（8 篇）

- [x] 🔴 `38-qgroupbox-beginner.md` — QGroupBox：分组框 (2025-04-22)
  - 带标题边框的控件分组容器
  - `setCheckable(true)` 可勾选分组框（整组启用/禁用）
  - `setAlignment` 标题对齐
  - 嵌套布局在分组框内的正确姿势

- [x] 🔴 `39-qtabwidget-beginner.md` — QTabWidget：标签页控件 (2025-04-22)
  - `addTab` / `insertTab` / `removeTab` 动态管理标签页
  - `setTabPosition`（上/下/左/右）与 `setTabShape`
  - `currentChanged(int)` 信号响应标签切换
  - `setTabIcon` / `setTabToolTip` 标签美化

- [x] 🟡 `40-qtabbar-beginner.md` — QTabBar：独立标签栏 (2025-04-22)
  - `QTabBar` 与 `QTabWidget` 的区别（可独立使用）
  - 自定义标签栏 + 自定义内容区域组合
  - `tabCloseRequested` 信号实现可关闭标签页
  - `setMovable(true)` 可拖动标签排序

- [x] 🔴 `41-qstackedwidget-beginner.md` — QStackedWidget：堆叠页面控件 (2025-04-22)
  - `addWidget` 添加页面 / `setCurrentIndex` 切换页面
  - 与 `QComboBox` / `QListWidget` 组合做导航菜单
  - `currentChanged` 信号响应页面切换
  - 区别于 `QTabWidget`（无标签头，适合自定义导航）

- [x] 🔴 `42-qsplitter-beginner.md` — QSplitter：可拖动分割容器 (2025-04-22)
  - 水平/垂直分割：`Qt::Horizontal` / `Qt::Vertical`
  - `setSizes()` / `sizes()` 设置/获取各区域宽度
  - `setCollapsible(index, false)` 禁止折叠特定区域
  - `saveState()` / `restoreState()` 持久化分割比例

- [x] 🟡 `43-qtoolbox-beginner.md` — QToolBox：工具箱折叠面板 (2025-04-22)
  - `addItem` / `insertItem` 添加面板
  - `currentChanged` 信号响应当前面板切换
  - `setItemEnabled` 禁用某个面板
  - 侧边栏导航的典型应用场景

- [x] 🔴 `44-qscrollarea-beginner.md` — QScrollArea：滚动区域容器 (2025-04-22)
  - `setWidget()` 设置被滚动的内容控件
  - `setWidgetResizable(true)` 内容自适应宽度
  - 动态添加内容后自动滚动到底部
  - 自定义滚动条样式 QSS

- [x] 🔴 `45-qframe-separator-beginner.md` — QFrame 作为分隔线的用法 (2025-04-22)
  - 水平分隔线 `QFrame::HLine` + `QFrame::Sunken`
  - 垂直分隔线在工具栏中的使用
  - `QFrame` 作为有边框容器控件的配置
  - 区别：`QFrame` vs 布局中的 `addSpacing`

---

## 03 · QtWidgets — 控件速查篇（列表/树/表格视图）（9 篇）

- [x] 🔴 `46-qlistwidget-beginner.md` — QListWidget：便捷列表控件 (2025-04-22)
  - `addItem` / `addItems` / `insertItem` 添加条目
  - `currentItem()` / `selectedItems()` 获取选中
  - `QListWidgetItem` 设置图标、复选框、自定义数据
  - `itemDoubleClicked` / `itemChanged` 常用信号

- [x] 🔴 `47-qlistview-beginner.md` — QListView：Model 驱动列表视图 (2025-04-22)
  - 与 `QStringListModel` 配合的完整示例
  - `setViewMode`（列表模式 vs 图标模式）
  - `setSpacing` / `setGridSize` 图标视图布局
  - 自定义 ItemDelegate 改变显示样式

- [x] 🔴 `48-qtreewidget-beginner.md` — QTreeWidget：便捷树形控件 (2025-04-22)
  - `QTreeWidgetItem` 构建层级树结构
  - `addTopLevelItem` / `insertChild` 增删节点
  - `setColumnCount` / `setHeaderLabels` 多列树表
  - `itemExpanded` / `itemCollapsed` / `itemClicked` 常用信号

- [x] 🔴 `49-qtreeview-beginner.md` — QTreeView：Model 驱动树视图 (2025-04-22)
  - 与 `QStandardItemModel` 配合树结构展示
  - `QFileSystemModel` + `QTreeView` 文件树示例
  - `setRootIndex()` 设置显示根节点
  - `expand()` / `collapse()` / `expandAll()` 节点展开控制

- [x] 🔴 `50-qtablewidget-beginner.md` — QTableWidget：便捷表格控件 (2025-04-22)
  - `setRowCount` / `setColumnCount` 设置行列数
  - `setItem()` / `item()` 单元格读写
  - `setHorizontalHeaderLabels` / `setVerticalHeaderLabels` 表头
  - `cellChanged` / `cellClicked` / `currentCellChanged` 信号

- [x] 🔴 `51-qtableview-beginner.md` — QTableView：Model 驱动表格视图 (2025-04-22)
  - 与 `QStandardItemModel` / `QSqlTableModel` 配合
  - `horizontalHeader()` / `verticalHeader()` 表头控制
  - `setSpan()` 合并单元格
  - `resizeColumnsToContents()` 自动列宽

- [x] 🟡 `52-qheaderview-beginner.md` — QHeaderView：表头控件 (2025-04-22)
  - `setSectionResizeMode`（固定/自适应/拉伸）
  - `setSortIndicator` / `setSortIndicatorShown` 排序指示
  - 隐藏某列：`hideSection(index)`
  - 自定义表头绘制（继承 + `paintSection`）

- [x] ⚪ `53-qcolumnview-beginner.md` — QColumnView：多列级联视图 (2025-04-22)
  - macOS Finder 列视图风格的树型导航
  - `setModel()` 与层级数据绑定
  - `updatePreviewWidget()` 预览面板集成
  - 适用场景：文件浏览器、设置面板导航

- [x] ⚪ `54-qundoview-beginner.md` — QUndoView：撤销历史视图 (2025-04-22)
  - 与 `QUndoStack` 绑定显示操作历史
  - `setCleanIcon()` 标记保存点
  - 点击历史条目实现时间线跳转
  - 在文档编辑器中的完整撤销重做系统

---

## 03 · QtWidgets — 控件速查篇（主窗口与对话框）（12 篇）

- [x] 🔴 `55-qmainwindow-beginner.md` — QMainWindow：主窗口完整配置 (2025-04-22)
  - 设置中央控件 `setCentralWidget()`
  - 菜单栏/工具栏/状态栏/Dock 完整搭建流程
  - `saveGeometry()` / `restoreGeometry()` 窗口尺寸持久化
  - 多 Dock 窗口布局策略

- [x] 🔴 `56-qmenubar-menu-action-beginner.md` — QMenuBar / QMenu / QAction：菜单系统 (2025-04-22)
  - `menuBar()->addMenu()` 添加顶级菜单
  - `QAction` 创建菜单项、设置图标、快捷键
  - 可检查动作 `setCheckable` 与菜单分隔线 `addSeparator`
  - 右键上下文菜单 `contextMenuEvent` 实现

- [x] 🔴 `57-qtoolbar-beginner.md` — QToolBar：工具栏 (2025-04-22)
  - `addAction` / `addWidget` / `addSeparator`
  - `setMovable` / `setFloatable` 工具栏停靠行为
  - `setIconSize` / `setToolButtonStyle`
  - 工具栏溢出菜单（控件超出工具栏宽度时）

- [x] 🔴 `58-qstatusbar-beginner.md` — QStatusBar：状态栏 (2025-04-22)
  - `showMessage(text, timeout)` 临时消息
  - `addWidget` / `addPermanentWidget` 嵌入永久控件
  - 进度条嵌入状态栏的典型模式
  - `clearMessage()` 与消息优先级

- [x] 🔴 `59-qdockwidget-beginner.md` — QDockWidget：可停靠浮动面板 (2025-04-22)
  - `setAllowedAreas` 限制停靠位置
  - `setFeatures` 控制是否可关闭/浮动/移动
  - `topLevelChanged` / `visibilityChanged` 信号
  - 多个 Dock 的 tabify 标签化合并

- [x] 🔴 `60-qdialog-beginner.md` — QDialog：自定义对话框 (2025-04-22)
  - 模态 `exec()` vs 非模态 `show()` 的应用场景
  - `accept()` / `reject()` / `done(int)` 返回值约定
  - 从对话框返回用户输入数据的正确模式
  - `setModal` vs `setWindowModality` 的区别

- [x] 🔴 `61-qdialogbuttonbox-beginner.md` — QDialogButtonBox：标准按钮盒 (2025-04-22)
  - `StandardButton` 枚举（OK/Cancel/Apply/Save/Discard…）
  - `accepted` / `rejected` / `clicked(QAbstractButton*)` 信号
  - `button(StandardButton)` 获取具体按钮做自定义
  - 与 `QDialog` 布局结合的标准对话框模板

- [x] 🔴 `62-qmessagebox-beginner.md` — QMessageBox：消息对话框 (2025-04-22)
  - 四种静态方法：`information` / `warning` / `critical` / `question`
  - 自定义按钮文字与详细信息展开
  - `setDetailedText()` 技术细节折叠显示
  - 在工作线程触发 MessageBox 的线程安全方式

- [x] 🟡 `63-qinputdialog-beginner.md` — QInputDialog：输入对话框 (2025-04-22)
  - `getText` / `getInt` / `getDouble` / `getItem` 静态方法
  - 自定义输入对话框的完整配置
  - 验证用户输入并阻止无效提交
  - 多输入字段时改用自定义 QDialog

- [x] 🟡 `64-qcolordialog-beginner.md` — QColorDialog：颜色选择对话框 (2025-04-22)
  - `getColor()` 静态方法快速获取颜色
  - `setOption(ShowAlphaChannel)` 启用透明度选择
  - `currentColorChanged` 实时预览颜色变化
  - 自定义调色板与颜色历史

- [x] 🟡 `65-qfontdialog-beginner.md` — QFontDialog：字体选择对话框 (2025-04-22)
  - `getFont()` 静态方法获取字体
  - `setCurrentFont()` 设置初始预选字体
  - `currentFontChanged` 实时预览字体变化
  - 过滤可选字体范围（仅等宽字体等）

- [x] 🔴 `66-qfiledialog-beginner.md` — QFileDialog：文件选择对话框 (2025-04-22)
  - `getOpenFileName` / `getSaveFileName` / `getExistingDirectory`
  - `getOpenFileNames` 多文件选择
  - 设置文件类型过滤器 `setNameFilter`
  - `setDirectory()` 设置默认打开目录

---

## 03 · QtWidgets — 控件速查篇（其他窗口控件）（5 篇）

- [x] 🟡 `67-qprogressdialog-beginner.md` — QProgressDialog：进度对话框 (2025-04-22)
  - `setLabelText` / `setValue` / `setRange` 基础配置
  - `canceled()` 信号响应用户取消
  - `setAutoClose` / `setAutoReset` 自动关闭行为
  - 与后台 QThread 配合的长任务进度汇报

- [x] ⚪ `68-qerrormessage-beginner.md` — QErrorMessage：错误消息对话框 (2025-04-22)
  - `showMessage()` 显示错误信息
  - "不再显示"复选框的记忆机制
  - 与 `qInstallMessageHandler` 结合捕获全局错误
  - 适用场景：可抑制的非致命错误通知

- [x] 🟡 `69-qwizard-beginner.md` — QWizard：向导对话框 (2025-04-22)
  - `addPage(QWizardPage*)` 添加向导页
  - `QWizardPage::initializePage()` / `validatePage()` 页面生命周期
  - 向导页间数据传递：`registerField` / `field()`
  - 自定义向导按钮文字与样式

- [x] 🟡 `70-qsplashscreen-beginner.md` — QSplashScreen：启动画面 (2025-04-22)
  - 应用启动时显示 Logo 与加载进度
  - `showMessage()` 更新加载状态文字
  - 主窗口就绪后 `finish(mainWindow)` 关闭
  - 与初始化耗时操作配合的完整启动序列

- [x] 🟡 `71-qmdiarea-mdisubwindow-beginner.md` — QMdiArea / QMdiSubWindow (2025-04-22)
  - `addSubWindow()` 添加子窗口
  - 子窗口排列：`tileSubWindows()` / `cascadeSubWindows()`
  - `activeSubWindowChanged` 信号追踪活动窗口
  - 子窗口菜单项自动更新

---

## 03 · QtWidgets — 控件速查篇（打印支持）（3 篇）

- [x] 🟡 `72-qprinter-beginner.md` — QPrinter：打印机抽象类 (2025-04-22)
  - `QPrinter::setPageSize` / `setOrientation` 页面设置
  - `QPainter` + `QPrinter` 实现自定义打印内容
  - 打印预览 `QPrintPreviewDialog`
  - 导出 PDF：`QPrinter::PdfFormat`

- [x] 🟡 `73-qprintdialog-beginner.md` — QPrintDialog：打印对话框 (2025-04-22)
  - `exec()` 弹出系统打印对话框
  - 获取用户配置：份数、打印范围、单/双面
  - 与 `QPainter` 联动的打印流程
  - 无打印机时的 PDF 回退方案

- [x] ⚪ `74-qprintpreviewdialog-beginner.md` — QPrintPreviewDialog：打印预览 (2025-04-22)
  - `paintRequested` 信号中执行实际绘制
  - 翻页与缩放操作
  - 自定义预览工具栏
  - `QPageSetupDialog` 页面参数配置

---

## 04 · QtNetwork（6 篇）

- [x] 🔴 `01-tcp-socket-beginner.md` — QTcpSocket / QTcpServer TCP 通信基础 (2025-04-22)
  - 客户端：`connectToHost` / `write` / `readAll` 基础流程
  - 服务端：`QTcpServer::listen` + `newConnection` 信号
  - `readyRead` 信号驱动异步读取
  - 断线检测：`disconnected` 信号与重连策略

- [x] 🟡 `02-udp-socket-beginner.md` — QUdpSocket UDP 通信基础 (2025-04-22)
  - `bind()` 绑定端口接收数据
  - `writeDatagram` / `readDatagram` 数据包收发
  - 广播地址 `QHostAddress::Broadcast` 发送广播
  - UDP 不可靠性与上层确认机制说明

- [x] 🔴 `03-network-access-manager-beginner.md` — QNetworkAccessManager HTTP 通信 (2025-04-22)
  - `get()` / `post()` 发起 HTTP 请求
  - `QNetworkReply::finished` 异步回调处理响应
  - 设置请求头 `QNetworkRequest::setHeader`
  - 下载文件并显示进度（`downloadProgress` 信号）

- [x] 🟡 `04-websocket-beginner.md` — QWebSocket 双向实时通信基础 (2026-04-23)
  - `open(url)` 建立 WebSocket 连接
  - `sendTextMessage` / `sendBinaryMessage` 发送数据
  - `textMessageReceived` / `binaryMessageReceived` 接收数据
  - `QWebSocketServer` 服务端实现

- [x] 🟡 `05-ssl-tls-beginner.md` — SSL/TLS 加密通信基础 (2026-04-23)
  - `QSslSocket` 与 `QNetworkRequest::setSslConfiguration`
  - `QSslCertificate` 加载证书文件
  - 忽略 SSL 错误（开发调试用，生产禁止）
  - OpenSSL 依赖配置与常见 SSL 错误排查

- [x] 🔴 `06-serial-port-beginner.md` — QtSerialPort 串口通信基础 (2026-04-23)
  - `QSerialPort::setPortName` / `setBaudRate` / `setDataBits`
  - `open(QIODevice::ReadWrite)` 打开串口
  - `readyRead` 信号异步读取，`write()` 发送数据
  - `QSerialPortInfo` 枚举系统可用串口

---

## 05 · 其他模块（25 篇）

- [x] 🔴 `01-qtsql-database-beginner.md` — QtSql 数据库连接与查询基础 (2025-04-22)
  - `QSqlDatabase::addDatabase("QSQLITE")` 连接 SQLite
  - `QSqlQuery::exec()` 执行 SQL 语句
  - `QSqlQuery::prepare` + `bindValue` 预编译防 SQL 注入
  - 错误处理：`QSqlError` 与日志记录

- [x] 🟡 `02-qtsql-tablemodel-beginner.md` — QSqlTableModel 数据库表格视图 (2025-04-22)
  - `QSqlTableModel` + `QTableView` 数据库直接绑定显示
  - `select()` / `submitAll()` / `revertAll()` 提交与回滚
  - `setFilter()` / `setSort()` 查询过滤与排序
  - `EditStrategy` 三种编辑策略选择

- [x] 🔴 `03-qtcharts-basic-beginner.md` — QtCharts / QtGraphs 图表基础 (2025-04-22)
  - `QChart` + `QChartView` 显示图表
  - `QLineSeries` / `QBarSeries` / `QPieSeries` 基础图表类型
  - 坐标轴 `QValueAxis` / `QBarCategoryAxis` 配置
  - 图表主题与动画效果

- [x] 🔴 `04-qtmultimedia-player-beginner.md` — QtMultimedia 音视频播放基础 (2025-04-22)
  - `QMediaPlayer` + `QVideoWidget` 视频播放
  - `play()` / `pause()` / `stop()` / `setPosition()` 控制
  - `QAudioOutput` 音频输出设置
  - `playbackStateChanged` / `errorOccurred` 状态信号

- [x] 🟡 `05-qtmultimedia-camera-beginner.md` — QtMultimedia 摄像头采集基础 (2025-04-22)
  - `QMediaDevices::videoInputs()` 枚举摄像头设备
  - `QCamera` + `QMediaCaptureSession` + `QVideoWidget` 预览
  - `QImageCapture` 截图保存
  - 摄像头权限申请（Windows / Linux 差异）

- [x] 🟡 `06-qtsvg-beginner.md` — QtSvg 矢量图形基础 (2025-04-22)
  - `QSvgWidget` 直接显示 SVG 文件
  - `QSvgRenderer` 在 QPainter 中渲染 SVG
  - SVG 动态着色与元素访问
  - 在图标系统中使用 SVG（高 DPI 适配）

- [x] 🟡 `07-qtprintsupport-overview-beginner.md` — QtPrintSupport 打印体系概览 (2025-04-22)
  - 模块引入与 CMake 配置
  - `QPrinter` / `QPrintDialog` / `QPrintPreviewDialog` 协作关系
  - 打印与导出 PDF 的统一流程
  - 跨平台打印差异说明

- [x] 🔴 `08-qtserialbus-modbus-beginner.md` — Qt Serial Bus Modbus 通信基础 (2025-04-22)
  - Modbus RTU vs TCP 连接类型选择
  - `QModbusClient::connectDevice()` 建立连接
  - 读写线圈、保持寄存器的请求构建
  - `QModbusReply` 异步结果处理

- [x] 🟡 `09-qtmqtt-beginner.md` — Qt MQTT 客户端基础 (2025-04-22)
  - `QMqttClient` 连接 Broker（地址/端口/ClientId）
  - `subscribe()` 订阅话题与 `messageReceived` 信号
  - `publish()` 发布消息
  - QoS 等级 0/1/2 的区别与选择

- [x] 🟡 `10-qtbluetooth-beginner.md` — QtBluetooth 蓝牙基础 (2025-04-22)
  - `QBluetoothDeviceDiscoveryAgent` 扫描蓝牙设备
  - `QBluetoothSocket` 经典蓝牙 SPP 连接
  - `QLowEnergyController` BLE 连接流程
  - 蓝牙权限配置（Android/Linux 差异）

- [x] ⚪ `11-qtnfc-beginner.md` — QtNFC 近场通信基础 (2025-04-22)
  - `QNearFieldManager` 检测 NFC 标签
  - `QNdefMessage` 读写 NDEF 数据
  - 平台支持限制（主要 Android/iOS）
  - NFC 标签类型（Type 1-5）与 NDEF 格式

- [x] 🟡 `12-qtstatemachine-beginner.md` — Qt StateMachine 状态机框架 (2025-04-22)
  - `QStateMachine` + `QState` + `QFinalState` 基础结构
  - `addTransition()` 添加转换条件（信号触发/事件触发）
  - 状态进入/离开时执行动作（`assignProperty` / 信号）
  - 层次状态机（子状态）构建

- [x] ⚪ `13-qtscxml-beginner.md` — Qt SCXML 状态图基础 (2025-04-22)
  - 从 `.scxml` 文件加载状态机
  - `QScxmlStateMachine` 启动与事件提交
  - 与 QML 集成：在 QML 中驱动 SCXML 状态机
  - SCXML vs Qt StateMachine 的适用场景对比

- [x] 🟡 `14-qt3d-basic-beginner.md` — Qt 3D 基础场景搭建 (2025-04-22)
  - `Qt3DExtras::Qt3DWindow` 创建 3D 窗口
  - Entity + Component 的 ECS 架构基础
  - 添加基础几何体：球体、立方体、平面
  - 相机、光源、材质的基础配置

- [x] 🟡 `15-qtquick3d-beginner.md` — QtQuick3D QML 3D 场景基础 (2025-04-22)
  - `View3D` + `PerspectiveCamera` + `DirectionalLight`
  - `Model` 节点加载 `.mesh` / 基础几何体
  - `PrincipledMaterial` 材质配置
  - 与 Qt Quick 2D 元素的混合显示

- [x] ⚪ `16-qtquick3d-physics-beginner.md` — QtQuick3D Physics 物理模拟基础 (2025-04-22)
  - `PhysicsWorld` 物理世界初始化
  - `StaticRigidBody` / `DynamicRigidBody` 刚体组件
  - `BoxShape` / `SphereShape` / `CapsuleShape` 碰撞体
  - 重力、弹性、摩擦力参数配置

- [x] 🟡 `17-qtpdf-beginner.md` — QtPdf PDF 渲染基础 (2025-04-22)
  - `QPdfDocument` 加载 PDF 文件
  - `QPdfPageRenderer` 渲染指定页面为 QImage
  - `QPdfView` 控件直接显示 PDF
  - 页面导航与缩放控制

- [x] 🟡 `18-qthttpserver-beginner.md` — QtHttpServer 嵌入式 HTTP 服务器 (2025-04-22)
  - `QHttpServer::route()` 注册路由处理函数
  - 处理 GET / POST 请求与返回 JSON 响应
  - `QHttpServerRequest` 读取请求体与请求头
  - 嵌入式 REST API 服务典型应用

- [x] 🟡 `19-qtwebsockets-server-beginner.md` — QtWebSockets 服务端进阶 (2025-04-22)
  - `QWebSocketServer` 监听端口、处理连接
  - 管理多个 `QWebSocket` 客户端连接
  - 广播消息给所有连接的客户端
  - SSL WebSocket（WSS）服务端配置

- [x] ⚪ `20-qtwebchannel-beginner.md` — QtWebChannel Web 与 Qt 互通 (2025-04-22)
  - `QWebChannel` 将 C++ 对象暴露给 JavaScript
  - `qwebchannel.js` 前端侧配置
  - 双向通信：JS 调用 Qt 方法 / Qt 发信号到 JS
  - 与 `QWebEngineView` / `QWebView` 集成

- [x] ⚪ `21-qtwebengine-beginner.md` — QtWebEngine 内嵌 Chromium 浏览器 (2025-04-22)
  - `QWebEngineView` 加载 URL 与本地 HTML
  - `QWebEnginePage::runJavaScript()` 注入执行 JS
  - `QWebEngineProfile` 管理 Cookie / 缓存
  - 资源占用与使用场景权衡

- [x] ⚪ `22-qtremoteobjects-beginner.md` — Qt Remote Objects 进程间对象共享 (2025-04-22)
  - `REPC` 文件定义远程对象接口
  - `QRemoteObjectHost` 发布对象
  - `QRemoteObjectNode` 获取远程副本
  - 局域网跨设备对象共享场景

- [x] ⚪ `23-qtspatial-audio-beginner.md` — Qt Spatial Audio 空间音频 (2025-04-22)
  - `QAudioEngine` 音频引擎初始化
  - `QSpatialSound` 设置 3D 音源位置
  - `QAudioListener` 听者位置与方向
  - 游戏/仿真中空间音效的典型应用

- [x] ⚪ `24-qttexttospeech-beginner.md` — Qt TextToSpeech 文字转语音 (2025-04-22)
  - `QTextToSpeech::say()` 朗读文本
  - `availableLocales()` / `availableVoices()` 枚举可用引擎
  - `setRate` / `setPitch` / `setVolume` 语音参数
  - 无障碍功能集成场景

- [x] 🟡 `25-qt5compat-migration-beginner.md` — Qt5Compat 兼容层与迁移指南 (2025-04-22)
  - Qt5Compat 模块包含哪些已废弃 API
  - `QRegExp` → `QRegularExpression` 迁移
  - `QTextCodec` → `QStringConverter` 迁移
  - 渐进式迁移策略：先用 Compat 再逐步替换

---

## 06 · QML 独立教程（7 篇）

- [x] 🔴 `01-qml-syntax-basics-beginner.md` — QML 语法基础与类型系统 (2025-04-22)
  - QML 文档结构：对象声明、属性赋值、层级关系
  - 基础类型：`int` / `real` / `string` / `bool` / `color` / `url`
  - `id` 机制：在 QML 文档内引用对象
  - JavaScript 表达式在属性值中的使用

- [x] 🔴 `02-property-binding-beginner.md` — 属性绑定与响应式数据流 (2025-04-22)
  - 属性绑定 `width: parent.width * 0.5` 的自动追踪
  - `property` 关键字声明自定义属性
  - `onPropertyChanged` 信号处理器
  - 绑定断裂（Binding Breakage）陷阱初识

- [x] 🔴 `03-qtquick-controls-beginner.md` — Qt Quick Controls 组件基础 (2025-04-22)
  - `ApplicationWindow` 主窗口结构
  - 常用控件：`Button` / `TextField` / `ComboBox` / `CheckBox` / `Slider`
  - `ColumnLayout` / `RowLayout` / `GridLayout` QML 布局
  - `Dialog` / `Popup` / `Menu` 弹出组件

- [x] 🔴 `04-cpp-qml-interop-beginner.md` — C++ 与 QML 互操作基础 (2025-04-23)
  - `Q_PROPERTY` 暴露 C++ 属性到 QML
  - `qmlRegisterType<T>()` / `QML_ELEMENT` 注册 C++ 类型
  - `QQmlContext::setContextProperty()` 注入单例对象
  - QML 调用 C++ 方法 / C++ 发信号到 QML

- [x] 🔴 `05-qml-animation-states-beginner.md` — QML 动画与状态机基础 (2025-04-23)
  - `NumberAnimation` / `ColorAnimation` / `PropertyAnimation`
  - `Behavior on property {}` 自动过渡动画
  - `State` + `Transition` 状态切换动画
  - `SequentialAnimation` / `ParallelAnimation` 组合动画

- [x] 🔴 `06-qml-model-delegate-beginner.md` — QML Model/Delegate 数据驱动视图 (2025-04-23)
  - `ListView` + `model` + `delegate` 三角关系
  - `ListModel` 纯 QML 数据模型
  - `GridLayout` + `Repeater` 网格布局
  - 从 C++ `QAbstractListModel` 驱动 QML 列表

- [x] 🟡 `07-qml-canvas-particles-beginner.md` — QML Canvas 绘图与粒子系统 (2025-04-23)
  - Canvas 2D 绘图 API 基础
  - `requestAnimationFrame` 动画循环
  - `ParticleSystem` + `Emitter` + `ImageParticle` 粒子特效
  - 性能注意事项
