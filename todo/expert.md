# 专家层（102 篇源码拆解）

## ⚡ 交接（2026-07-08，下次从这里接）

**现状**：18 篇产出。审结 2（`19-cow-implicit-sharing` / `04-cow-container-practice`）+ 待审 16（`01-qobject` / `02-signal-slot` / `03-qstring-memory` / `05-qvariant` / `06-memory-model` / `07-event-loop` / `08-file-io-iodevice` / `09-qthread-internals` / `10-qprocess` / `11-qtimer` / `12-plugin-loader` / `13-i18n` / `14-logging` / `15-regex-pcre` / `17-moc` / `21-object-tree`，已过一轮去 AI 味）。**剩 84 篇。**

**取证复用（省重复劳动）**：2026-06-24 一批硬核基石篇的取证已跑完存档，但正文当时都没落盘。开新篇先 `git log --all -- <正文文件名>` 核实：① 正文已在→直接审改；② 正文没有但取证已存档→直接取证据写正文，省一轮取证；③ 取证也没存→重新取证。**⚠️ 已存档主题：cow / qobject / signalslot / moc / objecttree / eventloop / qstring / memory / fileio / thread / timer / variant / qprocess / plugin / i18n / logging / regex 共 17 个；之外的篇（如 16-json-parser / 18-signal-slot-deep-dive）仍要重新取证。**

**成品风格规范（已定稿，新篇照办）**：
- **人称**：笔者（作者自称，写折腾 / 判断 / 第一次，**全文 ≥ 8 处**）/ 咱们（带读者）/ 您（警告读者）。不用「我们」。
- **去 AI 味**：粗体砍到 0（强调不靠 `**`，靠叙述本身）、动词口语化（卡壳 / 翻烂了 / 捅到 / 塞 / 踹醒）、长短句反差、偶尔吐槽。
- **格式**：源码引用用**普通代码块**（路径行 + 空行 + cpp 围栏），**不用 blockquote 嵌套**；moc `generator.cpp` 行号挂 `[^moc-gen]` 脚注；五段（前言 / 环境 / 核心概念 / 踩坑 / 官方链接），**不插测验不设练习**；提交前 `npx markdownlint-cli2` 0 error。
- **风格范本**：`../tutorial/expert/01-qtbase/01-qobject-meta-system-expert.md` §3.1。

**编号**：知识点队列（文件名序号 = 本文件「102 篇队列」里的号）。新篇按队列号落盘。

**下一篇**：`16-json-parser`（QJsonDocument/QJsonParseError 源码：JSON 解析器/CBOR 互转/QJson_value；🟡 中频；取证空白须重新取证）。**注：本次会话已连推 11 篇（03/05/06/08/09/10/11/12/13/14/15），均落盘待审，建议作者先审一批再继续。**

**文件位置**：正文 `tutorial/expert/01-qtbase/`；行号证据 `tutorial/expert/code-index/qtbase/`（进站点）；审查记录 `tutorial/expert/.audit/`（不进站点、不进 git）。

---

> 每篇带 `qt_src/qt6.9.1 文件:行号` 可复现证据，结论落「行号 + 逐字原文 + 一句话解读」，读者能拿行号自己开源码核对。

## 前置

- ✅ **编号体系已拍板（2026-07-08）**：采用**知识点队列编号**——文件名序号 = 该篇在 expert.md 102 篇队列里的知识点号。已据此重命名三篇：qobject=01（基石）、COW 容器实战=04（队列 04-containers-cow）、COW 隐式共享=19（队列 19 专属）。slug 保留作者原定（不改 cow-container-practice→containers-cow）。新篇一律按队列号落盘，不再用落盘顺序。
- ✅ **01 qobject 元对象系统篇已落盘（待审）**：`tutorial/expert/01-qtbase/01-qobject-meta-system-expert.md`。d_ptr 两层数据 → QMetaObject 静态表 → metacall 分发。五段 + mermaid + moc 脚注 + markdownlint 0 error，已过一轮去 AI 味。
- ✅ **02 signal-slot 信号槽底层篇已落盘（待审）**：`tutorial/expert/01-qtbase/02-signal-slot-internals-expert.md`。emit 空宏 → activate → doActivate → 连接表 → 四类分流 → QMetaCallEvent → 析构清理 → 重入保护。
- ✅ **07 event-loop 事件循环篇已落盘（待审）**：`tutorial/expert/01-qtbase/07-event-loop-internals-expert.md`。exec 心脏 → dispatcher 抽象 → notify/sendEvent/postEvent 双路 → DeferredDelete 全链 → 事件过滤器。
- ✅ **17 moc 编译器原理篇已落盘（待审）**：`tutorial/expert/01-qtbase/17-moc-compiler-expert.md`。runMoc 三段式 → ClassDef IR → generateCode 四段 → staticMetaObject 七字段 → 与运行期 metacall 对接。
- ✅ **21 object-tree 对象树所有权篇已落盘（待审）**：`tutorial/expert/01-qtbase/21-object-tree-ownership-expert.md`。setParent_helper 三段 → 析构级联 deleteChildren 手写循环 → deleteLater → moveToThread 同线程铁律 → QWidget willBeWidget。
- ✅ **03 qstring 内存模型篇已落盘（待审）**：`tutorial/expert/01-qtbase/03-qstring-memory-expert.md`。立论三柱：Qt6 QString 无 SSO（只有 DataPointer 值对象=3 word）、sharedNull 退场改 _empty、QStringLiteral 不构造头部（nullptr-head 编译期视图）。五段 + 3 mermaid + 去 AI 味 + markdownlint 0 error。
- ✅ **06 智能指针家族篇已落盘（待审）**：`tutorial/expert/01-qtbase/06-memory-model-expert.md`。立论：ExternalRefCountData 双计数（strongref 管对象寿命 / weakref 管 refcount 块寿命）、QWeakPointer 只动 weakref 不阻析构、QPointer Qt6 委托 QWeakPointer + strongref=-1 哨兵钉死不能升级、QScopedPointer 栈上 RAII（Qt6 仅 take/swap 成员 deprecated）。五段 + mermaid + 去 AI 味 + markdownlint 0 error。
- ✅ **08 QIODevice 与 QFile 篇已落盘（待审）**：`tutorial/expert/01-qtbase/08-file-io-iodevice-expert.md`。立论四柱：readData/writeData 纯虚 = 0（非默认返回 -1，QIODevice 是抽象类）、QFileDevice 持 QAbstractFileEngine 不直接持 fd（FileEngine 抽象层）、Unix 走 QT_OPEN 非 fopen + QFSFileEngine 断言不支持 buffered、readyRead/bytesWritten 子类 emit 唯 aboutToClose 自 emit。含 QRingBuffer 16KB 双路缓冲、QSaveFile 原子写四件套。五段 + mermaid + 去 AI 味 + markdownlint 0 error。
- ✅ **09 QThread 篇已落盘（待审）**：`tutorial/expert/01-qtbase/09-qthread-internals-expert.md`。立论：QThread 是管理者 QObject 非线程本身、run 默认调 exec、start→pthread_create/CreateThread→QThreadPrivate::start 入口（emit started 在 run 前）、QThreadData 每线程一个的身份包、协作式 requestInterruption（atomic bool，主线程判定基于 theMainThreadId）、terminate 危险（terminate 路径 finished 发送线程 undefined）、QThread::create 经 std::async(deferred)+QThreadCreateThread 子类。五段 + mermaid + 去 AI 味 + markdownlint 0 error。
- ✅ **11 QTimer 篇已落盘（待审）**：`tutorial/expert/01-qtbase/11-qtimer-expert.md`。立论：QTimer 是基于事件循环的软件定时器（继承 QObject、start→QObject::startTimer→dispatcher 注册、自己不计时）、TimerType 三档（Precise/Coarse/VeryCoarseTimer，StCoarseTimer 不存在；容差 Precise 不提前 / Coarse 5% / VeryCoarse 500ms）、QBasicTimer 绕过 startTimer 直接走 dispatcher、singleShot 用 QSingleShotTimer 私有类（持 QBasicTimer、自毁）。五段 + mermaid + 去 AI 味 + markdownlint 0 error。
- ✅ **05 QVariant 篇已落盘（待审）**：`tutorial/expert/01-qtbase/05-qvariant-expert.md`。立论：QVariant 是类型擦除容器（packedType 位域右移 2 位存 QMetaTypeInterface*，依赖 4 字节对齐）、SSO-like 存储（3*sizeof(void*)=24 字节内联阈值 + relocatable + align≤8，大类型 PrivateShared 堆分配）、QMetaTypeInterface 虚表驱动构造析构（trivially copyable 短路 memcpy）、convert 三层 fallback（moduleHelper/注册表/特殊路径）、Qt6 完全基于 QMetaType 重构（QVariant::Type deprecated）。两处 Qt5→Qt6 breaking：isNull 不再代理类型 isNull()、equals 类型不同不再通用 convert。五段 + mermaid + 去 AI 味 + markdownlint 0 error。

- ✅ **10 QProcess 篇已落盘（待审）**：`tutorial/expert/01-qtbase/10-qprocess-expert.md`。立论：继承 QIODevice 的不对称 IO 契约（readData 只报 EOF/真正读在 tryReadFromChannel，writeData 平台相关无跨平台实现）、setProcessState 是 protected、start→startProcess→startupNotification 调用链、Unix 用 vforkfd 非 posix_spawn（5 类环境回退 fork）、Qt6 无 QProcessManager/SIGCHLD 处理器（forkfd 事件化收割）、Windows terminate 是 EnumWindows+WM_CLOSE 对 console 无效 kill 才 TerminateProcess、error() 是函数非信号（信号叫 errorOccurred）、waitFor* 默认 30000ms、Windows CrashExit 靠 NT STATUS 范围 + KillProcessExitCode=0xf291 判定、startDetached double-fork+setsid。五段 + 3 mermaid + 4 踩坑 + 去 AI 味 + markdownlint 0 error。A 取证 48 claims + B 复核全过（9 处高价值纠偏逐字核实）。

- ✅ **12 QPluginLoader 篇已落盘（待审）**：`tutorial/expert/01-qtbase/12-plugin-loader-expert.md`。立论：持 QLibraryPrivate 组合非封装 QLibrary、instance() 双层单例（QLibraryPrivate::inst QPointer 缓存 + 插件层 static QPointer，非每次 new）、Q_PLUGIN_METADATA 零展开 moc 才干活、Qt6 metadata 是 CBOR 二进制非 Qt5 JSON 文本（JSON 只在 FILE 输入和 metaData() 输出两边界）、iid 校验只在 QFactoryLoader（QPluginLoader 给文件名即 load 不校验 iid）、findPatternUnloaded 不 dlopen mmap 读 metadata、QFactoryLoader 三层缓存 + PreventUnloadHint 永不卸载、同 key 择优「宁老勿新」、版本兼容「minor 允许插件老不允许新（用 > 非 !=）、major 必须 ==、patch 不查」、Qt5.7+ 默认 PreventUnloadHint（RTLD_NODELETE/GET_MODULE_HANDLE_EX_FLAG_PIN，dlclose 是 no-op）、双计数器 libraryRefCount/libraryUnloadCount、静态插件导出 qt_static_plugin_##MANGLEDNAME 带 mangled 非 dynamic 裸名、Q_PLUGIN_METADATA 缺 Q_OBJECT 是 moc continue 静默失效非报错。五段 + 3 mermaid + 4 踩坑 + 去 AI 味 + markdownlint 0 error。A 取证 38 claims + B 复核全过（6 处高价值纠偏逐字核实）。

- ✅ **13 i18n 篇已落盘（待审）**：`tutorial/expert/01-qtbase/13-i18n-expert.md`。立论：tr 由 Q_OBJECT 的 QT_TR_FUNCTIONS 植入（inline static 转发 staticMetaObject.tr）、context 是 moc 编译期烧进 staticMetaObject stringdata 的类名（非运行期）、tr 找不到翻译返 QString::fromUtf8(sourceText) 原文非空串、translators 列表 prepend 故后装优先、installTranslator 不去重（重复 install 留重复条目）、QM 文件 16 字节随机二进制 magic（mcookie 生成，非可读串）、QM 无版本号字段（TLV 分节，Qt5/Qt6 兼容）、context 查找哈希+桶线性、消息查找是 offsetArray 预排序数组上二分搜索非哈希表、hash=elfHash_continue(sourceText) 后 continue(comment) 累加（非 XOR）、translation 字段 UTF-16 大端非 UTF-8（sourceText/Language 才是 UTF-8）、复数 NumerusRules 字节码栈式虚拟机（Q_MOD_10/Q_EQ/Q_NEWRULE）、disambiguation=comment 新旧名并存、Dependencies 链式 subTranslators 查找、QTranslator 析构自摘除（QCoreApplication 不拥有所有权）。五段 + 3 mermaid + 4 踩坑 + 去 AI 味 + markdownlint 0 error。A 取证 37 claims + B 复核全过（9 处高价值纠偏逐字核实，行号零偏差）。

- ✅ **14 logging 篇已落盘（待审）**：`tutorial/expert/01-qtbase/14-logging-expert.md`。立论：qDebug 是宏展开成 QMessageLogger 成员函数指针取址（.debug 无括号，qDebug("x")=(QMessageLogger(...).debug)("x")）、QT_NO_DEBUG_OUTPUT 降级 while(false) noDebug（qCritical/qFatal 永远启用）、QtMsgType 不按严重性排序（Qt6 QtInfoMsg 在 QtFatalMsg 后）、QMessageLogContext release 默认丢 file/line/function（QT_NO_DEBUG→QT_NO_MESSAGELOGCONTEXT→nullptr）、qt_message_print 真正分流（grabMessageHandler thread_local 防自递归+原子 messageHandler）、qt_message_output 是带 isFatal 外层非 stderr 出口、qFatal 走 qAbort 非 exit（Windows fastfail/RaiseFailFast/TerminateProcess 其他 std::abort，不跑析构）、qInstallMessageHandler fetchAndStoreOrdered 直接替换不 wrap（返回旧 handler，nullptr restore）、QDebug RAII 析构输出（message_output=false 是禁用开关）、【重大爆点】QLoggingCategory 构造默认 0x01010101 全开（注释漏 info）、defaultCategoryFilter 只硬编码关 qt/qt.*（用户自定义默认开 debug）、4 套规则集 QtConfig→Config→Api→Environment（env 最高）、qCDebug for+Holder 编译期短路（Q_UNLIKELY）、Qt6.9 Q_LOGGING_CATEGORY 改函数返回引用（避免 static init fiasco，deprecation 仅 Qt 内部）、handler 在产生线程跑。五段 + 2 mermaid + 4 踩坑 + 去 AI 味 + markdownlint 0 error。A 取证 25 claims + B 复核全过（10 处高价值纠偏逐字核实）。

- ✅ **15 regex-pcre 篇已落盘（待审）**：`tutorial/expert/01-qtbase/15-regex-pcre-expert.md`。立论：QRegularExpression 用 QExplicitlySharedDataPointer 隐式共享、构造懒编译（首次 match/isValid 才 compilePattern，isValid 有副作用）、Qt 用 PCRE2 16 位宽 _16 接口不转 UTF-8（pattern.constData 直传 PCRE2_SPTR16）+ 强制 PCRE2_UTF、PatternOption 值与 PCRE2 不同需 convertToPcreOptions 显式映射（CaseInsensitiveOption=0x1 vs PCRE2_CASELESS）、JIT Release 默认开（Debug/macOS Rosetta 关，QT_ENABLE_REGEXP_JIT 覆盖）编译时自动 pcre2_jit_compile 三模式、Qt 用 pcre2_match_16 非 jit_match（PCRE2 内部自动检测 JIT，safe_pcre2_match_16 处理栈耗尽重试）、JIT 栈 thread_local、match_context/match_data per-call、ovector 拷贝依赖 PCRE2_UNSET==-1（未捕获 -1）、partial 回退 lookbehind 兼容 PCRE1、(?J) 重复命名 qWarning+放行不支持（captureIndexForName 假设 name 唯一）、线程安全三层（隐式共享 detach + mutex 编译 + thread_local JIT 栈，const 已编译对象可跨线程并发 match）、setPattern detach+isDirty 重编译、anchoredPattern \A(?:...)\z、wildcard 默认文件路径 glob。五段 + 2 mermaid + 4 踩坑 + 去 AI 味 + markdownlint 0 error。A 取证 29 claims + B 聚焦复核 14 全过（6 处高价值纠偏逐字核实）。

> 本批 5 篇的取证 2026-06-24 已跑完存档，但正文当时都没落盘；这次直接取存档证据写正文，省了一轮取证。每篇配 code-index（按机制分文件）+ markdownlint 0 error + 已过一轮去 AI 味。
- ✅ **19 COW 隐式共享篇已落盘（审结）**：2026-06-13 产出（2026-07-08 按知识点队列重编为 19），`tutorial/expert/01-qtbase/19-cow-implicit-sharing-expert.md`。K03/L04 已作者拍板。
- ✅ **04 COW 容器实战篇已落盘（审结）**：2026-06-13 产出（2026-07-08 按知识点队列重编为 04），`tutorial/expert/01-qtbase/04-cow-container-practice-expert.md`。
- ✅ **code-index 已落地（31 文件）**：`tutorial/expert/code-index/qtbase/` 按源码机制分文件（COW 5 + 本批 20：qobject-dptr-pimpl / qmetaobject-static-metadata / metacall-dispatch / signal-* / event-* / moc-* / object-tree-* + 03-qstring 新增 qstring-memory-layout，qarraydata.md 另补分配链路节 + 06-memory 新增 qsharedpointer-family + 08-file-io 新增 qiodevice-fileio + 09-qthread 新增 qthread-internals + 11-qtimer 新增 qtimer + 05-qvariant 新增 qvariant + 10-qprocess 新增 qprocess + 12-plugin 新增 qpluginloader + 13-i18n 新增 qtranslator + 14-logging 新增 qlogging + 15-regex 新增 qregularexpression），全进 VitePress 构建。
- ✅ **审查记录存 `.audit/`**：`tutorial/expert/.audit/`（18 篇各一份，含 03-qstring / 05-qvariant / 06-memory / 08-file-io / 09-qthread / 10-qprocess / 11-qtimer / 12-plugin / 13-i18n / 14-logging / 15-regex，不进站点、已 gitignore）。

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
