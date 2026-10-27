---
title: "3.21 QCommandLinkButton 进阶"
description: "入门篇把 QCommandLinkButton 的双层信息结构、向导对话框用法和跨平台外观回退过了一遍，进阶篇拆按钮面板静息跳过、悬停补画的绘制分岔，拆 QSS 颜色字号够得着却分不开标题与 description 的边界，再过 description 换行与 sizeHint 的联动，末尾按描述必要性与平台一致性两条判据过选型，配套示例在 examples/advanced/03-qtwidgets/21-qcommandlinkbutton-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.21——QCommandLinkButton 进阶

[入门篇 QCommandLinkButton 基础](../../beginner/03-qtwidgets/21-qcommandlinkbutton-beginner.md) 把 setText 加 setDescription 的双层信息结构、向导对话框里的用法连同 Linux 与 macOS 上的外观回退过了一遍。[3.20 QCheckBox 进阶](./20-qcheckbox-advanced.md) 收尾说过按钮五连在本篇收官，QCommandLinkButton 是 QPushButton 的子类，却是五连里脾气最偏的一个：同一份代码，咱们拿去 Windows 上编译，出来的是蓝标题加一枚箭头图标的引导按钮。挪到 Linux 上一跑，箭头还在，只是退化成一枚普通的右向箭头，标题也从主题深蓝变回普通字色，引导感弱了一大截。代码没写错，绘制路径本身就分了岔。

笔者把这类按钮最容易把人问住的场面摆出来。想用 QSS 单独把 description 调深一档颜色或缩一号字，color 和 font-size 写上去都生效，可标题跟着一起变，选择器翻遍文档也找不到只对准 description 的写法。给按钮 setIcon 配了自定义图标，默认箭头当场消失，自定义图标顶上了它的位置，文字的起点还跟着挪。同一个页面摆几个 QCommandLinkButton，description 长短不一，按钮高度参差不齐。这些场面各有一套根因，本篇顺着绘制路径、description 布局、选型三件事把它们拆完。

## 面板静息不画，悬停才补：分岔点在 CommandLinkButton 特性位

先把分岔点找准，它不在绘制元素的枚举上。普通按钮非 flat 时静息状态下画的同样是 PE_PanelButtonCommand，QCommandLinkButton 画的也是它，两个类在这枚元素上跑的是同一段代码。[3.17 QPushButton 进阶](./17-qpushbutton-advanced.md) 聊 flat 的时候已经见过 PE_PanelButtonCommand 这名字，flat 按钮靠跳过它的背景绘制做出扁平感，当时看的是按钮怎么变干净，这回换个角度：QCommonStyle 的分支条件里，Flat 与 CommandLinkButton 两个特性位写进同一条“静息跳过面板”的判断，按下与选中面板照补，悬停补画单独认 CommandLinkButton，flat 按钮悬停时依旧没有面板。区别在这个位是谁置的，flat 要您手动 setFlat(true)，QCommandLinkButton 则在 initStyleOption 里给自己置上 CommandLinkButton 位，待遇生来就有。示例第三分区恰好有一枚普通 QPushButton 摆在命令链接按钮底下，咱们静息状态对一眼，普通按钮有边框面板，命令链接按钮只有文字和箭头悬在空白上，鼠标挪上去面板才补出来。

Windows 的画法是专门修过门面的。QWindowsVistaStyle 在 polish 阶段就给 QCommandLinkButton 调好一套专属配色，ButtonText 角色设成深蓝，BrightText 角色设成亮蓝，字体顺带换成 Segoe UI，再给按钮置一个内部属性，标记它正处在 Vista 主题之下。画面板时样式拿着 CommandLinkButton 特性位换用命令链接的主题部件，调 Visual Styles API 只画背景。标题、description 和箭头都不走主题的文字绘制，箭头是一枚由主题位图生成的标准图标，两带文字全由按钮自己的 paintEvent 来画。咱们在 Windows 上看到的蓝，源头就是 polish 调进去的调色板。悬停的反馈是变色：光标停在按钮上且未按下时，两带文字一起取亮蓝，再跟当前颜色做插值，逐帧从深蓝向亮蓝过渡，全程没有下划线参与。这套配色只在 Vista 主题可用时成立，老系统或经典主题下按钮落回通用画法，跟 Linux 同路。

Linux 上没有 Windows 那套主题，Fusion 的处理就是上面说的共用分支，静息不画面板，悬停补画。咱们在 Linux 上看到的两行文字，标题粗体 9pt，description 同样 9pt，颜色读同一个调色板角色，视觉上描述显得浅一些是标题独占粗体、笔画粗细不同带来的错觉，颜色本身与标题一致。macOS 的默认样式是 macos 插件，它没有给命令链接准备专属的主题绘制，面板画法交回 QCommonStyle 的共用分支，观感与 Linux 相近。

| 平台 | 绘制路径与外观 |
| ---- | ---- |
| Windows（Vista 及以上，主题启用） | QWindowsVistaStyle 用命令链接主题部件画面板，polish 调好深蓝标题，悬停向亮蓝过渡，箭头是主题位图生成的图标，文字由按钮自绘 |
| Linux | Fusion 走 QCommonStyle 共用分支，静息无面板、悬停补画，标题粗体 9pt，description 同号同色 |
| macOS | 默认 macos 插件，面板画法交回 QCommonStyle 共用分支，观感与 Linux 相近 |

### QSS 够得着颜色字号，分不开两带文字

绘制分岔定下了 QSS 的能力边界，只是边界的位置跟直觉不一样。QSS 可以覆盖面板的背景和边框（background-color 与 border 属性），color 与 font 属性也够得着文字：color 落进调色板的 ButtonText 角色，而 paintEvent 里标题和 description 读的恰恰是同一个角色，两带文字一起变色，字号同样两边一起动。真正够不着的是单独控制，description 的绘制在 QCommandLinkButton 自己的 paintEvent 里完成，不经过任何样式的绘制分支，QSS 没有一个子控件选择器能单独对准它。开篇第一个场面的根因在这里，您怎么换写法都绕不开，能落笔的位置只有两带共享的同一处。

```cpp
// color 落进 QPalette::ButtonText，标题和 description 同角色，一起变色
// 背景、边框归面板绘制，QSS 同样够得着
btn->setStyleSheet(
    "QCommandLinkButton { color: #0066CC; background: transparent; border: none; }"
    "QCommandLinkButton:hover { color: #004499; }");
// 想只改 description 的颜色或字号，没有能单独对准它的选择器，写不出来
```

您要是必须精确控制 description 的外观（比如统一成深灰 12px），路有两条。一条从 QCommandLinkButton 派生，重写 paintEvent，面板照旧交给样式画，两带文字自己 drawItemText，想怎么分就怎么分，代价是悬停变色、按下位移这些细节得自己接住。另一条干脆不用 QCommandLinkButton，自己拼，方案在选型一节展开。

配套示例的第二分区摆着这套 QSS，写在 platform_style_demo.cpp 的 btnStyle 里，比上面的片段多补了边框、圆角与内边距（examples/advanced/03-qtwidgets/21-qcommandlinkbutton-advanced/，可执行文件与目录同名）。您把 btnStyle 里的 color 从 #0066CC 改成 #cc3300 重新编译，标题当场变色，description 跟着一起变，补一行 font-size 也一样两带同动。这个“分不开”正是能亲眼看到的真边界，边界不用背，看一眼就长在眼睛里。

## description 是画出来的，也是量出来的：换行与 sizeHint 联动

咱们说 description 的换行，它是自动的。文本超出按钮宽度时，绘制给 drawItemText 传了 Qt::TextWordWrap 加 Qt::ElideRight 的标志组，文字在按钮可用宽度内自动折行，放不下的行尾以省略号收场。换行本身省心，麻烦在换行牵动了尺寸：QCommandLinkButton 的 sizeHint 计算要考虑 description 的行数，三行描述就意味着按钮高度得容纳标题、间距和三行文字。运行时通过 setDescription() 修改描述长度，sizeHint 跟着变，按钮在布局里就自动重排，高度当场调整。

```cpp
btn->setDescription("短描述");   // 行数少，sizeHint 小，按钮矮
// 用户操作后换成一段长描述
btn->setDescription("一段长到自动换行的描述文字……");   // 行数变多，sizeHint 变大，布局重排
```

咱们到配套示例第三分区看，platform_style_demo.cpp 的 createDynamicSection 摆了一个 m_dynamicBtn 和一个切换按钮，您点一下切换按钮，描述就在短文本与长文本之间来回换，盯着 m_dynamicBtn 的高度，一伸一缩全是 sizeHint 在背后动。

咱们把几个这样的按钮摆到同一页面，这个特性就变成了开篇第三个场面的麻烦：description 长短不齐，按钮高度参差，一行排下来高高低低。处理方式有几种，代价各不相同：

| 处理方式 | 代价与边界 |
| ---- | ---- |
| 文案纪律，每个按钮的描述统一收在一到两行 | 零成本，靠写文案的人自觉 |
| setFixedHeight / setMinimumHeight 固定高度 | 长描述被右端省略号截断，装不下的信息照样丢 |
| QSS 设统一的 min-height | 只保下限，行数照样能撑超 |

布局上还有一个细节跟图标位有关。QCommandLinkButton 构造时会向样式要一枚命令链接标准图标放进自己的图标位，Fusion 把这个请求映射成普通右箭头，Windows 上才由主题位图生成那枚更有引导感的箭头。图标位只有一个，您再调 setIcon() 设置自定义图标，默认箭头当场被顶掉，自定义图标顶上它的位置。文字的起点又按当前图标的宽度来算，换一枚窄图标，标题和描述会整体左移。开篇第二个场面的来路在这里。想省掉箭头腾出空间，一行 setIcon(QIcon()) 就够，这就是隐藏它的正式办法。想在示例里亲眼看一下，给 createNativeSection 里的 fullBtn 补一行 setIcon()，图标被替换、文字起点跟着挪，宽度差异一眼可见。

## 选型就两条判据：描述文字要不要，平台一致要多少

平台差异和布局行为咱们都过了一遍，选型的问题就能落得干脆。判据是描述文字的必要性与平台一致性要求，两条定下来，选哪个基本不用犹豫：

| 需求场景 | 选什么 |
| ---- | ---- |
| 只要标题，不要描述 | QPushButton。QCommandLinkButton 不设描述也还挂着箭头和粗体标题，Linux 与 macOS 上静息无面板，高度也更高，与普通按钮的预期对不上 |
| 标题加描述，产品只发 Windows | QCommandLinkButton，原生风格零成本，效果好 |
| 标题加描述，要跨平台基本一致 | QCommandLinkButton 加 QSS，color 与 font-size 对两带文字一起生效，再配 background 与 border 统一面板，接受箭头位图的平台差异 |
| 标题加描述，要像素级一致或工程里大量复用 | 自定义 widget，一次封装换完全控制 |

咱们自己拼的时候，拿一个 QWidget 做容器，内部 QVBoxLayout 摆两个 QLabel，标题那个蓝色较大字号，描述那个灰色较小字号加 setWordWrap(true)，再配 setCursor(Qt::PointingHandCursor)，靠 mousePressEvent 模拟按钮的点击。关键片段长这样：

```cpp
m_title = new QLabel(title);
m_title->setStyleSheet("color: #0066CC; font-size: 13px; font-weight: bold;");
m_desc = new QLabel(desc);
m_desc->setStyleSheet("color: #666666; font-size: 12px;");
m_desc->setWordWrap(true);
setCursor(Qt::PointingHandCursor);
// 按钮的交互要自己补：enterEvent / leaveEvent 切悬停样式
// setFocusPolicy 定焦点策略，setAccessibleName 补辅助功能名称
```

这个方案的好处是每个元素的样式完全可控，代价是按钮的交互属性全要自己接，悬停效果靠 enterEvent / leaveEvent 切换样式，焦点管理靠 setFocusPolicy，辅助功能名称靠 setAccessibleName，一样都不能省。您工程里这类“标题 + 描述”的选项按钮要是量大，值得花时间把它封装成公共控件，摊薄一次就回本。

咱们回到那个具体场景收个尾：安装向导的“选择安装方式”页面，完整安装、自定义安装、最小安装三项各配两三行描述，Windows 上原生风格确实好看，产品还要上 macOS。笔者的答案是先拿 QCommandLinkButton 加 QSS 把文字颜色和边框统一起来，macOS 上没有主题配色撑场面，观感朴素一截，能接受就收工，完全追求像素级一致的投入产出比通常不划算，真被设计卡住，再把自定义 widget 封装出来顶上。示例已经把前两种方案摆进了同一个窗口，第一种在 createNativeSection（原生），第二种在 createStyledSection（QSS 美化），第三种留给您动手，照本节的片段在 platform_style_demo.cpp 里拼一个 CommandOption 类，在示例窗口的构造函数里加一行 addWidget 摆进主布局，点谁就让 m_resultLabel 显示谁的文案，完成标准是三种实现同页共存、每种都能点出反馈。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QCommandLinkButton 属于 QtWidgets 模块，绘制路径涉及 QStyle 与 QCommonStyle，示例可在任何支持 Qt 6 的桌面平台上编译运行，原生命令链接外观仅 Windows Vista 及以上可用。

[Qt 文档 · QCommandLinkButton](https://doc.qt.io/qt-6/qcommandlinkbutton.html) -- 命令链接按钮，setDescription 与绘制机制

[Qt 文档 · QPushButton](https://doc.qt.io/qt-6/qpushbutton.html) -- 父类，QCommandLinkButton 在它之上换掉了绘制路径与默认外观

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 样式系统，PE_PanelButtonCommand 绘制元素

[Qt 文档 · QCommonStyle](https://doc.qt.io/qt-6/qcommonstyle.html) -- 通用样式基类，按钮面板静息跳过、悬停补画的共用分支在这里

---

按钮五连到本篇收官，QPushButton、QToolButton、QRadioButton、QCheckBox 加 QCommandLinkButton 一路看下来，按下、选中、三态、引导四种交互各归各位。下一篇 [3.22 QLineEdit 进阶](./22-qlineedit-advanced.md) 转进输入类，从单行输入框的验证器与输入掩码讲起。想再动手的，请您把示例（examples/advanced/03-qtwidgets/21-qcommandlinkbutton-advanced/，可执行文件 21-qcommandlinkbutton-advanced）完整跑一轮，前两个分区对照两套外观，btnStyle 改一色看标题和描述一起变色，第三分区点切换按钮看高度一伸一缩，再照选型一节补上自定义 widget，三种方案同页点个遍。您手头要是正有要上双平台的向导页，先把两条判据过一遍再动手。
