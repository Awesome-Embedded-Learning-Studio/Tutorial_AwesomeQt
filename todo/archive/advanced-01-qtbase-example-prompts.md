# 01-qtbase 进阶代码样例生成 Prompts

> 以下 prompt 用于生成 `examples/advanced/01-qtbase/` 下的 16 个代码样例工程。
> 每个 prompt 包含：示例目标、文件结构、Qt 模块依赖、核心演示点。
> 将对应教程文件路径传入即可让 AI 理解上下文生成代码。

---

## 通用规范（所有样例共享）

**参考模板**：读取 `examples/beginner/01-qtbase/01-qobject-meta-system-beginner/` 目录下的文件作为格式参考, 看看如何进行文件分拆

---

## 示例 01：`01-qobject-property-system-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/01-qobject-property-system-advanced.md`

**目标**：演示 Q_PROPERTY 完整语法（READ/WRITE/NOTIFY/RESET/STORED/CONSTANT/FINAL）、动态属性、元对象反射枚举属性。

**文件结构**：
```
examples/advanced/01-qtbase/01-qobject-property-system-advanced/
├── main.cpp
├── configobject.h       // 带 RESET/STORED/CONSTANT/FINAL 的完整属性类
├── reflectivedemo.h     // 使用 QMetaObject/QMetaProperty 反射枚举属性
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- ConfigObject 类声明多个 Q_PROPERTY，包含 RESET 函数和 CONSTANT/FINAL 修饰
- setProperty()/property() 添加和读取动态属性
- 使用 QMetaObject::propertyCount()/propertyOffset() 遍历属性
- QMetaProperty 的 isReadable()/isWritable()/isResettable()/hasNotifySignal() 检查
- Q_CLASSINFO 附加元数据并通过 metaObject()->classInfo() 读取

---

## 示例 02：`02-signal-slot-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/02-signal-slot-advanced.md`

**目标**：演示 Qt 连接类型、Lambda 生命周期陷阱、手动连接管理。

**文件结构**：
```
examples/advanced/01-qtbase/02-signal-slot-advanced/
├── main.cpp
├── connectiontypes.h    // 演示 Auto/Direct/Queued/BlockingQueued/UniqueConnection
├── lambdatriap.h        // Lambda 捕获陷阱复现和安全模式
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- Worker 对象 moveToThread 后对比 DirectConnection vs QueuedConnection 的调用线程
- BlockingQueuedConnection 跨线程同步获取结果（附同线程死锁警告）
- Lambda 捕获裸指针 vs 使用 QPointer 安全捕获
- QMetaObject::Connection 手动 disconnect
- Qt::UniqueConnection 防止重复连接

---

## 示例 03：`03-qstring-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/03-qstring-advanced.md`

**目标**：演示编码陷阱、QStringView 零拷贝、QStringBuilder 拼接性能。

**文件结构**：
```
examples/advanced/01-qtbase/03-qstring-advanced/
├── main.cpp
├── encodingdemo.h       // Latin-1/UTF-8/UTF-16 隐式转换陷阱演示
├── stringperf.h         // QStringView vs QString、QStringBuilder 性能对比
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- QString::fromUtf8/fromLatin1/fromUtf16 转换及 QByteArray 中间对象开销
- QStringView 作为函数参数避免 QString 构造
- QStringBuilder (%) 操作符拼接 vs 传统 + 操作符的内存分配次数对比
- QStringLiteral vs普通字符串字面量
- QElapsedTimer 计时对比各方案性能

---

## 示例 04：`04-containers-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/04-containers-advanced.md`

**目标**：演示 COW 触发条件、STL 算法互操作、QHash 自定义键类型。

**文件结构**：
```
examples/advanced/01-qtbase/04-containers-advanced/
├── main.cpp
├── cowdemo.h            // COW 深拷贝触发实验（const vs 非const 访问）
├── customhash.h         // QHash 自定义键类型（operator== + qHash）
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- QList 的 constBegin vs begin() 触发 detach 的对比
- std::as_const() 防止意外深拷贝
- std::sort/std::find_if/std::transform 在 QList 上的使用
- QList::toVector() 和 QVector 到 QList 的互转
- 自定义结构体作为 QHash 键：重载 operator== 和 qHash()

---

## 示例 05：`05-qvariant-metatype-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/05-qvariant-metatype-advanced.md`

**目标**：演示自定义类型注册、QMetaType 反射、类型安全存取。

**文件结构**：
```
examples/advanced/01-qtbase/05-qvariant-metatype-advanced/
├── main.cpp
├── customtype.h         // 自定义结构体 + Q_DECLARE_METATYPE + qRegisterMetaType
├── metatypereflection.h // QMetaType 反射能力演示
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- 自定义结构体使用 Q_DECLARE_METATYPE 宏声明
- qRegisterMetaType<T>() 注册后可在信号槽跨线程传递
- QVariant::fromValue() 和 qvariant_cast<T> 存取
- QMetaType::metaObject<T>()、QMetaType::isRegistered()、QMetaType::typeName() 反射
- QVariant::canConvert() vs convert() 类型转换检查

---

## 示例 06：`06-memory-management-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/06-memory-management-advanced.md`

**目标**：演示智能指针对比、循环引用打破、QPointer 自动置空。

**文件结构**：
```
examples/advanced/01-qtbase/06-memory-management-advanced/
├── main.cpp
├── smartpointerdemo.h   // QSharedPointer/QWeakPointer/QScopedPointer 对比
├── circularfix.h         // 循环引用场景 + QWeakPointer 打破
├── qpointerdemo.h        // QPointer 自动置空演示
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- QSharedPointer 引用计数变化追踪（strongRef/weakRef）
- 双向关联中 QWeakPointer 打破循环引用
- QPointer 监视 QObject 自动置空
- QScopedPointer vs std::unique_ptr 互操作（take/release）
- QObject 对象树 vs 智能指针选择指南

---

## 示例 07：`07-event-system-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/07-event-system-advanced.md`

**目标**：演示自定义事件类、跨线程 postEvent、eventFilter 全局监听。

**文件结构**：
```
examples/advanced/01-qtbase/07-event-system-advanced/
├── main.cpp
├── customevent.h        // 自定义 QEvent 子类 + registerEventType
├── eventfilterdemo.h    // eventFilter 全局键盘/鼠标监听
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core, Gui（QKeyEvent/QMouseEvent 等），Widgets（QApplication 事件循环）

**核心演示点**：
- 继承 QEvent 创建自定义事件类型，携带自定义数据
- QEvent::registerEventType() 获取唯一事件 ID
- QCoreApplication::postEvent() 跨线程投递（sendEvent vs postEvent 对比）
- eventFilter() 安装在 QApplication 上实现全局键盘拦截
- customEvent() 接收并处理自定义事件

---

## 示例 08：`08-file-io-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/08-file-io-advanced.md`

**目标**：演示 QDataStream 版本兼容序列化、QFileSystemWatcher、QSaveFile 原子写入、QFile::map()。

**文件结构**：
```
examples/advanced/01-qtbase/08-file-io-advanced/
├── main.cpp
├── serializable.h       // QDataStream 自定义序列化 << / >> 运算符 + 版本号
├── fileopsdemo.h        // QSaveFile 原子写入 + QFile::map() 内存映射
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- QDataStream::setVersion() 版本兼容读写策略
- 自定义 << / >> 运算符序列化复合结构
- QSaveFile 原子写入（cancel 时自动丢弃）
- QFile::map() 内存映射读取大文件
- QFileSystemWatcher 监控文件变化（目录和文件）

---

## 示例 09：`09-qthread-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/09-qthread-advanced.md`

**目标**：演示 QThreadPool+QRunnable、QtConcurrent::run、QFuture+QFutureWatcher、QReadWriteLock。

**文件结构**：
```
examples/advanced/01-qtbase/09-qthread-advanced/
├── main.cpp
├── threadpooltasks.h    // QRunnable 任务 + QThreadPool 配置
├── concurrentdemo.h     // QtConcurrent::run + QFuture/QFutureWatcher
├── rwlockdemo.h         // QReadWriteLock 多读者单写者场景
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core, Concurrent

**核心演示点**：
- QRunnable 子类实现 + QThreadPool::start() 提交任务
- QThreadPool::maxThreadCount() 调优
- QtConcurrent::run() 异步执行函数，获取 QFuture
- QFutureWatcher 监控异步结果完成
- QReadWriteLock: 多个线程同时读 vs 写时互斥

---

## 示例 10：`10-qprocess-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/10-qprocess-advanced.md`

**目标**：演示异步流式读取、管道通信、崩溃检测、超时管理。

**文件结构**：
```
examples/advanced/01-qtbase/10-qprocess-advanced/
├── main.cpp
├── asyncreader.h        // readyReadStandardOutput 流式读取
├── pipedemo.h           // 管道链接两个进程
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- setProcessChannelMode(MergedChannels) 合并输出
- readyReadStandardOutput 信号驱动的异步流式读取
- setStandardOutputProcess() 管道链接（如 ls | grep）
- 进程崩溃检测：errorOccurred 信号 + exitStatus() 判断
- QTimer 配合实现进程超时终止（terminate → kill 两阶段）

---

## 示例 11：`11-qtimer-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/11-qtimer-advanced.md`

**目标**：演示三种定时器精度、QElapsedTimer 纳秒计时、QDeadlineTimer、零定时器模式。

**文件结构**：
```
examples/advanced/01-qtbase/11-qtimer-advanced/
├── main.cpp
├── precisiondemo.h      // PreciseTimer/CoarseTimer/VeryCoarseTimer 精度对比
├── elapsedtimerdemo.h   // QElapsedTimer + QDeadlineTimer 演示
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- QTimer 三种 timerType 精度实测对比（QElapsedTimer 计量偏差）
- QElapsedTimer::start()/elapsed()/restart() 纳秒级性能计时
- QDeadlineTimer 创建超时截止时间，用于循环中的 isExpired() 检查
- 0ms QTimer 零定时器延迟执行模式
- QBasicTimer 轻量级替代（timerEvent 直接收）

---

## 示例 12：`12-plugin-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/12-plugin-advanced.md`

**目标**：演示插件版本控制、热加载/卸载、自动发现插件目录。

**注意**：此示例需要生成两个项目：一个宿主程序和一个插件动态库。为简化可编译性，此处只生成宿主程序，使用静态插件模拟。

**文件结构**：
```
examples/advanced/01-qtbase/12-plugin-advanced/
├── main.cpp
├── plugininterface.h    // 插件接口定义（含版本号检查）
├── pluginmanager.h      // QPluginLoader 加载/卸载/版本检查
├── sampleplugin.h       // 内联示例插件实现（模拟动态插件）
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- 插件接口含版本号虚函数，加载后先检查版本兼容
- QPluginLoader::load()/unload() 热加载和卸载
- QPluginLoader::metaData() 读取 JSON 元数据
- QDir 遍历插件目录自动发现
- errorString() 诊断加载失败原因

---

## 示例 13：`13-i18n-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/13-i18n-advanced.md`

**目标**：演示动态语言切换、QLocale 本地化格式、复数形式。

**文件结构**：
```
examples/advanced/01-qtbase/13-i18n-advanced/
├── main.cpp
├── localizedemo.h       // QLocale 数字/日期/货币格式化
├── dynamictranslator.h  // 运行时动态切换 QTranslator
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- QTranslator::load() 加载 .qm 文件，remove() + install() 动态切换
- QLocale 的 numberFormatting/dateTimeFormat/currencySymbol
- tr() 包裹字符串 + QT_TR_NOOP 宏
- 复数形式的基本演示（使用 QTranslator::translate 的 n 参数）

---

## 示例 14：`14-logging-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/14-logging-advanced.md`

**目标**：演示自定义消息处理器、QLoggingCategory 多模块开关。

**文件结构**：
```
examples/advanced/01-qtbase/14-logging-advanced/
├── main.cpp
├── customhandler.h      // qInstallMessageHandler 自定义日志输出
├── categorydemo.h       // QLoggingCategory 多模块独立日志级别
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- qInstallMessageHandler 安装自定义处理器（格式化时间戳、级别、文件名、行号）
- Q_DECLARE_LOGGING_CATEGORY + qCDebug/qCWarning/qCCritical 分类日志
- setCategoryFilter 或 QT_LOGGING_RULES 环境变量控制日志开关
- Release 模式下保留 qWarning 禁用 qDebug 的策略

---

## 示例 15：`15-regex-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/15-regex-advanced.md`

**目标**：演示命名捕获组、JIT 预编译优化、全局匹配迭代、灾难性回溯预防。

**文件结构**：
```
examples/advanced/01-qtbase/15-regex-advanced/
├── main.cpp
├── namedcapture.h       // 命名捕获组 (?P<name>...) 演示
├── regexperf.h          // JIT optimize + globalMatch + 回溯超时
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- 命名捕获组 (?P<name>...) + captured("name") 按名称提取
- QRegularExpression::optimize() JIT 预编译性能对比
- globalMatch() + QRegularExpressionMatchIterator 流式匹配所有结果
- setMatchTimeout() 设置超时防止灾难性回溯
- PatternOptions：CaseInsensitive、Multiline、DotMatchesEverything

---

## 示例 16：`16-json-xml-advanced`

**教程文件**：`tutorial/advanced/01-qtbase/16-json-xml-advanced.md`

**目标**：演示 QXmlStreamWriter/Reader 状态机解析、JSON 流式构建、CBOR。

**文件结构**：
```
examples/advanced/01-qtbase/16-json-xml-advanced/
├── main.cpp
├── xmlstreamdemo.h      // QXmlStreamWriter 生成 + QXmlStreamReader 状态机解析
├── jsonstreamdemo.h     // JSON 大文档内存分析 + QCborValue 二进制 JSON
├── CMakeLists.txt
└── .gitignore
```

**Qt 模块**：Core

**核心演示点**：
- QXmlStreamWriter 生成格式化 XML（自动转义、命名空间）
- QXmlStreamReader 状态机解析（readNext + isStartElement/isEndElement）
- QJsonDocument 解析 JSON 并分析内存开销（QJsonDocument::toJson 的 Compact 格式）
- QCborValue/QCborMap 二进制 JSON 写入和读取
- 错误处理：QXmlStreamReader::error()、QJsonParseError
