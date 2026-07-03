# 实例库批量推进 · 累计清单（2026-06-27 起，作者远门期间）

> AI 自主推进 app/model 栏成品。本文件两栏累计，作者回来一眼审完：
> - **🟡 待拍板**：AI 拿不准、用默认推进了的设计决策（功能边界 / 依赖取舍 / 命名），作者定夺
> - **🟢 已验证**：每件的构建 / offscreen / 对抗 review / 链接 结论
>
> 推进纪律：每件过构建门(零 warning) + offscreen/review 验证 + 链接 0 broken + 本地 commit(无 AI 署名，不 push)。

## 🟢 已验证

### image-viewer ✅（app 栏·commit 20238db·分支 instance/app-image-viewer）
- 构建：从 app/ 层 cmake，零 warning 零 error
- offscreen 像素验证：原图上红下蓝 ✓ / 旋转 90° 左蓝右红（顺时针方向正确）✓ / 缩放 2x 画布 800x600 ✓
- 对抗 review：3 维度（Qt API / 交互边界 / 生命周期），核心渲染变换 + 对象树 + 生命周期确认正确无崩溃级 bug；修正 6 处交互/边界 risk（navigate 循环跳坏图·成功才提交游标 / 幻灯片态坏图静默 / 构造顺序 setupCentral 提前 / 手动翻页重置 timer / enterSlideshow 守 current_index_ / exitSlideshow 恢复 maximized）
- 链接：check_links 全库 0 broken
- 文档：Full 导览 + Handbook 5 文件（app 栏双文档范式首立）

### json-editor ✅（app 栏 01-dev-tools·本批·构建+offscreen+review 全过）
- 构建：gate 零 warning（对抗 review 修 7 处后重建）
- offscreen：树节点数/Format round-trip/Compact/bad-JSON 清空 全 PASS，无 stderr warning
- 对抗 review（2 维度）：修 3 真 bug——①整数判定 qFuzzyCompare 对 |d|≥1e12 量级失效→改精确判定 d==floor(d)&&isfinite + qint64 范围检查(避 UB) ②static_cast<qint64> 越界 UB ③Save 在 QTextStream flush 前 close QFile(缓冲截断)；+3 risk(Save 补 .json 后缀/parseError offset 标 @byte/光标注释)+1 nit(构造注释)

### sqlite-browser ✅（app 栏 10-database-tools·本批·构建+offscreen+review 全过）
- 构建：gate 零 warning（对抗 review 修 7 处 + static 函数 this 误用修正后）
- offscreen：表列表 [other,t]/选中 t 行数 3/状态栏 PASS；**removeDatabase "connection still in use" warning 已消除**（作用域修复生效，stderr 干净）
- 对抗 review（2 维度，带 qt_src 实证）：修 2 真 bug——①closeDatabase+openDatabase 的 removeDatabase 时局部 QSqlDatabase 副本仍活(Qt 反模式,实证 qsqldatabase.cpp:140)→嵌套作用域 ②rowCount 对 >255 行偏低(SQLite QuerySize=false,prefetch 255)→fetchMore 拉全量；+3 risk(连接名加 QAtomicInt 序号防多窗口撞名/OnFieldChange 切表前 submitAll/错误文本统一 err.text())+1 nit

### serial-tool ✅（app 栏 02-network-tools·本批·构建+offscreen(UI/配置)+review 全过）
- 构建：gate 零 warning（3 轮：Qt6 remove(char)/-Wshadow/全过；-Wall -Wextra -Wpedantic -Wshadow）+ 从 app/ 层构建四件绿
- offscreen：UI/配置逻辑 PASS（6 combo/3 button/4 radio + dataBits 4 项/parity 3 项/flow 2 项 + Hex 切换；本机 8 真实端口）。**真实串口收发无法 offscreen 验证（无硬件）**，靠 build 门 + 对抗 review
- 对抗 review（2 维度，带 qt_src 实证）：修 2 真 bug——①fromHex 对非 hex 字符是「跳过」非报错(实证 qbytearray.cpp:4641)混入非法字符静默截断→发送前正向校验(去空白+全 hex+偶数长度) ②write 只入队未 flush(close 时 pending 字节可能丢)→write 后 flush；+3 risk(onPortError 不可恢复错误主动 close 收敛/dataBits currentText().toInt()→currentIndex/errorString stale→errorToString 自带映射)

### network-tool ✅（app 栏 02-network-tools·第二批·构建+offscreen(loopback 真验)+review 全过）
- 构建：gate 零 warning + 从 app/ 层构建六件绿（demo 自带 find_package Network）
- offscreen：**真 loopback 验证**（无硬件限制）——TCP Server listen port 0→serverPort()→Client connect→write "ping"→Server RX=4→echo "pong"→Client TX=4；UDP bind+writeDatagram 自收 RX=9；累计 13
- 对抗 review（2 维度）：修 3 真 bug——①UDP target 控件隐藏只走 loopback 发不了外部→need_target 对 UDP 也 true ②客户端 socket 无父退出泄漏→setParent(this) ③Server 广播 TX 只计一份+中途失败抹成功字节→循环累加每 client 实发；+2 risk(Client 断开未禁 send→禁用+重连恢复/teardownAll 缺对称 disconnect→补)

### tetris ✅（app 栏 08-games·第二批·构建+offscreen(渲染/下落/暂停)+review 全过）
- 构建：gate 零 warning（1 轮）+ 从 app/ 层构建六件绿
- offscreen：渲染/下落/计分/移动/暂停链路通（初始 4 格→硬降 8 格→score 36→P 暂停 isPaused=1）
- 对抗 review（2 维度）：修 1 真 bug——lockPiece 丢弃 br<0 格子(漏判顶出)→顶出 game over；+2 risk(Key_P/Key_R 三处重复绑定→棋盘保留 Key_P 兼容 offscreen + window 删死代码 + R 交 QAction/statsChanged 每次 softDrop 落盘→推迟 gameOver+closeEvent)

### cpu-memory-monitor ✅（app 栏 04-system-tools·第三批·构建+offscreen(Linux /proc)+review 全过）
- 构建：gate 零 warning（-Wall -Wextra -Wpedantic）+ 从 app/ 层构建七件绿
- offscreen（Linux）：内存 52% **精确匹配** /proc 复算（diff=0）、CPU 4% vs 独立 3%（采样抖动合理）、断笔填充重写后正常渲染
- 对抗 review（2 维度）：修 1 渲染 risk（断笔处半透明填充越过缺口→按连续段分组每段单独 fill）+ 1 防御 risk（readCounters 单共享 ok 串接→必备字段独立校验，Qt6 实测复现畸形字段静默变 0）；+nit(startsWith("cpu")→"cpu "/clear 死代码删)
- **⚠ Windows 路径 100% 未验**：GlobalMemoryStatusEx(内存) + GetSystemTimes(CPU) 用 #ifdef Q_OS_WIN 隔离，Linux offscreen 不参与编译/执行，代码按 Win32 文档写（dwLength/kernel-idle 扣减/ULARGE_INTEGER）但需作者 Windows 实机复验 3 点：dwMemoryLoad 返回 / GetSystemTimes 差值对任务管理器 / kernel32 链接

## 🟡 待拍板（AI 用默认推进，作者回来定夺）

### json-editor
- **树结构**：用虚拟 (root) 顶层（顶层恒 1，doc 互链「顶层节点数」语义稳定）vs 扁平树（object 键直接当顶层）。AI 取虚拟 root；作者若要扁平改 populateTree 去 root 一层（offscreen expected_top 从 1 改 N）
- **Format/Compact 工具栏图标**：QStyle 无贴切 SP_ 图标，只有文字（菜单/工具栏都能用）。作者若要图标需引外部 svg（本成品零外部资源）
- **公共驱动接口** loadTextAndValidate / topLevelNodeCount / editorText：harness + 文档演示用。editorText 纯 harness 粗校验，可删（删则 harness 不测 compact 换行）

### sqlite-browser
- **public openDatabase(filePath)**：harness + 程序化打开用（非为测试硬塞的 private）。作者若不想整机 demo 暴露此 API 可改 private（harness 改走 QSqlDatabase 直验）
- **10-database-tools 新类目命名**：AI 按 registry 10-database-tools 立。作者若有命名约定（如 qt-sql/）可改路径
- **编辑回写 + 任意 SQL 执行未 offscreen 测**：靠 build 门 + 代码审覆盖（offscreen 模拟单元格编辑/SQL 执行较脆）。状态栏「3 row(s)」已证 loadTable→select 真链路通
- **多语句 SQL 被 SQLite 驱动拒绝**（SQLITE_MISUSE）：非 bug，文档/troubleshooting 已注明「仅支持单条 SQL」

### image-viewer（补充，commit 时未记）
- 无遗留待拍板（已审结合入范式）

### serial-tool
- **真实串口收发未 offscreen 验证**（无硬件环境）：UI/配置/编解码逻辑已验，收发靠 build+review+代码审。如需更强覆盖可加 socat 虚拟串口对(PTY)集成测（超本 demo 范围）
- **port_ 单例**：整生命周期持有 QSerialPort（构造 new、析构 close、open/close 复用），不反复 new/delete
- **Open/Close 单按钮两态**（非两按钮）+ 开态锁死所有配置控件
- **波特率 combo setEditable**：用户可手输任意合法波特率（如 230400）
- **ASCII 渲染 Latin-1 保底**（非 toUtf8，防丢非可打印字节）——控制字符(CR/NUL)显示可能失真，要逐字节精确请切 Hex
- **SoftwareControl 软流控未列**（少用）——combo 只 None/Hardware
- **suppress_error_ flag** 挡主动 close 的同步噪声，挡不住 close 后异步 errorOccurred（review risk，多数情况够用）
- **errorOccurred 不自动重连**：调试助手定位（非自动重连客户端），不可恢复错误主动 close 回关闭态

### network-tool
- **byte 计数会话级累计**（切协议不清零）——符合调试助手「一趟会话总流量」直觉
- **UDP target 留空默认 loopback**（发自己 bind 端口，单窗口自测）；填了按填的发外部
- **TCP Server 发送=广播所有客户端**（调试助手典型用法，非选某一个）
- **运行态用 server/socket 是否非空判断**（非按钮文案）+ 运行中锁协议切换/配置
- **onTcpDisconnected 用 lambda 捕获 socket 区分 Client 自身断开 vs Server 某客户端断开**（略有 hack 味，比 sender() 明确）
- **WebSocket 留进阶未做**（首版 TCP/UDP）

### tetris
- **旋转态全预排静态表**（不运行时转置，杜绝方向错位）——代价 7×4×16=448 int 静态表
- **投影按整块行偏移算 ghost**（非单格，缝隙形 S/Z/T 才正确）
- **board 只存类型索引 type+1**（颜色不入 board，查 shapeOf 取色）——0=空，1..7=方块
- **6 步壁踢（非 SRS 18 步）**——实测防卡墙够用，竞技级要 SRS
- **速度调参非标准**（700ms 起，每级 -60ms，下限 80ms）——非 Tetris Guideline 帧表
- **7-bag 随机未实现**（每次独立随机 0..6，理论可连出同型）——竞赛用 7-bag 每 7 个全形态
- **消行未单测**（clearFullLines 逻辑人工核验，offscreen 难构造满行）
- **Key_P 棋盘保留**（兼容 offscreen 直接投递 + 真实 UI QAction 先吃不冲突）；Key_R 交 QAction

### cpu-memory-monitor
- **⚠ Windows 路径 100% 未验**（核心待拍板）：GlobalMemoryStatusEx + GetSystemTimes 用 #ifdef Q_OS_WIN 隔离，offscreen 在 Linux 跑不到。需作者 Windows 实机复验 3 点（见 🟢 已验证栏）。代码 + 文档已统一标「尚未验证」
- **CpuSampler 滚动基线**（非固定基线）：utilization 每次把本次读数滚成下次 prev_，CPU% 反映「最近一间隔」负载；构造 sample() 拿 t0，第一拍 onTick 即出短窗差值（非 N/A）
- **CPU 历史曲线 CpuHistoryView**：60 点滚动窗口（约 60s）、QPainterPath 折线+半透明填充+25/50/75/100% 网格、无效点(-1)断笔（填充也断，不越过缺口）
- **进度条 setTextVisible(false) + QLabel 数值**（内存「52% (6.09 GB / 11.68 GB)」、CPU「3%」）
- **采样失败 -1 占容量槽**（长时间失败 60 槽被 -1 填满，恢复需 60 拍挤净）——边界降级，文档说明
- **mac/未覆盖平台 fallback** valid=false（UI 显示 N/A，不崩）

---
