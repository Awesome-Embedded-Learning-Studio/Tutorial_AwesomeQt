// 首页 + 状态栏数据源（工作台版）
//
// 溯源纪律：编译输出每一行都必须能在 scripts/build_examples.py 的 print 语句、
// .github/workflows/build-examples.yml 或其真实运行日志中找到出处，不许手写编造。
// 当前快照出处：run 31315533803（2026-08-09 · main@ad27220 "fix: ci crash" · success），
// 摘要行「总数/通过/失败/耗时」由四行日志压缩为一行，数值原样。
// 本地实测单元数会随实例库新增而漂移（快照日 253，2026-08-25 本地 dry-run 已 254），
// 刷新快照时以最新成功 run 为准，并同步下方 statusBar.ref。
//
// examplesTiles 的中文名/一句话来自各实例 index.md 的 title/description（2026-08-25 抄录）。

/** 编译输出面板会话行（首页 CompileOutput 面板） */
export type SessionLine = {
  kind: 'cmd' | 'out' | 'dim' | 'stamp'
  text: string
}

export const ideSession: SessionLine[] = [
  { kind: 'cmd', text: 'python3 scripts/build_examples.py --workers 6' },
  { kind: 'out', text: 'build_examples: 发现 253 个编译单元' },
  { kind: 'dim', text: '  ccache: on (/usr/bin/ccache)' },
  { kind: 'out', text: '  ✓ [128/253] examples/beginner/01-qtbase/02-signal-slot-beginner (13.5s)' },
  { kind: 'out', text: '  ✓ [251/253] app (73.6s)' },
  { kind: 'out', text: '  ✓ [252/253] model (70.8s)' },
  { kind: 'out', text: '  ✓ [253/253] widget (95.0s)' },
  { kind: 'dim', text: '  总数: 253 · 通过: 253 · 失败: 0 · 耗时: 774.6s' },
  { kind: 'out', text: '  全部编译通过！' },
  { kind: 'stamp', text: '[CI VERIFIED] build-examples · main@ad27220 · PASS' },
]

/** 状态栏（全站底部）：数字与 ref 为快照，随 CI 快照刷新 */
export const statusBar = {
  ready: '就绪',
  ci: '构建: CI ✓',
  units: '单元: 253/253',
  qt: 'Qt 6.9.1',
  kit: '桌面 QtWidgets · Qt 6.9.1',
  repo: 'https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt',
  ref: 'main@ad27220',
} as const

/** 三层学习卡（进度数字 = 篇数快照；专家层已审结 19 / 规划 102，2026-08-26 作者确认全审结） */
export type LayerCard = {
  key: string
  layer: string
  title: string
  subtitle: string
  done: number
  total: number
  href: string
  status: 'done' | 'wip'
}

export const layerCards: LayerCard[] = [
  {
    key: 'beginner',
    layer: 'LAYER 1',
    title: '入门',
    subtitle: '能跑起来，理解核心概念，知其然',
    done: 137,
    total: 137,
    href: '/beginner/',
    status: 'done',
  },
  {
    key: 'advanced',
    layer: 'LAYER 2',
    title: '进阶',
    subtitle: '知晓背后原理与高级 API，写稳健代码',
    done: 134,
    total: 134,
    href: '/advanced/',
    status: 'done',
  },
  {
    key: 'expert',
    layer: 'LAYER 3',
    title: '专家',
    subtitle: '逐篇拆 Qt 源码，每条结论带 文件:行号 证据',
    done: 19,
    total: 102,
    href: '/expert/',
    status: 'wip',
  },
]

/** 实例库示例画廊（中文名/一句话抄自各实例 index.md；img 仅 4 件有真图） */
export type ExampleTile = {
  lib: 'widget' | 'app' | 'model'
  name: string
  zh: string
  note: string
  img?: string
}

export const examplesTiles: ExampleTile[] = [
  // widget 栏 13 件
  { lib: 'widget', name: 'toggle-switch', zh: '滑动开关', note: '自绘开关，拖拽/点击切换 + 滑动动画', img: '/carousel/toggle-switch.webp' },
  { lib: 'widget', name: 'status-led', zh: '状态指示灯', note: '四态颜色平滑过渡 + 呼吸闪烁', img: '/carousel/status-led.webp' },
  { lib: 'widget', name: 'speed-meter', zh: '速度仪表盘', note: '动画指针平滑旋转 + 主/次刻度 + 数字读数' },
  { lib: 'widget', name: 'circle-progress', zh: '圆形进度环', note: '背景环 + 顺时针进度弧 + 中心百分比' },
  { lib: 'widget', name: 'line-chart', zh: '折线图', note: '纯 QPainter 自绘 Y 轴自适应折线图' },
  { lib: 'widget', name: 'editable-table', zh: '可编辑表格', note: '按列声明类型 + 委托校验 + 增删改' },
  { lib: 'widget', name: 'range-slider', zh: '范围滑块', note: '双手柄自绘滑块，支持区间拖动' },
  { lib: 'widget', name: 'checkbox-tree', zh: '勾选树', note: '三态勾选级联（父子联动）' },
  { lib: 'widget', name: 'checkbox-list', zh: '勾选列表', note: 'QListWidget 封装的全选/反选列表' },
  { lib: 'widget', name: 'log-viewer', zh: '日志视图', note: '按级别染色 + 自动滚底 + 关键词过滤' },
  { lib: 'widget', name: 'password-edit', zh: '密码输入', note: '显隐切换 + 实时强度指示' },
  { lib: 'widget', name: 'ip-edit', zh: 'IP 输入', note: '4 段八位组输入 + 点分隔 + 自动跳段' },
  { lib: 'widget', name: 'fade-animation', zh: '淡入淡出容器', note: 'QGraphicsOpacityEffect 透明度动画' },
  // app 栏 7 件
  { lib: 'app', name: 'image-viewer', zh: '图片查看器', note: 'QImage 加载 + 缩放旋转 + 缩略图栏' },
  { lib: 'app', name: 'json-editor', zh: 'JSON 编辑器', note: '语法高亮 + 树形预览 + 格式校验' },
  { lib: 'app', name: 'sqlite-browser', zh: 'SQLite 浏览器', note: '建表浏览数据 + SQL 执行 + 结果表格' },
  { lib: 'app', name: 'serial-tool', zh: '串口调试助手', note: 'QSerialPort 配置面板 + 收发日志' },
  { lib: 'app', name: 'network-tool', zh: '网络调试助手', note: 'TCP/UDP 服务端客户端 + 十六进制收发' },
  { lib: 'app', name: 'tetris', zh: '俄罗斯方块', note: '纯 Widgets 自绘棋盘 + 完整游戏逻辑' },
  { lib: 'app', name: 'cpu-memory-monitor', zh: '系统监控', note: '跨平台读取 CPU/内存 + 实时曲线' },
  // model 栏 5 件
  { lib: 'model', name: 'custom-model', zh: '自定义模型', note: 'QAbstractTableModel 子类化自管数据源' },
  { lib: 'model', name: 'proxy-model', zh: '代理模型', note: 'QSortFilterProxyModel 自定义排序过滤' },
  { lib: 'model', name: 'tree-drag-move', zh: '树拖拽', note: '重写 mimeTypes/mimeData 实现节点移动' },
  { lib: 'model', name: 'observer-pattern', zh: '观察者模式', note: '一个信号驱动多个面板的广播演示' },
  { lib: 'model', name: 'toast-notification', zh: 'Toast 提示', note: '无边框置顶气泡 + 透明度动画' },
]

/** 生态定位链接（首页底部一行 + 状态栏 GitHub） */
export const aelsLinks = [
  { text: 'Awesome-Embedded-Learning-Studio', link: 'https://github.com/Awesome-Embedded-Learning-Studio' },
  { text: 'EmbedBox', link: 'https://github.com/Awesome-Embedded-Learning-Studio/EmbedBox' },
  { text: 'Tutorial_AwesomeModernCPP', link: 'https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeModernCPP' },
  { text: 'QuarkWidgets', link: 'https://github.com/Awesome-Embedded-Learning-Studio/QuarkWidgets' },
]

/** CI 运行记录链接 */
export const ACTIONS_URL =
  'https://github.com/Awesome-Embedded-Learning-Studio/Tutorial_AwesomeQt/actions/workflows/build-examples.yml'
