# widget/ — 单个自定义控件清单

> 本文档列出所有计划实现的自定义控件实例。每个控件均为独立的、自包含的项目，
> 放置在项目根目录 `widget/<子类>/<目录名>/` 下，使用 Qt 6 + CMake 构建。

**总计：537 项 | 子类：22 个**

## 参考来源说明

| 标记 | 含义 |
|------|------|
| TTK | 参考 [TTKWidgetTools](https://github.com/Greedysky/TTKWidgetTools)（LGPL-3.0） |
| QWD | 参考 [QWidgetDemo](https://github.com/feiyangqingyun/QWidgetDemo) |
| MTC | 参考 [MyTestCode](https://github.com/gongjianbo/MyTestCode) |
| GH | 参考 GitHub 社区开源项目（各项目自有许可证） |
| NEW | 基于 GUI 通用需求全新设计 |

---

---

## 01-button/ — 按钮类 (30)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 1 | `toggle-switch` | ToggleSwitch | TTK | `Button/toggleWidget/` | Core, Gui, Widgets | — | iOS 风格开关按钮 | P0 |
| 2 | `flat-button` | FlatButton | TTK | `Button/flatButtonWidget/` | Core, Gui, Widgets | — | 扁平化按钮，hover 效果 | P1 |
| 3 | `check-button` | CheckButton | TTK | `Button/checkButtonWidget/` | Core, Gui, Widgets | — | 自定义复选按钮 | P1 |
| 4 | `radio-button` | RadioButton | TTK | `Button/radioButtonWidget/` | Core, Gui, Widgets | — | 自定义单选按钮 | P1 |
| 5 | `color-button` | ColorButton | TTK | `Button/colorButtonWidget/` | Core, Gui, Widgets | — | 点击弹出取色器按钮 | P1 |
| 6 | `device-button` | DeviceButton | QWD | `control/devicebutton/` | Core, Gui, Widgets | — | 设备状态按钮（多色/报警闪烁） | P2 |
| 7 | `light-button` | LightButton | QWD | `control/lightbutton/` | Core, Gui, Widgets | — | 发光按钮，渐变边框 | P2 |
| 8 | `image-switch` | ImageSwitch | QWD | `control/imageswitch/` | Core, Gui, Widgets | — | 图片切换开关 | P1 |
| 9 | `nav-button` | NavButton | QWD | `control/navbutton/` | Core, Gui, Widgets | — | 带图标/三角/装饰线导航按钮 | P1 |
| 10 | `tool-menu-button` | ToolMenuButton | TTK | `Button/toolMenuWidget/` | Core, Gui, Widgets | — | 弹出菜单/工具面板按钮 | P2 |
| 11 | `material-raised-button` | MaterialRaisedButton | GH | qt-material-widgets | Core, Gui, Widgets | — | Material Design 凸起按钮 | P1 |
| 12 | `material-flat-button` | MaterialFlatButton | GH | qt-material-widgets | Core, Gui, Widgets | — | Material Design 扁平按钮 | P1 |
| 13 | `material-icon-button` | MaterialIconButton | GH | qt-material-widgets | Core, Gui, Widgets | — | Material Design 图标按钮 | P1 |
| 14 | `material-fab` | MaterialFAB | GH | qt-material-widgets | Core, Gui, Widgets | — | Material Design 浮动操作按钮 | P1 |
| 15 | `split-button` | SplitButton | NEW | — | Core, Gui, Widgets | — | 分割按钮（点击+下拉箭头） | P1 |
| 16 | `drop-down-button` | DropDownButton | NEW | — | Core, Gui, Widgets | — | 下拉选择按钮 | P1 |
| 17 | `command-link-button` | CommandLinkButton | NEW | — | Core, Gui, Widgets | — | 带说明文字的命令链接按钮 | P1 |
| 18 | `gradient-button` | GradientButton | NEW | — | Core, Gui, Widgets | — | 渐变色按钮 | P2 |
| 19 | `icon-text-button` | IconTextButton | NEW | — | Core, Gui, Widgets | — | 图标+文字按钮（上/下/左/右布局） | P1 |
| 20 | `text-only-button` | TextOnlyButton | NEW | — | Core, Gui, Widgets | — | 纯文字按钮（自定义样式） | P2 |
| 21 | `icon-only-button` | IconOnlyButton | NEW | — | Core, Gui, Widgets | — | 纯图标按钮（多尺寸） | P2 |
| 22 | `checkbox-group` | CheckBoxGroup | NEW | — | Core, Gui, Widgets | — | 复选框组（全选/反选/半选） | P1 |
| 23 | `radio-button-group` | RadioButtonGroup | NEW | — | Core, Gui, Widgets | — | 单选按钮组（横向/纵向/网格） | P1 |
| 24 | `button-group-manager` | ButtonGroupManager | NEW | — | Core, Gui, Widgets | — | 按钮组管理器（互斥/多选） | P2 |
| 25 | `shortcut-button` | ShortcutButton | NEW | — | Core, Gui, Widgets | — | 带快捷键提示的按钮 | P2 |
| 26 | `heart-switch` | HeartSwitch | GH | Qt-ShowyWidgets | Core, Gui, Widgets | — | 心形开关按钮 | P2 |
| 27 | `line-switch` | LineSwitch | GH | Qt-ShowyWidgets | Core, Gui, Widgets | — | 线条风格开关按钮 | P2 |
| 28 | `multi-state-button` | MultiStateButton | NEW | — | Core, Gui, Widgets | — | 多状态切换按钮(2/3/N态) | P2 |
| 29 | `loading-button` | LoadingButton | NEW | — | Core, Gui, Widgets | — | 带加载动画的按钮 | P1 |
| 30 | `countdown-button` | CountdownButton | NEW | — | Core, Gui, Widgets | — | 倒计时按钮（发送验证码） | P1 |

---

## 02-label/ — 标签/显示类 (35)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 31 | `battery-label` | BatteryLabel | TTK | `Label/batteryLabel/` | Core, Gui, Widgets | — | 电池电量指示器 | P1 |
| 32 | `lcd-label` | LcdLabel | TTK | `Label/lcdLabel/` | Core, Gui, Widgets | — | LCD 风格数字显示 | P2 |
| 33 | `marquee-label` | MarqueeLabel | TTK | `Label/marqueeLabel/` | Core, Gui, Widgets | — | 跑马灯滚动文字 | P1 |
| 34 | `led-page-label` | LedPageLabel | TTK | `Label/ledPageLabel/` | Core, Gui, Widgets | — | LED 指示灯页状态 | P2 |
| 35 | `light-point-label` | LightPointLabel | TTK | `Label/lightPointLabel/` | Core, Gui, Widgets | — | 动态光点/发光标签 | P2 |
| 36 | `scan-label` | ScanLabel | TTK | `Label/scanLabel/` | Core, Gui, Widgets | — | 扫描线/光束动画 | P2 |
| 37 | `ant-line-label` | AntLineLabel | TTK | `Label/antLineLabel/` | Core, Gui, Widgets | — | 蚂蚁线动画边框 | P2 |
| 38 | `cross-line-label` | CrossLineLabel | TTK | `Label/crossLineLabel/` | Core, Gui, Widgets | — | 十字准线/校准线 | P2 |
| 39 | `bar-ruler-label` | BarRulerLabel | TTK | `Label/barRulerLabel/` | Core, Gui, Widgets | — | 标尺/刻度显示 | P2 |
| 40 | `heat-map-label` | HeatMapLabel | TTK | `Label/heatMapLabel/` | Core, Gui, Widgets | lodepng (FetchContent) | 热力图可视化 | P2 |
| 41 | `cpu-memory-label` | CpuMemoryLabel | TTK/QWD | `Label/cpuMemoryLabel/` | Core, Gui, Widgets | — | CPU/内存实时监控 | P2 |
| 42 | `net-traffic-label` | NetTrafficLabel | TTK | `Label/netTrafficLabel/` | Core, Gui, Widgets | — | 网络流量速度指示 | P2 |
| 43 | `cloud-panel-label` | CloudPanelLabel | TTK | `Label/cloudPanelLabel/` | Core, Gui, Widgets | — | 云标签/词云 | P2 |
| 44 | `code-area-label` | CodeAreaLabel | TTK | `Label/codeAreaLabel/` | Core, Gui, Widgets | — | 代码高亮显示区 | P1 |
| 45 | `round-animation-label` | RoundAnimationLabel | TTK | `Label/roundAnimationLabel/` | Core, Gui, Widgets | — | 圆形旋转动画标签 | P2 |
| 46 | `split-item-label` | SplitItemLabel | TTK | `Label/splitItemLabel/` | Core, Gui, Widgets | — | 分割线/分组标签 | P2 |
| 47 | `tile-background-label` | TileBackgroundLabel | TTK | `Label/tileBackgroundLabel/` | Core, Gui, Widgets | — | 平铺背景图案 | P2 |
| 48 | `transition-animation-label` | TransitionAnimationLabel | TTK | `Label/transitionAnimationLabel/` | Core, Gui, Widgets | — | 切换过渡动画标签 | P2 |
| 49 | `status-led` | StatusLed | NEW | — | Core, Gui, Widgets | — | 状态指示灯（绿/红/黄/灰+闪烁） | P0 |
| 50 | `breathing-led` | BreathingLed | NEW | — | Core, Gui, Widgets | — | 呼吸灯效果 | P2 |
| 51 | `watermark-label` | WatermarkLabel | NEW | — | Core, Gui, Widgets | — | 水印文字标签 | P2 |
| 52 | `gradient-text-label` | GradientTextLabel | NEW | — | Core, Gui, Widgets | — | 渐变色文字标签 | P2 |
| 53 | `outline-text-label` | OutlineTextLabel | NEW | — | Core, Gui, Widgets | — | 描边文字标签 | P1 |
| 54 | `shadow-text-label` | ShadowTextLabel | NEW | — | Core, Gui, Widgets | — | 阴影文字标签 | P2 |
| 55 | `rich-text-label` | RichTextLabel | NEW | — | Core, Gui, Widgets | — | 富文本显示标签 | P1 |
| 56 | `markdown-label` | MarkdownLabel | NEW | — | Core, Gui, Widgets | — | Markdown 渲染标签 | P1 |
| 57 | `time-display` | TimeDisplay | NEW | — | Core, Gui, Widgets | — | 时间显示标签（多种格式） | P1 |
| 58 | `countdown-label` | CountdownLabel | NEW | — | Core, Gui, Widgets | — | 倒计时显示标签 | P1 |
| 59 | `stock-ticker` | StockTicker | NEW | — | Core, Gui, Widgets | — | 股票涨跌显示 | P2 |
| 60 | `weather-icon` | WeatherIcon | NEW | — | Core, Gui, Widgets | — | 天气图标+温度 | P2 |
| 61 | `step-ring-label` | StepRingLabel | NEW | — | Core, Gui, Widgets | — | 步数环标签 | P2 |
| 62 | `elided-label` | ElidedLabel | NEW | — | Core, Gui, Widgets | — | 省略号标签（hover 展开全文） | P1 |
| 63 | `clickable-label` | ClickableLabel | NEW | — | Core, Gui, Widgets | — | 可点击/可选中标签 | P1 |
| 64 | `badge-label` | BadgeLabel | NEW | — | Core, Gui, Widgets | — | 徽章/角标数字 | P1 |
| 65 | `avatar-label` | AvatarLabel | NEW | — | Core, Gui, Widgets | — | 头像标签（圆形/方形/状态点） | P1 |

---

## 03-input/ — 输入控件类 (30)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 66 | `ip-edit` | IpEdit | TTK/QWD | `LineEdit/ipEditWidget/` | Core, Gui, Widgets | — | IP 地址输入 | P0 |
| 67 | `styled-line-edit` | StyledLineEdit | TTK | `LineEdit/lineEditWidget/` | Core, Gui, Widgets | — | 增强行编辑 | P1 |
| 68 | `custom-combo-box` | CustomComboBox | MTC | `Qt/MyComboBox/` | Core, Gui, Widgets | — | 可删除项下拉框 | P1 |
| 69 | `date-picker` | DatePicker | NEW | — | Core, Gui, Widgets | — | 日期选择器（日历弹窗） | P0 |
| 70 | `time-picker` | TimePicker | NEW | — | Core, Gui, Widgets | — | 时间选择器（钟面/滚动） | P1 |
| 71 | `datetime-picker` | DateTimePicker | NEW | — | Core, Gui, Widgets | — | 日期时间选择器 | P1 |
| 72 | `color-picker` | ColorPicker | QWD/TTK | `widget/colorwidget/` | Core, Gui, Widgets | — | HSL 调色板取色器 | P0 |
| 73 | `color-picker-simple` | ColorPickerSimple | NEW | — | Core, Gui, Widgets | — | 简易取色器（预设色+自定义） | P1 |
| 74 | `color-picker-gradient` | ColorPickerGradient | NEW | — | Core, Gui, Widgets | — | 渐变色选择器 | P2 |
| 75 | `font-picker` | FontPicker | NEW | — | Core, Gui, Widgets | — | 字体选择器（预览+筛选） | P1 |
| 76 | `file-picker` | FilePicker | NEW | — | Core, Gui, Widgets | — | 文件选择器（带预览） | P1 |
| 77 | `dir-picker` | DirPicker | NEW | — | Core, Gui, Widgets | — | 目录选择器 | P1 |
| 78 | `search-edit` | SearchEdit | NEW | — | Core, Gui, Widgets | — | 搜索输入框（带清除按钮+图标） | P0 |
| 79 | `password-edit` | PasswordEdit | NEW | — | Core, Gui, Widgets | — | 密码输入框（显隐切换+强度指示） | P0 |
| 80 | `verification-code-edit` | VerificationCodeEdit | NEW | — | Core, Gui, Widgets | — | 验证码输入框（N位分离+自动跳转） | P1 |
| 81 | `spin-box` | SpinBox | NEW | — | Core, Gui, Widgets | — | 数字步进器（整数/浮点） | P1 |
| 82 | `dial-input` | DialInput | NEW | — | Core, Gui, Widgets | — | 旋钮输入控件 | P2 |
| 83 | `signature-pad` | SignaturePad | NEW | — | Core, Gui, Widgets | — | 手写签名板 | P2 |
| 84 | `formula-edit` | FormulaEdit | NEW | — | Core, Gui, Widgets | — | 公式编辑器 | P2 |
| 85 | `regex-tester` | RegexTester | NEW | — | Core, Gui, Widgets | — | 正则表达式测试器 | P1 |
| 86 | `multi-text-edit` | MultiTextEditor | NEW | — | Core, Gui, Widgets | — | 多行文本编辑器（行号/语法高亮） | P1 |
| 87 | `numeric-edit` | NumericEdit | NEW | — | Core, Gui, Widgets | — | 纯数字输入框（千分位/前缀后缀） | P1 |
| 88 | `currency-edit` | CurrencyEdit | NEW | — | Core, Gui, Widgets | — | 货币输入框 | P2 |
| 89 | `phone-edit` | PhoneEdit | NEW | — | Core, Gui, Widgets | — | 电话号码输入（自动格式化） | P1 |
| 90 | `email-edit` | EmailEdit | NEW | — | Core, Gui, Widgets | — | 邮箱输入（格式验证） | P1 |
| 91 | `url-edit` | UrlEdit | NEW | — | Core, Gui, Widgets | — | URL 输入（协议+验证） | P1 |
| 92 | `tag-edit` | TagEdit | NEW | — | Core, Gui, Widgets | — | 标签输入框（回车添加+删除） | P1 |
| 93 | `color-line-edit` | ColorLineEdit | NEW | — | Core, Gui, Widgets | — | 带色块的颜色值输入 | P2 |
| 94 | `slider-line-edit` | SliderLineEdit | GH | QLineEditExt | Core, Gui, Widgets | — | 滑块+文本+进度条混合输入 | P2 |
| 95 | `auto-complete-edit` | AutoCompleteEdit | NEW | — | Core, Gui, Widgets | — | 带自动补全的输入框 | P1 |

---

## 04-progress/ — 进度条类 (35)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 96 | `circle-progress` | CircleProgress | TTK | `Progress/circleProgressWidget/` | Core, Gui, Widgets | — | 圆形进度环 | P0 |
| 97 | `wave-progress` | WaveProgress | TTK | `Progress/waveProgressWidget/` | Core, Gui, Widgets | — | 水波填充进度条 | P0 |
| 98 | `donut-wait-progress` | DonutWaitProgress | TTK | `Progress/donutWaitProgressWidget/` | Core, Gui, Widgets | — | 甜甜圈旋转等待 | P1 |
| 99 | `ring-progress` | RingProgress | TTK | `Progress/ringProgressWidget/` | Core, Gui, Widgets | — | 单环进度指示器 | P1 |
| 100 | `pie-wait-progress` | PieWaitProgress | TTK | `Progress/pieWaitProgressWidget/` | Core, Gui, Widgets | — | 饼图旋转加载器 | P1 |
| 101 | `line-wait-progress` | LineWaitProgress | TTK | `Progress/lineWaitProgressWidget/` | Core, Gui, Widgets | — | 水平线性加载条 | P1 |
| 102 | `circular-progress` | CircularProgress | TTK | `Progress/circularProgressWidget/` | Core, Gui, Widgets | — | 渐变圆形加载器 | P1 |
| 103 | `radius-progress` | RadiusProgress | TTK | `Progress/radiusProgressWidget/` | Core, Gui, Widgets | — | 圆角进度条 | P2 |
| 104 | `round-progress` | RoundProgress | TTK | `Progress/roundProgressWidget/` | Core, Gui, Widgets | — | 圆形进度条 | P2 |
| 105 | `animation-progress` | AnimationProgress | TTK | `Progress/animationProgressWidget/` | Core, Gui, Widgets | — | 动画进度条 | P2 |
| 106 | `rings-progress` | RingsProgress | TTK | `Progress/ringsProgressWidget/` | Core, Gui, Widgets | — | 多环加载器 | P2 |
| 107 | `rings-map-progress` | RingsMapProgress | TTK | `Progress/ringsMapProgressWidget/` | Core, Gui, Widgets | — | 多环地图进度 | P2 |
| 108 | `zoom-wait-progress` | ZoomWaitProgress | TTK | `Progress/zoomWaitProgressWidget/` | Core, Gui, Widgets | — | 缩放动画加载器 | P2 |
| 109 | `gif-progress` | GifProgress | TTK | `Progress/gifProgressWidget/` | Core, Gui, Widgets | — | GIF 动画进度 | P2 |
| 110 | `loading-progress` | LoadingProgress | TTK | `Progress/loadingProgressWidget/` | Core, Gui, Widgets | — | 通用加载进度 | P2 |
| 111 | `circle-wait-progress` | CircleWaitProgress | TTK | `Progress/circleWaitProgressWidget/` | Core, Gui, Widgets | — | 旋转圆形等待 | P2 |
| 112 | `gradient-progress` | GradientProgress | NEW | — | Core, Gui, Widgets | — | 渐变色水平进度条 | P1 |
| 113 | `text-progress` | TextProgress | NEW | — | Core, Gui, Widgets | — | 带百分比文字的进度条 | P1 |
| 114 | `segment-progress` | SegmentProgress | NEW | — | Core, Gui, Widgets | — | 多段分段进度条 | P1 |
| 115 | `step-progress` | StepProgress | NEW | — | Core, Gui, Widgets | — | 步骤进度条（横向/纵向） | P1 |
| 116 | `task-progress` | TaskProgress | NEW | — | Core, Gui, Widgets | — | 任务队列进度（多任务） | P1 |
| 117 | `download-progress` | DownloadProgress | NEW | — | Core, Gui, Widgets | — | 下载进度条（速度+剩余时间） | P1 |
| 118 | `upload-progress` | UploadProgress | NEW | — | Core, Gui, Widgets | — | 上传进度条 | P1 |
| 119 | `buffer-progress` | BufferProgress | NEW | — | Core, Gui, Widgets | — | 缓冲进度条（播放/缓冲双条） | P1 |
| 120 | `material-circular-progress` | MaterialCircularProgress | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 圆形进度 | P1 |
| 121 | `material-linear-progress` | MaterialLinearProgress | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 线性进度 | P1 |
| 122 | `domino-loading` | DominoLoading | NEW | — | Core, Gui, Widgets | — | 骨牌加载动画 | P2 |
| 123 | `skeleton-loading` | SkeletonLoading | NEW | — | Core, Gui, Widgets | — | 骨架屏加载 | P1 |
| 124 | `dots-loading` | DotsLoading | NEW | — | Core, Gui, Widgets | — | 跳动圆点加载 | P2 |
| 125 | `bar-loading` | BarLoading | NEW | — | Core, Gui, Widgets | — | 交错横条加载 | P2 |
| 126 | `square-loading` | SquareLoading | NEW | — | Core, Gui, Widgets | — | 方块旋转加载 | P2 |
| 127 | `arc-loading` | ArcLoading | NEW | — | Core, Gui, Widgets | — | 弧形旋转加载 | P2 |
| 128 | `pulse-loading` | PulseLoading | NEW | — | Core, Gui, Widgets | — | 脉冲加载动画 | P2 |
| 129 | `orbit-loading` | OrbitLoading | NEW | — | Core, Gui, Widgets | — | 轨道旋转加载 | P2 |
| 130 | `semicircle-progress` | SemicircleProgress | NEW | — | Core, Gui, Widgets | — | 半圆进度条 | P2 |

---

## 05-meter/ — 仪表盘类 (40)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 131 | `speed-meter` | SpeedMeter | TTK | `Meter/speedMeterWidget/` | Core, Gui, Widgets | — | 速度表（动画指针+刻度） | P0 |
| 132 | `arc-meter` | ArcMeter | TTK | `Meter/arcMeterWidget/` | Core, Gui, Widgets | — | 弧形/圆形进度仪表 | P1 |
| 133 | `car-meter` | CarMeter | TTK | `Meter/carMeterWidget/` | Core, Gui, Widgets | — | 汽车仪表盘 | P2 |
| 134 | `clock-meter` | ClockMeter | TTK | `Meter/clockMeterWidget/` | Core, Gui, Widgets | — | 模拟时钟 | P1 |
| 135 | `compass-meter` | CompassMeter | TTK | `Meter/compassMeterWidget/` | Core, Gui, Widgets | — | 指南针/方向指示器 | P2 |
| 136 | `dial-meter` | DialMeter | TTK | `Meter/dialMeterWidget/` | Core, Gui, Widgets | — | 旋转表盘/旋钮 | P2 |
| 137 | `mini-meter` | MiniMeter | TTK | `Meter/miniMeterWidget/` | Core, Gui, Widgets | — | 迷你仪表盘 | P2 |
| 138 | `panel-meter` | PanelMeter | TTK | `Meter/panelMeterWidget/` | Core, Gui, Widgets | — | 面板仪表集群 | P2 |
| 139 | `percent-meter` | PercentMeter | TTK | `Meter/percentMeterWidget/` | Core, Gui, Widgets | — | 百分比圆环 | P2 |
| 140 | `progress-meter` | ProgressMeter | TTK | `Meter/progressMeterWidget/` | Core, Gui, Widgets | — | 线性/圆形进度表 | P2 |
| 141 | `radar-meter` | RadarMeter | TTK | `Meter/radarMeterWidget/` | Core, Gui, Widgets | — | 雷达/扫描显示 | P2 |
| 142 | `round-meter` | RoundMeter | TTK | `Meter/roundMeterWidget/` | Core, Gui, Widgets | — | 圆形仪表盘 | P2 |
| 143 | `speed-ring-meter` | SpeedRingMeter | TTK | `Meter/speedRingMeterWidget/` | Core, Gui, Widgets | — | 环形速度指示器 | P2 |
| 144 | `temperature-meter` | TemperatureMeter | TTK | `Meter/temperatureMeterWidget/` | Core, Gui, Widgets | — | 温度计/温度表 | P2 |
| 145 | `time-meter` | TimeMeter | TTK | `Meter/timeMeterWidget/` | Core, Gui, Widgets | — | 时间计时器表 | P2 |
| 146 | `paint-meter` | PaintMeter | TTK | `Meter/paintMeterWidget/` | Core, Gui, Widgets | — | 自绘仪表盘 | P2 |
| 147 | `humidity-meter` | HumidityMeter | NEW | — | Core, Gui, Widgets | — | 湿度计 | P2 |
| 148 | `pressure-meter` | PressureMeter | NEW | — | Core, Gui, Widgets | — | 气压计 | P2 |
| 149 | `noise-meter` | NoiseMeter | NEW | — | Core, Gui, Widgets | — | 噪声计 | P2 |
| 150 | `light-meter` | LightMeter | NEW | — | Core, Gui, Widgets | — | 光照计 | P2 |
| 151 | `wind-speed-meter` | WindSpeedMeter | NEW | — | Core, Gui, Widgets | — | 风速计 | P2 |
| 152 | `voltage-meter` | VoltageMeter | NEW | — | Core, Gui, Widgets | — | 电压表 | P2 |
| 153 | `current-meter` | CurrentMeter | NEW | — | Core, Gui, Widgets | — | 电流表 | P2 |
| 154 | `power-meter` | PowerMeter | NEW | — | Core, Gui, Widgets | — | 功率表 | P2 |
| 155 | `spectrum-meter` | SpectrumMeter | NEW | — | Core, Gui, Widgets | — | 频谱分析仪 | P2 |
| 156 | `oscilloscope-meter` | OscilloscopeMeter | NEW | — | Core, Gui, Widgets | — | 示波器显示 | P2 |
| 157 | `heart-rate-meter` | HeartRateMeter | NEW | — | Core, Gui, Widgets | — | 心率监测 | P2 |
| 158 | `bmi-meter` | BmiMeter | NEW | — | Core, Gui, Widgets | — | BMI 指示器 | P2 |
| 159 | `fuel-meter` | FuelMeter | NEW | — | Core, Gui, Widgets | — | 油量计 | P2 |
| 160 | `water-meter` | WaterMeter | NEW | — | Core, Gui, Widgets | — | 水表 | P2 |
| 161 | `step-counter-meter` | StepCounterMeter | NEW | — | Core, Gui, Widgets | — | 计步器 | P2 |
| 162 | `level-meter` | LevelMeter | NEW | — | Core, Gui, Widgets | — | 水平仪 | P2 |
| 163 | `angle-meter` | AngleMeter | NEW | — | Core, Gui, Widgets | — | 角度仪/量角器 | P2 |
| 164 | `stopwatch-meter` | StopwatchMeter | NEW | — | Core, Gui, Widgets | — | 码表/秒表 | P1 |
| 165 | `countdown-meter` | CountdownMeter | NEW | — | Core, Gui, Widgets | — | 倒计时表 | P1 |
| 166 | `gauge-meter` | GaugeMeter | NEW | — | Core, Gui, Widgets | — | 通用仪表盘（可配刻度/颜色/范围） | P1 |
| 167 | `multi-gauge` | MultiGauge | NEW | — | Core, Gui, Widgets | — | 多表盘组合 | P2 |
| 168 | `digital-meter` | DigitalMeter | NEW | — | Core, Gui, Widgets | — | 数字显示仪表 | P2 |
| 169 | `bar-meter` | BarMeter | NEW | — | Core, Gui, Widgets | — | 条形仪表（水平/垂直） | P2 |
| 170 | `thermometer-meter` | ThermometerMeter | NEW | — | Core, Gui, Widgets | — | 温度计（水银柱风格） | P2 |

---

## 06-slider/ — 滑块类 (20)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 171 | `moving-label-slider` | MovingLabelSlider | TTK | `Slider/movingLabelSlider/` | Core, Gui, Widgets | — | 带跟随标签滑块 | P1 |
| 172 | `shining-slider` | ShiningSlider | TTK | `Slider/shiningSlider/` | Core, Gui, Widgets | — | 发光效果滑块 | P2 |
| 173 | `styled-slider` | StyledSlider | TTK | `Slider/styleSlider/` | Core, Gui, Widgets | — | 自定义样式滑块 | P1 |
| 174 | `range-slider` | RangeSlider | GH | QDoubleRangeSlider | Core, Gui, Widgets | — | 双端范围滑块 | P0 |
| 175 | `multi-handle-slider` | MultiHandleSlider | NEW | — | Core, Gui, Widgets | — | 多手柄滑块（N个值） | P2 |
| 176 | `tick-slider` | TickSlider | NEW | — | Core, Gui, Widgets | — | 带刻度标记的滑块 | P1 |
| 177 | `gradient-slider` | GradientSlider | NEW | — | Core, Gui, Widgets | — | 颜色渐变滑块 | P1 |
| 178 | `date-range-slider` | DateRangeSlider | NEW | — | Core, Gui, Widgets | — | 日期范围滑块 | P2 |
| 179 | `material-slider` | MaterialSlider | GH | qt-material-widgets | Core, Gui, Widgets | — | Material Design 滑块 | P1 |
| 180 | `volume-knob` | VolumeKnob | NEW | — | Core, Gui, Widgets | — | 音量旋钮 | P2 |
| 181 | `temperature-knob` | TemperatureKnob | NEW | — | Core, Gui, Widgets | — | 温度旋钮 | P2 |
| 182 | `color-hue-slider` | ColorHueSlider | NEW | — | Core, Gui, Widgets | — | 色相滑块 | P2 |
| 183 | `color-saturation-slider` | ColorSaturationSlider | NEW | — | Core, Gui, Widgets | — | 饱和度滑块 | P2 |
| 184 | `color-lightness-slider` | ColorLightnessSlider | NEW | — | Core, Gui, Widgets | — | 明度滑块 | P2 |
| 185 | `zoom-slider` | ZoomSlider | NEW | — | Core, Gui, Widgets | — | 缩放滑块 | P1 |
| 186 | `seek-slider` | SeekSlider | NEW | — | Core, Gui, Widgets | — | 媒体进度滑块（带缓冲条） | P1 |
| 187 | `log-slider` | LogSlider | NEW | — | Core, Gui, Widgets | — | 对数刻度滑块 | P2 |
| 188 | `circular-slider` | CircularSlider | NEW | — | Core, Gui, Widgets | — | 圆形/弧形滑块 | P2 |
| 189 | `cut-corner-slider` | CutCornerSlider | NEW | — | Core, Gui, Widgets | — | 切角风格滑块 | P2 |
| 190 | `timeline-slider` | TimelineSlider | NEW | — | Core, Gui, Widgets | — | 时间轴滑块（视频编辑风格） | P2 |

---

## 07-chart/ — 自绘图表类 (25)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 191 | `pie-chart` | PieChart | MTC | `Qt/QtPainter/SimplePieChart/` | Core, Gui, Widgets | — | 自绘饼图 | P1 |
| 192 | `custom-pie-chart` | CustomPieChart | TTK | `Widget/customPieWidget/` | Core, Gui, Widgets | — | 可定制饼图 | P2 |
| 193 | `custom-ring-chart` | CustomRingChart | TTK | `Widget/customRingWidget/` | Core, Gui, Widgets | — | 可定制环形图 | P2 |
| 194 | `smooth-curve` | SmoothCurve | QWD | `control/smoothcurve/` | Core, Gui, Widgets | — | 样条平滑曲线 | P1 |
| 195 | `heat-map` | HeatMap | MTC | `Qt/MyHeatMap/` | Core, Gui, Widgets | — | 热力图 | P2 |
| 196 | `line-chart` | LineChart | NEW | — | Core, Gui, Widgets | — | 折线图（多系列/平滑/阶梯） | P0 |
| 197 | `bar-chart` | BarChart | NEW | — | Core, Gui, Widgets | — | 柱状图（纵向/横向） | P0 |
| 198 | `stacked-bar-chart` | StackedBarChart | NEW | — | Core, Gui, Widgets | — | 堆叠柱状图 | P1 |
| 199 | `area-chart` | AreaChart | NEW | — | Core, Gui, Widgets | — | 面积图 | P1 |
| 200 | `scatter-chart` | ScatterChart | NEW | — | Core, Gui, Widgets | — | 散点图（气泡大小/颜色映射） | P1 |
| 201 | `bubble-chart` | BubbleChart | NEW | — | Core, Gui, Widgets | — | 气泡图 | P1 |
| 202 | `candlestick-chart` | CandlestickChart | NEW | — | Core, Gui, Widgets | — | K 线图（股票） | P1 |
| 203 | `radar-chart` | RadarChart | NEW | — | Core, Gui, Widgets | — | 雷达图/蜘蛛网图 | P1 |
| 204 | `box-plot` | BoxPlot | NEW | — | Core, Gui, Widgets | — | 箱线图 | P1 |
| 205 | `funnel-chart` | FunnelChart | NEW | — | Core, Gui, Widgets | — | 漏斗图 | P2 |
| 206 | `sankey-chart` | SankeyChart | NEW | — | Core, Gui, Widgets | — | 桑基图 | P2 |
| 207 | `tree-chart` | TreeChart | NEW | — | Core, Gui, Widgets | — | 矩形树图 | P2 |
| 208 | `sunburst-chart` | SunburstChart | NEW | — | Core, Gui, Widgets | — | 旭日图 | P2 |
| 209 | `word-cloud-chart` | WordCloudChart | NEW | — | Core, Gui, Widgets | — | 词云图 | P2 |
| 210 | `gauge-chart` | GaugeChart | NEW | — | Core, Gui, Widgets | — | 仪表盘图 | P1 |
| 211 | `realtime-chart` | RealtimeChart | NEW | — | Core, Gui, Widgets | — | 实时滚动曲线图 | P0 |
| 212 | `timeline-chart` | TimelineChart | NEW | — | Core, Gui, Widgets | — | 甘特图/时间轴图 | P1 |
| 213 | `org-chart` | OrgChart | NEW | — | Core, Gui, Widgets | — | 组织架构图 | P2 |
| 214 | `flow-chart` | FlowChart | NEW | — | Core, Gui, Widgets | — | 流程图 | P2 |
| 215 | `mind-map` | MindMap | NEW | — | Core, Gui, Widgets | — | 思维导图 | P1 |

---

## 08-table/ — 表格类 (25)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 216 | `sortable-table` | SortableTable | NEW | — | Core, Gui, Widgets | — | 可排序表格（点击列头排序） | P0 |
| 217 | `filterable-table` | FilterableTable | NEW | — | Core, Gui, Widgets | — | 可过滤表格（搜索过滤行） | P0 |
| 218 | `editable-table` | EditableTable | NEW | — | Core, Gui, Widgets | — | 可编辑表格 | P0 |
| 219 | `tree-table` | TreeTable | NEW | — | Core, Gui, Widgets | — | 树形表格 | P1 |
| 220 | `paged-table` | PagedTable | QWD | `other/dbpage/` | Core, Gui, Widgets, Sql | — | 分页表格 | P0 |
| 221 | `frozen-table` | FrozenTable | NEW | — | Core, Gui, Widgets | — | 冻结行列表格 | P1 |
| 222 | `merged-cell-table` | MergedCellTable | NEW | — | Core, Gui, Widgets | — | 合并单元格表格 | P1 |
| 223 | `zebra-table` | ZebraTable | NEW | — | Core, Gui, Widgets | — | 斑马纹表格 | P1 |
| 224 | `drag-row-table` | DragRowTable | MTC | `Qt/QTableViewMoveAction/` | Core, Gui, Widgets | — | 拖拽排序行 | P1 |
| 225 | `context-menu-table` | ContextMenuTable | NEW | — | Core, Gui, Widgets | — | 右键菜单表格 | P1 |
| 226 | `selection-table` | SelectionTable | NEW | — | Core, Gui, Widgets | — | 多选/单选行表格 | P1 |
| 227 | `checkbox-table` | CheckBoxTable | NEW | — | Core, Gui, Widgets | — | 带复选框列表格 | P0 |
| 228 | `progress-column-table` | ProgressColumnTable | NEW | — | Core, Gui, Widgets | — | 进度条列表格 | P1 |
| 229 | `star-rating-table` | StarRatingTable | NEW | — | Core, Gui, Widgets | — | 星级评分列表格 | P1 |
| 230 | `color-cell-table` | ColorCellTable | TTK | `Widget/colorTableWidget/` | Core, Gui, Widgets | — | 颜色单元格表格 | P2 |
| 231 | `rich-text-cell-table` | RichTextCellTable | NEW | — | Core, Gui, Widgets | — | 富文本单元格表格 | P2 |
| 232 | `virtual-scroll-table` | VirtualScrollTable | NEW | — | Core, Gui, Widgets | — | 虚拟滚动大表格(10万+行) | P1 |
| 233 | `json-table` | JsonTable | MTC | `Qt/QJsonAndTreeView/` | Core, Gui, Widgets | — | JSON 表格展示 | P1 |
| 234 | `csv-table` | CsvTable | NEW | — | Core, Gui, Widgets | — | CSV 文件表格 | P1 |
| 235 | `sort-header-table` | SortHeaderTable | MTC | `Qt/SortHeaderView/` | Core, Gui, Widgets | — | 自定义排序表头 | P1 |
| 236 | `smooth-table` | SmoothTable | TTK | `Widget/smoothMovingTableWidget/` | Core, Gui, Widgets | — | 平滑滚动表格 | P2 |
| 237 | `icon-delegate-table` | IconDelegateTable | NEW | — | Core, Gui, Widgets | — | 图标委托表格 | P1 |
| 238 | `button-delegate-table` | ButtonDelegateTable | NEW | — | Core, Gui, Widgets | — | 按钮委托表格 | P1 |
| 239 | `combobox-delegate-table` | ComboBoxDelegateTable | NEW | — | Core, Gui, Widgets | — | 下拉框委托表格 | P1 |
| 240 | `tooltip-table` | TooltipTable | NEW | — | Core, Gui, Widgets | — | 悬浮提示表格 | P2 |

---

## 09-tree/ — 树形控件类 (20)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 241 | `checkbox-tree` | CheckBoxTree | NEW | — | Core, Gui, Widgets | — | 带复选框树（级联选中） | P0 |
| 242 | `filterable-tree` | FilterableTree | NEW | — | Core, Gui, Widgets | — | 带搜索过滤树 | P0 |
| 243 | `editable-tree` | EditableTree | NEW | — | Core, Gui, Widgets | — | 可编辑节点树 | P1 |
| 244 | `drag-tree` | DragTree | NEW | — | Core, Gui, Widgets | — | 拖拽排序树 | P1 |
| 245 | `multi-column-tree` | MultiColumnTree | NEW | — | Core, Gui, Widgets | — | 多列树 | P1 |
| 246 | `icon-tree` | IconTree | NEW | — | Core, Gui, Widgets | — | 自定义图标/颜色节点树 | P1 |
| 247 | `context-menu-tree` | ContextMenuTree | NEW | — | Core, Gui, Widgets | — | 右键菜单树 | P1 |
| 248 | `lazy-load-tree` | LazyLoadTree | NEW | — | Core, Gui, Widgets | — | 懒加载树（按需展开） | P1 |
| 249 | `proxy-model-tree` | ProxyModelTree | NEW | — | Core, Gui, Widgets | — | 代理模型树（排序/过滤） | P1 |
| 250 | `file-system-tree` | FileSystemTree | NEW | — | Core, Gui, Widgets | — | 文件系统树 | P0 |
| 251 | `class-tree` | ClassTree | NEW | — | Core, Gui, Widgets | — | 类继承树 | P2 |
| 252 | `property-tree` | PropertyTree | NEW | — | Core, Gui, Widgets | — | 属性树（Key-Value） | P1 |
| 253 | `animated-tree` | AnimatedTree | NEW | — | Core, Gui, Widgets | — | 展开/折叠动画树 | P2 |
| 254 | `line-edit-tree` | LineEditTree | QWD | `other/lineeditnext/` | Core, Gui, Widgets | — | 回车跳转下一节点树 | P2 |
| 255 | `tooltip-tree` | TooltipTree | NEW | — | Core, Gui, Widgets | — | 悬浮提示树 | P2 |
| 256 | `drag-drop-between-trees` | DragDropBetweenTrees | NEW | — | Core, Gui, Widgets | — | 跨树拖放 | P1 |
| 257 | `tree-with-header` | TreeWithHeader | NEW | — | Core, Gui, Widgets | — | 带表头树 | P1 |
| 258 | `radio-tree` | RadioTree | NEW | — | Core, Gui, Widgets | — | 单选树 | P2 |
| 259 | `color-node-tree` | ColorNodeTree | NEW | — | Core, Gui, Widgets | — | 彩色节点树 | P2 |
| 260 | `search-highlight-tree` | SearchHighlightTree | NEW | — | Core, Gui, Widgets | — | 搜索高亮树 | P1 |

---

## 10-list/ — 列表类 (20)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 261 | `drag-sort-list` | DragSortList | NEW | — | Core, Gui, Widgets | — | 拖拽排序列表 | P1 |
| 262 | `multi-select-list` | MultiSelectList | NEW | — | Core, Gui, Widgets | — | 多选列表 | P1 |
| 263 | `checkbox-list` | CheckBoxList | NEW | — | Core, Gui, Widgets | — | 带复选框列表 | P0 |
| 264 | `avatar-list` | AvatarList | NEW | — | Core, Gui, Widgets | — | 带头像列表 | P1 |
| 265 | `progress-list` | ProgressList | NEW | — | Core, Gui, Widgets | — | 带进度列表 | P1 |
| 266 | `action-list` | ActionList | NEW | — | Core, Gui, Widgets | — | 带操作按钮列表 | P1 |
| 267 | `grouped-list` | GroupedList | NEW | — | Core, Gui, Widgets | — | 分组列表 | P1 |
| 268 | `virtual-scroll-list` | VirtualScrollList | NEW | — | Core, Gui, Widgets | — | 虚拟滚动大列表 | P1 |
| 269 | `waterfall-list` | WaterfallList | NEW | — | Core, Gui, Widgets | — | 瀑布流列表 | P1 |
| 270 | `tag-list` | TagList | NEW | — | Core, Gui, Widgets | — | 标签列表（可添加/删除） | P1 |
| 271 | `icon-text-list` | IconTextList | NEW | — | Core, Gui, Widgets | — | 图标+文字列表 | P1 |
| 272 | `searchable-list` | SearchableList | NEW | — | Core, Gui, Widgets | — | 可搜索过滤列表 | P1 |
| 273 | `sortable-list` | SortableList | NEW | — | Core, Gui, Widgets | — | 可排序列表 | P1 |
| 274 | `deletable-list` | DeletableList | NEW | — | Core, Gui, Widgets | — | 可删除项列表 | P1 |
| 275 | `checkable-list` | CheckableList | NEW | — | Core, Gui, Widgets | — | 可选中列表 | P1 |
| 276 | `hover-list` | HoverList | NEW | — | Core, Gui, Widgets | — | Hover 高亮列表 | P1 |
| 277 | `animation-list` | AnimationList | NEW | — | Core, Gui, Widgets | — | 增删动画列表 | P2 |
| 278 | `swipe-list` | SwipeList | NEW | — | Core, Gui, Widgets | — | 左滑/右滑操作列表 | P1 |
| 279 | `sticky-header-list` | StickyHeaderList | NEW | — | Core, Gui, Widgets | — | 吸顶分组标题列表 | P2 |
| 280 | `nested-list` | NestedList | NEW | — | Core, Gui, Widgets | — | 嵌套子列表 | P1 |

---

## 11-calendar/ — 日历类 (10)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 281 | `custom-calendar` | CustomCalendar | TTK | `Widget/calendarWidget/` | Core, Gui, Widgets | — | 自定义日历 | P1 |
| 282 | `lunar-calendar` | LunarCalendar | QWD | `widget/lunarcalendarwidget/` | Core, Gui, Widgets | — | 农历日历（1901-2099） | P1 |
| 283 | `date-picker-calendar` | DatePickerCalendar | NEW | — | Core, Gui, Widgets | — | 日期选择日历 | P1 |
| 284 | `date-range-calendar` | DateRangeCalendar | NEW | — | Core, Gui, Widgets | — | 日期范围选择日历 | P1 |
| 285 | `schedule-calendar` | ScheduleCalendar | NEW | — | Core, Gui, Widgets | — | 日程日历（带事件标记） | P1 |
| 286 | `year-month-picker` | YearMonthPicker | NEW | — | Core, Gui, Widgets | — | 年月选择器 | P1 |
| 287 | `week-picker` | WeekPicker | NEW | — | Core, Gui, Widgets | — | 周选择器 | P2 |
| 288 | `timeline-calendar` | TimelineCalendar | NEW | — | Core, Gui, Widgets | — | 时间轴日历 | P2 |
| 289 | `mini-calendar` | MiniCalendar | NEW | — | Core, Gui, Widgets | — | 迷你日历（单行/紧凑） | P1 |
| 290 | `gantt-calendar` | GanttCalendar | NEW | — | Core, Gui, Widgets | — | 甘特图日历 | P2 |

---

## 12-terminal/ — 终端/文本显示类 (10)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 291 | `terminal-widget` | TerminalWidget | NEW | — | Core, Gui, Widgets | — | 终端模拟器控件 | P1 |
| 292 | `log-viewer` | LogViewer | NEW | — | Core, Gui, Widgets | — | 日志查看控件（大文件/实时追加） | P0 |
| 293 | `text-diff` | TextDiff | NEW | — | Core, Gui, Widgets | — | 文本差异对比控件 | P1 |
| 294 | `code-editor` | CodeEditor | GH | QCodeEditor | Core, Gui, Widgets | — | 代码编辑器（行号/语法高亮/折叠） | P0 |
| 295 | `hex-editor` | HexEditor | NEW | — | Core, Gui, Widgets | — | 十六进制编辑器 | P1 |
| 296 | `markdown-editor` | MarkdownEditor | NEW | — | Core, Gui, Widgets | — | Markdown 编辑器（预览） | P1 |
| 297 | `rich-text-editor` | RichTextEditor | NEW | — | Core, Gui, Widgets | — | 富文本编辑器（工具栏） | P1 |
| 298 | `text-search` | TextSearch | NEW | — | Core, Gui, Widgets | — | 文本搜索控件（高亮/替换） | P1 |
| 299 | `console-widget` | ConsoleWidget | NEW | — | Core, Gui, Widgets | — | 控制台输出控件 | P1 |
| 300 | `ascii-table` | AsciiTable | NEW | — | Core, Gui, Widgets | — | ASCII 字符表控件 | P2 |

---

## 13-spacialized/ — 专用控件类 (25)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 301 | `color-palette` | ColorPalette | NEW | — | Core, Gui, Widgets | — | 调色板控件（预设色+自定义色板） | P1 |
| 302 | `font-preview` | FontPreview | NEW | — | Core, Gui, Widgets | — | 字体预览控件 | P1 |
| 303 | `emoji-picker` | EmojiPicker | NEW | — | Core, Gui, Widgets | — | Emoji 选择器 | P2 |
| 304 | `keyboard-widget` | KeyboardWidget | NEW | — | Core, Gui, Widgets | — | 虚拟键盘控件 | P2 |
| 305 | `numpad-widget` | NumpadWidget | NEW | — | Core, Gui, Widgets | — | 数字键盘控件 | P2 |
| 306 | `color-bar` | ColorBar | NEW | — | Core, Gui, Widgets | — | 色带/色谱条 | P2 |
| 307 | `ruler-widget` | RulerWidget | NEW | — | Core, Gui, Widgets | — | 标尺控件（水平/垂直） | P2 |
| 308 | `crosshair-widget` | CrosshairWidget | QWD | `other/mouseline/` | Core, Gui, Widgets | — | 十字准星控件 | P2 |
| 309 | `zoom-widget` | ZoomWidget | NEW | — | Core, Gui, Widgets | — | 缩放控件（放大镜） | P1 |
| 310 | `minimap-widget` | MinimapWidget | NEW | — | Core, Gui, Widgets | — | 代码/文档小地图 | P1 |
| 311 | `breadcrumb-widget` | BreadcrumbWidget | NEW | — | Core, Gui, Widgets | — | 面包屑导航控件 | P1 |
| 312 | `avatar-widget` | AvatarWidget | NEW | — | Core, Gui, Widgets | — | 头像控件（在线状态/角标） | P1 |
| 313 | `rating-widget` | RatingWidget | NEW | — | Core, Gui, Widgets | — | 星级评分控件 | P1 |
| 314 | `tag-widget` | TagWidget | NEW | — | Core, Gui, Widgets | — | 标签/徽章控件 | P1 |
| 315 | `tooltip-widget` | TooltipWidget | NEW | — | Core, Gui, Widgets | — | 自定义富文本提示 | P1 |
| 316 | `popover-widget` | PopoverWidget | NEW | — | Core, Gui, Widgets | — | 弹出气泡控件 | P1 |
| 317 | `drop-indicator` | DropIndicator | NEW | — | Core, Gui, Widgets | — | 拖放指示器 | P2 |
| 318 | `resize-handle` | ResizeHandle | NEW | — | Core, Gui, Widgets | — | 调整大小手柄 | P2 |
| 319 | `splitter-widget` | SplitterWidget | NEW | — | Core, Gui, Widgets | — | 分割器（拖拽/比例锁定） | P1 |
| 320 | `scrollbar-widget` | ScrollbarWidget | GH | qt-material-widgets | Core, Gui, Widgets | — | 自定义滚动条 | P1 |
| 321 | `scroll-area-enhanced` | ScrollAreaEnhanced | NEW | — | Core, Gui, Widgets | — | 增强滚动区域（惯性/回弹） | P1 |
| 322 | `tab-bar-widget` | TabBarWidget | GH | qt-material-widgets | Core, Gui, Widgets | — | 自定义标签栏 | P1 |
| 323 | `tab-widget-enhanced` | TabWidgetEnhanced | NEW | — | Core, Gui, Widgets | — | 增强标签页（关闭/拖拽/滚动） | P1 |
| 324 | `toolbar-widget` | ToolbarWidget | NEW | — | Core, Gui, Widgets | — | 工具栏控件 | P1 |
| 325 | `statusbar-widget` | StatusbarWidget | NEW | — | Core, Gui, Widgets | — | 状态栏控件 | P1 |

---

## 14-animation/ — 动画效果类 (30)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 326 | `slide-animation` | SlideAnimation | NEW | — | Core, Gui, Widgets | — | 滑动动画（上/下/左/右） | P1 |
| 327 | `fade-animation` | FadeAnimation | NEW | — | Core, Gui, Widgets | — | 淡入淡出动画 | P1 |
| 328 | `zoom-animation` | ZoomAnimation | NEW | — | Core, Gui, Widgets | — | 缩放动画 | P1 |
| 329 | `flip-animation` | FlipAnimation | NEW | — | Core, Gui, Widgets | — | 3D 翻转动画 | P1 |
| 330 | `rotate-animation` | RotateAnimation | NEW | — | Core, Gui, Widgets | — | 旋转动画 | P1 |
| 331 | `bounce-animation` | BounceAnimation | NEW | — | Core, Gui, Widgets | — | 弹跳动画 | P2 |
| 332 | `shake-animation` | ShakeAnimation | NEW | — | Core, Gui, Widgets | — | 抖动动画 | P1 |
| 333 | `typewriter-animation` | TypewriterAnimation | NEW | — | Core, Gui, Widgets | — | 打字机效果 | P1 |
| 334 | `number-rolling` | NumberRolling | NEW | — | Core, Gui, Widgets | — | 数字滚动效果 | P1 |
| 335 | `count-up-animation` | CountUpAnimation | NEW | — | Core, Gui, Widgets | — | 数字递增动画 | P1 |
| 336 | `particle-effect` | ParticleEffect | NEW | — | Core, Gui, Widgets | — | 粒子效果 | P2 |
| 337 | `confetti-effect` | ConfettiEffect | NEW | — | Core, Gui, Widgets | — | 彩纸/庆祝效果 | P2 |
| 338 | `smoke-effect` | SmokeEffect | NEW | — | Core, Gui, Widgets | — | 烟雾效果 | P2 |
| 339 | `ripple-effect` | RippleEffect | NEW | — | Core, Gui, Widgets | — | 水波纹效果 | P1 |
| 340 | `glow-effect` | GlowEffect | NEW | — | Core, Gui, Widgets | — | 发光效果 | P2 |
| 341 | `shadow-effect` | ShadowEffect | NEW | — | Core, Gui, Widgets | — | 动态阴影效果 | P2 |
| 342 | `blur-effect` | BlurEffect | NEW | — | Core, Gui, Widgets | — | 模糊效果（叠加层） | P1 |
| 343 | `transition-stack` | TransitionStack | TTK | `Widget/animationStackedWidget/` | Core, Gui, Widgets | — | 页面切换过渡动画 | P1 |
| 344 | `layout-add-remove-anim` | LayoutAddRemoveAnim | TTK | `Widget/layoutAnimationWidget/` | Core, Gui, Widgets | — | 布局增删动画 | P2 |
| 345 | `color-transition` | ColorTransition | NEW | — | Core, Gui, Widgets | — | 颜色过渡动画 | P1 |
| 346 | `path-animation` | PathAnimation | NEW | — | Core, Gui, Widgets | — | 路径动画 | P2 |
| 347 | `morph-animation` | MorphAnimation | NEW | — | Core, Gui, Widgets | — | 形变动画 | P2 |
| 348 | `matrix-rain` | MatrixRain | NEW | — | Core, Gui, Widgets | — | 黑客帝国矩阵雨效果 | P2 |
| 349 | `starfield` | Starfield | NEW | — | Core, Gui, Widgets | — | 星空穿越效果 | P2 |
| 350 | `water-ripple` | WaterRipple | NEW | — | Core, Gui, Widgets | — | 水面波纹效果 | P2 |
| 351 | `fire-effect` | FireEffect | NEW | — | Core, Gui, Widgets | — | 火焰效果 | P2 |
| 352 | `snow-effect` | SnowEffect | NEW | — | Core, Gui, Widgets | — | 雪花飘落效果 | P2 |
| 353 | `page-curl` | PageCurl | NEW | — | Core, Gui, Widgets | — | 翻页效果 | P2 |
| 354 | `reveal-animation` | RevealAnimation | NEW | — | Core, Gui, Widgets | — | 揭示动画 | P2 |
| 355 | `stagger-animation` | StaggerAnimation | NEW | — | Core, Gui, Widgets | — | 交错动画 | P2 |

---

## 15-opengl/ — OpenGL/3D 类 (15)

| # | 目录名 | 类名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|-------------|---------|------|------|--------|
| 356 | `gl-cube` | GlCube | MTC | `Qt/QtExampleCube/` | Core, Gui, Widgets, OpenGL | — | OpenGL 立方体 | P1 |
| 357 | `gl-texture` | GlTexture | MTC | `Qt/TestQt_20251030_GLWidget/` | Core, Gui, Widgets, OpenGL | — | OpenGL 纹理渲染 | P1 |
| 358 | `gl-triangle` | GlTriangle | NEW | — | Core, Gui, Widgets, OpenGL | — | OpenGL 三角形 | P1 |
| 359 | `gl-shader-widget` | GlShaderWidget | NEW | — | Core, Gui, Widgets, OpenGL | — | OpenGL 着色器控件 | P1 |
| 360 | `gl-3d-scene` | Gl3dScene | MTC | `Qt/TestQt_20211018_Assimp/` | Core, Gui, Widgets, OpenGL | Assimp (FetchContent) | 3D 模型查看器 | P1 |
| 361 | `gl-picking` | GlPicking | NEW | — | Core, Gui, Widgets, OpenGL | — | OpenGL 拾取 | P2 |
| 362 | `gl-chart-3d` | GlChart3d | NEW | — | Core, Gui, Widgets, OpenGL | — | 3D 图表 | P2 |
| 363 | `gl-terrain` | GlTerrain | NEW | — | Core, Gui, Widgets, OpenGL | — | 地形渲染 | P2 |
| 364 | `gl-particle` | GlParticle | NEW | — | Core, Gui, Widgets, OpenGL | — | OpenGL 粒子系统 | P2 |
| 365 | `gl-camera` | GlCamera | NEW | — | Core, Gui, Widgets, OpenGL | — | 3D 相机控制 | P2 |
| 366 | `gl-lighting` | GlLighting | NEW | — | Core, Gui, Widgets, OpenGL | — | 光照模型 | P2 |
| 367 | `gl-text-rendering` | GlTextRendering | NEW | — | Core, Gui, Widgets, OpenGL | — | OpenGL 文字渲染 | P2 |
| 368 | `gl-framebuffer` | GlFramebuffer | NEW | — | Core, Gui, Widgets, OpenGL | — | FBO 帧缓冲 | P2 |
| 369 | `gl-shadow-mapping` | GlShadowMapping | NEW | — | Core, Gui, Widgets, OpenGL | — | 阴影映射 | P2 |
| 370 | `gl-post-process` | GlPostProcess | NEW | — | Core, Gui, Widgets, OpenGL | — | 后处理效果 | P2 |

---

## 统计

| 子分类 | P0 | P1 | P2 | 合计 |
|--------|----|----|-----|------|
| 01-button | 1 | 13 | 16 | 30 |
| 02-label | 1 | 12 | 22 | 35 |
| 03-input | 4 | 18 | 8 | 30 |
| 04-progress | 2 | 13 | 20 | 35 |
| 05-meter | 1 | 5 | 34 | 40 |
| 06-slider | 1 | 8 | 11 | 20 |
| 07-chart | 3 | 13 | 9 | 25 |
| 08-table | 4 | 15 | 6 | 25 |
| 09-tree | 3 | 13 | 4 | 20 |
| 10-list | 1 | 16 | 3 | 20 |
| 11-calendar | 0 | 8 | 2 | 10 |
| 12-terminal | 2 | 7 | 1 | 10 |
| 13-specialized | 0 | 17 | 8 | 25 |
| 14-animation | 0 | 14 | 16 | 30 |
| 15-opengl | 0 | 5 | 10 | 15 |
| **合计(15类)** | **23** | **167** | **170** | **370** |

---

## 16-datetime/ — 日期时间控件 (15)

| # | 目录名 | 类名 | 来源 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|---------|------|------|--------|
| 371 | `date-display` | DateDisplay | NEW | Core, Gui, Widgets | — | 日期显示（多种格式/中英文） | P1 |
| 372 | `time-display` | TimeDisplay | NEW | Core, Gui, Widgets | — | 时间显示（12/24小时制） | P1 |
| 373 | `digital-clock` | DigitalClock | NEW | Core, Gui, Widgets | — | 数字时钟 | P1 |
| 374 | `analog-clock` | AnalogClock | TTK | Core, Gui, Widgets | — | 模拟时钟 | P1 |
| 375 | `stopwatch-widget` | StopwatchWidget | NEW | Core, Gui, Widgets | — | 秒表控件 | P1 |
| 376 | `timer-widget` | TimerWidget | NEW | Core, Gui, Widgets | — | 倒计时控件 | P1 |
| 377 | `date-range-picker` | DateRangePicker | NEW | Core, Gui, Widgets | — | 日期范围选择 | P1 |
| 378 | `duration-picker` | DurationPicker | NEW | Core, Gui, Widgets | — | 时间段选择器 | P2 |
| 379 | `timezone-clock` | TimezoneClock | NEW | Core, Gui, Widgets | — | 多时区时钟 | P2 |
| 380 | `alarm-clock-widget` | AlarmClockWidget | NEW | Core, Gui, Widgets | — | 闹钟控件 | P2 |
| 381 | `countdown-display` | CountdownDisplay | NEW | Core, Gui, Widgets | — | 倒计时显示（天/时/分/秒） | P1 |
| 382 | `elapsed-time` | ElapsedTime | NEW | Core, Gui, Widgets | — | 已用时间显示 | P1 |
| 383 | `schedule-timeline` | ScheduleTimeline | NEW | Core, Gui, Widgets | — | 日程时间线 | P2 |
| 384 | `date-validator` | DateValidator | NEW | Core, Gui, Widgets | — | 日期验证控件 | P1 |
| 385 | `lunar-date-display` | LunarDateDisplay | NEW | Core, Gui, Widgets | — | 农历日期显示 | P2 |

---

## 17-network-widget/ — 网络相关控件 (15)

| # | 目录名 | 类名 | 来源 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|---------|------|------|--------|
| 386 | `connection-indicator` | ConnectionIndicator | NEW | Core, Gui, Widgets, Network | — | 网络连接状态指示器 | P1 |
| 387 | `bandwidth-widget` | BandwidthWidget | NEW | Core, Gui, Widgets | — | 带宽占用显示 | P2 |
| 388 | `download-progress-widget` | DownloadProgressWidget | NEW | Core, Gui, Widgets, Network | — | 下载进度（速度+剩余+可暂停） | P1 |
| 389 | `upload-progress-widget` | UploadProgressWidget | NEW | Core, Gui, Widgets, Network | — | 上传进度控件 | P1 |
| 390 | `network-config-widget` | NetworkConfigWidget | NEW | Core, Gui, Widgets, Network | — | 网络配置面板（代理/端口） | P1 |
| 391 | `url-input` | UrlInput | NEW | Core, Gui, Widgets | — | URL 输入框（协议图标+验证） | P1 |
| 392 | `server-status-panel` | ServerStatusPanel | NEW | Core, Gui, Widgets, Network | — | 服务端状态面板 | P2 |
| 393 | `latency-display` | LatencyDisplay | NEW | Core, Gui, Widgets, Network | — | 网络延迟显示 | P2 |
| 394 | `port-input` | PortInput | NEW | Core, Gui, Widgets | — | 端口号输入框 | P1 |
| 395 | `ip-range-input` | IpRangeInput | NEW | Core, Gui, Widgets | — | IP 范围输入 | P2 |
| 396 | `mac-address-input` | MacAddressInput | NEW | Core, Gui, Widgets | — | MAC 地址输入 | P2 |
| 397 | `ftp-browser` | FtpBrowser | NEW | Core, Gui, Widgets, Network | — | FTP 目录浏览控件 | P2 |
| 398 | `websocket-status` | WebSocketStatus | NEW | Core, Gui, Widgets, WebSockets | — | WebSocket 连接状态 | P1 |
| 399 | `transfer-queue` | TransferQueue | NEW | Core, Gui, Widgets, Network | — | 传输队列（多个文件排队） | P1 |
| 400 | `network-traffic-chart` | NetworkTrafficChart | NEW | Core, Gui, Widgets | — | 网络流量实时图表 | P2 |

---

## 18-data-display/ — 数据展示控件 (20)

| # | 目录名 | 类名 | 来源 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|---------|------|------|--------|
| 401 | `kpi-card` | KpiCard | NEW | Core, Gui, Widgets | — | KPI 指标卡片（数值+趋势+图标） | P1 |
| 402 | `stat-widget` | StatWidget | NEW | Core, Gui, Widgets | — | 统计数值控件（标题+数值+描述） | P1 |
| 403 | `comparison-card` | ComparisonCard | NEW | Core, Gui, Widgets | — | 对比卡片（A vs B） | P2 |
| 404 | `profile-card` | ProfileCard | NEW | Core, Gui, Widgets | — | 个人资料卡片（头像+信息） | P1 |
| 405 | `info-card` | InfoCard | NEW | Core, Gui, Widgets | — | 信息卡片（图标+标题+内容+操作） | P1 |
| 406 | `media-card` | MediaCard | NEW | Core, Gui, Widgets | — | 媒体卡片（封面+标题+描述） | P1 |
| 407 | `list-card` | ListCard | NEW | Core, Gui, Widgets | — | 列表卡片（多行数据） | P1 |
| 408 | `timeline-card` | TimelineCard | NEW | Core, Gui, Widgets | — | 时间线卡片 | P2 |
| 409 | `notification-card` | NotificationCard | NEW | Core, Gui, Widgets | — | 通知卡片 | P1 |
| 410 | `chart-card` | ChartCard | NEW | Core, Gui, Widgets | — | 图表卡片（迷你图表+数据） | P1 |
| 411 | `table-card` | TableCard | NEW | Core, Gui, Widgets | — | 表格卡片（紧凑表格） | P1 |
| 412 | `metric-tile` | MetricTile | NEW | Core, Gui, Widgets | — | 指标磁贴 | P2 |
| 413 | `sparkline` | Sparkline | NEW | Core, Gui, Widgets | — | 迷你折线图 | P1 |
| 414 | `avatar-group` | AvatarGroup | NEW | Core, Gui, Widgets | — | 头像组（重叠显示多人） | P1 |
| 415 | `user-status-badge` | UserStatusBadge | NEW | Core, Gui, Widgets | — | 用户状态徽章 | P1 |
| 416 | `data-summary` | DataSummary | NEW | Core, Gui, Widgets | — | 数据摘要（关键指标一览） | P1 |
| 417 | `change-indicator` | ChangeIndicator | NEW | Core, Gui, Widgets | — | 变化指示器（涨跌/百分比） | P1 |
| 418 | `progress-card` | ProgressCard | NEW | Core, Gui, Widgets | — | 进度卡片（标题+进度条+百分比） | P1 |
| 419 | `breadcrumb-item` | BreadcrumbItem | NEW | Core, Gui, Widgets | — | 面包屑项 | P1 |
| 420 | `pagination-widget` | PaginationWidget | NEW | Core, Gui, Widgets | — | 分页控件（上/下页/跳转） | P1 |

---

## 19-multimedia-widget/ — 多媒体控件 (20)

| # | 目录名 | 类名 | 来源 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|---------|------|------|--------|
| 421 | `video-widget` | VideoWidget | NEW | Core, Gui, Widgets, Multimedia | — | 视频渲染控件 | P1 |
| 422 | `audio-waveform` | AudioWaveform | NEW | Core, Gui, Widgets | — | 音频波形显示 | P1 |
| 423 | `audio-spectrum` | AudioSpectrum | NEW | Core, Gui, Widgets, Multimedia | — | 音频频谱显示 | P1 |
| 424 | `playback-controls` | PlaybackControls | NEW | Core, Gui, Widgets | — | 播放控制条（播放/暂停/进度/音量） | P0 |
| 425 | `volume-control` | VolumeControl | NEW | Core, Gui, Widgets, Multimedia | — | 音量控件（滑块+静音） | P1 |
| 426 | `seek-bar` | SeekBar | NEW | Core, Gui, Widgets | — | 媒体进度条（可拖拽+预览） | P1 |
| 427 | `playlist-widget` | PlaylistWidget | NEW | Core, Gui, Widgets | — | 播放列表面板 | P1 |
| 428 | `lyrics-widget` | LyricsWidget | NEW | Core, Gui, Widgets | — | 歌词显示控件（同步滚动） | P2 |
| 429 | `audio-level-meter` | AudioLevelMeter | NEW | Core, Gui, Widgets, Multimedia | — | 音频电平表 | P2 |
| 430 | `video-overlay` | VideoOverlay | NEW | Core, Gui, Widgets | — | 视频叠加层（OSD 字幕） | P1 |
| 431 | `camera-viewfinder` | CameraViewfinder | NEW | Core, Gui, Widgets, Multimedia | — | 摄像头取景器 | P1 |
| 432 | `thumbnail-strip` | ThumbnailStrip | NEW | Core, Gui, Widgets | — | 缩略图条（视频时间轴） | P1 |
| 433 | `subtitle-widget` | SubtitleWidget | NEW | Core, Gui, Widgets | — | 字幕显示控件 | P2 |
| 434 | `media-info-panel` | MediaInfoPanel | NEW | Core, Gui, Widgets | — | 媒体信息面板（时长/分辨率/编码） | P1 |
| 435 | `recording-indicator` | RecordingIndicator | NEW | Core, Gui, Widgets | — | 录制指示器（红点闪烁） | P1 |
| 436 | `fullscreen-toggle` | FullscreenToggle | NEW | Core, Gui, Widgets | — | 全屏切换按钮 | P1 |
| 437 | `aspect-ratio-selector` | AspectRatioSelector | NEW | Core, Gui, Widgets | — | 宽高比选择 | P2 |
| 438 | `capture-button` | CaptureButton | NEW | Core, Gui, Widgets | — | 截图/拍照按钮 | P1 |
| 439 | `picture-in-picture` | PictureInPicture | NEW | Core, Gui, Widgets | — | 画中画控件 | P2 |
| 440 | `media-playlist-table` | MediaPlaylistTable | NEW | Core, Gui, Widgets | — | 媒体播放列表表格 | P1 |

---

## 20-map-location-widget/ — 地图/定位控件 (15)

| # | 目录名 | 类名 | 来源 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|---------|------|------|--------|
| 441 | `map-widget` | MapWidget | GH | QGeoView | Core, Gui, Widgets, Location | — | 地图显示控件 | P2 |
| 442 | `coordinate-input` | CoordinateInput | NEW | Core, Gui, Widgets | — | 经纬度输入控件 | P2 |
| 443 | `location-picker` | LocationPicker | NEW | Core, Gui, Widgets | — | 位置选择器 | P2 |
| 444 | `distance-calculator` | DistanceCalculator | NEW | Core, Gui, Widgets | — | 距离计算控件 | P2 |
| 445 | `route-display` | RouteDisplay | NEW | Core, Gui, Widgets | — | 路线显示控件 | P2 |
| 446 | `poi-marker` | PoiMarker | NEW | Core, Gui, Widgets | — | 兴趣点标记 | P2 |
| 447 | `geofence-widget` | GeofenceWidget | NEW | Core, Gui, Widgets | — | 电子围栏控件 | P2 |
| 448 | `gps-tracker` | GpsTracker | NEW | Core, Gui, Widgets, Positioning | — | GPS 轨迹追踪 | P2 |
| 449 | `heatmap-overlay` | HeatmapOverlay | NEW | Core, Gui, Widgets | — | 热力图叠加层 | P2 |
| 450 | `zoom-control` | MapZoomControl | NEW | Core, Gui, Widgets | — | 地图缩放控件 | P2 |
| 451 | `layer-switcher` | LayerSwitcher | NEW | Core, Gui, Widgets | — | 图层切换控件 | P2 |
| 452 | `map-search` | MapSearch | NEW | Core, Gui, Widgets | — | 地图搜索控件 | P2 |
| 453 | `coordinate-grid` | CoordinateGrid | NEW | Core, Gui, Widgets | — | 坐标网格控件 | P2 |
| 454 | `scale-bar` | ScaleBar | NEW | Core, Gui, Widgets | — | 比例尺控件 | P2 |
| 455 | `mini-map` | MiniMap | NEW | Core, Gui, Widgets | — | 小地图/鹰眼图 | P2 |

---

## 21-print-scanner-widget/ — 打印/扫描控件 (15)

| # | 目录名 | 类名 | 来源 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|---------|------|------|--------|
| 456 | `print-preview-widget` | PrintPreviewWidget | NEW | Core, Gui, Widgets, PrintSupport | — | 打印预览控件 | P1 |
| 457 | `page-setup-widget` | PageSetupWidget | NEW | Core, Gui, Widgets, PrintSupport | — | 页面设置控件 | P1 |
| 458 | `printer-select` | PrinterSelect | NEW | Core, Gui, Widgets, PrintSupport | — | 打印机选择器 | P1 |
| 459 | `print-range` | PrintRange | NEW | Core, Gui, Widgets, PrintSupport | — | 打印范围选择 | P1 |
| 460 | `copies-widget` | CopiesWidget | NEW | Core, Gui, Widgets, PrintSupport | — | 打印份数/双面设置 | P1 |
| 461 | `barcode-display` | BarcodeDisplay | GH | prison | Core, Gui, Widgets | prison (FetchContent) | 条形码显示控件 | P1 |
| 462 | `qr-code-display` | QrCodeDisplay | NEW | Core, Gui, Widgets | libqrencode (FetchContent) | QR 码显示控件 | P1 |
| 463 | `scan-preview` | ScanPreview | NEW | Core, Gui, Widgets | — | 扫描预览控件 | P2 |
| 464 | `ocr-preview` | OcrPreview | NEW | Core, Gui, Widgets | tesseract (FetchContent) | OCR 预览控件 | P2 |
| 465 | `label-template` | LabelTemplate | NEW | Core, Gui, Widgets | — | 标签模板控件 | P2 |
| 466 | `envelope-template` | EnvelopeTemplate | NEW | Core, Gui, Widgets | — | 信封模板控件 | P2 |
| 467 | `watermark-widget` | WatermarkPrintWidget | NEW | Core, Gui, Widgets, PrintSupport | — | 水印打印控件 | P2 |
| 468 | `header-footer-widget` | HeaderFooterWidget | NEW | Core, Gui, Widgets, PrintSupport | — | 页眉页脚编辑控件 | P2 |
| 469 | `margin-widget` | MarginWidget | NEW | Core, Gui, Widgets, PrintSupport | — | 页边距调整控件 | P2 |
| 470 | `orientation-select` | OrientationSelect | NEW | Core, Gui, Widgets, PrintSupport | — | 纸张方向选择 | P1 |

---

## 22-misc-widget/ — 杂项控件 (30)

| # | 目录名 | 类名 | 来源 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|------|---------|------|------|--------|
| 471 | `material-appbar` | MaterialAppBar | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 应用栏 | P1 |
| 472 | `material-drawer` | MaterialDrawer | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 抽屉 | P1 |
| 473 | `material-tabs` | MaterialTabs | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 标签页 | P1 |
| 474 | `material-textfield` | MaterialTextField | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 文本输入框 | P1 |
| 475 | `material-autocomplete` | MaterialAutoComplete | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 自动补全 | P1 |
| 476 | `material-avatar` | MaterialAvatar | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 头像 | P2 |
| 477 | `material-badge` | MaterialBadge | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 徽章 | P2 |
| 478 | `material-scrollbar` | MaterialScrollBar | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 滚动条 | P1 |
| 479 | `selection-model-demo` | SelectionModelDemo | NEW | Core, Gui, Widgets | — | 选择模型演示 | P1 |
| 480 | `item-delegate-demo` | ItemDelegateDemo | NEW | Core, Gui, Widgets | — | 自定义委托演示 | P1 |
| 481 | `styled-item-delegate` | StyledItemDelegate | NEW | Core, Gui, Widgets | — | 样式化项委托 | P1 |
| 482 | `persistent-editor` | PersistentEditor | NEW | Core, Gui, Widgets | — | 持久编辑器 | P2 |
| 483 | `drag-source-target` | DragSourceTarget | NEW | Core, Gui, Widgets | — | 拖拽源和目标演示 | P1 |
| 484 | `mime-data-custom` | MimeDataCustom | NEW | Core, Gui, Widgets | — | 自定义 MIME 数据 | P2 |
| 485 | `drop-area-widget` | DropAreaWidget | NEW | Core, Gui, Widgets | — | 拖放区域控件 | P1 |
| 486 | `rubber-band-selection` | RubberBandSelection | NEW | Core, Gui, Widgets | — | 橡皮筋框选控件 | P1 |
| 487 | `cursor-widget` | CursorWidget | NEW | Core, Gui, Widgets | — | 自定义光标控件 | P2 |
| 488 | `focus-frame` | FocusFrame | NEW | Core, Gui, Widgets | — | 焦点框控件 | P1 |
| 489 | `keyboard-navigation` | KeyboardNavigation | NEW | Core, Gui, Widgets | — | 键盘导航演示 | P1 |
| 490 | `accessibility-widget` | AccessibilityWidget | NEW | Core, Gui, Widgets, Accessibility | — | 无障碍控件 | P2 |
| 491 | `touch-gesture-widget` | TouchGestureWidget | NEW | Core, Gui, Widgets | — | 触摸手势控件 | P1 |
| 492 | `gesture-recognizer` | GestureRecognizer | NEW | Core, Gui, Widgets | — | 手势识别演示 | P2 |
| 493 | `shortcut-editor` | ShortcutEditor | NEW | Core, Gui, Widgets | — | 快捷键编辑控件 | P1 |
| 494 | `size-grip-widget` | SizeGripWidget | NEW | Core, Gui, Widgets | — | 窗口调整大小手柄 | P2 |
| 495 | `whatsthis-widget` | WhatsThisWidget | NEW | Core, Gui, Widgets | — | "这是什么"帮助控件 | P2 |
| 496 | `status-tip-widget` | StatusTipWidget | NEW | Core, Gui, Widgets | — | 状态提示控件 | P1 |
| 497 | `tool-tip-custom` | ToolTipCustom | NEW | Core, Gui, Widgets | — | 自定义工具提示 | P1 |
| 498 | `action-group-demo` | ActionGroupDemo | NEW | Core, Gui, Widgets | — | QAction 分组演示 | P1 |
| 499 | `menu-bar-custom` | MenuBarCustom | NEW | Core, Gui, Widgets | — | 自定义菜单栏 | P1 |
| 500 | `tool-bar-custom` | ToolBarCustom | NEW | Core, Gui, Widgets | — | 自定义工具栏 | P1 |

---

## 总统计

| 子分类 | P0 | P1 | P2 | 合计 |
|--------|----|----|-----|------|
| 01-button | 1 | 13 | 16 | 30 |
| 02-label | 1 | 12 | 22 | 35 |
| 03-input | 4 | 18 | 8 | 30 |
| 04-progress | 2 | 13 | 20 | 35 |
| 05-meter | 1 | 5 | 34 | 40 |
| 06-slider | 1 | 8 | 11 | 20 |
| 07-chart | 3 | 13 | 9 | 25 |
| 08-table | 4 | 15 | 6 | 25 |
| 09-tree | 3 | 13 | 4 | 20 |
| 10-list | 1 | 16 | 3 | 20 |
| 11-calendar | 0 | 8 | 2 | 10 |
| 12-terminal | 2 | 7 | 1 | 10 |
| 13-specialized | 0 | 17 | 8 | 25 |
| 14-animation | 0 | 14 | 16 | 30 |
| 15-opengl | 0 | 5 | 10 | 15 |
| 16-datetime | 0 | 9 | 6 | 15 |
| 17-network | 0 | 8 | 7 | 15 |
| 18-data-display | 0 | 18 | 2 | 20 |
| 19-multimedia | 1 | 15 | 4 | 20 |
| 20-map-location | 0 | 0 | 15 | 15 |
| 21-print-scanner | 0 | 5 | 10 | 15 |
| 22-misc | 0 | 21 | 9 | 30 |
| **总计** | **24** | **243** | **233** | **500** |
