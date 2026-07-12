# AwesomeQt · 待办

> 项目待办入口。按工作区分文件，翻到对应区开干。完成一项把 `[ ]` 改 `[x]` + 日期。

## 速览

```
入门 137✅ · 进阶 134✅ · 专家 2 审结 + 16 待审（01·02 COW 审结 · 待审 16 篇：01 qobject / 02 signal-slot / 03 qstring-memory / 05 qvariant / 06 memory-model / 07 event-loop / 08 file-io / 09 qthread / 10 qprocess / 11 qtimer / 12 plugin-loader / 13 i18n / 14 logging / 15 regex-pcre / 17 moc / 21 object-tree，见 expert.md）
实例库 widget 13/13 ✅收齐（status-led✅ + toggle-switch✅ + circle-progress✅ + speed-meter✅ + range-slider✅ + line-chart✅ + editable-table✅ + checkbox-tree✅ + checkbox-list✅ + log-viewer✅ + password-edit✅ + ip-edit✅ + fade-animation✅），app image-viewer✅ + json-editor✅ + sqlite-browser✅ + serial-tool✅ + network-tool✅ + tetris✅ + cpu-memory-monitor✅(7 件整机成品·已合 main PR#16 · Windows 路径待作者实机复验) · model/industrial 骨架已立
examples 275✅ · 基建 P0✅ 基本清完
```

## 接力点（下次会话从这里接 · 2026-07-07 更）

- **已合入 main**：
  - **PR#14（fcde408）** — widget 栏 13/13 全收齐（status-led/toggle-switch/circle-progress/speed-meter/range-slider/line-chart/editable-table/checkbox-tree/checkbox-list/log-viewer/password-edit/ip-edit/fade-animation）
  - **PR#16（764f035）** — app 栏 7 件整机成品（image-viewer/json-editor/sqlite-browser/serial-tool/network-tool/tetris/cpu-memory-monitor）+ 双文档 + app/CMakeLists 纳入 + 累计清单 instance-batch-log.md
  - **PR#17（de7eeb4）** — 站点样式大更新（侧栏拖拽 + Qt 绿身份 + 字号切换 + 代码折叠 + 阅读进度条）

  每件实例构建门零 warning + offscreen 验证 + 对抗 review 修正 + Full 导览 + Handbook。**widget/app 双范式 + 双文档范式已立**。
- **专家层本批 5 篇 QtBase 硬核基石已落盘（待审·未合 main）**：01 qobject / 02 signal-slot / 07 event-loop / 17 moc / 21 object-tree，全在 `tutorial/expert/01-qtbase/`。复用 2026-06-24 已存档的取证（正文当时没落盘），五段 + mermaid + 普通代码块格式（不用 blockquote）+ markdownlint 0 error。**编号体系已拍板：知识点队列**（qobject=01 / COW 容器=04 / COW 隐式共享=19，三篇已重命名）。行号证据进 `code-index/qtbase/`（出一片补一片）。
- **专家层 03-qstring-memory 已落盘（待审·未合 main）**：立论三柱——Qt6 QString 无 SSO、sharedNull 退场改 `_empty`、QStringLiteral 不构造头部（nullptr-head 编译期视图）。详见 expert.md。
- **专家层 06-memory-model 已落盘（待审·未合 main）**：立论——ExternalRefCountData 双计数（strongref 管对象寿命 / weakref 管 refcount 块寿命）、QPointer Qt6 委托 QWeakPointer + strongref=-1 哨兵钉死不能升级、QScopedPointer 栈上 RAII（Qt6 仅 take/swap 成员 deprecated）。详见 expert.md。
- **专家层 08-file-io-iodevice 已落盘（待审·未合 main）**：立论——readData/writeData 纯虚契约、QFileDevice 持 FileEngine 不直接持 fd、open 走 POSIX open 非 fopen、readyRead/bytesWritten 子类 emit 唯 aboutToClose 自 emit、QSaveFile 原子写四件套。详见 expert.md。
- **专家层 09-qthread-internals 已落盘（待审·未合 main）**：立论——QThread 是管理者非线程本身、run 默认调 exec、start→平台 API→QThreadPrivate::start 入口、QThreadData 身份包、协作式 requestInterruption、terminate 危险（finished 发送线程 undefined）、QThread::create lambda 工厂。详见 expert.md。
- **专家层 11-qtimer 已落盘（待审·未合 main）**：立论——QTimer 是基于事件循环的软件定时器（自己不计时，甩给 dispatcher）、TimerType 三档（VeryCoarseTimer 非 StCoarseTimer，容差 Precise 不提前/Coarse 5%/VeryCoarse 500ms）、QBasicTimer 绕过 startTimer 直接走 dispatcher、singleShot 用 QSingleShotTimer 私有类。详见 expert.md。
- **专家层 05-qvariant 已落盘（待审·未合 main）**：立论——packedType 位域类型擦除、SSO-like 24 字节内联、QMetaTypeInterface 虚表、convert 三层 fallback、Qt6 完全基于 QMetaType 重构；两处 Qt5→Qt6 breaking（isNull 不再代理类型 isNull()、equals 类型不同不再通用 convert）。详见 expert.md。
- **专家层 10-qprocess 已落盘（待审·未合 main）**：立论——继承 QIODevice 不对称 IO 契约、setProcessState protected、Unix 用 vforkfd 非 posix_spawn（5 类环境回退 fork）、Qt6 无 QProcessManager/SIGCHLD 处理器（forkfd 事件化收割）、Windows terminate 是 EnumWindows+WM_CLOSE 对 console 无效 kill 才 TerminateProcess、error() 是函数非信号（信号叫 errorOccurred）、waitFor* 默认 30000ms、Windows CrashExit 靠 NT STATUS 范围+KillProcessExitCode=0xf291 判定、startDetached double-fork+setsid。详见 expert.md。
- **专家层 12-plugin-loader 已落盘（待审·未合 main）**：立论——持 QLibraryPrivate 组合非封装 QLibrary、instance() 双层单例（QLibraryPrivate::inst QPointer + 插件层 static QPointer，非每次 new）、Qt6 metadata 是 CBOR 二进制非 Qt5 JSON 文本、iid 校验只在 QFactoryLoader（QPluginLoader 不校验）、findPatternUnloaded 不 dlopen mmap 读 metadata、QFactoryLoader 同 key 择优「宁老勿新」、版本兼容「minor 允许老不允许新」、Qt5.7+ 默认 PreventUnloadHint（dlclose 是 no-op）、Q_PLUGIN_METADATA 缺 Q_OBJECT 静默失效非报错。详见 expert.md。
- **专家层 13-i18n 已落盘（待审·未合 main）**：立论——tr 由 Q_OBJECT 植入/context 是 moc 编译期类名、tr 找不到返原文非空串、installTranslator 不去重后装优先、QM 16 字节随机 magic 非 mcookie 生成非可读串、QM 无版本号字段 Qt5/Qt6 兼容、消息查找二分搜索预排序哈希数组非哈希表、translation 字段 UTF-16 大端非 UTF-8、复数 NumerusRules 字节码虚拟机、disambiguation=comment 新旧名并存、QTranslator 析构自摘除。详见 expert.md。
- **专家层 14-logging 已落盘（待审·未合 main）**：立论——qDebug 是宏展开成成员函数指针取址（.debug 无括号）、release 默认丢 file/line/function、qt_message_print 真正分流 qt_message_output 是外层、qFatal 走 qAbort 非 exit（Windows fastfail/TerminateProcess 其他 std::abort 不跑析构）、qInstallMessageHandler fetchAndStoreOrdered 直接替换不 wrap 返回旧 handler nullptr restore、【重大爆点】QLoggingCategory 构造默认 0x01010101 全开（注释漏 info）、defaultCategoryFilter 只硬编码关 qt.* 用户自定义默认开 debug、4 套规则集 QtConfig→Config→Api→Environment、qCDebug 编译期短路 for+Holder、Qt6.9 Q_LOGGING_CATEGORY 改函数返回引用、handler 在产生线程跑。详见 expert.md。
- **专家层 15-regex-pcre 已落盘（待审·未合 main）**：立论——QRegularExpression 封装 PCRE2、构造懒编译（isValid 有副作用）、PCRE2 16 位宽 _16 接口不转 UTF-8 + 强制 PCRE2_UTF、PatternOption 值与 PCRE2 不同需映射、JIT Release 默认开编译时自动搞定、用 pcre2_match_16 非 jit_match（内部自动检测 JIT）、(?J) 重复命名 qWarning 放行不支持、线程安全三层（隐式共享+mutex+thread_local JIT 栈）const 对象可跨线程并发 match、anchoredPattern \A(?:...)\z、wildcard 默认文件路径 glob。详见 expert.md。
- **下一步**：app 栏暂收（7 件完整）；转 model 栏放量（17 件照 undo-redo 范式）/ industrial hmi-dashboard 复用 widget 链 / 专家层另线推进；cpu-memory-monitor 的 Windows 路径需作者 Windows 实机复验
- **挂账**：05-other-modules ~25 篇缺踩坑段——按「不编坑」原则，等真写到该模块再补真坑（memory: no-fabricated-pitfalls）
- ⚠ **作者会在终端并行 commit/push/merge**：AI 改文件前先 `git status`；提交 / push 全归作者；commit / PR **不带任何 AI 署名**（memory: no-ai-attribution / user-handles-all-pushes）

## 工作区 → 文件

| 工作区 | 文件 | 内容 | 当下 |
|---|---|---|---|
| 专家层 | [expert.md](expert.md) | 102 篇源码拆解（每篇带 qt_src 行号证据） | ✅ 01·02 COW 审结 · 待审 16 篇（qobject/signal-slot/qstring-memory/qvariant/memory-model/event-loop/file-io/qthread/qprocess/qtimer/plugin-loader/i18n/logging/regex-pcre/moc/object-tree）→ 下一篇 16-json-parser |
| 实例库 | [instance-library.md](instance-library.md) | widget/app/model/industrial 成品 + 两套文档 | widget 13/13✅ · app 7/7✅（均合 main）· model 1/18（骨架）· industrial 1/1（骨架） |
| 基建 | [infra.md](infra.md) | P0/P0.5/P1/P4 + widget 化简 + 地基债 | ✅ P0 基本清完 · 剩 sidebar/结构漂移 |
| embedded | [embedded.md](embedded.md) | Layer1 公共基础 + Layer2 板级 | ⚠ 生产方式待定 |
| 延后 | [parked.md](parked.md) | community/translation/interactive/CI门… | 不投入 |

## 当下优先

- 专家层：01·02 审结 · 待审 16 篇（01/02/03/05/06/07/08/09/10/11/12/13/14/15/17/21，本次会话连推 11 篇 03/05/06/08/09/10/11/12/13/14/15）→ 审过后接 16-json-parser 等（一次一篇）
- 实例库：widget 13/13 + app 7/7 全合 main（PR#14/#16）→ 转 model 栏放量（17 件照 undo-redo 范式）
- 基建：P0✅ 基本清完（死链 + 需要注意的是×30 + 风格违例已入 main）→ 剩 专家 sidebar 收敛 + 入门结构漂移

## gate

- ✅ 01·02 篇已审结（2026-06-13，gate 实质过）→ expert.md
- ✅ status-led 中等档标杆 + toggle-switch 已合入 main，构建门通
- embedded 生产方式待定 → embedded.md

## 全量实例清单（参考，非待办）

选下一波时查 [registries/](registries/)（widget 500 / app 200 / model 317 / qml 52 / industrial 6）。
