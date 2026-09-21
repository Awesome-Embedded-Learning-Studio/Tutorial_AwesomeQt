# 基建（填缝：P0/P0.5/P1/P4 + widget 化简 + 地基债）

> 单点细节批量处理：扫出清单 → 逐个修。

## widget/ 构建化简（✅ 已落盘 2026-06-14 · cmake 验证通过）

把 widget/ 从「发布库式」瘦身为「教学最小形态」。9 步（A4 须先于 A3）已全部落盘：

1. ✅ **A4** demo 子目录迁移：`StatusLEDWindow.{h,cpp}`→`demo/status_led_window.{h,cpp}`、`main.cpp`→`demo/main.cpp`，改 include（类名不动）
2. ✅ **A5** `src/status_led.cpp` 补 `#include <algorithm>`
3. ✅ **A3** `status-led/CMakeLists.txt` 换 STATIC 库 + `add_subdirectory(demo)`
4. ✅ **A2** 根 `widget/CMakeLists.txt` 换 root-owns-config（砍 MERGE_MODE/install/cache）
5. ✅ **A1** 删 `cmake/AwesomeQtWidgets.cmake` + 空 `cmake/` 目录
6. ✅ **B1** `.claude/example_style.md` 成员命名改尾下划线（⚠ 见遗留①）
7. ✅ **B2** 文件头统一 `@file`/`@brief`（删 @author/@version/@date）
8. ✅ **C1** 新增 `widget/README.md`
9. ✅ **E1** `cd widget && cmake -B build && cmake --build build` 一次成功（2026-06-14 实测）

外加：目录重命名 `statusled`→`status-led`（instance-library kebab 拍板）；删冗余 `status-led/.gitignore`（根 `**/build/`·`**/.cache/` 已覆盖）。

边界遵守：无 SHARED/install/MERGE_MODE/双.so；cmake 模块未迁为专门样本；局部变量 camelCase 未动。

### 遗留（待作者定夺）

- **①B1 命名规范与存量矛盾**：✅ 已决（2026-06-14）——采纳 (a) 维持现状。`example_style.md` 规范前瞻用尾下划线，`examples/` 存量 ~7674 处 `m_` 前缀不动（迁移收益低风险高）；新 example / 实例库按尾下划线生成。

## P0 清债（✅ 大部分 2026-06-16 清完）
- [x] 死链校验（全库）— D3 扫描：内部 0、外部 2 修（jetbrains CLion cmake + qt.io qdoublespinbox 大写），入 PR#11
- [x] 「需要注意的是」全库替换 — PR#11（30 处）+ 风格违例值得注意的是/一般来说/大概（Task A，入 main）
- [ ] 专家 sidebar 收敛防扑空
- [ ] 入门结构漂移修正（05-other ~25 篇缺踩坑段挂账：等真写到该模块再补真坑，不编）

## P0.5 全库扫描批（结构漂移/死链/风格泄漏/重复段/单点bug → 清单后逐个修）
- [ ] `examples/CMakePresets.json` + `cmake/Qt6Defaults.cmake` 共享 + 空 app//model/ 骨架
- [ ] `qt_src/` 进 `.gitignore` + `scripts/fetch_qt_source.sh` + CLAUDE.md 说明

## P1 加固
- [ ] 三层交叉链接 + lint
- [ ] H2 措辞统一
- [ ] C++17·23 统一
- [ ] multithreading 踩坑章重构
- [ ] QML6.2 进阶重写

## 站点风格精细度对齐 tamcpp（✅ 已落盘 2026-09-21 · build 双跑通过 · 待作者实机看效果）

> 方向拍板：对齐的是**精细程度**（tamcpp 每个正文元素一套完整设计：变量集亮暗双套+装饰+微交互+无障碍+响应式+打印），不是换皮肤。Qt 绿身份 + IDE 工作台隐喻不动；quiz/在线编译等 interactive 功能不搬（S7）。

落盘清单（改动全在 `site/.vitepress/`）：

- **样式分层**（对齐 tamcpp 三文件架构）：新建 `theme/article-code.css`（代码卡）+ `theme/article-quote.css`（引用块）；`custom.css` 892→~900 行重组（令牌+排版+IDE chrome，正文元素专项外迁）
- **代码卡**（Creator 编辑器隐喻）：卡片化（边框+径向头部 tint+双层轻阴影 #254 档）、lang 徽章（</> mask 图标+Qt 绿 badge）、copy 按钮精修（focus-visible 焦点环）、行号列贴头部、hover 上浮（hover:hover+pointer:fine+no-preference 三条件）、code-group 集成去壳
- **折叠条升级**：summary 四格 grid（icon 徽章+标题+行数徽章+chevron 旋转），`code-fold-plugin.ts` DOM 同步升级（照 tamcpp 结构）
- **引用块**（概念批注隐喻）：SVG mask 装饰引号（不进复制/屏幕阅读器）+hover 上浮+嵌套降级+移动端+打印，配色微绿白+Qt 绿 accent
- **code-label-plugin.ts**（新）：lang 徽章文案美化 cpp→C++/qml→QML/cmake→CMake/bash→Shell 等（全站分布实测映射），挂 shared.ts
- **mermaid 灯箱**（新，搬 tamcpp 全套）：maximize 按钮（hover 显形）+ MermaidLightbox 全屏模态（panzoom 缩放拖拽+焦点陷阱+ESC）；新增依赖 `@panzoom/panzoom@4.6.2`（动态 import 不进首屏）
- **router-hooks.ts**（新，搬 tamcpp）：onAfterRouteChange 单值回调的订阅器根治（mermaid-client 从直接赋值换订阅；ReadingProgress 已是 watch 无现行冲突，此为防复发加固）
- **细节四件套**：::selection 选区配色、滚动条 8px thin+Firefox 标准属性、图片阴影调轻（0.08→0.05）、暗色/打印/reduced-motion 全覆盖

验证：`pnpm build` 双跑 SUCCESS（211s/206s·10 卷 4832 篇）；dist 抽查折叠条新 DOM（icon+count）✓、lang 标签 C++ 1176 处无裸 cpp 残留 ✓、panzoom/lightbox 进 chunk ✓。

**追加修复 · 宽屏布局塌陷（作者 2026-09-21 报「默认打开宽度抽象」，puppeteer 实测定位）**：
- 根因：VitePress ≥1440px 把「布局居中留白」塞进侧栏盒（盒宽 = (100vw−(layout_max−64))/2+sidebar_w−32）。tamcpp 布局宽 1560+侧栏底色=页面底色所以隐身；咱们布局 1280+侧栏是 IDE 白面板 → 2560 屏白面板假宽 1120px、正文被挤剩 576px。**PR#17 照搬 tamcpp 贴左手法时埋的雷，非本次改造引入**
- 修法三件（custom.css + ResizableSidebar.vue）：①≥1440 侧栏盒宽钉死=变量宽（白面板只包真实内容），正文避让=侧栏右缘+48px，居中留白全让右侧；②正文列放开 VitePress 默认 content-container 688px 上限→1380（类名加倍压 scoped 特异性）；③自适应侧栏上限改视口相关 min(480, max(272, vw/5))+大纲 flex-shrink:0
- 实测（puppeteer·beginner 首页）：2560 屏正文 576→1047px；1440 屏大纲 128→256px；1280 屏大纲 96→256px；全视口正常。build SUCCESS

待作者拍板（视觉决策，未擅动）：
- tamcpp 正文 1.05rem/代码 0.91rem vs 咱们 0.82rem/0.75rem（偏小一档，tamcpp 注释明说改大是 1080p+ 阅读反馈）——改的话牵动五档字号 normal 档基准，老用户存的档位意义会漂移
- 首页 feature 卡 rail 左条手法（tamcpp VPFeature 专用，咱们首页是自定义组件 LayerCards，要移植得改组件，属后续可选）

## P4 时效
- [ ] QtCharts 弃用横幅（迁 QtGraphs）
- [ ] QtGraphs
- [ ] QML AOT
- [ ] ⚠QProperty·QBindable 专家章（待决策：并入 102→103，或单列。见 [expert.md](expert.md) 待决策）

## 地基债
- [ ] 建 `tutorial/expert/00-environment-setup/` 空目录占位（实测缺失）
- [ ] `examples/expert/` 根 CMakeLists.txt（实测缺失）
- 今天可建空占位，内容随「开 00 环境篇 / 产 example」补
