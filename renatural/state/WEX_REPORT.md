# W-EX 示例债盘点报告（base 卷）

- 生成：2026-09-28 · 数据源：6 章 sweep 实跑（134 个示例逐一 `cmake -B build && cmake --build build`）+ discover 覆盖扫描 · 机器数据：`renatural/state/wex-inventory.json`（结构化原始行 + 各章实跑笔记）
- 裁决依据：ROADMAP_BASE_RESTYLE.md M0-6（W-EX 盘点定价）、D4 附带裁决（无 example 篇检查点暂用 B/C）、D9（agent 承接，独立于主线）
- 环境实况：cmake 4.4.3 + gcc 16.2.1 + Qt6 = qt6-base 6.11.2 + qt6-translations（CMake 配置实际在 `/usr/lib/cmake/Qt6`；任务简报所写 `/usr/lib/qt6/cmake` 本机不存在，后续 W-EX 批次的环境简报应予更正）
- 本盘点未改动 `examples/` 下任何文件；各章 sweep 产生的 `build/` 目录留置原地，实测被仓库根 `.gitignore`（`**/build/`）覆盖，git status 干净

## a) 总览

| 章 | 示例数 | 五件套齐全 | build pass | build fail | build skip |
|---|---:|---:|---:|---:|---:|
| examples/beginner/01-qtbase | 16 | 0 | 15 | 0 | 1 |
| examples/beginner/02-qtgui | 6 | 0 | 6 | 0 | 0 |
| examples/beginner/03-qtwidgets | 74 | 0 | 74 | 0 | 0 |
| examples/beginner/04-qtnetwork | 6 | 0 | 4 | 2 | 0 |
| examples/beginner/05-other-modules | 25 | 0 | 3 | 22 | 0 |
| examples/beginner/06-qml | 7 | 0 | 0 | 7 | 0 |
| **合计** | **134** | **0** | **102** | **31** | **1** |

一句话结论：**债不在编译，在结构**。102/134 直接 build pass；31 个 fail 全部死于 configure 阶段的环境缺 Qt 模块 dev 包（代码正确性未被检验，无一是已证实的代码缺陷）。真正的欠账是五件套结构：134/134 缺字面名 `widget.h`+`widget.cpp` 对，118/134 缺自有 `.gitignore`，1/134 缺 `CMakeLists.txt`。main.cpp 134/134 齐备。

## b) 缺件清单（按章 × 每示例）

缺件计数（全卷）：`widget.h` 134 · `widget.cpp` 134 · `CMakeLists.txt` 1 · `.gitignore` 118 · `main.cpp` 0。

**01-qtbase（16 个）**
- 01–07、09–16 共 15 个：缺 `{widget.h, widget.cpp}`（main.cpp / CMakeLists.txt / .gitignore 三件齐）。
- 08-file-io-beginner：缺 `{widget.h, widget.cpp, CMakeLists.txt}` —— 全卷唯一缺 CMakeLists 的示例，也因此是唯一 build skip。
- 形态注记：8 个有领域命名头文件（01 simpleobject/taskitem/taskmanager、02 counter/lambdademo/slotreceiver/worker、06 六个 *demo、07 eventreceiver/keylogger/myeventhandler、11 stopwatch、12 textprocessor_interface+插件、13 i18nwindow、14 databasemanager/networkmanager/workerthread）；7 个为 main.cpp 单文件（03/04/05/08/10/15/16）；09 未被 sweep 笔记逐一点名，开工补齐时现场核对归哪类。

**02-qtgui（6 个）**：01–06 全部缺 `{widget.h, widget.cpp, .gitignore}`，无一例外。均有领域命名替身（01 barchartwidget+shapegallerywidget、02 analogclockwidget+transformdemowidget、03 imagedisplaywidget+pixeldemowidget+simpleimageviewer、04 fontdemowidget+metricslayoutwidget+richtextcardwidget、05 mainwindow+triangleglwidget、06 dragsourcewidget+droptargetwidget）。

**03-qtwidgets（74 个）**：01–74 全部缺 `{widget.h, widget.cpp, .gitignore}`，无一例外。均有类名命名的等价 widget 文件且 CMakeLists 自洽（如 01 mainwindow.h/cpp、34 labeldemowidget.h/cpp、72 MainWindow.h/cpp 大小写混杂），全部可编译。

**04-qtnetwork（6 个）**：01–06 全部缺 `{widget.h, widget.cpp, .gitignore}`（01 echoserver+tcpclient、02 udpsender+udpreceiver、03 httpget/httppost/httpdownloaddemo、04 chatclient+chatserver、05 httpsrequest+tlsprobe 等、06 serialmanager+serialportlist 等）。

**05-other-modules（25 个）**：01–25 全部缺 `{widget.h, widget.cpp, .gitignore}`。其中 15-qtquick3d、16-qtquick3d-physics 连类文件对都没有（仅 main.cpp + Main.qml），其余为类名命名对（Demo、MainWindow、svgviewerwindow 等）。

**06-qml（7 个）**：01–07 全部缺 `{widget.h, widget.cpp, .gitignore}`。04 有 app_controller.h/cpp、06 有 fruit_model.h/cpp，其余五个为 main.cpp + Main.qml 纯 QML 形态。

## c) build 失败分组（按错误类型聚类）

**组 1 · 环境缺 Qt 模块 dev 包 → find_package 在 configure 期失败 —— 31 例**（04-qtnetwork 2 + 05-other-modules 22 + 06-qml 7）。两种措辞、同一根因：
- 代表错误（“Failed to find required Qt component” 型，04-qtnetwork/04-websocket）：
  `CMake Error at CMakeLists.txt:16 (find_package): Failed to find required Qt component "WebSockets". Expected Config file at "/usr/lib/cmake/Qt6WebSockets/Qt6WebSocketsConfig.cmake" does NOT exist`
- 代表错误（“Could NOT find Qt6Xxx” 型，05-other-modules/03-qtcharts-basic）：
  `Could NOT find Qt6Charts (missing: Qt6Charts_DIR); CMake Error at CMakeLists.txt:13 (find_package)`

缺失模块 → 受达示例（均为 configure 期死亡，未到编译）：

| 缺失模块 | 受达示例 |
|---|---|
| Quick / Qml 栈（qt6-declarative 未装） | 05/15、05/16、06-qml 全 7 例（共 9） |
| Multimedia（SpatialAudio 同模块组） | 05/04、05/05、05/23 |
| WebSockets | 04-qtnetwork/04、05/19 |
| SerialPort | 04-qtnetwork/06 |
| Charts / Svg / SerialBus / Mqtt / Bluetooth / Nfc / StateMachine / Scxml / 3DCore / Pdf / HttpServer / WebChannel / WebEngineWidgets / RemoteObjects / TextToSpeech / Core5Compat | 各 1 例：05/03、05/06、05/08、05/09、05/10、05/11、05/12、05/13、05/14、05/17、05/18、05/20、05/21、05/22、05/24、05/25 |

**组 2 · 缺 CMakeLists.txt 无法配置（skip）—— 1 例**：01-qtbase/08-file-io-beginner，`CMakeLists.txt 缺失，无法配置，跳过构建（cmake -B build -S . 无 CMakeLists 输入）`。

**组 3 · 代码级编译失败 —— 0 例**。现有证据下不存在；但组 1 的 31 例代码正确性尚未被检验，须环境补包复跑后才能下最终结论。

## d) 工作包定价

裁决规则（ROADMAP M0-6 / D9）：文中断言错改文、示例缺失补五件套。据此定价：

- **X = 134 个 widget.h+widget.cpp 对**，分三档单价：
  - 改名收拢型 118 个（已有领域命名类对、CMake 自洽，机械改名/收拢 + 引用更新 + 复编）：02-qtgui 6、03-qtwidgets 74、04-qtnetwork 6、05-other 23、01-qtbase 9；
  - 拆分型 7 个（main.cpp 单文件示例拆出 widget 对）：01-qtbase 03/04/05/08/10/15/16；
  - 待裁决型 9 个（纯 QML 形态：06-qml 01–03/05/07 + 05/15、16）——widget.* 命名对纯 QML 工程语义不符，按 AGENTS.md 五件套字面规则计入 X，但是否给 QML 工程设命名豁免条目需主控裁决后才能定单价。
- **Y = 118 个 .gitignore**（02-qtgui 6 + 03-qtwidgets 74 + 04-qtnetwork 6 + 05-other 25 + 06-qml 7）。模板化补齐，01-qtbase 现有 16 个（内容含 build/、CMakeCache、CMakeFiles、moc_*.cpp 等）直接作模板；单价最低。
- **Z = 32 项 build 非过**，其中：
  - 31 项 = 环境补包后复跑验证（Debian/Ubuntu 系包名形如 qt6-websockets-dev、qt6-serialport-dev、qt6-charts-dev、qt6-multimedia-dev、qt6-svg-dev、qt6-serialbus-dev、qt6-mqtt-dev、qt6-bluetooth-dev、qt6-nfc-dev、qt6-statemachine-dev、qt6-scxml-dev、qt6-3d-dev、qt6-declarative-dev、qt6-pdf-dev、qt6-httpserver-dev、qt6-webchannel-dev、qt6-webengine-dev、qt6-remoteobjects-dev、qt6-speech-dev、qt6-core5compat-dev，约 20 个包，按实际发行版命名核对）——sweep 判断大概率可过；
  - 1 项 = 08-file-io 补 CMakeLists.txt（归五件套债）后复跑；
  - 代码级修复量：待环境补齐复检后定，现有证据预期 ≈0。

**按波「开打前必须先补」硬前置清单**：

| 波（篇目） | 硬前置 | 量 | 备注 |
|---|---|---|---|
| **M1A**（02-qtgui 6 篇） | 6 对 widget.* + 6 个 .gitignore，改名后复编 6 例（现全 pass，防改名引入破坏） | 12 件 | 无环境依赖，可即刻开工；检查点 A 铁律「先实测再写」以补齐后的工程为准 |
| **M1B**（03 按钮 6 篇：12 族长 + 17–21） | 6 对 widget.* + 6 个 .gitignore + 复编 | 12 件 | 12-qabstractbutton 是族长篇（检查点 ≥2 且 ≥1 个 A），其示例补齐是检查点 A 实测的铁律前置 |
| **W3**（01-qtbase 16 篇） | 16 对 widget.*（8 改名 + 7 拆分 + 09 现场核对）+ 08-file-io 补 CMakeLists.txt 并复跑（全卷唯一 skip 例） | 17 件 | .gitignore 16/16 已齐、零欠账；02-signal-slot 已试点新声但示例债同样在，检查点升 A 需要它 |
| **W5**（03 其余篇目） | **先过 AG1 架构裁决门**：.gitignore 补齐与 build 复验与架构正交、可先做；widget.* 补齐须按裁决后的篇目集执行——若裁弧合并（6→1 类），部分示例随篇目合并退役，盲目全补会白做 | 按裁决后篇目集计（现状 68 篇 ×2 件 = 136 件） | 口径差：roadmap M2 表写 W5「其余 58 篇」，queue.json 实排 W5 = 68 篇（74 − M1B 6，含主题篇 01–10）；本报告按 queue 68 计，差异 10 篇待主控对齐 |

补充（虽不在本次点名四波，roadmap 有明文）：W4（04-qtnetwork）roadmap 写明「缺五件套先补」——6 对 widget.* + 6 个 .gitignore + 装 WebSockets/SerialPort 两包复验 2 个 fail 例；W6/W7 开打前先完成对应环境补包（W7 另含上述 QML 命名裁决）。M1A/M1B/W3 三波不依赖任何环境补包，全部示例已 build pass（或补齐 CMakeLists 后即可跑），是关键路径上的第一梯队。

## e) 覆盖缺口（无示例的文章）

无配套 example 的文章共 3 篇（全部在 00-environment-setup）：

- `00-environment-setup/00-qt6-install-beginner`
- `00-environment-setup/01-ide-setup-beginner`
- `00-environment-setup/02-cmake-first-project-beginner`

按 D4 附带裁决：这三篇的检查点暂用 B/C 形态（预测题 / 找错题）；example 补齐计价入 W-EX 后升 A。本报告计价建议：环境篇主题（装 Qt / 配 IDE / 首个 cmake 工程）与五件套 widget 工程形态不匹配，不建议为其新建示例；02-cmake-first-project 如日后需要 A 形态，可挂靠 01-qtbase 已有示例（W3 把 08-file-io 的 CMakeLists.txt 补齐后，正好是「cmake -B build 直跑」检查点的现成场地）——是否采纳由主控定，未计入 X/Y/Z。

反向缺口：`examples_without_article` = 0——134 个示例全部有对应文章，无孤儿工程。覆盖口径：137 篇正文 = 3（无示例，B/C 形态）+ 134（有示例），篇-示例一一对齐。

---

## f) 主控裁决（2026-09-28，全权委托下，可否决）

1. **QML 命名豁免（待裁决型 9 例）**：推迟到 W7 开打前裁决——届时定「QML 变体五件套」（main.cpp + 主 .qml + 逻辑文件 + CMakeLists.txt + .gitignore）并同步 CONTRIBUTING 一行；现在不动宪法、不给 9 例计价。
2. **W5 口径差**：以 queue.json 实排 **68 篇**为准（74 − M1B 6，含主题篇 01–10；roadmap M2 表「其余 58 篇」系漏计主题篇，已修正）。W5 widget.* 补齐**卡 AG1**，.gitignore 与 build 复验与架构正交、先行。
3. **00-env 三篇**：不新建五件套示例，检查点用 B/C 形态（D4 附带裁决既定）；02-cmake-first-project 的 A 形态场地挂靠 08-file-io 补齐后的工程——采纳，计入 W3 硬前置的验收项。
4. **环境补包节奏**：呼应作者「不要全家桶」——M1A/M1B/W3 零补包开工；W4 前装 WebSockets/SerialPort 两包复验 2 例；W6/W7 前按批装对应模块包（约 20 个，Arch 包名形如 qt6-websockets / qt6-multimedia / qt6-svg…届时代理核对），全部计入各波 preflight。
5. **X 补齐执行序**：第一梯队 = M1A 6 对 + M1B 6 对 + W3 16 对（改名收拢 118 档为主），随各波开工前由 agent 补齐（D9：W-EX 由 agent 承接），复编通过才准该篇 preflight 绿灯——「先实测再写检查点 A」的铁律以补齐后工程为准。

---

*机器可读原始数据（含每示例逐行 five_piece/build 结果与各章 sweep 笔记）见 `wex-inventory.json`；本报告数字全部由该文件重算得出（examples 134 / five_piece_ok 0 / pass 102 / fail 31 / skip 1）。*
