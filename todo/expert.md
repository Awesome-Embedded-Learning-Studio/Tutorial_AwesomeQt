# 专家层（102 篇源码拆解）

> 每篇带 `qt_src/qt6.9.1 文件:行号` 可复现证据，结论落「行号 + 逐字原文 + 一句话解读」，读者能拿行号自己开源码核对。

## 前置

- ⚠ **01 篇落盘待核实**：2026-06-12 核查 `tutorial/expert/01-qtbase/` 仅 index.md、git 无记录。01 是后续篇的引用基石，状态确认前不推进后续。需补则重做。
- **code-index（源码索引资产）待决策**：是否建 `tutorial/expert/code-index/`（按模块分子目录，供读者对照源码自行验证）。暂不落地。落地时改 4 处：paradigm §2.5/2.6/2.8 + 01 篇删交接单/加链接 + 新建 `code-index/qtbase/01-*.md` + `site/sidebar.ts`。
- [ ] **建 `.claims-registry.yaml`**（`tutorial/expert/.claims-registry.yaml`，按模块分片·append-only·canonical/superseded，防已查证结论被后续篇引用扩散）｜ paradigm§2.9 ｜ 前台配套 ｜ 待产 ｜ 前置：**第 2 篇专家篇开工前**，未建好前靠交接单人工对账

## 建议顺序

QtBase 硬核机制篇最先（MOC / activate 调用链 / COW / 事件循环 / 对象树）——是后续所有篇的引用基石。

- 第一批：`01-qobject` · `21-object-tree`（引用基石）
- 第二批（01 落实后）：`02`·`18`·`07`·`20` + COW `03`·`04`·`19`

## 待决策

- P4「QProperty·QBindable 专家章」不在现有 102。并入→103，或单列。见 [infra.md](infra.md) P4。

## 102 篇队列

> 图例：🔴做 · 🟡中频 · ⚪延后/砍 · 〔🔀折叠进枢纽〕〔🔗同构合并〕〔➕新增〕。开工时以 qt_src 实地核对为准。

**00 环境（2）** 🟡`00-qt6-build-from-source` · 🔴`01-cmake`（生成器表达式 · INTERFACE/PUBLIC/PRIVATE 传播 · 交叉工具链）

**01 QtBase（21）** 🔴`01-qobject-meta-system`⚠ · 🔴`02-signal-slot-internals`(activate调用链) · 🔴`03-qstring-memory`(SSO+COW) · 🔴`04-containers-cow` · 🟡`05-qvariant` · 🔴`06-memory-model` · 🔴`07-event-loop-internals` · 🔴`08-file-io-iodevice` · 🔴`09-qthread-internals` · 🟡`10-qprocess` · 🟡`11-qtimer` · 🟡`12-plugin-loader` · 🟡`13-i18n` · 🟡`14-logging` · 🟡`15-regex-pcre` · 🟡`16-json-parser` · 🔴`17-moc-compiler`【专属】 · 🔴`18-signal-slot-deep-dive`【专属】 · 🔴`19-cow-implicit-sharing`【专属】 · 🔴`20-event-loop-deep-dive`【专属】 · 🔴`21-object-tree-ownership`〔B3➕·引用基石〕(setParent_helper/deleteChildren/deleteLater/线程亲和)

**02 QtGui（6）** 🔴`01-qpainter-backend` · 🟡`02-qtransform` · 🔴`03-qimage-format` · 🟡`04-font-engine` · 🟡`05-opengl-context` · 🟡`06-drag-drop-platform`

**03 主题能力（11）** 🔴`01-layout` · 🔴`02-event-dispatch`(QApplication::notify) · 🔴`03-model-view` · 🔴`04-qss` · 🔴`05-widget-rendering`(脏矩形/BackingStore) · 🔴`06-dialog-event-loop` · 🔴`07-main-window-layout` · 🟡`08-graphics-view-bsp` · 🟡`09-animation-timer` · 🔴`10-mdi-subwindow` · 🔴`11-qstyle-painting-delegation`〔➕公共篇·须排控件速查前·🔀吸收QProgressBar/QFrame〕(drawPrimitive/drawControl/drawComplexControl+QStyleOption)

**03 控件速查（32）**
- 基础枢纽：🔴`01-qwidget` · 🔴`02-qabstractbutton`〔🔀Radio/Check〕· 🟡`03-qabstractscrollarea`〔🔀QScrollArea〕· 🔴`04-qabstractitemview`〔🔀Calendar/ColumnView〕· 🟡`05-qabstractspinbox`〔🔀QSpinBox〕
- 按钮差异：🔴`06-qpushbutton` · 🔴`07-qtoolbutton` · 🔴`08-qradiobutton-qcheckbox`〔🔗〕
- 文本：🔴`09-qlineedit` · 🔴`10-qtextedit`〔🔀QLabel/QTextBrowser·富文本引擎同源〕· 🟡`11-qplaintextedit`(块级布局+maximumBlockCount环形缓冲)
- 数值/滑块/选择：🔴`12-qcombobox`〔🔀QFontComboBox〕· 🟡`13-qdatetimeedit`(Section状态机·2843行独立) · 🔴`14-qabstract-slider`〔🔗QSlider+QScrollBar+QDial·atan2非线性〕
- 容器/布局：🔴`15-qgroupbox` · 🔴`16-qtabwidget` · 🟡`17-qtabbar`(qGeomCalc·2933行独立) · 🔴`18-qsplitter`
- ItemView：🔴`19-qlistview`〔🔀QListWidget〕· 🔴`20-qtreeview`〔🔀QTreeWidget〕· 🔴`21-qtableview`〔🔀QTableWidget〕· 🟡`22-qheaderview`(4448行)
- 主窗口：🔴`23-qmainwindow`〔🔀QStatusBar〕· 🔴`24-qmenubar` · 🔴`25-qtoolbar` · 🔴`26-qdockwidget`
- 对话框：🔴`27-qdialog`〔🔀InputDialog/FontDialog/ButtonBox〕· 🔴`28-qmessagebox` · 🟡`29-qcolordialog` · 🔴`30-qfiledialog` · 🟡`31-qprogressdialog` · 🟡`32-qwizard`
- ⚪无专家层(8 trivial，源码无自研算法)：QLCDNumber · QStackedWidget · QKeySequenceEdit · QCommandLinkButton · QToolBox · QUndoView · QErrorMessage · QSplashScreen

**04 Network（6）** 🔴`01-tcp-socket`(十一态状态机) · 🟡`02-udp` · 🔴`03-network-access` · 🟡`04-websocket-frame` · 🟡`05-ssl-openssl` · 🔴`06-serialport-platform`(termios/DCB)

**05 其他（17活·S3砍8）** 🔴`01-qtsql-driver` · 🟡`02-qtsql-model` · 🔴`03-qtcharts-render` · 🔴`04-qtmultimedia-backend` · 🟡`05-camera` · 🟡`06-qtsvg` · 🔴`07-qtprintsupport`〔🔀QPrinter全族〕· 🔴`08-modbus` · 🟡`09-mqtt` · 🟡`10-bluetooth` · 🟡`12-statemachine` · 🟡`14-qt3d-ecs` · 🟡`15-qtquick3d-render` · 🟡`17-qtpdf` · 🟡`18-qthttpserver` · 🟡`19-qtwebsockets-server` · 🟡`25-qt5compat` · ⚪S3砍(8，稳定后再评估)：NFC·SCXML·Quick3D-Physics·WebChannel·WebEngine·RemoteObjects·SpatialAudio·TTS

**06 QML（7活·B2延后2）** 🔴`01-qml-binding-engine` · 🔴`02-qtquick-controls-style` · 🔴`03-cpp-qml-type-system` · 🔴`04-qml-animation-engine` · 🔴`05-qml-component-compiler` · 🔴`06-qml-model-view` · 🟡`07-workerscript-thread` · ⚪B2远期(2)：`08-v4-engine`·`09-qqmlengine-type-bridge`
