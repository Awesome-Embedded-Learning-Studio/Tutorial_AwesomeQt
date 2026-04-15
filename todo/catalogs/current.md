# 当前练习项目需求清单

> 挑选标准：无外部依赖、仅 Core/Gui/Widgets、代码量小、适合练手
> 建议练习顺序：Widget → Model → App

---

## 一、Widget（单个自定义控件）

### 1. toggle-switch — iOS 风格开关按钮

| 项目 | 内容 |
|------|------|
| 目录 | `widget/01-button/toggle-switch/` |
| 类名 | `ToggleSwitch` |
| 参考 | TTK `Button/toggleWidget/` |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 开/关两种状态，点击或拖拽切换
- 开启时滑块滑到右侧，背景色变为绿色；关闭时滑到左侧，背景灰色
- 滑块移动带平滑动画过渡（QPropertyAnimation，时长 ~200ms）
- 圆角矩形外框 + 圆形滑块，纯 paintEvent 绘制
- 提供 API：`setChecked(bool)`、`isChecked()`、`toggle()`
- 发出信号：`toggled(bool checked)`

**界面规格：**
- 默认尺寸约 48×26 px（可拉伸宽度）
- 外框圆角 = 高度/2，滑块直径 = 高度 - 4px
- 开启色 `#4CD964`，关闭色 `#E5E5EA`，滑块白色

**验收标准：**
- [ ] 点击切换状态，信号正确发出
- [ ] 拖拽切换（鼠标按下→移动→释放）
- [ ] 动画平滑无跳变
- [ ] resizeEvent 后比例正确

---

### 2. status-led — 状态指示灯

| 项目 | 内容 |
|------|------|
| 目录 | `widget/02-label/status-led/` |
| 类名 | `StatusLed` |
| 参考 | 无（NEW） |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 支持 4 种颜色状态：绿色(正常)、红色(错误)、黄色(警告)、灰色(离线)
- 支持闪烁模式：通过 QTimer 控制亮/灭交替
- API：`setColor(QColor)` / `setState(State)`、`setBlinking(bool)`、`setBlinkInterval(int ms)`
- 枚举 `State { Ok, Error, Warning, Off }`
- 信号：`clicked()`（可选）

**界面规格：**
- 默认尺寸 16×16 px，正圆形
- 发光效果：用 QRadialGradient 绘制，中心亮边缘暗
- 闪烁时透明度在 1.0 ↔ 0.3 之间交替

**验收标准：**
- [ ] 设置不同状态显示对应颜色
- [ ] 闪烁模式正常工作
- [ ] 闪烁间隔可配置
- [ ] 控件尺寸可缩放，始终为正圆

---

### 3. search-edit — 搜索输入框

| 项目 | 内容 |
|------|------|
| 目录 | `widget/03-input/search-edit/` |
| 类名 | `SearchEdit` |
| 参考 | 无（NEW） |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 继承 QLineEdit，左侧搜索图标，输入内容后右侧出现清除按钮
- 占位文字提示（placeholder），默认 "搜索..."
- 输入文字变化时发出信号：`textChanged(const QString&)`（QLineEdit 原生）
- 点击清除按钮清空输入框并聚焦
- API：`setPlaceholderText(QString)`、`setClearButtonEnabled(bool)`

**界面规格：**
- 高度 32px，左侧图标边距 8px
- 无输入时：显示搜索图标 + 占位文字，清除按钮隐藏
- 有输入时：显示搜索图标 + 文字 + 清除按钮
- 可通过 QSS 设置边框、圆角、背景色
- focus 时边框高亮

**验收标准：**
- [ ] 无输入时无清除按钮，有输入时出现
- [ ] 点击清除按钮清空并聚焦
- [ ] 回车发出 returnPressed 信号（原生支持）
- [ ] QSS 可正常美化

---

## 二、Model（控件组合/模式）

### 4. form-layout — 表单布局

| 项目 | 内容 |
|------|------|
| 目录 | `model/02-navigation-layout/form-layout/` |
| 参考 | 无（NEW） |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 使用 QFormLayout 构建一个标准表单页面
- 包含以下控件组合：
  - QLineEdit（单行文本）
  - QTextEdit（多行文本）
  - QComboBox（下拉选择）
  - QSpinBox（数字步进）
  - QCheckBox（复选框）
  - QRadioButton（单选，2个一组）
  - QDateEdit（日期选择）
  - QPushButton（提交/重置）
- 标签文字左对齐，控件左对齐，统一间距
- 提交按钮：收集所有控件值，用 QMessageBox 显示
- 重置按钮：恢复所有控件默认值

**界面规格：**
- 窗口标题 "表单布局示例"
- 表单区域居中，宽度约 400px
- 字段行高约 30px，字段间距 10px
- 提交和重置按钮水平排列，右对齐

**验收标准：**
- [ ] 所有控件正常显示和交互
- [ ] 提交弹出 QMessageBox 显示所有字段值
- [ ] 重置恢复默认值
- [ ] 窗口 resize 时布局自适应

---

### 5. toast-notification — Toast 通知系统

| 项目 | 内容 |
|------|------|
| 目录 | `model/05-notification-dialog/toast-notification/` |
| 类名 | `ToastNotification` |
| 参考 | TTK `Window/notifyWindow/` |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 静态方法 `show(const QString &message, QWidget *parent, int duration = 3000)`
- 在 parent 窗口顶部居中弹出，自动消失
- 支持类型区分：`Info`（蓝）、`Success`（绿）、`Warning`（橙）、`Error`（红）
- 多条 toast 同时出现时垂直堆叠，间距 8px
- 出现动画：从上方滑入（透明度 0→1）；消失动画：淡出 + 向上滑出
- 动画结束后自动 deleteLater

**界面规格：**
- 宽度自适应文字，最大 360px，最小 200px
- 高度约 40px，圆角 6px
- 白色背景 + 左侧 4px 颜色条（根据类型）
- 文字居左，14px，颜色 `#333333`

**验收标准：**
- [ ] 调用 show() 弹出通知，duration 后自动消失
- [ ] 4 种类型颜色正确
- [ ] 多条同时弹出时垂直排列不重叠
- [ ] 出现/消失动画流畅
- [ ] parent resize 时位置跟随

---

### 6. confirm-dialog — 操作确认对话框

| 项目 | 内容 |
|------|------|
| 目录 | `model/05-notification-dialog/confirm-dialog/` |
| 类名 | `ConfirmDialog` |
| 参考 | 无（NEW） |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 构造函数：`ConfirmDialog(const QString &title, const QString &message, QWidget *parent = nullptr)`
- 静态便捷方法：`static int confirm(const QString &title, const QString &message, QWidget *parent)`
- 返回值：`QMessageBox::Yes` / `QMessageBox::No`
- 两个按钮：「确认」(主按钮)、「取消」(次按钮)
- 支持设置按钮文字：`setConfirmText(QString)`、`setCancelText(QString)`
- 支持设置图标：默认警告图标，可选问号/错误/信息
- 支持 Esc 关闭（等同于取消）

**界面规格：**
- 固定宽度 380px，高度自适应
- 标题区：图标 + 标题文字（16px 加粗）
- 内容区：消息文字（14px），支持换行
- 按钮区：右对齐，按钮间距 10px

**验收标准：**
- [ ] 静态方法正常弹出并返回正确值
- [ ] 确认返回 Yes，取消/Esc 返回 No
- [ ] 自定义按钮文字生效
- [ ] 不同图标类型显示正确

---

## 三、App（完整应用）

### 7. code-counter — 代码行数统计

| 项目 | 内容 |
|------|------|
| 目录 | `app/01-dev-tools/code-counter/` |
| 参考 | QWD `tool/countcode/` |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 选择文件/文件夹（QFileDialog）
- 递归遍历目录，按扩展名筛选（可配置：.cpp .h .java .py 等）
- 统计项：总文件数、总行数、代码行（非空非注释）、注释行、空白行
- 用 QTableWidget 展示结果：文件路径 | 总行数 | 代码行 | 注释行 | 空白行
- 底部状态栏显示汇总统计
- 支持导出结果为 CSV

**界面规格：**
- 菜单栏：「文件」→ 选择文件 / 选择文件夹 / 退出
- 工具栏：扩展名过滤器输入框（ QLineEdit + "扫描" QPushButton ）
- 中央：QTableWidget，列可排序
- 底部状态栏："共 X 个文件，Y 总行数，Z 代码行"

**验收标准：**
- [ ] 选择单个文件正确统计
- [ ] 选择文件夹递归遍历统计
- [ ] 扩展名过滤器生效
- [ ] 表格列点击可排序
- [ ] 导出 CSV 文件正确

---

### 8. base64-helper — Base64 编解码工具

| 项目 | 内容 |
|------|------|
| 目录 | `app/01-dev-tools/base64-helper/` |
| 参考 | QWD `tool/base64helper/` |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 上下两个 QTextEdit，上方输入，下方输出
- 编码按钮：将上方文本 Base64 编码后显示在下方
- 解码按钮：将下方文本 Base64 解码后显示在上方
- 文件模式：支持选择文件进行 Base64 编码/解码（二进制安全）
- 一键复制结果到剪贴板
- 自动检测输入是否为有效 Base64 并提示
- 支持 UTF-8 和 Latin-1 编码切换

**界面规格：**
- 左侧操作面板（宽 160px）：
  - 编码 / 解码 / 交换 三个 QPushButton
  - 编码选择：QComboBox（UTF-8 / Latin-1）
  - 文件编码 / 文件解码 两个按钮
- 右侧上下分栏（QSplitter）：
  - 上方 QTextEdit：输入区，placeholder "输入原文..."
  - 下方 QTextEdit：输出区，placeholder "结果将显示在这里..."

**验收标准：**
- [ ] 编码/解码功能正确（中英文混合）
- [ ] 交换按钮互换上下内容
- [ ] 复制按钮复制结果到剪贴板
- [ ] 文件编解码正确（图片等二进制）
- [ ] 无效 Base64 输入有错误提示

---

### 9. ini-editor — INI 配置编辑器

| 项目 | 内容 |
|------|------|
| 目录 | `app/01-dev-tools/ini-editor/` |
| 参考 | MTC `Qt/TestQt_20210425_ini/` |
| Qt 模块 | Core, Gui, Widgets |
| 依赖 | 无 |

**功能需求：**
- 打开 .ini 文件（QFileDialog，过滤器 `*.ini`）
- 用 QTreeWidget 展示 INI 结构：Section（一级节点）→ Key=Value（二级节点）
- 双击 Value 列可编辑
- 右键菜单：新增 Section / 新增 Key / 删除
- 工具栏：新建 / 打开 / 保存 / 另存为
- 修改后未保存时标题栏显示 `*` 提示
- 保存时使用 QSettings 写入

**界面规格：**
- 菜单栏：「文件」→ 新建 / 打开 / 保存 / 另存为 / 退出
- 中央：QTreeWidget，2 列（Key / Value），宽度比 1:2
- 底部状态栏：当前文件路径 + Section/Key 总数

**验收标准：**
- [ ] 打开 INI 文件正确解析为树形结构
- [ ] 双击编辑 Value，回车确认
- [ ] 新增/删除 Section 和 Key
- [ ] 保存后文件内容正确
- [ ] 未保存修改有提示

---

## 练习顺序建议

```
Week 1: Widget
  toggle-switch    ← paintEvent + QPropertyAnimation
  status-led       ← QTimer + QRadialGradient
  search-edit      ← QLineEdit 继承 + QAction

Week 2: Model
  form-layout      ← QFormLayout 标准用法
  confirm-dialog   ← 对话框组合 + 静态方法
  toast-notification ← 定时器 + 动画 + 多实例管理

Week 3: App
  code-counter     ← 文件遍历 + QTableWidget
  base64-helper    ← QByteArray 编解码 + QSplitter
  ini-editor       ← QSettings + QTreeWidget + 文件 IO
```

## 目录结构约定

```
practice/
├── widget/
│   ├── toggle-switch/
│   │   ├── CMakeLists.txt
│   │   ├── main.cpp
│   │   └── toggleswitch.h / toggleswitch.cpp
│   ├── status-led/
│   └── search-edit/
├── model/
│   ├── form-layout/
│   ├── toast-notification/
│   └── confirm-dialog/
└── app/
    ├── code-counter/
    ├── base64-helper/
    └── ini-editor/
```

每个项目独立的 CMakeLists.txt，`find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)` 构建。
