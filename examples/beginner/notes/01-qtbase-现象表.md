# 01-qtbase 代码样例现象表

> 以下为各示例运行时的预期现象描述，供逐个核实时对照。

## 01-qobject-meta-system-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 基本 QObject 和 Q_OBJECT | 控制台输出：打印 "创建 SimpleObject: 简单对象"，然后打印对象名称 "简单对象"，再通过 `metaObject()->className()` 输出类名 "SimpleObject" |
| 2 | 属性系统动态读写 | 通过 `setProperty("name", ...)` 动态修改属性值，随后 `property("name")` 读回，控制台打印 "修改后的名称: 修改后的名称" |
| 3 | 对象树自动内存管理 | 在作用域内创建父对象和两个子对象，打印 "父对象有 2 个子对象"；离开作用域时父对象析构，自动连带删除两个子对象，依次打印 "销毁 SimpleObject: 子对象1"、"销毁 SimpleObject: 子对象2"（子对象先于父对象销毁），随后打印 "父对象已销毁，所有子对象也被清理" |
| 4 | qobject_cast 类型转换 | 将 `QObject*` 指针 `qobject_cast` 向下转型为 `SimpleObject*` 成功，打印 "qobject_cast 转换成功: 转换测试"；转型为不兼容的 `TaskItem*` 失败返回 `nullptr`，打印 "qobject_cast 转换失败（类型不兼容），返回 nullptr" |
| 5 | 任务管理器（对象树实战） | 创建 TaskManager 并添加三个 TaskItem，打印所有任务列表（标题、优先级、完成状态）；修改 task1 的 completed 为 true、task2 的 priority 为 0 后，通过元对象系统遍历并打印各自的所有 Q_PROPERTY 属性（title、priority、completed 及其当前值） |
| 6 | 动态属性 | 给 task3 设置未在 Q_PROPERTY 中声明的动态属性 "assignee"，值为 "张三"；检查 `dynamicPropertyNames()` 包含 "assignee" 后打印 "动态属性 assignee: 张三" |
| 7 | 程序退出析构 | main 返回时 TaskManager 析构，打印 "销毁 TaskManager，所有子任务将自动清理"，随后三个 TaskItem 按逆序析构，各打印 "销毁 TaskItem: ..." |

## 02-signal-slot-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 基础信号槽连接 | 创建 Counter 和 SlotReceiver，用函数指针语法 connect；调用 `increment()` 两次再 `reset()`，控制台依次输出 "[槽函数调用] 值改变为: 1 (第 1 次)"、"值改变为: 2 (第 2 次)"、"值改变为: 0 (第 3 次)" |
| 2 | Lambda 表达式作为槽 | Counter2 的 valueChanged 连接到一个捕获局部变量引用的 Lambda；两次 increment 后分别打印 "[Lambda] 捕获到值变化: 1  (Lambda 调用次数: 1)"、"值变化: 2  (Lambda 调用次数: 2)" |
| 3 | 一个信号连接多个槽 | 同一个信号连接到 receiver1、receiver2 的 onValueChanged 以及一个额外 Lambda；触发一次 increment 后，三个槽均被调用，依次打印 receiver1 的 "值改变为: 1 (第 1 次)"、receiver2 的 "值改变为: 1 (第 1 次)"、"[额外 Lambda] 也收到通知: 1" |
| 4 | 连接的管理与断开 | 保存 `QMetaObject::Connection` 对象；第一次 increment 触发槽，打印 "[槽函数调用] 值改变为: 1 (第 1 次)"；调用 `disconnect(conn)` 后再 increment，无槽函数输出，打印 "断开连接后再触发信号:" |
| 5 | QTimer singleShot | 使用 `QTimer::singleShot(500, ...)` 创建单次定时器，主线程通过 `processEvents()` 循环等待；500ms 后 Lambda 触发，打印 "[定时器] 单次触发 Tick 1" |
| 6 | 跨线程信号槽 | 创建 QThread 并将 Worker 通过 `moveToThread` 移入；线程启动后执行 doWork，控制台打印 "[工作线程] doWork 在线程 0x... 中执行"，随后主线程槽收到 workFinished 信号，打印 "[主线程] 收到工作结果: 工作完成！"；线程自动退出 |
| 7 | Lambda 捕获类成员 | LambdaDemo 的 setupConnections 将 timer2 的 timeout 连接到两个 Lambda（一个捕获 this 访问成员计数器，一个捕获局部 prefix 字符串）；通过 processEvents 触发后打印 "[Lambda] 定时器触发，计数: 1" 和 "[Lambda] 定时器 持续运行中..." |

## 03-string-encoding-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | QString vs QByteArray | 打印 QString "你好世界" 的 length 为 4（字符数）；转为 UTF-8 后 QByteArray 长度为 12（每个汉字 3 字节），并以十六进制打印字节值（如 "e4 bd a0 e5 a5 bd e4 b8 96 e7 95 8c"）；再从 UTF-8 字节还原回 QString 成功 |
| 2 | QStringView 零拷贝 | 从 QString "Hello, Qt 6!" 创建 QStringView，打印视图内容与 size（13）；从字面量 `u"String literal"` 创建视图；`mid(7, 3)` 截取子串视图 "Qt"，全程零拷贝 |
| 3 | 常用字符串操作 | split 按逗号分割 "apple,banana,cherry,date" 得到 QStringList；contains 检查 "cherry" 返回 true；indexOf 定位 "banana" 在位置 6；replace 将 "banana" 替换为 "blueberry"；arg 格式化输出 "用户 张三 登录成功，状态码 200"；`QString::number(3.14159, 'f', 2)` 输出 "3.14"，toDouble 还原为 3.14 |
| 4 | 编码转换 | UTF-8 往返转换成功打印 "中文测试 Hello World"；使用 QStringDecoder/QStringEncoder 尝试 GBK 编码，若系统支持则解码 "\xd6\xd0\xce\xc4" 得到 "中文"，否则打印 "GBK encoding not supported"；Latin1 转换打印 "From Latin1: Hello" |
| 5 | QStringLiteral 编译期优化 | 分别用运行时构造和 `QSqlQueryLiteral` 构造 QString，打印内容分别为 "Runtime string" 和 "Compile-time string" |
| 6 | 常见坑点 | 错误方式：QByteArray "你好" 直接构造 QString 会按 Latin1 解析产生乱码；正确方式：`QString::fromUtf8()` 得到正确中文。`QString::number(pi)` 默认精度 6 位输出 "3.14159"；指定 10 位小数输出 "3.1415926535" |

## 04-container-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | QList 基本操作 | 初始列表 "0 1 2 3 4 5"（实际先初始化 {1,2,3,4,5}，再 append(6) 变 {1,2,3,4,5,6}，再 prepend(0) 变 {0,1,2,3,4,5,6}）；insert(2, 99) 后变为 {0,1,99,2,3,4,5,6}；at(3) 和 operator[](3) 均返回 2；底层指针首元素为 0；范围 for 遍历打印全部元素 |
| 2 | QMap 有序映射 | 插入 Alice(95)、Charlie(88)、Bob(87) 后按 key 字母序迭代输出 Alice: 95、Bob: 87、Charlie: 88；`contains("Alice")` 为 true；查找不存在的 "David" 使用 `value("David", -1)` 返回 -1，不会插入新键 |
| 3 | QHash 哈希表 | 插入相同数据但迭代顺序不确定（取决于哈希值）；演示 operator[] 陷阱：访问不存在的 "David" 会自动插入默认值 0，之后 contains("David") 变为 true；正确查询用 `value("Eve", -1)` 返回 -1 且不插入 |
| 4 | QSet 集合去重 | 从 {1,2,2,3,3,3,4,4,4,4} 构造 QSet 得到去重后的 {1,2,3,4}（顺序不定）；set1={1,2,3,4} 与 set2={3,4,5,6} 做交集得 {3,4}、并集得 {1,2,3,4,5,6}（注意 intersect/unite 会修改原集合） |
| 5 | 隐式共享（写时复制） | list1 和 list2 赋值后共享同一内存地址；修改 list2[0]=99 触发 detach（写时复制），此后 list1 仍为 {1,2,3,4,5}，list2 变为 {99,2,3,4,5}，两者地址不同 |
| 6 | 字符频率统计 | 对 "hello world" 统计每个字符出现次数，如 h:1、e:1、l:3、o:2、' ':1、w:1、r:1、d:1（空格也被计入） |

## 05-variant-type-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | QVariant 基础用法 | 分别存储 int、double、bool、QString、QColor 并用 toInt()/toDouble()/toBool()/toString()/value<QColor>() 取回，打印对应值；空的 QVariant isValid() 返回 false |
| 2 | 类型检查的重要性 | 字符串 "123" 的 toInt() 看似正确返回 123；但 "hello" 的 toInt() 返回 0（错误）；正确做法：先用 `type()` 检查是否为 String，或用 `canConvert<int>()` 加 `toString().toInt(&ok)` 确认转换成功；`typeId() == QMetaType::Int` 可用于判断具体类型 |
| 3 | 容器类型与 QVariant | QVariantList 存储 int、string、double、bool 后遍历，按类型分别打印 "Int: 1"、"String: two"、"Double: 3.14"、"Bool: 1"；QVariantMap 存储混合类型配置并按键取出打印 |
| 4 | 自定义类型与 QVariant | 用 `Q_DECLARE_METATYPE(Person)` 注册自定义结构体，通过 `QVariant::fromValue()` 存入，`value<Person>()` 取出；typeName() 返回 "Person"；可存入 QList<QVariant> 并遍历 |
| 5 | 配置系统实战 | 用 QVariantMap 模拟配置系统，存储窗口尺寸、标题、全屏开关、主题颜色、网络超时等；通过 getConfig Lambda 统一读取（支持默认值），打印所有配置项；修改后打印新值 |
| 6 | 类型转换陷阱 | "abc" 转 int 返回 0（无错误提示），正确做法需用 ok 参数检查；3.99 转 int 截断为 3（精度丢失）；int 2 转 bool 为 true，bool true 转 int 为 1 |

## 06-memory-management-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 对象树自动清理 | 在 ObjectTreeDemo 内创建 parent（父对象为 this），再以 parent 为父创建三个 Task；当 ObjectTreeDemo 离开作用域时，打印 "[Task] 销毁任务: 任务 C"、"任务 B"、"任务 A"（子对象逆序析构），随后 parent 析构 |
| 2 | QSharedPointer 共享所有权 | 创建共享指针 task1 指向 "共享任务 A"（引用计数 1）；内层作用域复制为 task2（引用计数 2），task2 调用 execute；内层结束后 task2 析构（引用计数回到 1）；再创建独立 task3 指向 "共享任务 B"；函数结束后两个共享指针析构，对象自动删除，打印 "[Task] 销毁任务: 共享任务 B"、"共享任务 A" |
| 3 | QWeakPointer 弱引用 | 创建强引用指向 "弱引用演示任务"，创建弱引用不增加引用计数；通过 `toStrongRef()` 锁定成功并访问对象；清除强引用后，弱引用 isNull() 为 true，打印 "弱引用已失效（对象已被删除）"；Task 对象在强引用清除时即被析构 |
| 4 | QPointer QObject 弱引用 | 创建 Task 的原始指针并用 QPointer 包装，访问正常；手动 delete 原始指针后，QPointer 自动置空，`if (safePtr)` 判断为 false，打印 "QPointer 已自动置空，避免悬空指针" |
| 5 | 循环引用（错误与正确） | 错误示例：NodeA 和 NodeB 互相持有 QSharedPointer，离开作用域后引用计数各减为 1 而不会归零，NodeA 和 NodeB 的析构函数均不会被调用（内存泄漏）；正确示例：NodeACorrected 强引用 NodeBCorrected，NodeBCorrected 用 QWeakPointer 弱引用对方，离开作用域后两者正常析构，打印 "[NodeACorrected] 析构" 和 "[NodeBCorrected] 析构" |

## 07-event-system-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 基础事件处理 | 向 MyEventHandler 发送模拟的 KeyPress(A) 事件，打印 "[event() KeyPress] 按键: A 键码: 65 修饰键: 0" 及 "事件已 accept()"；将 acceptEvents 设为 false 后发送 KeyPress(B)，打印 "事件已 ignore()，将继续传播"；发送 KeyRelease(A) 事件，打印 "[event() KeyRelease] 释放按键: A" |
| 2 | 事件过滤器 | 在 target 上安装 KeyLogger 事件过滤器；发送 KeyPress(A) 时，过滤器先拦截打印 "[EventFilter] 拦截到来自 TargetObject 的按键: A"，然后 target 的 event() 也处理；发送 KeyPress(Escape) 时，过滤器拦截并返回 true（打印 "ESC 键被拦截，目标对象不会收到此事件"），target 不处理；禁用过滤器后发送 KeyPress(B)，事件正常通过 |
| 3 | postEvent vs sendEvent | sendEvent 同步发送自定义 User 事件，立即打印 "[Receiver] 收到 User 事件" 及当前时间，随后打印 "[sendEvent] 返回"；postEvent 异步发送，立即返回并打印 "事件已加入队列，稍后处理"；进入事件循环后约 100ms 定时器触发，异步事件被处理；最后打印 "[Timer] 事件循环处理完毕" 后退出 |

## 08-file-io-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | QFile 基础文本读写 | 创建 example_text.txt 写入三行文本，打印 "文本文件写入完成"；读取时先 readAll 打印全部内容，再逐行读取打印 "行1: Hello, Qt File IO!"、"行2: 这是第二行文本。"、"行3: 中文内容测试：Qt 的文本处理很强大。" |
| 2 | QTextStream 流式文本 | 以流式操作写入 config.ini，内容包含 [Server] 节（host/port/timeout/debug）和 [Database] 节（driver/path）；读取时逐行打印非空行 |
| 3 | QDataStream 二进制序列化 | 向 data.bin 写入 int 42、double 3.1415926、QString "Hello, Binary!"、QByteArray "ABC"、QList<int> {10,20,30,40,50}、QStringList {Alice,Bob,Charlie}；按写入顺序读回并打印，值与写入时一致 |
| 4 | QDir 目录操作 | 用 mkpath 递归创建 test_data/subdir1/deep 目录；在 test_data 下创建 file1.txt 和 file2.md，在 subdir1 下创建 file3.txt；遍历 test_data 打印目录内容（含 [DIR] subdir1 和 [FILE] file1.txt、file2.md）；用 "*.txt" 过滤后只列出 file1.txt |
| 5 | QFileInfo 文件信息查询 | 创建 test_info.txt 后查询：文件名 "test_info.txt"、后缀 "txt"、基础名称 "test_info"、大小（字节数）、可读写属性、创建/修改/访问时间；演示 archive.tar.gz 的 suffix() 返回 "gz"、completeSuffix() 返回 "tar.gz" |
| 6 | 简单日志管理器 | 在 logs/ 目录下创建按日期命名的日志文件，写入 5 条不同级别日志（INFO/DEBUG/WARN/INFO/ERROR），每条带时间戳和级别标签；读取并打印当天所有日志 |

## 09-multithreading-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | QThread + moveToThread | Worker 通过 moveToThread 移入新线程；线程启动后 doWork 在子线程执行，每秒发一次 progressChanged 信号（20%/40%/60%/80%/100%），主线程槽打印进度；5 秒后 workFinished 触发，主线程打印 "主线程收到结果: 后台任务1 任务完成！"，线程退出并清理资源 |
| 2 | QThreadPool 线程池 | 设置最大线程数 4，提交 8 个 SimpleTask；由于线程池最多并行 4 个，前 4 个任务立即开始，每个 sleep(1) 秒后完成，后续任务接力；所有任务完成后打印 "示例 2 完成"；QRunnable 默认自动删除 |
| 3 | QtConcurrent::run | 在后台线程执行 heavyCalculation(10)，每次迭代 sleep 100ms 并累加 i*i；约 1 秒后完成，QFutureWatcher 的 finished 信号触发，主线程打印 "主线程收到结果: 285"（0^2+1^2+...+9^2 = 285） |
| 4 | QMutex 线程安全 | 10 个线程任务各对 SafeCounter 执行 1000 次 increment（共 10000 次）；由于 QMutexLocker 保护，最终计数正确打印 "最终计数: 10000"，并输出 "正确！多线程访问安全" |
| 5 | 跨线程通信正确做法 | 再次演示 moveToThread 模式，Worker 执行 doWork("跨线程测试")，主线程通过信号槽安全接收进度更新，打印 "主线程安全地收到进度更新: XX %"；完成后线程正常退出 |

## 10-qprocess-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 同步启动进程 | 阻塞执行 `echo "Hello from QProcess!"`，waitForFinished 后读取 stdout 打印 "输出: Hello from QProcess!"，退出码为 0 |
| 2 | 异步启动进程 | 非阻塞启动 `sleep 2`（Linux）或 `timeout /t 2`（Windows）；通过 readyReadStandardOutput 信号实时获取输出；2 秒后 finished 信号触发，打印 "进程正常结束，退出码: 0" |
| 3 | 分离式启动进程 | 调用 `QProcess::startDetached` 启动 xdg-open（Linux）/ open（macOS）/ notepad.exe（Windows），打开 /tmp 目录或记事本窗口，打印 "已独立启动程序: ..."；父进程退出后子进程继续运行 |
| 4 | 工作目录与环境变量 | 设置工作目录为 /tmp，插入自定义环境变量 MY_CUSTOM_VAR="Hello from Qt!"；启动 sh 打印该变量，输出 "环境变量输出: Hello from Qt!" |
| 5 | 向进程写入数据（管道） | 启动 `grep Hello`，通过 write 向其 stdin 写入 "Hello World\n" 和 "Goodbye World\n"，关闭写入通道；grep 过滤后只输出包含 "Hello" 的行，打印 "过滤结果: Hello World" |
| 6 | 定时退出 | QTimer 3 秒后调用 `QCoreApplication::quit`，确保异步示例有足够时间完成 |

## 11-timer-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 基础重复定时器 | 创建 500ms 间隔的 QTimer，通过 QEventLoop 阻塞等待；每 500ms 打印 "[重复定时器] Tick 1" 到 "Tick 5"，5 次后调用 stop() 并打印 "[重复定时器] 停止" |
| 2 | 单次定时器 | 方式一：setSingleShot(true) 加 1000ms 间隔，1 秒后打印 "[单次定时器] 触发（只执行这一次）"；方式二：静态方法 `QTimer::singleShot(3000, ...)` ，3 秒后打印 "[单次定时器] 静态方法触发！" |
| 3 | QElapsedTimer 高精度计时 | 启动 QElapsedTimer 后 sleep 100ms，elapsed() 返回约 100 毫秒；hasExpired(200) 返回 false（未超时）；restart() 返回之前经过的时间；再 sleep 50ms 后打印新的 elapsed 约 50ms；最后打印纳秒精度值 |
| 4 | 定时器生命周期管理 | 创建 parentTimer 和 childTimer（parent 为 parentTimer），分别以 1000ms 和 500ms 间隔运行；2 秒后删除 parentTimer，childTimer 也随之被自动删除，打印 "[生命周期] 删除父定时器，子定时器也会被删除" |
| 5 | 定时器状态查询 | 启动前 isActive() 为 false；启动后为 true；interval() 为 1000；remainingTime() 约为 1000；500ms 后 remainingTime() 约为 500；stop 后 isActive() 为 false、remainingTime() 为 -1 |
| 6 | 模拟秒表 | Stopwatch 对象连接 timeout 信号到 onTick，每秒打印 "[秒表] 00:01" 到 "00:05"；5 秒后暂停打印 "[秒表] 暂停"，然后重置打印 "[秒表] 重置: 00:00" |
| 7 | 定时器精度演示 | 50ms 间隔定时器触发 10 次，每次打印实际经过时间、预期时间和偏差（通常偏差在数毫秒内）；注意偏差会逐渐累积，这是正常现象 |

## 12-plugin-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 自动发现并加载插件 | 扫描 plugins/ 目录下的 .so/.dll/.dylib 文件；对每个文件尝试 QPluginLoader 加载并用 qobject_cast 验证 TextProcessorInterface 接口；成功则打印插件名称和版本，并对文件名调用 process() 展示效果 |
| 2 | 手动加载单个插件 | 指定路径加载第一个插件文件，打印 "加载成功！"；通过 qobject_cast 验证接口后打印插件名称（如 "Upper Case Converter" 或 "Text Reverser"）、版本 "1.0.0"；对 "Hello Qt Plugins!" 调用 process()，Upper Case 插件输出 "HELLO QT PLUGINS!"，Reverse 插件输出 "!sugilP tQ olleH" |
| 3 | 读取插件元数据 | 通过 QPluginLoader::metaData() 读取 JSON 元数据，打印 IID（"org.example.TextProcessorInterface.1.0"）和 className |
| 4 | 错误处理 | 尝试加载不存在的插件路径 "/nonexistent/path/plugin.so"，打印 "预期的错误（文件不存在）: ..." ；若无任何插件文件则打印 "警告：没有成功加载任何插件" |
| 注意 | 插件系统要求 | 需要先编译 UpperCasePlugin 和 ReversePlugin 为动态库并放入 plugins 目录，否则示例 1/2/3 均会报告找不到插件 |

## 13-i18n-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | GUI 窗口展示 | 这是一个 **GUI 应用**（使用 QApplication）。启动后弹出一个 400x200 的窗口，标题为 "Internationalization Demo"（或 "国际化演示"，取决于是否已加载翻译文件） |
| 2 | 默认界面内容 | 窗口中央显示标签 "Hello, World!"；下方显示复数形式标签 "You have 0 message(s)"；一个 "Add Message" 按钮；一个语言选择下拉框（含 "English" 和 "中文" 两个选项）；两个消歧义标签分别显示 "File"（菜单项含义）和 "File"（动词含义） |
| 3 | 点击 Add Message | 每次点击按钮，m_messageCount 递增，标签更新为 "You have 1 message(s)"、"You have 2 message(s)" 等；若已加载中文翻译则显示 "您有 1 条消息"、"您有 2 条消息" |
| 4 | 切换语言到中文 | 下拉框选择 "中文" 后，加载 i18n-example_zh_CN.qm 翻译文件；调用 retranslateUi() 刷新界面：标题变为 "国际化演示"，标签变为 "你好，世界！"，按钮变为 "添加消息"，消歧义标签分别变为 "文件" 和 "归档" |
| 5 | 切换回 English | 下拉框选择 "English" 后，移除翻译器，界面恢复为英文默认文本 |
| 注意 | 翻译文件依赖 | 中文翻译需要先将 .ts 文件用 lrelease 编译为 .qm 文件并放入资源系统（:/translations/），否则切换到中文后仍显示英文，控制台打印 "Failed to load translation: ..." |

## 14-logging-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 基础日志宏 | 依次调用 qDebug、qInfo、qWarning、qCritical，经过自定义消息处理器格式化后输出带时间戳、线程 ID、类别、级别的日志行 |
| 2 | 分类日志 | 使用 Q_LOGGING_CATEGORY 定义 app.main/app.network/app.database/app.performance 四个类别；分别用 qCDebug/qCInfo/qCWarning/qCCritical 输出不同级别的分类日志（默认只有 Info 及以上级别会输出，Debug 级别需加 --verbose 启用） |
| 3 | 检查日志级别 | 通过 `mainLog().isDebugEnabled()` 判断调试日志是否启用；`networkLog().isWarningEnabled()` 判断警告级别是否启用 |
| 4 | NetworkManager 演示 | 模拟连接服务器：前两次失败打印 networkLog 的 Warning "连接失败，这是第 1/2 次重试"；第三次成功打印 Info "连接成功"；downloadData 模拟 100KB 下载，打印耗时和下载速度（perfLog 级别） |
| 5 | DatabaseManager 演示 | connect() 打印 databaseLog Info "数据库连接成功"；空查询打印 Warning "查询语句为空"；DROP 语句打印 Critical "检测到危险操作: DROP 语句" |
| 6 | 多线程日志 | 启动两个 WorkerThread，各运行 3 次循环（每次 msleep 50ms），打印带线程 ID 的日志；自定义消息处理器通过 QMutex 保证文件写入线程安全 |
| 7 | 流式输出 | 演示 qDebug 和 qCDebug 的流式语法，输出混合类型数据 |
| 8 | 条件日志 | 先用 `perfLog().isDebugEnabled()` 检查再执行耗时的统计操作，避免在日志禁用时浪费性能 |
| 全局 | 自定义消息处理器 | 所有日志经 customMessageHandler 格式化为 "[时间][线程ID][类别][级别] 消息"，同时输出到控制台和日志文件 app_YYYYMMDD.log |

## 15-regex-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 基础匹配 | 正则 `\d+` 匹配 "价格：123元，折扣：45%" 中的数字，captured(0) 为 "123"，位置在索引 3，长度 3 |
| 2 | 捕获组 | 正则 `(\d{4})-(\d{2})-(\d{2})` 匹配日期，captured(0) 为 "2025-03-17"，captured(1/2/3) 分别为 "2025"/"03"/"17"；命名捕获组 `(?<year>...)` 可用 captured("year") 等获取 |
| 3 | 全局匹配 | 邮箱正则匹配 "alice@example.com 或 bob@test.org，cc: charlie@company.co.uk"，依次提取三个邮箱地址 |
| 4 | 常用模式 | 从文本中提取 IPv4 地址 "192.168.1.1" 和 "10.0.0.1"；提取 URL "https://www.qt.io" 和 "https://doc.qt.io"；提取颜色代码 "#ffffff"、"#000"、"#FF5733" |
| 5 | 非贪婪匹配 | 贪婪 `.*` 匹配 "<div>内容1</div><div>内容2</div>" 整体；非贪婪 `.*?` 分别匹配 "<div>内容1</div>" 和 "<div>内容2</div>" |
| 6 | 精确匹配 | `\d{3}-\d{4}` 对 "123-4567" 和 "我的电话是123-4567" 都 hasMatch；加 `^...$` 后只有纯电话号码格式通过验证 |
| 7 | 错误处理 | 无效正则 `[abc`（括号不匹配）和 `\`（无效转义）均被 isValid() 检测到，打印错误信息 |
| 8 | 邮箱提取填空题 | 从 "联系我：alice@example.com 或 bob@test.org" 中用 globalMatch 提取 "alice@example.com" 和 "bob@test.org" |

## 16-json-xml-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | JSON 解析 | 解析嵌套 JSON 字符串，提取 app.name="MyQtApp"、app.version="1.0.0"、app.debug=true；提取 features 数组 ["network","database","ui"]；提取 settings.maxConnections=100、settings.timeout=30.5 |
| 2 | JSON 构建 | 用 QJsonObject/QJsonArray API 构建 JSON，打印格式化（Indented）和紧凑（Compact）两种输出；格式化版本带缩进换行，紧凑版本无空白 |
| 3 | JSON 错误处理 | 测试四种无效 JSON（缺闭合括号、值未加引号、键未加引号、末尾多余逗号），每种都打印错误信息和偏移位置；演示安全访问键值：contains() 检查和 value().toString("default") 使用默认值 |
| 4 | JSON 文件读写 | 将配置对象写入 config.json，再读回并打印 "窗口标题: Demo Application"、"宽度: 800"、"高度: 600"、"全屏: false"；测试完毕后删除临时文件 |
| 5 | QJsonValue 类型检查 | 遍历含多种类型的 JSON 对象，打印 string: String、number: Double、float: Double、bool: Bool、null: Null、array: Array、object: Object |
| 6 | JSON 数组提取填空题 | 从 users 数组提取 Alice(30)、Bob(25)、Charlie(35) |
| 7 | XML 解析 | 用 QXmlStreamReader 逐令牌解析图书目录 XML，依次打印 3 本书的 ID/分类、标题、作者、价格、库存状态；最后打印 "图书总数: 3"、"总价格: 120.49" |
| 8 | XML 写入 | 用 QXmlStreamWriter 生成格式化的图书目录 XML（含 XML 声明、注释、嵌套元素和属性），打印生成内容并保存到 catalog.xml（测试后删除） |
| 9 | XML 错误处理 | 解析标签未闭合的无效 XML，打印 "XML 解析错误: Expected character data." 及错误行号和列号 |
| 10 | JSON 转 XML | 将 JSON {person:{name:"Alice",age:30,hobbies:["reading","gaming","coding"]}} 转为 XML 格式输出， hobbies 数组展开为多个 `<hobby>` 子元素 |
