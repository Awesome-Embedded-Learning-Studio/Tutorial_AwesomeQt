> 以下进阶层 03-qtwidgets 主题能力篇文章已全部完成，归档于 (2026-05-10)

## 03 · QtWidgets 进阶 — 主题能力篇（10 篇）

- [x] 🔴 `01-layout-system-advanced.md` — 布局进阶：尺寸策略与动态布局切换 ✅ 完成于 2026-05-10
  - `QSizePolicy` 六种策略（Fixed/Minimum/Maximum/Preferred/Expanding/Ignored）
  - 布局内插入/删除控件并刷新
  - `QStackedLayout` 实现动画页面切换
  - 嵌套布局性能优化与 `layout()` 调试

- [x] 🔴 `02-event-handling-advanced.md` — 事件处理进阶：键盘修饰键与原生事件 ✅ 完成于 2026-05-10
  - `grabMouse()` / `grabKeyboard()` 强制捕获输入
  - `QApplication::keyboardModifiers()` 获取修饰键状态
  - `QWheelEvent::angleDelta()` 滚轮精细处理
  - `nativeEvent()` 处理平台原生窗口消息

- [x] 🔴 `03-model-view-advanced.md` — Model/View 进阶：自定义 Model 与 Delegate ✅ 完成于 2026-05-10
  - 继承 `QAbstractTableModel` 实现完整 Model
  - `QSortFilterProxyModel` 代理模型实现搜索过滤与排序
  - 自定义 `QStyledItemDelegate` 实现单元格编辑器
  - `QAbstractItemModel::beginInsertRows()` 数据增删的正确通知方式

- [x] 🔴 `04-qss-advanced.md` — QSS 进阶：动态主题切换与复杂选择器 ✅ 完成于 2026-05-10
  - `setObjectName` 配合 `#id` 选择器精准定位
  - `QStyle::polish()` 结合 QSS 动态皮肤系统
  - `qproperty-*` 通过 QSS 设置 Q_PROPERTY 值
  - 高 DPI 下 QSS 像素值的缩放处理

- [x] 🔴 `05-custom-widget-advanced.md` — 自定义控件进阶：子控件与 QStyle ✅ 完成于 2026-05-10
  - `QStyleOption` + `QStylePainter` 跟随系统风格绘制
  - `QStyle::subControlRect()` 子控件矩形计算
  - `QSizePolicy::HeightForWidth` 保持宽高比
  - 发布自定义控件到 Qt Designer 插件

- [x] 🔴 `06-dialog-advanced.md` — 对话框进阶：模态策略与数据验证 ✅ 完成于 2026-05-10
  - `Qt::ApplicationModal` vs `Qt::WindowModal` 模态范围
  - 输入验证失败阻止 `accept()` 的正确姿势
  - 多步骤向导对话框数据流管理
  - 对话框记忆上次位置与尺寸

- [x] 🔴 `07-main-window-advanced.md` — 主窗口进阶：Dock 管理与状态持久化 ✅ 完成于 2026-05-10
  - `saveState()` / `restoreState()` 持久化整个主窗口布局
  - `QSettings` 保存/恢复窗口几何信息
  - 动态显示/隐藏 Dock 的菜单同步
  - `QMainWindow::setCorner()` 角落 Dock 区域归属

- [x] 🟡 `08-graphics-view-advanced.md` — 图形视图进阶：自定义 Item 与碰撞检测 ✅ 完成于 2026-05-10
  - 继承 `QGraphicsItem` 实现完整自定义图元
  - `QGraphicsScene::collidingItems()` 碰撞检测
  - `itemChange()` 拦截位置/状态变化
  - `QGraphicsEffect` 给 Item 加模糊/阴影/颜色效果

- [x] 🟡 `09-animation-advanced.md` — 动画进阶：状态机驱动与并行动画组 ✅ 完成于 2026-05-10
  - `QStateMachine` + 动画 `addTransition` 状态切换动画
  - `QParallelAnimationGroup` 多属性同时动画
  - `QAbstractAnimation::updateCurrentValue()` 自定义插值动画
  - 动画在 QML 与 Widgets 混合界面中的协调

- [x] ⚪ `10-mdi-advanced.md` — MDI 进阶：子窗口策略与文档管理 ✅ 完成于 2026-05-10
  - 子窗口关闭前保存确认（拦截 `QCloseEvent`）
  - `QMdiArea::SubWindowActivated` 同步菜单状态
  - MDI vs 多标签（`QTabWidget`）的场景选择
  - 最大化子窗口时菜单栏合并处理
