# app/ — 完整应用 Demo 清单

> 本文档列出所有计划实现的完整应用实例。每个应用均为可独立运行的程序，
> 放置在项目根目录 `app/<子类>/<目录名>/` 下，使用 Qt 6 + CMake 构建。

**总计：200 项 | 子类：10 个**

## 参考来源说明

| 标记 | 含义 |
|------|------|
| TTK | 参考 [TTKWidgetTools](https://github.com/Greedysky/TTKWidgetTools)（LGPL-3.0） |
| QWD | 参考 [QWidgetDemo](https://github.com/feiyangqingyun/QWidgetDemo) |
| MTC | 参考 [MyTestCode](https://github.com/gongjianbo/MyTestCode) |
| GH | 参考 GitHub 社区开源项目（各项目自有许可证） |
| NEW | 基于常见应用场景全新设计 |

---

---

## 01-dev-tools/ — 开发工具 (30)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 1 | `code-counter` | QWD | `tool/countcode/` | Core, Gui, Widgets | — | 代码行数统计 | P1 |
| 2 | `base64-helper` | QWD | `tool/base64helper/` | Core, Gui, Widgets | — | Base64 编解码 | P1 |
| 3 | `json-editor` | NEW | — | Core, Gui, Widgets | — | JSON 编辑器（格式化/验证/树形/表格视图） | P0 |
| 4 | `yaml-editor` | NEW | — | Core, Gui, Widgets | yaml-cpp (FetchContent) | YAML 编辑器 | P1 |
| 5 | `toml-editor` | NEW | — | Core, Gui, Widgets | toml++ (FetchContent) | TOML 编辑器 | P1 |
| 6 | `xml-editor` | NEW | — | Core, Gui, Widgets | — | XML 编辑器（格式化/XPath/验证） | P1 |
| 7 | `ini-editor` | MTC | `Qt/TestQt_20210425_ini/` | Core, Gui, Widgets | — | INI 配置编辑器 | P1 |
| 8 | `regex-tester` | NEW | — | Core, Gui, Widgets | — | 正则表达式测试器（高亮匹配/分组/替换） | P0 |
| 9 | `api-debugger` | NEW | — | Core, Gui, Widgets, Network | — | API 调试工具（HTTP/REST/GraphQL） | P1 |
| 10 | `markdown-editor` | NEW | — | Core, Gui, Widgets, WebEngineWidgets | — | Markdown 编辑器（实时预览） | P1 |
| 11 | `diff-tool` | NEW | — | Core, Gui, Widgets | — | 文件/文本对比工具 | P1 |
| 12 | `code-snippet-manager` | NEW | — | Core, Gui, Widgets, Sql | — | 代码片段管理器（分类/搜索/语法高亮） | P2 |
| 13 | `sql-query-tool` | NEW | — | Core, Gui, Widgets, Sql | — | SQL 查询工具（多数据库/结果表格） | P1 |
| 14 | `git-gui-client` | NEW | — | Core, Gui, Widgets, Network | — | Git 可视化客户端 | P1 |
| 15 | `cmake-gui-editor` | NEW | — | Core, Gui, Widgets | — | CMake 配置编辑器 | P2 |
| 16 | `log-viewer` | NEW | — | Core, Gui, Widgets | — | 日志查看器（大文件/实时/过滤/高亮） | P0 |
| 17 | `hex-editor` | NEW | — | Core, Gui, Widgets | — | 十六进制编辑器 | P1 |
| 18 | `property-editor` | NEW | — | Core, Gui, Widgets | — | 属性编辑器（类似 Qt Designer） | P2 |
| 19 | `ui-previewer` | NEW | — | Core, Gui, Widgets, UiTools | — | .ui 文件预览器 | P2 |
| 20 | `translation-editor` | NEW | — | Core, Gui, Widgets, LinguistTools | — | 翻译编辑器（.ts 文件） | P2 |
| 21 | `resource-editor` | NEW | — | Core, Gui, Widgets | — | .qrc 资源文件编辑器 | P2 |
| 22 | `build-system-gui` | NEW | — | Core, Gui, Widgets | — | 构建系统 GUI（编译/运行/输出） | P2 |
| 23 | `debugger-gui` | NEW | — | Core, Gui, Widgets | — | 调试器 GUI（变量/调用栈/断点） | P2 |
| 24 | `crontab-editor` | NEW | — | Core, Gui, Widgets | — | 定时任务编辑器 | P2 |
| 25 | `protobuf-viewer` | NEW | — | Core, Gui, Widgets | protobuf (FetchContent) | Protobuf 消息查看器 | P2 |
| 26 | `qss-editor` | NEW | — | Core, Gui, Widgets | — | QSS 样式编辑器（实时预览） | P1 |
| 27 | `serial-data-analyzer` | NEW | — | Core, Gui, Widgets, SerialPort | — | 串口数据分析器（波形显示） | P1 |
| 28 | `plugin-manager-gui` | NEW | — | Core, Gui, Widgets | — | 插件管理器（加载/卸载/配置） | P2 |
| 29 | `test-runner-gui` | NEW | — | Core, Gui, Widgets | — | 测试运行器 GUI | P2 |
| 30 | `benchmark-tool` | NEW | — | Core, Gui, Widgets | — | 性能基准测试工具 | P2 |

---

## 02-network-tools/ — 网络工具 (20)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 31 | `serial-tool` | QWD | `tool/comtool/` | Core, Gui, Widgets, Network, SerialPort | qextserialport (FetchContent) | 串口调试助手 | P0 |
| 32 | `network-tool` | QWD | `tool/nettool/` | Core, Gui, Widgets, Network, WebSockets | — | TCP/UDP/WebSocket 调试 | P0 |
| 33 | `network-relay` | QWD | `tool/netserver/` | Core, Gui, Widgets, Network | — | 网络数据转发服务器 | P1 |
| 34 | `email-tool` | QWD | `tool/emailtool/` | Core, Gui, Widgets, Network | smtpclient (FetchContent) | SMTP 邮件发送 | P1 |
| 35 | `ntp-client` | QWD | `other/ntpclient/` | Core, Gui, Widgets, Network | — | NTP 网络校时 | P2 |
| 36 | `tcp-scanner` | MTC | `Qt/ScanTcpServer/` | Core, Gui, Widgets, Network | — | TCP 服务端扫描 | P2 |
| 37 | `http-server` | NEW | — | Core, Gui, Widgets, Network | — | HTTP 服务端 | P1 |
| 38 | `websocket-client` | MTC | `Qt/QtWebSocketDemo/` | Core, Gui, Widgets, WebSockets | — | WebSocket 客户端 | P1 |
| 39 | `websocket-server` | MTC | `Qt/QtWebSocketDemo/` | Core, Gui, Widgets, WebSockets | — | WebSocket 服务端 | P1 |
| 40 | `mqtt-client` | NEW | — | Core, Gui, Widgets, Network | paho-mqtt-c (FetchContent) | MQTT 客户端 | P1 |
| 41 | `ftp-client` | NEW | — | Core, Gui, Widgets, Network | — | FTP 客户端（上传/下载/目录浏览） | P1 |
| 42 | `ftp-server` | GH | onlyet/FtpServer | Core, Gui, Widgets, Network | — | FTP 服务器 | P2 |
| 43 | `port-scanner` | NEW | — | Core, Gui, Widgets, Network | — | 端口扫描器 | P2 |
| 44 | `dns-lookup` | NEW | — | Core, Gui, Widgets, Network | — | DNS 查询工具 | P2 |
| 45 | `ping-tool` | NEW | — | Core, Gui, Widgets, Network | — | Ping 网络检测工具 | P1 |
| 46 | `bandwidth-monitor` | NEW | — | Core, Gui, Widgets, Network | — | 网络带宽监控 | P1 |
| 47 | `http-file-transfer` | MTC | `Qt/TestQt_20210807_HttpFile/` | Core, Gui, Widgets, Network | — | HTTP 文件上传下载 | P1 |
| 48 | `http-client` | MTC | `Qt/Qt5HttpDemo/` | Core, Gui, Widgets, Network | — | HTTP 客户端（GET/POST/HEAD） | P1 |
| 49 | `local-socket-tool` | MTC | `Qt/TestLocalSocket/` | Core, Gui, Widgets, Network | — | 本地套接字 IPC 工具 | P2 |
| 50 | `remote-objects-demo` | MTC | `Qt/QtRemoteObjects/` | Core, Gui, Widgets, RemoteObjects | — | Qt Remote Objects IPC 演示 | P2 |

---

## 03-file-tools/ — 文件工具 (25)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 51 | `file-manager` | NEW | — | Core, Gui, Widgets | — | 文件管理器（复制/移动/删除/属性） | P0 |
| 52 | `batch-rename` | NEW | — | Core, Gui, Widgets | — | 批量重命名（正则/序号/日期） | P1 |
| 53 | `file-search` | NEW | — | Core, Gui, Widgets | — | 文件搜索工具（名称/内容/大小/日期） | P1 |
| 54 | `file-compare` | NEW | — | Core, Gui, Widgets | — | 文件对比（二进制/文本） | P1 |
| 55 | `file-shredder` | NEW | — | Core, Gui, Widgets | — | 文件粉碎（安全删除） | P2 |
| 56 | `disk-usage-analyzer` | NEW | — | Core, Gui, Widgets | — | 磁盘空间分析（树形图） | P1 |
| 57 | `file-sync` | NEW | — | Core, Gui, Widgets | — | 文件同步工具（双向） | P1 |
| 58 | `backup-tool` | NEW | — | Core, Gui, Widgets | — | 备份工具（增量/压缩/定时） | P1 |
| 59 | `zip-tool` | MTC | `Qt/TestQt_20240226_QZip/` | Core, Gui, Widgets | — | ZIP 压缩解压 | P1 |
| 60 | `icon-extractor` | NEW | — | Core, Gui, Widgets | — | 文件图标/图片提取 | P2 |
| 61 | `recent-files` | NEW | — | Core, Gui, Widgets | — | 最近文件管理器 | P2 |
| 62 | `duplicate-finder` | NEW | — | Core, Gui, Widgets | — | 重复文件查找（MD5/SHA） | P1 |
| 63 | `path-tool` | NEW | — | Core, Gui, Widgets | — | 路径工具（长路径/环境变量展开） | P2 |
| 64 | `file-organizer` | NEW | — | Core, Gui, Widgets | — | 文件自动整理（按类型/日期/规则） | P2 |
| 65 | `disk-info` | QWD | `control/devicesizetable/` | Core, Gui, Widgets | — | 磁盘空间信息 | P2 |
| 66 | `text-replace` | NEW | — | Core, Gui, Widgets | — | 批量文本替换（多文件/正则） | P1 |
| 67 | `encoding-converter` | MTC | `Qt/TestQt_20250320_TextDecode/` | Core, Gui, Widgets | uchardet (FetchContent) | 文本编码转换 | P1 |
| 68 | `line-ending-converter` | NEW | — | Core, Gui, Widgets | — | 行尾符转换（CRLF/LF/CR） | P2 |
| 69 | `file-merger` | NEW | — | Core, Gui, Widgets | — | 文件合并工具 | P2 |
| 70 | `file-splitter` | NEW | — | Core, Gui, Widgets | — | 文件分割工具 | P2 |
| 71 | `checksum-tool` | NEW | — | Core, Gui, Widgets | — | 校验和计算（MD5/SHA/CRC） | P1 |
| 72 | `permission-manager` | NEW | — | Core, Gui, Widgets | — | 文件权限管理（chmod） | P2 |
| 73 | `symlink-manager` | NEW | — | Core, Gui, Widgets | — | 符号链接/快捷方式管理 | P2 |
| 74 | `temp-file-cleaner` | NEW | — | Core, Gui, Widgets | — | 临时文件清理 | P2 |
| 75 | `pdf-tool` | NEW | — | Core, Gui, Widgets, Pdf | — | PDF 信息查看/合并/拆分 | P1 |

---

## 04-system-tools/ — 系统工具 (25)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 76 | `app-launcher` | QWD | `tool/livetool/` | Core, Gui, Widgets, Network | — | 应用启动器（托盘+快捷方式） | P1 |
| 77 | `process-manager` | NEW | — | Core, Gui, Widgets | — | 进程管理器（列表/结束/优先级） | P0 |
| 78 | `env-var-editor` | NEW | — | Core, Gui, Widgets | — | 环境变量编辑器 | P1 |
| 79 | `startup-manager` | NEW | — | Core, Gui, Widgets | — | 启动项管理 | P2 |
| 80 | `hotkey-manager` | QWD | `third/hotkey/`+`third/shortcut/` | Core, Gui, Widgets | qhotkey (FetchContent) | 全局快捷键管理 | P1 |
| 81 | `macro-recorder` | NEW | — | Core, Gui, Widgets | — | 鼠标键盘宏录制/回放 | P1 |
| 82 | `screen-recorder` | NEW | — | Core, Gui, Widgets, Multimedia | — | 屏幕录制工具 | P1 |
| 83 | `ocr-tool` | NEW | — | Core, Gui, Widgets | tesseract (FetchContent) | OCR 文字识别 | P1 |
| 84 | `clipboard-manager` | NEW | — | Core, Gui, Widgets | — | 剪贴板管理器（历史/搜索/分组） | P1 |
| 85 | `system-info` | NEW | — | Core, Gui, Widgets | — | 系统信息查看（CPU/内存/OS/显示器） | P1 |
| 86 | `cpu-memory-monitor` | QWD | `control/cpumemorylabel/` | Core, Gui, Widgets | — | CPU/内存实时监控 | P1 |
| 87 | `device-hotplug` | MTC | `Qt/DeviceHotplug_Win/`+`_Mac/` | Core, Gui, Widgets | — | USB 设备热插拔检测 | P2 |
| 88 | `crash-handler` | MTC | `Qt/TestQt_20210211_Dump/` | Core, Gui, Widgets | — | 崩溃转储/MiniDump | P2 |
| 89 | `dpi-tool` | MTC | `Qt/TestQt_20231221_Dpi/` | Core, Gui, Widgets | — | DPI 缩放设置/测试 | P2 |
| 90 | `show-in-folder` | MTC | `Qt/TestQt_20230218_ShowInFolder/` | Core, Gui, Widgets | — | 文件夹中打开文件 | P2 |
| 91 | `window-activator` | MTC | `Qt/TestQt_20250314_WindowActiveHook/` | Core, Gui, Widgets | — | 窗口激活状态检测 | P2 |
| 92 | `service-manager` | NEW | — | Core, Gui, Widgets | — | 系统服务管理 | P2 |
| 93 | `scheduled-task` | NEW | — | Core, Gui, Widgets | — | 定时任务管理 | P2 |
| 94 | `registry-editor` | NEW | — | Core, Gui, Widgets | — | 注册表编辑器 | P2 |
| 95 | `uninstaller` | NEW | — | Core, Gui, Widgets | — | 程序卸载管理 | P2 |
| 96 | `update-checker` | NEW | — | Core, Gui, Widgets, Network | — | 程序更新检查 | P1 |
| 97 | `auto-updater` | NEW | — | Core, Gui, Widgets, Network | — | 自动更新工具 | P1 |
| 98 | `tray-icon-manager` | QWD | `other/trayicon/` | Core, Gui, Widgets, Network | — | 系统托盘管理 | P1 |
| 99 | `notification-center` | NEW | — | Core, Gui, Widgets | — | 通知中心 | P2 |
| 100 | `battery-monitor` | NEW | — | Core, Gui, Widgets | — | 电池监控（笔记本） | P2 |

---

## 05-image-tools/ — 图像工具 (20)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 101 | `screen-capture` | QWD | `widget/screenwidget/` | Core, Gui, Widgets | — | 全屏/区域截图 | P0 |
| 102 | `gif-recorder` | QWD | `widget/gifwidget/` | Core, Gui, Widgets | giflib (FetchContent) | GIF 屏幕录制 | P1 |
| 103 | `image-cropper` | QWD | `netfriend/imagecropper/` | Core, Gui, Widgets | — | 图片裁剪（头像） | P1 |
| 104 | `image-viewer` | NEW | — | Core, Gui, Widgets | — | 图片查看器（缩放/旋转/幻灯片） | P0 |
| 105 | `image-batch-processor` | NEW | — | Core, Gui, Widgets | — | 图片批量处理（缩放/格式/水印） | P1 |
| 106 | `image-converter` | NEW | — | Core, Gui, Widgets | — | 图片格式转换 | P1 |
| 107 | `watermark-tool` | NEW | — | Core, Gui, Widgets | — | 图片加水印 | P1 |
| 108 | `image-stitcher` | NEW | — | Core, Gui, Widgets | — | 图片拼接 | P2 |
| 109 | `icon-extractor` | NEW | — | Core, Gui, Widgets | — | EXE/DLL 图标提取 | P2 |
| 110 | `qr-tool` | MTC | `Qt/QtQRencodeVS2019/`+`QZXingVS2019/` | Core, Gui, Widgets | libqrencode (FetchContent) | QR 码生成/识别 | P0 |
| 111 | `barcode-tool` | GH | prison | Core, Gui, Widgets | prison (FetchContent) | 条形码生成 | P1 |
| 112 | `image-metadata` | NEW | — | Core, Gui, Widgets | — | 图片元数据查看（EXIF） | P2 |
| 113 | `image-compare` | NEW | — | Core, Gui, Widgets | — | 图片对比工具 | P1 |
| 114 | `png-optimizer` | QWD | `tool/pngtool/` | Core, Gui, Widgets | — | PNG 优化/警告清理 | P2 |
| 115 | `color-picker-app` | NEW | — | Core, Gui, Widgets | — | 屏幕取色器 | P1 |
| 116 | `image-resize` | NEW | — | Core, Gui, Widgets | — | 图片批量缩放 | P1 |
| 117 | `screenshot-uploader` | NEW | — | Core, Gui, Widgets, Network | — | 截图上传工具 | P2 |
| 118 | `image-viewer-3d` | QWD | `netfriend/imageviewwindow/` | Core, Gui, Widgets | — | 3D 翻转图片浏览 | P2 |
| 119 | `photo-wall` | NEW | — | Core, Gui, Widgets | — | 照片墙生成 | P2 |
| 120 | `sprite-sheet-tool` | NEW | — | Core, Gui, Widgets | — | 精灵图切割工具 | P2 |

---

## 06-media-tools/ — 多媒体工具 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 121 | `media-player` | QWD | `video/playffmpeg/` | Core, Gui, Widgets, Multimedia | FFmpeg (FetchContent) | 视频播放器 | P1 |
| 122 | `audio-player` | NEW | — | Core, Gui, Widgets, Multimedia | — | 音频播放器（播放列表/均衡器） | P1 |
| 123 | `audio-visualizer` | NEW | — | Core, Gui, Widgets, Multimedia | — | 音频可视化（频谱/波形） | P1 |
| 124 | `lyrics-display` | NEW | — | Core, Gui, Widgets, Multimedia | — | 歌词显示（LRC 解析/滚动） | P2 |
| 125 | `playlist-manager` | NEW | — | Core, Gui, Widgets, Multimedia | — | 播放列表管理 | P1 |
| 126 | `audio-converter` | MTC | `Qt/SilkToWav/` | Core, Gui, Widgets, Multimedia | — | 音频格式转换 | P1 |
| 127 | `audio-recorder` | NEW | — | Core, Gui, Widgets, Multimedia | — | 录音机 | P1 |
| 128 | `video-recorder` | NEW | — | Core, Gui, Widgets, Multimedia | — | 视频录制 | P1 |
| 129 | `camera-capture` | MTC | `Qt/DirectShowCamera20230108/` | Core, Gui, Widgets, Multimedia | — | 摄像头采集 | P1 |
| 130 | `get-audio-info` | MTC | `Qt/GetAudioInfo/` | Core, Gui, Widgets | FFmpeg (FetchContent) | 音频信息提取 | P2 |
| 131 | `audio-wave-editor` | NEW | — | Core, Gui, Widgets | libsndfile (FetchContent) | 音频波形编辑器 | P2 |
| 132 | `subtitle-editor` | NEW | — | Core, Gui, Widgets | — | 字幕编辑器（SRT/ASS） | P2 |
| 133 | `media-info` | NEW | — | Core, Gui, Widgets | — | 媒体文件信息查看 | P1 |
| 134 | `gif-maker` | NEW | — | Core, Gui, Widgets | giflib (FetchContent) | GIF 动画制作器 | P1 |
| 135 | `video-thumbnail` | NEW | — | Core, Gui, Widgets | FFmpeg (FetchContent) | 视频缩略图生成 | P2 |

---

## 07-office-tools/ — 办公工具 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 136 | `todo-app` | NEW | — | Core, Gui, Widgets, Sql | — | 待办事项（分类/优先级/截止日期） | P0 |
| 137 | `calendar-app` | NEW | — | Core, Gui, Widgets, Sql | — | 日历应用（日程/提醒） | P1 |
| 138 | `note-app` | NEW | — | Core, Gui, Widgets, Sql | — | 笔记应用（Markdown/标签/搜索） | P1 |
| 139 | `password-manager` | NEW | — | Core, Gui, Widgets, Sql | openssl (FetchContent) | 密码管理器（加密存储） | P1 |
| 140 | `scientific-calculator` | NEW | — | Core, Gui, Widgets | — | 科学计算器 | P1 |
| 141 | `programmer-calculator` | NEW | — | Core, Gui, Widgets | — | 程序员计算器（进制转换/位运算） | P1 |
| 142 | `unit-converter` | NEW | — | Core, Gui, Widgets | — | 单位转换器（长度/重量/温度） | P1 |
| 143 | `exchange-rate` | NEW | — | Core, Gui, Widgets, Network | — | 汇率转换 | P2 |
| 144 | `accounting-app` | NEW | — | Core, Gui, Widgets, Sql | — | 记账本（收支/分类/图表） | P1 |
| 145 | `contacts-app` | NEW | — | Core, Gui, Widgets, Sql | — | 通讯录（搜索/分组/导入导出） | P1 |
| 146 | `stopwatch-app` | NEW | — | Core, Gui, Widgets | — | 秒表/计时器 | P1 |
| 147 | `alarm-clock` | NEW | — | Core, Gui, Widgets, Multimedia | — | 闹钟 | P1 |
| 148 | `countdown-timer` | NEW | — | Core, Gui, Widgets, Multimedia | — | 倒计时器 | P1 |
| 149 | `world-clock` | NEW | — | Core, Gui, Widgets, Network | — | 世界时钟（多时区） | P2 |
| 150 | `pomodoro` | NEW | — | Core, Gui, Widgets, Multimedia | — | 番茄钟 | P2 |

---

## 08-games/ — 游戏 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 151 | `puzzle-game` | TTK/QWD | `Widget/puzzleWidget/` | Core, Gui, Widgets | — | 滑块拼图 | P2 |
| 152 | `slide-captcha` | QWD | `netfriend/slidepuzzlewidget/` | Core, Gui, Widgets | — | 滑块验证码 | P2 |
| 153 | `minesweeper` | NEW | — | Core, Gui, Widgets | — | 扫雷 | P1 |
| 154 | `gomoku` | NEW | — | Core, Gui, Widgets | — | 五子棋（人人/人机） | P1 |
| 155 | `snake` | NEW | — | Core, Gui, Widgets | — | 贪吃蛇 | P1 |
| 156 | `tetris` | NEW | — | Core, Gui, Widgets | — | 俄罗斯方块 | P1 |
| 157 | `game-2048` | NEW | — | Core, Gui, Widgets | — | 2048 | P1 |
| 158 | `sudoku` | NEW | — | Core, Gui, Widgets | — | 数独 | P1 |
| 159 | `solitaire` | NEW | — | Core, Gui, Widgets | — | 纸牌 | P2 |
| 160 | `chess` | NEW | — | Core, Gui, Widgets | — | 国际象棋 | P2 |
| 161 | `pinball` | NEW | — | Core, Gui, Widgets | — | 弹球 | P2 |
| 162 | `breakout` | NEW | — | Core, Gui, Widgets | — | 打砖块 | P1 |
| 163 | `maze-generator` | NEW | — | Core, Gui, Widgets | — | 迷宫生成器 | P2 |
| 164 | `maze-solver` | NEW | — | Core, Gui, Widgets | — | 迷宫求解 | P2 |
| 165 | `life-game` | NEW | — | Core, Gui, Widgets | — | 生命游戏（Conway） | P2 |

---

## 09-security-tools/ — 安全工具 (10)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 166 | `key-generator` | QWD | `tool/keytool/`+`tool/keydemo/` | Core, Gui, Widgets | — | 许可证密钥生成/验证 | P2 |
| 167 | `password-generator` | NEW | — | Core, Gui, Widgets | — | 密码生成器（规则/强度） | P1 |
| 168 | `hash-calculator` | NEW | — | Core, Gui, Widgets | — | 哈希计算器（MD5/SHA/BLAKE） | P1 |
| 169 | `file-encryptor` | NEW | — | Core, Gui, Widgets | openssl (FetchContent) | 文件加密/解密 | P1 |
| 170 | `certificate-viewer` | NEW | — | Core, Gui, Widgets | openssl (FetchContent) | 证书查看器 | P2 |
| 171 | `secure-delete` | NEW | — | Core, Gui, Widgets | — | 安全删除（多次覆写） | P2 |
| 172 | `portable-vault` | NEW | — | Core, Gui, Widgets | openssl (FetchContent) | 加密保险箱 | P2 |
| 173 | `ssh-key-manager` | NEW | — | Core, Gui, Widgets | — | SSH 密钥管理器 | P2 |
| 174 | `firewall-config` | NEW | — | Core, Gui, Widgets | — | 防火墙配置 GUI | P2 |
| 175 | `network-sniffer` | NEW | — | Core, Gui, Widgets, Network | — | 网络抓包工具 | P2 |

---

## 10-database-tools/ — 数据库工具 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 176 | `sqlite-browser` | NEW | — | Core, Gui, Widgets, Sql | — | SQLite 数据库浏览器 | P0 |
| 177 | `sql-editor` | NEW | — | Core, Gui, Widgets, Sql | — | SQL 编辑器（语法高亮/自动补全） | P1 |
| 178 | `data-import-export` | NEW | — | Core, Gui, Widgets, Sql | — | 数据导入导出（CSV/SQL/JSON） | P1 |
| 179 | `schema-designer` | NEW | — | Core, Gui, Widgets | — | 数据库表结构设计器 | P2 |
| 180 | `data-compare` | NEW | — | Core, Gui, Widgets, Sql | — | 数据对比工具 | P2 |
| 181 | `query-builder` | NEW | — | Core, Gui, Widgets, Sql | — | 可视化查询构建器 | P2 |
| 182 | `migration-tool` | NEW | — | Core, Gui, Widgets, Sql | — | 数据库迁移工具 | P2 |
| 183 | `connection-manager` | NEW | — | Core, Gui, Widgets, Sql | — | 数据库连接管理器 | P1 |
| 184 | `db-threaded-demo` | MTC | `Qt/TestQt_20210709_DatabaseThread/` | Core, Gui, Widgets, Sql, Concurrent | — | 多线程数据库访问演示 | P1 |
| 185 | `orm-demo` | NEW | — | Core, Gui, Widgets, Sql | — | ORM 模式演示 | P2 |
| 186 | `db-page-demo` | QWD | `other/dbpage/` | Core, Gui, Widgets, Sql | — | 数据库分页演示 | P1 |
| 187 | `sql-log` | NEW | — | Core, Gui, Widgets, Sql | — | SQL 执行日志 | P2 |
| 188 | `data-generator` | NEW | — | Core, Gui, Widgets, Sql | — | 测试数据生成器 | P2 |
| 189 | `db-backup` | NEW | — | Core, Gui, Widgets, Sql | — | 数据库备份恢复 | P1 |
| 190 | `sqlite-demo` | MTC | `Qt/TestQt_20210123_SQLite/` | Core, Gui, Widgets, Sql | — | SQLite CRUD 演示 | P1 |

---

## 统计

| 子分类 | P0 | P1 | P2 | 合计 |
|--------|----|----|-----|------|
| 01-dev-tools | 3 | 14 | 13 | 30 |
| 02-network-tools | 2 | 13 | 5 | 20 |
| 03-file-tools | 1 | 14 | 10 | 25 |
| 04-system-tools | 1 | 11 | 13 | 25 |
| 05-image-tools | 3 | 10 | 7 | 20 |
| 06-media-tools | 0 | 12 | 3 | 15 |
| 07-office-tools | 1 | 12 | 2 | 15 |
| 08-games | 0 | 7 | 8 | 15 |
| 09-security-tools | 0 | 3 | 7 | 10 |
| 10-database-tools | 1 | 8 | 6 | 15 |
| **合计** | **12** | **104** | **74** | **190** |
