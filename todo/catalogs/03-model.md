# model/ — 控件组合与设计模式清单

> 本文档列出所有计划实现的控件组合与设计模式实例。每个项目展示多个控件协作的方式，
> 放置在项目根目录 `model/<子类>/<目录名>/` 下，使用 Qt 6 + CMake 构建。

**总计：317 项 | 子类：17 个**

## 参考来源说明

| 标记 | 含义 |
|------|------|
| TTK | 参考 [TTKWidgetTools](https://github.com/Greedysky/TTKWidgetTools)（LGPL-3.0） |
| QWD | 参考 [QWidgetDemo](https://github.com/feiyangqingyun/QWidgetDemo) |
| MTC | 参考 [MyTestCode](https://github.com/gongjianbo/MyTestCode) |
| GH | 参考 GitHub 社区开源项目（各项目自有许可证） |
| NEW | 基于常见 GUI 模式全新设计 |

---

---

## 01-window-framework/ — 窗口框架 (20)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 1 | `frameless-window` | QWD | `widget/framelesswidget/` | Core, Gui, Widgets | — | 无边框窗口（标题栏+拖拽+缩放+Aero Snap） | P0 |
| 2 | `movable-widget` | QWD/TTK | `widget/movewidget/`+`Window/moveWidget/` | Core, Gui, Widgets | — | 通用可拖拽窗口基类 | P1 |
| 3 | `shaped-window` | QWD | `other/bgdemo/` | Core, Gui, Widgets | — | 不规则/异形窗口 | P2 |
| 4 | `splash-screen` | TTK | `Window/splashScreen/`+`anSplashScreen/` | Core, Gui, Widgets | — | 启动画面（带进度条/动画） | P1 |
| 5 | `mdi-window` | NEW | — | Core, Gui, Widgets | — | MDI 多文档窗口 | P1 |
| 6 | `tab-window` | NEW | — | Core, Gui, Widgets | — | Tab 多页窗口 | P1 |
| 7 | `dock-window` | GH | Qt-Advanced-Docking-System | Core, Gui, Widgets | ads (FetchContent) | 高级 Dock 停靠窗口 | P0 |
| 8 | `ribbon-window` | GH | SARibbon | Core, Gui, Widgets | SARibbon (FetchContent) | Ribbon 工具栏窗口 | P1 |
| 9 | `multi-window-manager` | NEW | — | Core, Gui, Widgets | — | 多窗口管理器（cascade/tile） | P1 |
| 10 | `window-state-memory` | NEW | — | Core, Gui, Widgets | — | 窗口状态记忆（位置/大小/最大化） | P0 |
| 11 | `window-shadow` | NEW | — | Core, Gui, Widgets | — | 窗口阴影效果 | P1 |
| 12 | `window-blur` | NEW | — | Core, Gui, Widgets | — | 窗口模糊背景（Win10 Acrylic） | P2 |
| 13 | `window-topmost` | NEW | — | Core, Gui, Widgets | — | 窗口置顶切换 | P1 |
| 14 | `window-minimize-tray` | NEW | — | Core, Gui, Widgets | — | 最小化到托盘 | P1 |
| 15 | `window-drag-file` | NEW | — | Core, Gui, Widgets | — | 窗口接受文件拖放 | P1 |
| 16 | `window-resize-grip` | NEW | — | Core, Gui, Widgets | — | 窗口右下角调整大小手柄 | P2 |
| 17 | `window-animate-show-hide` | NEW | — | Core, Gui, Widgets | — | 窗口显示/隐藏动画 | P1 |
| 18 | `window-single-instance` | NEW | — | Core, Gui, Widgets, Network | — | 单实例窗口（QLocalServer） | P0 |
| 19 | `window-taskbar-progress` | NEW | — | Core, Gui, Widgets | — | 任务栏进度条 | P1 |
| 20 | `window-messagebox` | NEW | — | Core, Gui, Widgets | — | 自定义消息框（替代 QMessageBox） | P1 |

---

## 02-navigation-layout/ — 导航与布局 (25)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 21 | `sidebar-navigation` | TTK | `Title/functionListHWidget/`+`functionListVWidget/` | Core, Gui, Widgets | — | 侧边栏导航（水平/垂直） | P0 |
| 22 | `nav-function-bar` | TTK | `Title/functionNormalWidget/`+`functionNavigationWidget/` | Core, Gui, Widgets | — | 功能导航栏 | P1 |
| 23 | `collapsible-toolbox` | TTK | `Title/functionToolboxWidget/` | Core, Gui, Widgets | — | 可折叠工具箱 | P1 |
| 24 | `multi-panel-grid` | QWD | `video/videopanel/` | Core, Gui, Widgets | — | 多面板网格布局 | P1 |
| 25 | `nine-grid-menu` | QWD | `ui/uidemo09/` | Core, Gui, Widgets | — | 九宫格导航 | P2 |
| 26 | `flat-main-ui` | QWD | `ui/uidemo10/` | Core, Gui, Widgets | — | 扁平化主界面 | P1 |
| 27 | `breadcrumb-nav` | NEW | — | Core, Gui, Widgets | — | 面包屑导航 | P1 |
| 28 | `wizard-page` | NEW | — | Core, Gui, Widgets | — | 向导界面（上一步/下一步/完成） | P0 |
| 29 | `stepper` | NEW | — | Core, Gui, Widgets | — | 步骤条（横向/纵向） | P1 |
| 30 | `tab-bar-navigation` | NEW | — | Core, Gui, Widgets | — | 标签栏导航（可滚动/可关闭） | P1 |
| 31 | `drawer-navigation` | NEW | — | Core, Gui, Widgets | — | 抽屉导航（左/右滑出） | P1 |
| 32 | `hamburger-menu` | NEW | — | Core, Gui, Widgets | — | 汉堡菜单 | P1 |
| 33 | `timeline-layout` | NEW | — | Core, Gui, Widgets | — | 时间线布局 | P2 |
| 34 | `accordion-layout` | NEW | — | Core, Gui, Widgets | — | 手风琴折叠面板 | P1 |
| 35 | `splitter-layout` | NEW | — | Core, Gui, Widgets | — | 分割器布局（可拖拽/比例锁定） | P1 |
| 36 | `responsive-layout` | NEW | — | Core, Gui, Widgets | — | 响应式布局（自适应窗口大小） | P1 |
| 37 | `card-layout` | NEW | — | Core, Gui, Widgets | — | 卡片布局 | P1 |
| 38 | `masonry-layout` | NEW | — | Core, Gui, Widgets | — | 瀑布流布局 | P1 |
| 39 | `grid-flow-layout` | NEW | — | Core, Gui, Widgets | — | 流式网格布局 | P1 |
| 40 | `form-layout` | NEW | — | Core, Gui, Widgets | — | 表单布局（标签+输入对齐） | P0 |
| 41 | `overlay-layout` | NEW | — | Core, Gui, Widgets | — | 叠加层布局 | P1 |
| 42 | `empty-state` | NEW | — | Core, Gui, Widgets | — | 空状态占位（图标+文字+操作按钮） | P1 |
| 43 | `loading-state` | NEW | — | Core, Gui, Widgets | — | 加载状态（骨架屏/Spinner） | P1 |
| 44 | `error-state` | NEW | — | Core, Gui, Widgets | — | 错误状态（重试按钮） | P1 |
| 45 | `scroll-position-indicator` | NEW | — | Core, Gui, Widgets | — | 滚动位置指示器 | P2 |

---

## 03-page-transition/ — 页面切换 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 46 | `animation-stack` | TTK/QWD | `Widget/animationStackedWidget/` | Core, Gui, Widgets | — | 动画切换堆叠页面 | P0 |
| 47 | `image-carousel` | TTK/MTC | `Widget/pictureBannerWidget/` | Core, Gui, Widgets | — | 图片轮播/Banner | P1 |
| 48 | `cover-flow` | TTK | `Widget/pictureFlowWidget/` | Core, Gui, Widgets | — | 3D Cover Flow | P2 |
| 49 | `layout-animation` | TTK | `Widget/layoutAnimationWidget/` | Core, Gui, Widgets | — | 布局增删动画 | P2 |
| 50 | `flip-transition` | NEW | — | Core, Gui, Widgets | — | 3D 翻转过渡 | P1 |
| 51 | `fade-transition` | NEW | — | Core, Gui, Widgets | — | 淡入淡出过渡 | P1 |
| 52 | `slide-transition` | NEW | — | Core, Gui, Widgets | — | 滑动过渡（多方向） | P1 |
| 53 | `zoom-transition` | NEW | — | Core, Gui, Widgets | — | 缩放过渡 | P1 |
| 54 | `blinds-transition` | NEW | — | Core, Gui, Widgets | — | 百叶窗过渡 | P2 |
| 55 | `checker-transition` | NEW | — | Core, Gui, Widgets | — | 棋盘格过渡 | P2 |
| 56 | `circle-reveal-transition` | NEW | — | Core, Gui, Widgets | — | 圆形展开过渡 | P2 |
| 57 | `wipe-transition` | NEW | — | Core, Gui, Widgets | — | 擦除过渡 | P2 |
| 58 | `dissolve-transition` | NEW | — | Core, Gui, Widgets | — | 溶解过渡 | P2 |
| 59 | `push-transition` | NEW | — | Core, Gui, Widgets | — | 推入过渡 | P2 |
| 60 | `crossfade-transition` | NEW | — | Core, Gui, Widgets | — | 交叉淡入淡出 | P1 |

---

## 04-theme-styling/ — 主题与样式 (20)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 61 | `theme-system` | QWD/MTC | `ui/styledemo/`+`Qt/QtSwitchSkin/` | Core, Gui, Widgets | — | 多主题切换系统 | P0 |
| 62 | `flat-ui-style` | QWD | `ui/flatui/`+`ui/uidemo01/` | Core, Gui, Widgets | — | 扁平化 UI 风格合集 | P1 |
| 63 | `icon-font-system` | QWD | `ui/iconhelper/` | Core, Gui, Widgets | — | 图标字体系统 | P0 |
| 64 | `dark-theme` | NEW | — | Core, Gui, Widgets | — | 暗色主题 | P0 |
| 65 | `light-theme` | NEW | — | Core, Gui, Widgets | — | 亮色主题 | P0 |
| 66 | `system-theme` | NEW | — | Core, Gui, Widgets | — | 跟随系统主题 | P0 |
| 67 | `qss-engine` | NEW | — | Core, Gui, Widgets | — | 自定义 QSS 引擎（变量/嵌套） | P1 |
| 68 | `icon-theme` | NEW | — | Core, Gui, Widgets | — | 图标主题管理 | P1 |
| 69 | `font-theme` | NEW | — | Core, Gui, Widgets | — | 字体主题管理 | P1 |
| 70 | `animation-theme-switch` | NEW | — | Core, Gui, Widgets | — | 动画主题切换效果 | P1 |
| 71 | `color-scheme` | NEW | — | Core, Gui, Widgets | — | 配色方案管理（Material/自定义） | P1 |
| 72 | `widget-style-factory` | NEW | — | Core, Gui, Widgets | — | QStyle 工厂（自定义 Style） | P1 |
| 73 | `style-sheet-inheritance` | NEW | — | Core, Gui, Widgets | — | QSS 继承体系 | P1 |
| 74 | `qss-image-scaling` | MTC | `Qt/TestQt_20260407_QssImage/` | Core, Gui, Widgets | — | QSS 图片 DPI 缩放 | P1 |
| 75 | `theme-preview` | NEW | — | Core, Gui, Widgets | — | 主题预览（所有控件一览） | P1 |
| 76 | `dpi-scaling` | MTC | `Qt/TestQt_20231221_Dpi/` | Core, Gui, Widgets | — | 高 DPI 自适应缩放 | P0 |
| 77 | `font-scaling` | NEW | — | Core, Gui, Widgets | — | 字体缩放适配 | P1 |
| 78 | `spacing-system` | NEW | — | Core, Gui, Widgets | — | 间距系统（统一 margin/padding） | P1 |
| 79 | `widget-gallery` | NEW | — | Core, Gui, Widgets | — | 控件展览馆（所有标准控件展示） | P1 |
| 80 | `material-theme` | GH | qt-material-widgets | Core, Gui, Widgets | — | Material Design 主题 | P1 |

---

## 05-notification-dialog/ — 通知与弹窗 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 81 | `toast-notification` | TTK | `Window/notifyWindow/` | Core, Gui, Widgets | — | Toast 通知系统 | P0 |
| 82 | `custom-dialog` | TTK | `Window/moveDialog/`+`colorDialog/` | Core, Gui, Widgets | — | 自定义对话框 | P1 |
| 83 | `mask-overlay` | QWD | `widget/maskwidget/` | Core, Gui, Widgets | — | 半透明遮罩/加载遮罩 | P1 |
| 84 | `progress-notification` | NEW | — | Core, Gui, Widgets | — | 进度通知（后台任务） | P1 |
| 85 | `confirm-dialog` | NEW | — | Core, Gui, Widgets | — | 操作确认对话框 | P0 |
| 86 | `action-feedback` | NEW | — | Core, Gui, Widgets | — | 操作反馈提示 | P1 |
| 87 | `tray-notification` | NEW | — | Core, Gui, Widgets | — | 系统托盘通知 | P1 |
| 88 | `bubble-notification` | NEW | — | Core, Gui, Widgets | — | 气泡通知（桌面弹出） | P1 |
| 89 | `snackbar` | GH | qt-material-widgets | Core, Gui, Widgets | — | Material Snackbar | P1 |
| 90 | `material-dialog` | GH | qt-material-widgets | Core, Gui, Widgets | — | Material 对话框 | P1 |
| 91 | `drawer-dialog` | NEW | — | Core, Gui, Widgets | — | 抽屉式对话框（底部/侧边） | P1 |
| 92 | `fullscreen-dialog` | NEW | — | Core, Gui, Widgets | — | 全屏对话框 | P2 |
| 93 | `notification-queue` | NEW | — | Core, Gui, Widgets | — | 通知队列管理 | P1 |
| 94 | `rich-tooltip` | NEW | — | Core, Gui, Widgets | — | 富文本提示 | P1 |
| 95 | `popover` | NEW | — | Core, Gui, Widgets | — | 弹出气泡（锚定到目标） | P1 |

---

## 06-form-pattern/ — 表单模式 (20)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 96 | `form-validation` | NEW | — | Core, Gui, Widgets | — | 表单验证（实时/提交时） | P0 |
| 97 | `form-layout-builder` | NEW | — | Core, Gui, Widgets | — | 表单布局生成器 | P1 |
| 98 | `dynamic-form` | NEW | — | Core, Gui, Widgets | — | 动态表单（运行时添加字段） | P1 |
| 99 | `conditional-form` | NEW | — | Core, Gui, Widgets | — | 条件表单（联动显示/隐藏） | P1 |
| 100 | `multi-step-form` | NEW | — | Core, Gui, Widgets | — | 多步表单（分步填写） | P1 |
| 101 | `form-serializer` | NEW | — | Core, Gui, Widgets | — | 表单序列化/反序列化 | P1 |
| 102 | `form-auto-save` | NEW | — | Core, Gui, Widgets | — | 表单自动保存 | P1 |
| 103 | `form-reset` | NEW | — | Core, Gui, Widgets | — | 表单重置（确认/撤销） | P1 |
| 104 | `login-form` | MTC | `Qml/QmlLoginPage/` | Core, Gui, Widgets, Sql | — | 登录表单 | P0 |
| 105 | `register-form` | NEW | — | Core, Gui, Widgets | — | 注册表单（密码强度/确认） | P1 |
| 106 | `search-form` | NEW | — | Core, Gui, Widgets | — | 搜索表单（多条件/高级搜索） | P0 |
| 107 | `filter-form` | NEW | — | Core, Gui, Widgets | — | 筛选表单 | P1 |
| 108 | `settings-form` | NEW | — | Core, Gui, Widgets | — | 设置表单 | P1 |
| 109 | `contact-form` | NEW | — | Core, Gui, Widgets | — | 联系信息表单 | P1 |
| 110 | `address-form` | NEW | — | Core, Gui, Widgets | — | 地址表单（省市区联动） | P1 |
| 111 | `payment-form` | NEW | — | Core, Gui, Widgets | — | 支付表单 | P2 |
| 112 | `feedback-form` | NEW | — | Core, Gui, Widgets | — | 反馈表单 | P2 |
| 113 | `survey-form` | NEW | — | Core, Gui, Widgets | — | 调查问卷表单 | P2 |
| 114 | `import-form` | NEW | — | Core, Gui, Widgets | — | 数据导入表单 | P1 |
| 115 | `export-form` | NEW | — | Core, Gui, Widgets | — | 数据导出表单 | P1 |

---

## 07-settings-help-about/ — 设置/帮助/关于 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 116 | `settings-page` | NEW | — | Core, Gui, Widgets | — | 通用设置页面 | P0 |
| 117 | `searchable-settings` | NEW | — | Core, Gui, Widgets | — | 可搜索设置页面 | P1 |
| 118 | `grouped-settings` | NEW | — | Core, Gui, Widgets | — | 分组设置页面（左侧列表+右侧内容） | P1 |
| 119 | `registry-style-settings` | NEW | — | Core, Gui, Widgets | — | 注册表风格设置 | P2 |
| 120 | `about-dialog` | NEW | — | Core, Gui, Widgets | — | 标准关于对话框 | P1 |
| 121 | `update-check` | NEW | — | Core, Gui, Widgets, Network | — | 更新检查对话框 | P1 |
| 122 | `license-dialog` | NEW | — | Core, Gui, Widgets | — | 许可证信息对话框 | P2 |
| 123 | `help-browser` | NEW | — | Core, Gui, Widgets | — | 帮助浏览器 | P1 |
| 124 | `search-help` | NEW | — | Core, Gui, Widgets | — | 搜索帮助 | P1 |
| 125 | `first-run-wizard` | NEW | — | Core, Gui, Widgets | — | 首次运行向导 | P1 |
| 126 | `preferences-system` | NEW | — | Core, Gui, Widgets | — | 偏好设置系统 | P1 |
| 127 | `shortcut-settings` | NEW | — | Core, Gui, Widgets | — | 快捷键设置 | P1 |
| 128 | `language-settings` | NEW | — | Core, Gui, Widgets | — | 语言设置 | P1 |
| 129 | `theme-settings` | NEW | — | Core, Gui, Widgets | — | 主题设置 | P1 |
| 130 | `backup-restore-settings` | NEW | — | Core, Gui, Widgets | — | 设置备份恢复 | P2 |

---

## 08-print-report/ — 打印与报表 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 131 | `print-preview` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 打印预览 | P1 |
| 132 | `header-footer` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 页眉页脚打印 | P1 |
| 133 | `report-template` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 报表模板系统 | P1 |
| 134 | `pdf-export` | NEW | — | Core, Gui, Widgets | — | PDF 导出 | P0 |
| 135 | `chart-print` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 图表打印 | P1 |
| 136 | `table-print` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 表格打印 | P1 |
| 137 | `rich-text-print` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 富文本打印 | P1 |
| 138 | `label-print` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 标签打印（邮件/条码） | P2 |
| 139 | `batch-print` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 批量打印 | P2 |
| 140 | `page-setup` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 页面设置对话框 | P1 |
| 141 | `watermark-print` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 打印水印 | P2 |
| 142 | `print-to-image` | NEW | — | Core, Gui, Widgets | — | 打印为图片 | P1 |
| 143 | `report-designer` | NEW | — | Core, Gui, Widgets | — | 报表设计器 | P2 |
| 144 | `receipt-print` | NEW | — | Core, Gui, Widgets, PrintSupport | — | 小票打印 | P2 |
| 145 | `docx-xlsx-generation` | MTC | `Qt/TestQt_20211012_DocXls/` | Core, Gui, Widgets | — | Word/Excel 文档生成 | P1 |

---

## 09-property-editor/ — 属性编辑器 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 146 | `variant-property-editor` | NEW | — | Core, Gui, Widgets | — | 属性表格（QtVariantPropertyManager 风格） | P1 |
| 147 | `color-property` | NEW | — | Core, Gui, Widgets | — | 颜色属性编辑器 | P1 |
| 148 | `font-property` | NEW | — | Core, Gui, Widgets | — | 字体属性编辑器 | P1 |
| 149 | `enum-property` | NEW | — | Core, Gui, Widgets | — | 枚举属性编辑器 | P1 |
| 150 | `bool-property` | NEW | — | Core, Gui, Widgets | — | 布尔属性编辑器 | P1 |
| 151 | `range-property` | NEW | — | Core, Gui, Widgets | — | 范围属性编辑器 | P1 |
| 152 | `string-property` | NEW | — | Core, Gui, Widgets | — | 字符串属性编辑器 | P1 |
| 153 | `number-property` | NEW | — | Core, Gui, Widgets | — | 数值属性编辑器（整数/浮点） | P1 |
| 154 | `date-property` | NEW | — | Core, Gui, Widgets | — | 日期属性编辑器 | P1 |
| 155 | `time-property` | NEW | — | Core, Gui, Widgets | — | 时间属性编辑器 | P1 |
| 156 | `file-property` | NEW | — | Core, Gui, Widgets | — | 文件路径属性编辑器 | P1 |
| 157 | `custom-property` | NEW | — | Core, Gui, Widgets | — | 自定义属性编辑器 | P2 |
| 158 | `grouped-property` | NEW | — | Core, Gui, Widgets | — | 分组属性编辑器 | P1 |
| 159 | `read-only-property` | NEW | — | Core, Gui, Widgets | — | 只读属性显示 | P1 |
| 160 | `serialized-property` | NEW | — | Core, Gui, Widgets | — | 属性序列化/反序列化 | P1 |

---

## 10-drag-drop-pattern/ — 拖拽模式 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 161 | `list-drag-sort` | NEW | — | Core, Gui, Widgets | — | 列表拖拽排序 | P1 |
| 162 | `tree-drag-move` | NEW | — | Core, Gui, Widgets | — | 树拖拽移动节点 | P1 |
| 163 | `table-drag-row-col` | MTC | `Qt/QTableViewMoveAction/` | Core, Gui, Widgets | — | 表格拖拽行列 | P1 |
| 164 | `file-drag-drop` | NEW | — | Core, Gui, Widgets | — | 文件拖放接收 | P1 |
| 165 | `cross-window-drag` | NEW | — | Core, Gui, Widgets | — | 跨窗口拖放 | P1 |
| 166 | `drag-data-encode` | NEW | — | Core, Gui, Widgets | — | 自定义拖拽数据编码 | P1 |
| 167 | `drop-target-highlight` | NEW | — | Core, Gui, Widgets | — | 拖放目标高亮 | P1 |
| 168 | `drag-preview` | NEW | — | Core, Gui, Widgets | — | 自定义拖拽预览 | P1 |
| 169 | `tab-drag-detach` | MTC | `Qt/MyTabWidget/` | Core, Gui, Widgets | — | 标签页拖拽分离 | P1 |
| 170 | `item-reorder` | TTK | `Widget/grabItemWidget/` | Core, Gui, Widgets | — | 项目拖拽重排 | P1 |
| 171 | `clipboard-drag` | NEW | — | Core, Gui, Widgets | — | 剪贴板拖放 | P1 |
| 172 | `drag-proxy-model` | NEW | — | Core, Gui, Widgets | — | 代理模型拖拽 | P2 |
| 173 | `multi-select-drag` | NEW | — | Core, Gui, Widgets | — | 多选拖拽 | P2 |
| 174 | `drag-constraints` | NEW | — | Core, Gui, Widgets | — | 拖拽约束（边界/吸附） | P2 |
| 175 | `nested-drag` | NEW | — | Core, Gui, Widgets | — | 嵌套拖拽（树/列表） | P2 |

---

## 11-command-pattern/ — 命令模式 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 176 | `undo-redo-framework` | NEW | — | Core, Gui, Widgets | — | 撤销/重做框架（QUndoStack） | P0 |
| 177 | `command-history` | NEW | — | Core, Gui, Widgets | — | 命令历史列表 | P1 |
| 178 | `macro-command` | NEW | — | Core, Gui, Widgets | — | 宏命令（录制/回放多个操作） | P1 |
| 179 | `command-palette` | NEW | — | Core, Gui, Widgets | — | 命令面板（Ctrl+P 模糊搜索） | P1 |
| 180 | `text-undo-redo` | NEW | — | Core, Gui, Widgets | — | 文本编辑撤销重做 | P1 |
| 181 | `draw-undo-redo` | NEW | — | Core, Gui, Widgets | — | 绘图撤销重做 | P1 |
| 182 | `table-edit-undo` | NEW | — | Core, Gui, Widgets | — | 表格编辑撤销重做 | P1 |
| 183 | `tree-edit-undo` | NEW | — | Core, Gui, Widgets | — | 树编辑撤销重做 | P2 |
| 184 | `file-operation-undo` | NEW | — | Core, Gui, Widgets | — | 文件操作撤销 | P2 |
| 185 | `transaction-command` | NEW | — | Core, Gui, Widgets | — | 事务命令（全部成功或全部回滚） | P1 |
| 186 | `composite-command` | NEW | — | Core, Gui, Widgets | — | 组合命令 | P1 |
| 187 | `redo-limit` | NEW | — | Core, Gui, Widgets | — | 重做次数限制/清理 | P2 |
| 188 | `command-log` | NEW | — | Core, Gui, Widgets | — | 命令日志 | P2 |
| 189 | `scriptable-command` | NEW | — | Core, Gui, Widgets | — | 可脚本化的命令 | P2 |
| 190 | `network-command` | NEW | — | Core, Gui, Widgets, Network | — | 网络操作命令 | P2 |

---

## 12-data-binding/ — 数据绑定 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 191 | `data-widget-mapper` | MTC | `Qt/TestQt_20200615_DataMapper/` | Core, Gui, Widgets | — | QDataWidgetMapper 模式 | P1 |
| 192 | `mvvm-pattern` | NEW | — | Core, Gui, Widgets | — | MVVM 模式 | P1 |
| 193 | `mvc-pattern` | NEW | — | Core, Gui, Widgets | — | MVC 模式 | P1 |
| 194 | `reactive-data` | NEW | — | Core, Gui, Widgets | — | 响应式数据绑定 | P1 |
| 195 | `model-delegate` | NEW | — | Core, Gui, Widgets | — | Model-Delegate 模式 | P1 |
| 196 | `proxy-model` | NEW | — | Core, Gui, Widgets | — | 代理模型（排序/过滤/转换） | P0 |
| 197 | `identity-proxy-model` | NEW | — | Core, Gui, Widgets | — | 身份代理模型 | P2 |
| 198 | `custom-model` | MTC | `Qt/QJsonAndTreeView/` | Core, Gui, Widgets | — | 自定义数据模型 | P1 |
| 199 | `tree-model` | NEW | — | Core, Gui, Widgets | — | 树形数据模型 | P1 |
| 200 | `table-model` | MTC | `Qt/QTableViewMoveAction/` | Core, Gui, Widgets | — | 表格数据模型 | P1 |
| 201 | `list-model` | NEW | — | Core, Gui, Widgets | — | 列表数据模型 | P1 |
| 202 | `string-list-model` | NEW | — | Core, Gui, Widgets | — | QStringListModel 使用 | P1 |
| 203 | `sql-query-model` | NEW | — | Core, Gui, Widgets, Sql | — | QSqlQueryModel | P1 |
| 204 | `sql-table-model` | NEW | — | Core, Gui, Widgets, Sql | — | QSqlTableModel（可编辑） | P1 |
| 205 | `sql-relational-model` | NEW | — | Core, Gui, Widgets, Sql | — | QSqlRelationalTableModel（外键） | P1 |

---

## 13-chart-integration/ — 图表集成 (20)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 206 | `qt-charts-demo` | MTC | `Qt/TestQt_20190806_QChartsBackground/` | Core, Gui, Widgets, Charts | — | QCharts 图表示例 | P0 |
| 207 | `qcustomplot-demo` | QWD | `third/qcustomplotdemo/` | Core, Gui, Widgets | qcustomplot (FetchContent) | QCustomPlot 图表 | P1 |
| 208 | `qwt-demo` | QWD | `third/qwtdemo/` | Core, Gui, Widgets | Qwt (FetchContent) | Qwt 科学图表 | P1 |
| 209 | `jkqtplotter-demo` | GH | JKQTPlotter | Core, Gui, Widgets | JKQTPlotter (FetchContent) | JKQTPlotter 高级图表 | P2 |
| 210 | `chart-qt-demo` | GH | chart-qt | Core, Gui, Widgets | chart-qt (FetchContent) | Chart-Qt 高性能图表 | P2 |
| 211 | `realtime-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 实时滚动图表 | P0 |
| 212 | `interactive-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 交互式图表（缩放/平移/选区） | P1 |
| 213 | `multi-axis-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 多轴图表 | P1 |
| 214 | `combined-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 组合图表（折线+柱状+面积） | P1 |
| 215 | `stock-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 股票图表（K线/成交量） | P1 |
| 216 | `thermometer-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 温度计图表 | P2 |
| 217 | `dashboard-gauge` | NEW | — | Core, Gui, Widgets, Charts | — | 仪表盘面板 | P1 |
| 218 | `geographic-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 地理图表 | P2 |
| 219 | `chart-export` | NEW | — | Core, Gui, Widgets, Charts | — | 图表导出（PNG/SVG/PDF） | P1 |
| 220 | `chart-theme` | NEW | — | Core, Gui, Widgets, Charts | — | 图表主题管理 | P1 |
| 221 | `echart-embed` | QWD/MTC | `other/echartgauge/`+`Qml/QmlWebEngineECharts/` | Core, Gui, Widgets, WebEngineWidgets | — | ECharts Web 嵌入 | P1 |
| 222 | `logarithmic-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 对数刻度图表 | P2 |
| 223 | `polar-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 极坐标图表 | P2 |
| 224 | `boxplot-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 箱线图 | P1 |
| 225 | `error-bar-chart` | NEW | — | Core, Gui, Widgets, Charts | — | 误差棒图表 | P2 |

---

## 14-utility-pattern/ — 工具类/设计模式 (30)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 226 | `log-redirect` | QWD | `control/savelog/` | Core, Gui, Widgets, Network | — | 日志重定向 | P0 |
| 227 | `pimpl-pattern` | MTC | `Qt/QtPimpl/` | Core, Gui, Widgets | — | PIMPL 模式 | P1 |
| 228 | `shared-data-pattern` | MTC | `Qt/TestQt_20211225_SharedData/` | Core, Gui, Widgets | — | 隐式共享 | P1 |
| 229 | `event-flow` | MTC | `Qt/TestQt_20220915_EventFlow/`+`TestQt_20230710_CustomEvent/` | Core, Gui, Widgets | — | 事件传播+自定义事件 | P1 |
| 230 | `chinese-pinyin` | QWD | `control/zhtopy/` | Core, Gui, Widgets | — | 汉字转拼音 | P2 |
| 231 | `runtime-tracker` | QWD | `control/saveruntime/` | Core, Gui, Widgets | — | 运行时长追踪 | P2 |
| 232 | `singleton-pattern` | NEW | — | Core, Gui, Widgets | — | 单例模式 | P1 |
| 233 | `observer-pattern` | NEW | — | Core, Gui, Widgets | — | 观察者模式 | P1 |
| 234 | `state-machine` | NEW | — | Core, Gui, Widgets, StateMachine | — | 状态机框架 | P1 |
| 235 | `thread-pool` | MTC | `Qt/TestQt_20200622_QFuture/` | Core, Gui, Widgets, Concurrent | — | 线程池 | P1 |
| 236 | `promise-future` | MTC | `Qt6/TestQt_20251220_Future/` | Core, Gui, Widgets, Concurrent | — | QPromise/QFuture 异步 | P1 |
| 237 | `object-pool` | NEW | — | Core, Gui, Widgets | — | 对象池 | P2 |
| 238 | `memory-leak-detector` | NEW | — | Core, Gui, Widgets | — | 内存泄漏检测 | P2 |
| 239 | `performance-timer` | NEW | — | Core, Gui, Widgets | — | 性能计时器 | P1 |
| 240 | `config-manager` | NEW | — | Core, Gui, Widgets | — | 配置管理（INI/JSON/XML） | P0 |
| 241 | `plugin-loader` | MTC | `Qt/QtDynamicPlugin/` | Core, Gui, Widgets | — | 插件加载器 | P1 |
| 242 | `recent-files-manager` | NEW | — | Core, Gui, Widgets | — | 最近文件管理器 | P1 |
| 243 | `auto-updater` | NEW | — | Core, Gui, Widgets, Network | — | 自动更新框架 | P1 |
| 244 | `crash-handler` | MTC | `Qt/TestQt_20210211_Dump/` | Core, Gui, Widgets | — | 崩溃处理框架 | P1 |
| 245 | `json-tree-editor` | MTC | `Qt/QJsonAndTreeView/` | Core, Gui, Widgets | — | JSON 树形编辑器 | P1 |
| 246 | `multi-thread-progress` | MTC | `Qt/TestQt_20200625_QFuture/` | Core, Gui, Widgets, Concurrent | — | 多线程进度同步 | P1 |
| 247 | `ui-subthread-interop` | MTC | `Qt/UiAndSubThread/` | Core, Gui, Widgets | — | UI 与子线程交互 | P1 |
| 248 | `data-mapper` | MTC | `Qt/TestQt_20200615_DataMapper/` | Core, Gui, Widgets | — | 数据映射器 | P1 |
| 249 | `thread-safe-queue` | TTK | `TTKCommon/TTKLibrary/ttkconcurrentqueue.h` | Core | — | 线程安全队列 | P2 |
| 250 | `spin-lock` | TTK | `TTKCommon/TTKLibrary/ttkspinlock.h` | Core | — | 自旋锁 | P2 |
| 251 | `type-safe-any` | TTK | `TTKCommon/TTKLibrary/ttkany.h` | Core | — | 类型安全容器 | P2 |
| 252 | `smart-pointer-utils` | TTK | `TTKCommon/TTKLibrary/ttksmartptr.h` | Core | — | 智能指针工具 | P2 |
| 253 | `command-line-parser` | TTK | `TTKCommon/TTKLibrary/ttkcommandline.h` | Core | — | 命令行参数解析 | P1 |
| 254 | `file-association` | TTK | `TTKCommon/TTKLibrary/ttkfileassociation.h` | Core | — | 文件关联管理 | P2 |
| 255 | `dispatch-manager` | TTK | `TTKCommon/TTKLibrary/ttkdispatchmanager.h` | Core | — | 事件分发管理 | P2 |

---

## 15-complete-ui-demo/ — 完整界面 Demo (20)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 256 | `login-page` | MTC | `Qml/QmlLoginPage/` | Core, Gui, Widgets, Sql | — | 登录页面 | P0 |
| 257 | `chat-ui` | MTC | `Qml/TalkList/` | Core, Gui, Widgets, Multimedia | — | 聊天界面 | P1 |
| 258 | `system-tray` | QWD | `other/trayicon/` | Core, Gui, Widgets, Network | — | 系统托盘 | P1 |
| 259 | `multi-signal-slot` | QWD | `other/multobj2slot/` | Core, Gui, Widgets | — | 多对象连接单槽 | P2 |
| 260 | `draw-rect-bench` | QWD | `other/drawrect/` | Core, Gui, Widgets | — | 随机矩形绘制性能测试 | P2 |
| 261 | `im-client-ui` | NEW | — | Core, Gui, Widgets | — | 即时通讯客户端界面 | P1 |
| 262 | `email-client-ui` | NEW | — | Core, Gui, Widgets | — | 邮件客户端界面 | P1 |
| 263 | `music-player-ui` | NEW | — | Core, Gui, Widgets, Multimedia | — | 音乐播放器界面 | P1 |
| 264 | `video-player-ui` | NEW | — | Core, Gui, Widgets, Multimedia | — | 视频播放器界面 | P1 |
| 265 | `file-manager-ui` | NEW | — | Core, Gui, Widgets | — | 文件管理器界面 | P1 |
| 266 | `settings-ui` | NEW | — | Core, Gui, Widgets | — | 设置界面 | P1 |
| 267 | `dashboard-ui` | NEW | — | Core, Gui, Widgets | — | 仪表盘界面 | P1 |
| 268 | `admin-panel-ui` | NEW | — | Core, Gui, Widgets | — | 后台管理面板 | P1 |
| 269 | `ide-layout` | NEW | — | Core, Gui, Widgets | — | IDE 布局（菜单+工具栏+Dock+状态栏） | P1 |
| 270 | `monitor-dashboard` | NEW | — | Core, Gui, Widgets | — | 监控大屏界面 | P1 |
| 271 | `photo-gallery-ui` | NEW | — | Core, Gui, Widgets | — | 照片墙界面 | P2 |
| 272 | `weather-app-ui` | NEW | — | Core, Gui, Widgets, Network | — | 天气应用界面 | P2 |
| 273 | `news-reader-ui` | NEW | — | Core, Gui, Widgets, Network | — | 新闻阅读器界面 | P2 |
| 274 | `map-app-ui` | NEW | — | Core, Gui, Widgets | — | 地图应用界面 | P2 |
| 275 | `social-media-ui` | NEW | — | Core, Gui, Widgets, Network | — | 社交媒体界面 | P2 |

---

## 16-context-menu/ — 右键菜单 (10)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 276 | `animated-context-menu` | GH | Qt-FacileMenu | Core, Gui, Widgets | — | 动画右键菜单 | P1 |
| 277 | `multi-level-menu` | NEW | — | Core, Gui, Widgets | — | 多级菜单 | P1 |
| 278 | `icon-menu` | NEW | — | Core, Gui, Widgets | — | 带图标菜单 | P1 |
| 279 | `checkable-menu` | NEW | — | Core, Gui, Widgets | — | 可选中菜单 | P1 |
| 280 | `radio-menu` | NEW | — | Core, Gui, Widgets | — | 互斥菜单 | P1 |
| 281 | `separator-menu` | NEW | — | Core, Gui, Widgets | — | 带分隔符菜单 | P1 |
| 282 | `shortcut-menu` | NEW | — | Core, Gui, Widgets | — | 带快捷键提示菜单 | P1 |
| 283 | `recent-menu` | NEW | — | Core, Gui, Widgets | — | 最近使用菜单 | P1 |
| 284 | `color-menu` | NEW | — | Core, Gui, Widgets | — | 颜色选择菜单 | P2 |
| 285 | `toolbar-menu` | NEW | — | Core, Gui, Widgets | — | 工具栏弹出菜单 | P1 |

---

## 17-status-feedback/ — 状态反馈 (15)

| # | 目录名 | 来源 | 原路径/参考 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|-------------|---------|------|------|--------|
| 286 | `status-bar-message` | NEW | — | Core, Gui, Widgets | — | 状态栏消息 | P1 |
| 287 | `progress-status` | NEW | — | Core, Gui, Widgets | — | 进度状态（状态栏+进度条） | P1 |
| 288 | `connection-status` | NEW | — | Core, Gui, Widgets | — | 连接状态指示器 | P1 |
| 289 | `online-status` | NEW | — | Core, Gui, Widgets | — | 在线状态（绿/黄/灰点） | P1 |
| 290 | `sync-status` | NEW | — | Core, Gui, Widgets | — | 同步状态（旋转/完成/错误） | P1 |
| 291 | `validation-status` | NEW | — | Core, Gui, Widgets | — | 验证状态（对勾/叉/警告） | P1 |
| 292 | `busy-indicator` | NEW | — | Core, Gui, Widgets | — | 忙碌指示器 | P1 |
| 293 | `error-display` | NEW | — | Core, Gui, Widgets | — | 错误信息展示 | P1 |
| 294 | `warning-display` | NEW | — | Core, Gui, Widgets | — | 警告信息展示 | P1 |
| 295 | `info-display` | NEW | — | Core, Gui, Widgets | — | 提示信息展示 | P1 |
| 296 | `empty-state-display` | NEW | — | Core, Gui, Widgets | — | 空状态展示 | P1 |
| 297 | `offline-indicator` | NEW | — | Core, Gui, Widgets, Network | — | 离线指示器 | P1 |
| 298 | `battery-status` | NEW | — | Core, Gui, Widgets | — | 电池状态 | P2 |
| 299 | `volume-status` | NEW | — | Core, Gui, Widgets, Multimedia | — | 音量状态 | P2 |
| 300 | `gps-status` | NEW | — | Core, Gui, Widgets, Positioning | — | GPS 定位状态 | P2 |

---

## 统计

| 子分类 | P0 | P1 | P2 | 合计 |
|--------|----|----|-----|------|
| 01-window-framework | 4 | 10 | 6 | 20 |
| 02-navigation-layout | 2 | 18 | 5 | 25 |
| 03-page-transition | 1 | 7 | 7 | 15 |
| 04-theme-styling | 4 | 14 | 2 | 20 |
| 05-notification-dialog | 2 | 12 | 1 | 15 |
| 06-form-pattern | 2 | 16 | 2 | 20 |
| 07-settings-help-about | 1 | 13 | 1 | 15 |
| 08-print-report | 1 | 9 | 5 | 15 |
| 09-property-editor | 0 | 14 | 1 | 15 |
| 10-drag-drop-pattern | 0 | 11 | 4 | 15 |
| 11-command-pattern | 1 | 9 | 5 | 15 |
| 12-data-binding | 1 | 13 | 1 | 15 |
| 13-chart-integration | 2 | 14 | 4 | 20 |
| 14-utility-pattern | 3 | 14 | 13 | 30 |
| 15-complete-ui-demo | 1 | 14 | 5 | 20 |
| 16-context-menu | 0 | 9 | 1 | 10 |
| 17-status-feedback | 0 | 14 | 1 | 15 |
| **合计** | **25** | **201** | **64** | **300** |
