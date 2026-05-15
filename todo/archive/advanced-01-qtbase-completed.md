> 以下进阶层 01-qtbase 文章已全部完成，归档于 (2026-05-10)

## 01 · QtBase（16 篇）

- [x] 🔴 `01-qobject-property-system-advanced.md` — QObject 属性系统进阶：Q_PROPERTY 与动态属性 ✅ 完成于 2026-05-10
  - `Q_PROPERTY` 完整语法：READ/WRITE/NOTIFY/RESET/STORED
  - `setProperty()` / `property()` 动态属性（运行时添加）
  - 属性变化通知 NOTIFY 信号的最佳实践
  - `QMetaObject::propertyCount()` 反射枚举所有属性

- [x] 🔴 `02-signal-slot-advanced.md` — 信号槽进阶：连接类型、Lambda 陷阱、跨线程 ✅ 完成于 2026-05-10
  - `Qt::BlockingQueuedConnection` 使用场景与死锁风险
  - Lambda 捕获对象指针的生命周期陷阱
  - `QObject::connect` 返回 `QMetaObject::Connection` 手动管理
  - `QSignalSpy` 单元测试中验证信号触发

- [x] 🔴 `03-qstring-advanced.md` — QString 进阶：编码陷阱、QStringView、性能优化 ✅ 完成于 2026-05-10
  - Latin-1 / UTF-8 / UTF-16 隐式转换陷阱
  - `QStringView` 零拷贝字符串视图减少内存分配
  - `QString::arg()` 多参数替换的正确顺序
  - `QStringBuilder` (`%` 操作符) 拼接性能优化

- [x] 🔴 `04-containers-advanced.md` — 容器进阶：隐式共享、算法、STL 互操作 ✅ 完成于 2026-05-10
  - COW（写时复制）何时触发拷贝，如何避免意外深拷贝
  - `<QtAlgorithms>` / `std::` 算法在 Qt 容器上的使用
  - `QList` ↔ `std::vector` 互转的零拷贝技巧
  - `QHash` 自定义键类型：重载 `operator==` 与 `qHash`

- [x] 🟡 `05-qvariant-metatype-advanced.md` — QVariant 进阶：自定义类型注册 ✅ 完成于 2026-05-10
  - `Q_DECLARE_METATYPE(T)` + `qRegisterMetaType<T>()` 完整流程
  - 自定义类型在信号槽跨线程传递的注册要求
  - `QVariant::fromValue<T>` / `qvariant_cast<T>` 类型安全存取
  - `QMetaType` 反射能力：构造/析构/比较自定义类型

- [x] 🔴 `06-memory-management-advanced.md` — 内存管理进阶：智能指针与循环引用 ✅ 完成于 2026-05-10
  - `QSharedPointer` + `QWeakPointer` 打破循环引用
  - `QPointer<T>` 弱指针自动置空于对象销毁（Qt 专属）
  - `QScopedPointer` vs `std::unique_ptr` 的互操作
  - 内存泄漏检测：Valgrind / AddressSanitizer 实战

- [x] 🔴 `07-event-system-advanced.md` — 事件系统进阶：自定义事件与过滤器 ✅ 完成于 2026-05-10
  - 自定义事件类：继承 `QEvent` + `QEvent::registerEventType()`
  - `QCoreApplication::postEvent()` 跨线程安全投递自定义事件
  - `eventFilter()` 全局鼠标/键盘监听
  - `QAbstractNativeEventFilter` 截获原生系统消息

- [x] 🔴 `08-file-io-advanced.md` — 文件 IO 进阶：序列化与文件监控 ✅ 完成于 2026-05-10
  - `QDataStream` 二进制序列化：版本兼容策略
  - `QFileSystemWatcher` 监控文件/目录变化
  - `QSaveFile` 原子写入防止数据损坏
  - 内存映射文件 `QFile::map()` 大文件处理

- [x] 🔴 `09-qthread-advanced.md` — 多线程进阶：线程池与 QtConcurrent ✅ 完成于 2026-05-10
  - `QThreadPool` + `QRunnable` 任务队列
  - `QtConcurrent::run()` 将函数异步提交线程池
  - `QFuture<T>` + `QFutureWatcher<T>` 异步结果监控
  - `QReadWriteLock` 读写锁优化多读少写场景

- [x] 🟡 `10-qprocess-advanced.md` — QProcess 进阶：异步读写与进程间通信 ✅ 完成于 2026-05-10
  - `setProcessChannelMode(MergedChannels)` 合并 stdout/stderr
  - `readyReadStandardOutput` 异步流式读取大输出
  - 管道通信：将一个进程输出作为另一个的输入
  - 进程崩溃检测与退出码分析

- [x] 🟡 `11-qtimer-advanced.md` — 定时器进阶：高精度计时与性能分析 ✅ 完成于 2026-05-10
  - `QTimer::timerType()` 三种精度级别选择
  - `QElapsedTimer` 纳秒级性能计时
  - 定时器聚合：避免大量小定时器的性能问题
  - `QDeadlineTimer` 超时控制在异步 API 中的应用

- [x] 🟡 `12-plugin-advanced.md` — 插件系统进阶：版本管理与热加载 ✅ 完成于 2026-05-10
  - 插件接口版本控制防二进制不兼容
  - `QPluginLoader::unload()` 热卸载与资源释放顺序
  - `QDir::entryList()` 自动发现插件目录
  - 插件依赖链管理（插件 A 依赖插件 B）

- [x] 🟡 `13-i18n-advanced.md` — 国际化进阶：复数规则与动态语言切换 ✅ 完成于 2026-05-10
  - 复数形式 `%n` 与各语言规则差异
  - 运行时动态切换语言（`QTranslator` 重新安装）
  - `QLocale` 数字/日期/货币的本地化格式
  - `lupdate` 扫描范围配置与过期字符串清理

- [x] 🟡 `14-logging-advanced.md` — 日志进阶：自定义处理器与分类过滤 ✅ 完成于 2026-05-10
  - `qInstallMessageHandler` 自定义日志输出到文件
  - `QLoggingCategory` 多模块独立日志开关
  - Release 构建保留 qWarning 但禁用 qDebug
  - 结合 spdlog 构建工程级日志系统

- [x] 🟡 `15-regex-advanced.md` — 正则进阶：命名捕获与性能分析 ✅ 完成于 2026-05-10
  - 命名捕获组 `(?P<name>...)` 提高可读性
  - `QRegularExpression::optimize()` JIT 预编译
  - 全局匹配 `QRegularExpression::globalMatch()` 迭代所有匹配
  - 灾难性回溯（Catastrophic Backtracking）预防

- [x] 🟡 `16-json-xml-advanced.md` — JSON/XML 进阶：流式处理大文件 ✅ 完成于 2026-05-10
  - `QJsonDocument` 解析大 JSON 的内存开销分析
  - 流式 JSON 构建：手动拼接 vs `QJsonDocument` 性能对比
  - `QXmlStreamWriter` 生成格式化 XML
  - `QXmlStreamReader` 状态机解析嵌套 XML 结构
