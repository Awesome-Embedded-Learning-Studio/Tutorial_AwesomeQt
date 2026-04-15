# qml/ — QML 专项清单

> 本文档列出所有计划实现的 QML/Qt Quick 相关实例。
> 代码放置在项目根目录 `qml/<子类>/<目录名>/` 下，教程放置在 `tutorial/qml/` 下。

**总计：52 项 | 子类：16 个**

## 参考来源说明

| 标记 | 含义 |
|------|------|
| MTC | 参考 [MyTestCode](https://github.com/gongjianbo/MyTestCode) |
| NEW | 基于常见 QML 需求全新设计 |

---

---

## cpp-interop/ — QML/C++ 互调

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 1 | `qml-call-cpp` | MTC | `Qml/QmlCallCpp2020/` | Quick, Qml | — | QML 调用 C++ 对象（Q_PROPERTY/Q_INVOKABLE/signals/slots） | P0 |
| 2 | `cpp-call-qml` | MTC | `Qml/CppCallQml2020/` | Quick, Qml | — | C++ 调用 QML 函数（QMetaObject::invokeMethod） | P0 |
| 3 | `qml-cpp-callback` | MTC | `Qml/TestQml_20220908_Callback/` | Quick | — | QML/C++ 双向回调 | P0 |
| 4 | `dynamic-component` | MTC | `Qml/TestQml_20201118_createComponent/` | Quick | — | 动态创建 QML 组件 | P1 |
| 5 | `basic-file-dialog` | MTC | `Qml/BasicFileDialog/` | Quick, Widgets | — | QML 中使用 QtWidgets 文件对话框 | P1 |

---

## animation/ — 动画/特效

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 6 | `qml-animation` | MTC | `Qml/QmlAnimation/` | Quick | — | QML 动画基础示例 | P0 |
| 7 | `particle-heart` | MTC | `Qml/TestQml_20200521_Love/` | Quick | — | 粒子系统心形动画 | P2 |
| 8 | `wave-ripple` | MTC | `Qml/TestQml_20191128_Wave/` | Quick | — | 水波纹扩散效果 | P1 |
| 9 | `canvas-wave-progress` | MTC | `Qml/TestQml_20210310_Wave/` | Quick | — | Canvas 水波进度球 | P1 |
| 10 | `qml-ripple` | MTC | `Qml/QmlRipple/` | Quick | — | Material Design 涟漪效果 | P1 |
| 11 | `mask-opacity` | MTC | `Qml/TestQml_20210307_Mask/` | Quick | — | 遮罩/透明度效果（OpacityMask） | P1 |
| 12 | `shape-gradient` | MTC | `Qml/TestQml_20220412_ShapeGradient/` | Quick | — | QML Shapes 渐变填充 | P2 |
| 13 | `shader-effect` | MTC | `Qml/TestQml_20210429_ShaderEffect/` | Quick | — | GLSL ShaderEffect 着色器特效 | P2 |

---

## custom-painting/ — 自定义绘制

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 14 | `qml-painter` | MTC | `Qml/TestQml_20210217_Painter/` | Quick | — | QML 中使用 QPainter（自定义 QPaintDevice） | P0 |
| 15 | `sg-line` | MTC | `Qml/LearnQSG_20210614_Line/` | Quick | — | Scene Graph 线条绘制（QSGNode/QSGGeometry） | P1 |
| 16 | `sg-texture` | MTC | `Qml/LearnQSG_20210624_Texture/` | Quick | — | Scene Graph 纹理（QSGTexture+自定义着色器+SPIR-V） | P2 |
| 17 | `canvas-vs-painted-item` | MTC | `Qml/TestCanvasAndPaintedItem/` | Quick | — | Canvas 与 PaintedItem 性能对比 | P1 |
| 18 | `qml-progress-bar` | MTC | `Qml/TestQml_20210106_ProgressBar/` + `TestQml_20220210_ProgressBar/` | Quick | — | QML 自定义进度条（圆形/波浪） | P1 |

---

## components/ — 通用组件

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 19 | `qml-toast` | MTC | `Qml/TestQml_20240622_Toast/` | Quick | — | QML Toast 弹窗通知 | P0 |
| 20 | `qml-pagination` | MTC | `Qml/QmlPagination/` | Quick | — | QML 分页组件 | P1 |
| 21 | `qml-ip-input` | MTC | `Qml/TestQml_20210717_IpInput/` | Quick | — | QML IP 地址输入框 | P1 |
| 22 | `qml-completer` | MTC | `Qml/QmlCompleter/` + `OldTextFieldCompleter/` | Quick | — | QML 自动补全输入框 | P1 |
| 23 | `qml-tooltip` | MTC | `Qml/TestQml_20210303_ToolTip/` | Quick | — | QML 自定义 ToolTip | P1 |
| 24 | `qml-dialog` | MTC | `Qml/TestQml_20201106_MyDialog/` + `TestQml_20240725_Dialog/` | Quick | — | QML 自定义对话框（ESC/Android 返回键处理） | P1 |
| 25 | `qml-close-event` | MTC | `Qml/QmlCloseEvent/` | Quick | — | QML 应用退出确认 | P1 |
| 26 | `qml-calendar` | MTC | `Qml6/TestQml_20220416_Calendar/` | Quick (Qt6) | — | QML 日历样式定制 | P2 |

---

## model-view/ — Model/View

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 27 | `qml-tree-view` | MTC | `Qml/TestQml_20221120_TreeSelection/` + `Qml6/TestQml_20220422_TreeView/` | Quick | — | QML TreeView（树形模型+选择） | P0 |
| 28 | `qml-list-move` | MTC | `Qml/TestQml_20201113_ListMove/` | Quick | — | QML ListView 项目拖拽排序 | P1 |
| 29 | `qml-path-view` | MTC | `Qml/TestQml_20220313_PathView/` | Quick | — | QML PathView 圆形菜单 | P1 |
| 30 | `qml-section-grid` | MTC | `Qml/TestQml_20240205_SectionGrid/` | Quick | — | QML GridView 分组标题 | P1 |
| 31 | `qml-search-condition` | MTC | `Qml/TestQml_20220517_SearchCondition/` | Quick | — | QML 搜索条件 UI | P1 |
| 32 | `qml-shared-data` | MTC | `Qml/TestQml_20220709_CommonData/` | Quick | — | QML 多 View 共享数据 | P1 |

---

## plugin/ — 插件系统

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 33 | `qml-dynamic-plugin` | MTC | `Qml/QmlDynamicPlugin/` | Quick, Qml | — | QML 扩展插件（QQmlExtensionPlugin 动态加载） | P1 |

---

## theme/ — 换肤

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 34 | `qml-skin` | MTC | `Qml/QmlSkin/` | Quick | — | QML 运行时换肤系统 | P1 |

---

## network/ — 网络

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 35 | `qml-ajax` | MTC | `Qml/QmlAjax/` | Quick | — | QML XMLHttpRequest 网络请求 | P1 |
| 36 | `qml-webengine-echarts` | MTC | `Qml/QmlWebEngineECharts/` | Quick, WebEngine | — | QML WebEngine + ECharts 实时曲线 | P2 |

---

## input/ — 输入/事件

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 37 | `qml-shortcut` | MTC | `Qml/TestQml_20210517_ShortcutKey/` + `KeysFilter/` | Quick | — | QML 快捷键（Keys/eventFilter） | P1 |
| 38 | `qml-mouse-forward` | MTC | `Qml/TestQml_20240424_MouseForward/` | Quick | — | QML MouseArea 事件转发 | P2 |
| 39 | `qml-drag-drop` | MTC | `Qml/TestQml_20240305_DragDrop/` | Quick | — | QML 拖放操作 | P1 |
| 40 | `qml-copy-paste` | MTC | `Qml/TestQml_20221125_CopyPaste/` | Quick | — | QML 图片拖放+剪贴板操作 | P1 |

---

## image/ — 图片

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 41 | `qml-image-provider` | MTC | `Qml/TestQml_20221225_ImageProvider/` | Quick | — | QML QQuickImageProvider 同步图片 | P1 |
| 42 | `qml-async-image-provider` | MTC | `Qml/TestQml_20240118_AsyncImageProvider/` | Quick | — | QML QQuickAsyncImageProvider 异步图片 | P1 |
| 43 | `qml-grab-image` | MTC | `Qml/TestQml_20240905_GrabImage/` | Quick | — | QML grabToImage 组件截图 | P1 |

---

## window/ — 窗口

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 44 | `qml-frameless-window` | MTC | `Qml/QmlFramelessWindow/` | Quick | — | QML 无边框窗口 | P0 |
| 45 | `qml-desktop-tip` | MTC | `Qml/TestQml_20200619_desktoptip/` | Quick | — | QML 桌面悬浮提示 | P2 |

---

## opengl/ — OpenGL/3D

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 46 | `qml-opengl-under` | MTC | `Qml/QtExampleOpenGLUnderQML/` | Quick, QuickWidgets | — | QML 下层 OpenGL 渲染 | P2 |
| 47 | `qml-fbo` | MTC | `Qml/TestQml_20200128_FBO/` | Quick | — | QML FBO 三角形（帧缓冲） | P2 |
| 48 | `qml-qt3d` | MTC | `Qml/TestQml_20200610_qt3d/` | Quick, 3D | — | QML Qt3D 3D 场景 | P2 |

---

## i18n/ — 国际化

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 49 | `qml-translator` | MTC | `Qml/TestQml_20211215_Translator/` | Quick | — | QML 多语言翻译（QTranslator） | P1 |

---

## performance/ — 性能/工具

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 50 | `qml-fps` | MTC | `Qml/TestQml_20230211_QmlFps/` | Quick | — | QML FPS 帧率统计 | P1 |

---

## dpi/ — DPI 适配

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 51 | `qml-dpi` | MTC | `Qml/TestQml_20231221_Dpi/` | Quick | — | QML DPI 缩放比例获取（Attached Property） | P1 |

---

## multimedia/ — 多媒体

| # | 目录名 | 来源 | 原路径 | Qt 模块 | 依赖 | 说明 | 优先级 |
|---|--------|------|--------|---------|------|------|--------|
| 52 | `qml-audio-recorder` | MTC | `Qml/AudioRecorderView/` | Quick, Multimedia | — | QML 音频录制 UI | P2 |

---

## 统计

| 子分类 | P0 | P1 | P2 | 合计 |
|--------|----|----|-----|------|
| cpp-interop | 3 | 2 | 0 | 5 |
| animation | 1 | 3 | 4 | 8 |
| custom-painting | 1 | 4 | 1 | 6 |
| components | 1 | 6 | 1 | 8 |
| model-view | 1 | 5 | 0 | 6 |
| plugin | 0 | 1 | 0 | 1 |
| theme | 0 | 1 | 0 | 1 |
| network | 0 | 1 | 1 | 2 |
| input | 0 | 3 | 1 | 4 |
| image | 0 | 3 | 0 | 3 |
| window | 1 | 0 | 1 | 2 |
| opengl | 0 | 0 | 3 | 3 |
| i18n | 0 | 1 | 0 | 1 |
| performance | 0 | 1 | 0 | 1 |
| dpi | 0 | 1 | 0 | 1 |
| multimedia | 0 | 0 | 1 | 1 |
| **合计** | **8** | **33** | **13** | **53** |
