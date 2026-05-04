# 06-qml 代码样例现象表

> 以下为各示例运行时的预期现象描述，供逐个核实时对照。

## 01-qml-syntax-basics-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | QML 对象声明与层级关系 | 弹出 640x480 窗口，标题为 "QML Syntax Basics"。窗口内左右两栏布局：左侧白色圆角面板显示类型信息，右侧白色圆角面板显示进度条可视化。初始背景为浅灰色 (#f5f5f5)。 |
| 2 | 基础类型初始值展示（左侧面板） | 左侧面板逐行列出：`int (clickCount): 0`、`real (fillRatio): 0.00`、`string: Ready`、`bool (highlighted): false`、`color: #f5f5f5`。 |
| 3 | 进度条可视化（右侧面板） | 右侧面板底部有一条进度填充条，初始宽度为 0（fillRatio=0），颜色为红色 (#F44336)；中央显示文字 "Fill: 0%"。 |
| 4 | 鼠标点击更新 int/string/real 类型 | 在窗口任意位置点击后：clickCount 递增，fillRatio 变为鼠标 x 坐标占窗口宽度的百分比，statusMessage 更新为 "Click #N at (x, y)"，背景色在 #f5f5f5 与 #E3F2FD 之间切换。左侧面板数值随之实时更新。 |
| 5 | 进度条颜色随 fillRatio 变化 | fillRatio < 0.33 时进度条红色 (#F44336)，0.33~0.66 时蓝色 (#FF9800，即 themeColor)，>= 0.66 时绿色 (#4CAF50)。当 fillRatio 达到 1.0 时中央文字变为 "Full!"。 |
| 6 | id 引用与 JavaScript 表达式 | 进度条宽度通过 `parent.width * fillRatio` 计算；颜色通过 JavaScript if/else 表达式选择；百分比为 `Math.round(fillRatio * 100)`。控制台每次点击打印类似 `Click event: {"x":320,"y":240,"count":1,"ratio":"0.50"}` 的 JSON。 |

## 02-property-binding-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | 属性绑定与窗口布局 | 弹出 700x500 窗口，标题为 "Property Binding Demo"。顶部蓝色 (#2196F3) 标题栏显示 "Property Binding Demo"。中间区域左右两栏：左侧为仪表盘面板，右侧为属性信息面板。底部为控制栏。 |
| 2 | 自定义属性初始值（右侧信息面板） | 右侧面板列出：`sliderValue (int): 50`、`statusText (string): Normal`、`themeColor (color): #2196f3`、`useDarkMode (bool): false`、`scaleFactor (real): 1.00`，以及窗口尺寸和仪表宽度。 |
| 3 | 仪表盘可视化（左侧面板） | 左侧显示一个圆角进度条，初始填充 50%（sliderValue/100=0.5），颜色为蓝色 (#2196F3)，下方显示 "50%" 大字和 "Status: Normal"，底部显示 "scaleFactor: 1.00"。 |
| 4 | onSliderValueChanged 信号处理器 | 拖动底部滑块时：sliderValue < 30 时 statusText="Low"，进度条变红 (#F44336)；30~70 时 statusText="Normal"，进度条为蓝色；>= 70 时 statusText="High"，进度条变绿 (#4CAF50)。控制台打印 `Slider value changed: N Status: xxx`。 |
| 5 | 缩放按钮 | 底部中央有 "Smaller" 和 "Larger" 两个按钮。点击 Smaller 将 scaleFactor 减 0.1（下限 0.5），点击 Larger 加 0.1（上限 2.0）。scaleFactor 值实时反映在左侧面板底部和右侧信息面板中。 |
| 6 | 暗/亮模式切换 | 底部右侧有主题切换按钮，显示 "Light Mode"。点击后 useDarkMode 变为 true，背景变为深色 (#1a1a2e)，所有面板变为深蓝色 (#16213e)，标题栏变为紫色 (#BB86FC)，文字变亮。再次点击恢复浅色。控制台打印 `Theme changed, dark mode: true/false`。 |
| 7 | 属性绑定的响应式联动 | 拖动滑块时，仪表盘进度条、百分比数字、状态文字、右侧面板数值全部同步更新；切换主题时，所有面板背景色、文字色、按钮色同步切换——体现了 QML 属性绑定的自动追踪机制。 |

## 03-qtquick-controls-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | ApplicationWindow 结构 | 弹出 800x600 的 ApplicationWindow，标题为 "Qt Quick Controls Demo"。顶部有工具栏（含标题、主题下拉框、About 按钮），底部有状态栏（左侧显示 "Ready"，右侧显示 "No user set"），中央为 GridLayout 表单。 |
| 2 | 菜单栏 | 菜单栏含 File 和 Help 两个菜单。File 下有 "Reset Settings" 和 "Quit"；Help 下有 "About"（点击弹出 About 对话框）。 |
| 3 | TextField 控件 | 表单中 "Username" 输入框，占位文字为 "Enter your username"。输入内容后，底部状态栏右侧变为绿色 "User: xxx"，Preview 区域同步更新。 |
| 4 | ComboBox 控件 | 表单中有 Language 下拉框，选项为 English/Chinese/Japanese/German。选择后控制台打印 `Language: xxx`，Preview 区域同步显示。 |
| 5 | Slider 控件 | Volume 滑块范围 0~1.0，步进 0.05，初始值 50%。拖动时右侧实时显示百分比（如 "75%"），Preview 同步更新。 |
| 6 | CheckBox 控件 | 两个复选框："Enable notifications" 和 "Enable auto-save"，默认勾选；另有一个 "Show advanced options" 复选框，勾选/取消时控制台打印 `Advanced: true/false`。 |
| 7 | Preview 实时预览区 | 表单下方有灰色圆角预览框，实时显示 Name、Language、Volume、Notifications、Auto-save 的当前值，随表单操作联动更新。 |
| 8 | Apply 按钮与弹出通知 | 点击 "Apply" 按钮，控制台打印 JSON 格式的完整设置信息，同时窗口底部弹出绿色圆角提示 "Settings Applied"，2 秒后自动消失。 |
| 9 | Reset 按钮与确认对话框 | 点击 "Reset" 弹出模态确认对话框 "Are you sure you want to reset all settings to default values?"，点 Yes 后所有表单恢复默认值，控制台打印 `Settings reset to defaults`。 |
| 10 | About 对话框 | 点击工具栏 About 按钮或 Help > About，弹出模态对话框显示 "Qt Quick Controls Demo / Version 1.0 / Built with Qt 6.9.1"。 |
| 11 | 工具栏主题切换 | 工具栏有 ComboBox 可选 System/Light/Dark，切换时控制台打印 `Theme changed to: xxx`。 |
| 12 | 右键上下文菜单 | 在窗口空白处右键弹出菜单，含 "Reset to Default" 和 "Show Current Settings"。"Show Current Settings" 在控制台打印完整 JSON 设置信息。 |

## 04-cpp-qml-interop-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | C++ 属性暴露到 QML（Q_PROPERTY） | 弹出 640x520 窗口，标题为 "C++ / QML Interop Demo"。中央 GridLayout 表单展示 C++ AppController 的属性：userName、counter、themeColor。初始状态：userName 为空，counter 为 0，themeColor 为 "#3498db"。 |
| 2 | TextField 双向绑定 | 输入用户名后，文字同步写入 `appController.userName`（QML -> C++），下方状态面板 "userName: xxx" 实时更新。 |
| 3 | Counter 显示与操作 | Counter 区域显示大字号数字（初始 0），右侧有 "+1" 和 "Reset" 按钮。点击 "+1" 调用 `appController.increment()`，数字递增；点击 "Reset" 调用 `appController.reset()`，归零。 |
| 4 | Q_INVOKABLE 方法返回值 | Greeting 区域是一个以 themeColor 为背景色的圆角矩形，显示 `appController.greeting()` 的返回值。未输入用户名时显示 "Hello, stranger!"；输入后显示 "Hello, xxx! Count: N"。 |
| 5 | Theme 颜色选择 | ComboBox 选择颜色（#3498db/#e74c3c/#2ecc71/#f39c12/#9b59b6），选中后 Greeting 区域背景色平滑过渡（ColorAnimation 300ms）到所选颜色，状态面板同步更新。 |
| 6 | 状态面板实时反映 C++ 属性 | 下方灰色面板持续显示 `userName`、`counter`、`themeColor` 三个属性的当前值，所有操作后实时联动。 |
| 7 | C++ 信号到 QML（定时通知） | 每 5 秒，C++ 端 QTimer 触发 `notificationRequested` 信号。若 userName 非空，窗口顶部弹出深色通知条，显示 "Hey xxx, count is N"，3 秒后淡出（opacity 从 1 渐变到 0，耗时 300ms）。 |
| 8 | 控制台输出 | 操作过程中无显式 console.log 输出（本示例主要通过 GUI 展示互操作）。C++ 端通过 Q_PROPERTY 的 NOTIFY 信号驱动 QML 刷新。 |

## 05-qml-animation-states-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | NumberAnimation 位移动画 | 弹出 800x700 可滚动窗口，标题 "QML Animation & States Demo"。第一个 GroupBox "NumberAnimation - Position" 中有一个蓝色圆球（60x60），标注 "Click"。点击球后，球在 x 方向从 20 到容器右边缘做 InOutCubic 缓动往返动画（周期 1200ms，无限循环），同时 y 方向做微小弹跳（20~30，InOutCubic）。再次点击可暂停/恢复。 |
| 2 | ColorAnimation 颜色渐变 | 第二个 GroupBox "ColorAnimation - Color Transition" 中有一个红色 (#e74c3c) 圆角矩形，中央显示当前颜色值。点击后在红/蓝/绿/橙/紫五色间循环切换，每次颜色过渡 500ms（InOutQuad 缓动），中央文字同步更新。 |
| 3 | Behavior on 悬停效果卡片 | 第三个 GroupBox "Behavior on - Hover Effects" 中有三张并排卡片（Card A 蓝色、Card B 红色、Card C 绿色），各含一个彩色圆圈和标题。鼠标悬停时卡片放大到 1.05 倍、不透明度升为 1.0；移开后恢复 1.0 倍、不透明度 0.8。过渡时长 150ms。 |
| 4 | State + Transition 展开/折叠面板 | 第四个 GroupBox "State + Transition - Expandable Panel" 中有一个深色 (#2c3e50) 圆角面板，初始高度 50px（折叠态），显示 "Click to expand / collapse" 和向下箭头。点击后面板高度动画扩展到 180px，箭头旋转 180 度，内容文字淡入，显示说明文字。再次点击折叠回去。展开动画 300ms（OutCubic），折叠 250ms（InCubic），三者并行。 |
| 5 | SequentialAnimation 脉冲效果 | 第五个 GroupBox "SequentialAnimation - Pulse Effect" 中有一个红色圆角按钮 "Pulse Me"。点击后按钮按顺序执行：放大到 1.2（120ms）-> 缩小到 0.95（80ms）-> 放大到 1.05（60ms）-> 恢复 1.0（40ms），形成弹跳脉冲视觉效果。 |

## 06-qml-model-delegate-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | C++ QAbstractListModel 数据展示 | 弹出 700x600 窗口，标题 "QML Model/Delegate Demo"。初始加载 6 条水果数据（Apple/$5.5/In Stock、Banana/$3.2/In Stock、Grape/$12.0/Out of Stock、Orange/$4.0/In Stock、Blueberry/$18.5/In Stock、Watermelon/$8.0/Out of Stock），每项带有对应颜色的圆点标识。底部统计显示 "Total: 6 items"。 |
| 2 | ListView 列表视图（默认 Tab） | 默认显示 "List View" 标签页。每行为白色圆角矩形，左侧彩色圆点，中间水果名和价格，右侧库存状态（绿色 "In Stock" 或红色 "Out of Stock"），以及 "Del" 删除按钮。鼠标悬停时行略微放大（scale 1.01）。点击某行控制台打印 `Clicked: xxx index: N`。 |
| 3 | GridView 网格视图 | 切换到 "Grid View" 标签页。数据以 3 列网格排列，每格含彩色圆点、水果名、价格、库存状态。点击某格直接删除该项。 |
| 4 | 添加水果 | 顶部操作栏含 Name 输入框、Color 下拉框（6 种颜色）、Price 数字框（默认 $10.0）、In Stock 复选框（默认勾选）、"Add" 和 "Clear All" 按钮。输入名称后点击 Add，新水果追加到列表末尾。 |
| 5 | 删除水果 | 列表视图中点击 "Del" 按钮，或网格视图中点击卡片，调用 `fruitModel.removeFruit(index)`，对应条目从视图中移除，底部统计数字更新。 |
| 6 | 清空所有 | 点击 "Clear All" 调用 `fruitModel.clearAll()`，所有数据清空，列表/网格变为空，统计显示 "Total: 0 items"。 |
| 7 | C++ Model 驱动视图更新 | 所有增删操作通过 C++ FruitModel 的 `beginInsertRows`/`endInsertRows`、`beginRemoveRows`/`endRemoveRows`、`beginResetModel`/`endResetModel` 通知 QML 视图自动刷新，无需手动更新 UI。 |

## 07-qml-canvas-particles-beginner

| 序号 | 演示要点 | 预期现象 |
|------|----------|----------|
| 1 | Canvas 涂鸦板 | 弹出 800x750 窗口，标题 "QML Canvas & Particles Demo"。第一个 GroupBox "Canvas - Drawing Pad" 含工具栏和白色画布。工具栏有 6 种颜色圆点可选（红/蓝/绿/橙/紫/深蓝）、笔刷大小滑块（2~20，默认 4）、Clear 按钮。选中颜色后圆点显示边框高亮。按住鼠标在画布上拖动可自由绘图，笔触为选定颜色和粗细的圆头线条。点击 Clear 清空画布为白色。 |
| 2 | Canvas 轨道动画 | 第二个 GroupBox "Canvas - Orbital Animation" 显示深蓝色 (#0d1b2a) 画布。画布中央有一个金色发光"太阳"（径向渐变，核心黄色 #f1c40f），外围三个同心轨道环（半透明白色圆环）。三个彩色球体（红/绿/蓝，外层球最大、内层最小，带径向渐变光晕）以不同速度沿轨道运转，内层最快。动画以约 60fps（16ms 定时器）持续运行，形成太阳系运行效果。 |
| 3 | ParticleSystem 烟花粒子 | 第三个 GroupBox "ParticleSystem - Fireworks" 展示粒子系统。画面中有两类粒子：(1) 环境粒子——缓慢向下飘落的微小光点（速率 30/s，寿命 4s），营造氛围；(2) 烟花爆发——每 3 秒在画面随机位置发射一批火花（速率 120/s，最多 300 个，寿命 2s），初始向上发射（角度 270 +/- 30 度），受重力（向下 magnitude=40）和湍流（strength=10）影响，形成弧形下落轨迹。粒子使用发光圆点贴图（glowdot.png），橙色基调 (#f39c12)，颜色变化 0.6，带有淡入效果。 |
