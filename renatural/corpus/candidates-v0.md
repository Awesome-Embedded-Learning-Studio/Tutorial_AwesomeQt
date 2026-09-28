# 语料候选清单 candidates-v0

- 归档日期：2026-09-28
- 归档人：改版战役档案员（agent）
- 用途：renatural 战役作者声线语料候选，供主控复核分流
- 状态：**候选**——待主控复核后才可进 content_forge 语料库（见文末入库门槛）

## 来源与 provenance 记法

三路矿脉：

| 路别 | 标签 | 说明 |
|---|---|---|
| content_forge 矿脉 | forge | `.claude/content_forge/tools/humanizer-zh/corpus/` 五件全实读：VOICE.md(17行)/approved.toml(120行)/rejected.toml(226行)/annotations.toml(32行)/COMPARE.md(664行全读未抽读)，另读 corpus/README.md 与 EDITORIAL.md L30 作场景分类依据。记法：`C` = `/home/charliechen/Tutorial_AwesomeQt/.claude/content_forge/tools/humanizer-zh/corpus`，`Lx` 为该文件行号，approved 样本附条目 id |
| qtbase 新声 | qtbase | 本仓 `tutorial/beginner/00-environment-setup/` 三篇（00-qt6-install / 01-ide-setup / 02-cmake-first-project，新声）+ `tutorial/beginner/01-qtbase/` 两篇（01-qobject-meta-system / 02-signal-slot，试点新声），绝对路径 + 行号 |
| postwaver 旁路 | postwaver | 指定矿脉（post_waver）本身是断的：`.claude/content_forge/post_waver/.claude/state/article-registry.json` 不存在，state/ 与 content/ 软链树均被 content_forge/.gitignore 列为私密不入库。样本经旁路验证（approved.toml 的 author_verbatim 条目逐字命中本机源文件且行号对齐）后，从本机源仓库摘录的成稿，绝对路径 + 行号 |

场景分法：EDITORIAL.md 第 30 行七分法（opening / personal_history / concept_entry / mechanism / transition / code_explanation / ending）。

## 覆盖度

| 场景 | forge | qtbase | postwaver | 合计 | 目标 | 状态 |
|---|---:|---:|---:|---:|---:|---|
| opening | 4 | 4 | 1 | 9 | ≥3 | 达标 |
| personal_history | 3 | 4 | 1 | 8 | ≥3 | 达标 |
| concept_entry | 3 | 6 | 1 | 10 | ≥3 | 达标 |
| mechanism | 8 | 5 | 1 | 14 | ≥3 | 达标 |
| transition | 4 | 6 | 1 | 11 | ≥3 | 达标 |
| code_explanation | 5 | 5 | 1 | 11 | ≥3 | 达标 |
| ending | 3 | 5 | 1 | 9 | ≥3 | 达标 |
| **合计** | **30** | **35** | **7** | **72** | — | 七场景全部 ≥3，无留空场景 |

### 缺口说明

硬缺口：无——七个场景总量均 ≥3，且 forge、qtbase 两路主矿各自也全部 ≥3（每场景 ≥3）。

软缺口与风险（不阻塞复核，但入库前须知）：

1. **postwaver 路每场景仅 1 条**：只能作补充声部，不足以单独支撑场景。且该路矿脉断裂，样本经旁路验证（author_verbatim 逐字命中、waitings git 历史与本机仓库目录同构）而非 registry 直证，**最终归属仍待作者验声**；AwesomeModernCPP ch00/00-preface.md L53-54 有作者自述「仓库含 LLM 辅助内容、发布前须重写抹痕」，故该路只选人称与自述密度最高的段落。若要把正规矿脉接回：需原机的 article-registry.json（或确认各仓库别名与 scanGlob 后重跑注册）。
2. **forge 路权威等级不均**：approved.toml 的 author_verbatim 只覆盖 personal_history / concept_entry / mechanism 三类，opening / transition / code_explanation / ending 四类全由 COMPARE.md「后」版本（Good／好的／我会写／我的版本／我会把它松开／OK 作者逐字样例等改稿）补齐；且 personal_history / concept_entry / ending 三场景恰好压线 3 条，续挖优先。
3. **voice_only 样本 3 条**（ME-02 / ME-03 / ME-04）：声线可用，技术断言不可直接复用——栈占用须按最大递归深度核算、struct 示例数值必须绑定 ABI 或实测、constexpr 立即数/寄存器说法须看反汇编（annotations.toml fact_note 约束）。
4. **入库前清洗义务**：这批「后」版本成稿早于部分硬禁令——单字「先」在 forge 样本中多处出现（L142/L243/L335/L378/L532/L579 等）；「宿主机」「真机」后来进了改写映射（COMPARE.md L31-32）；「拆开/掰开」现行 VOICE.md 已列禁词（postwaver TR-11 照录原文）；Cinux-Book 全库半角逗号/冒号，与 AwesomeModernCPP 全角标点不同，lint 校准须按仓库分治。**语料学的是句法节奏与人称参与，句子入文前必须过禁词/映射清洗。**
5. **引号形态**：弯引号 “ ” 不属禁符（禁的是直角引号「」），各样本引号原样保留，未做归一化。qtbase 五篇经字节级核对全部使用 ASCII 直双引号（0x22）。
6. **负样本交叉污染已规避，须持续执行**：rejected.toml 有多条 text 直接摘自 COMPARE.md「我的版本」段落内部（reader-reaction-scripted←L245、mechanical-loosening-scaffold←L475、pronoun-padding-hardens←L486、technical-paragraph-without-people←L594、voice-particle-without-trigger←L634 句首、casual-closing-announcement←L632、loosened-into-unrelated-warmth←L416 尾句）——即作者改稿后来又被继续挑剔；本次采样已避开或明确截断并在 note 标注，05-3 与 06-3 尾段因此整段弃采。后续新增挖掘必须继续做此交叉核对。
7. **VOICE.md 未贡献样本**：其全文是写手规则与画像描述、无文章逐字句，「画像句」均为规约语言，混入会污染正样本。
8. **qtbase 路语域分流**：00-env 三篇与 02-signal-slot 以「咱们/您」为主语域（新声主流）；01-qobject 试点以「你/我们」为主且保留「说实话/大功告成/血压拉满」一类更旧的口吻（EN-08 的「大功告成」新声慎用），note 已逐条标注，写手按目标语域取用。

---

## 候选正文

条目编号：场景缩写（OP/PH/CE/ME/TR/CX/EN）+ 两位序号，路别以【forge】【qtbase】【postwaver】标注，各场景内按 forge → qtbase → postwaver 排序。

## opening（9 条）

### OP-01【forge】

> # 旅程之前
>
> 我们马上就要启程，咱们为了保证这一路不会太过坎坷，一些规范咱们要是尽早建立，可以剩下不少心思滴~。比如说，我们马上编写的测试框架代码和一些base库，现在是在宿主机(对的！也就是您手头的笔记本，或者是炫酷的台式机！)上被测试翻来覆去地被折磨！
>
> 也许下一集，我们就要把他们收归到内核上，那个对大部分新手而言非常可怕的，一个连标准库运行时都没有的世界。
>
> 所以嘛，哪一些咱们是真的能用，哪一些真不是，是C++ Runtime悄悄给咱们小小的补充的，要盘点一下，否则编译一个大逼兜，大家都不嘻嘻。

- provenance：`C/COMPARE.md L101-107`（#1 对照的「Good」块）
- note：章节开场把「尽早建立规范」翻译成带滴~和大逼兜的旅程召唤，是作者对「丑话说在前面」式 AI 开场的整段替换稿。

### OP-02【forge】

> 排内核的错，最怕的不是它大大方方地炸给您看，而是日志里只肯留下一个孤零零的 `failed`。谁失败了？为什么失败？坏在了哪一步？对不起，它一概没说。咱们最后能做的，也就是盯着这七个字母，试图和三个月前的自己心灵感应一下。
>
> 这种代码通常是怎么来的呢？其实吧，倒也不复杂。因为我们到时候写自己的内核就知道了，我们到处都有可能失败的操作！比如说分配一页内存分不到、按编号找一个槽位发现这地方被人占用了、问一块设备要一点数据发现设备直道歉，对不起没找到！我们只是为了省事，偷懒的给函数返回一个 `int`，成功给 0，失败给 -1。头几天写起来确实痛快，判断一下是不是 -1，直呼一声完事。写起来足够爽吧！
>
> 可是，当我们把时间再往后拨掉三个月。可当咱们把时间往后拨三个月。现在有个函数会按照编号查槽位，查到了就返回槽位号。它交回来一个 3，没什么好说的，3 号槽位嘛；它要是交回来一个 -1，麻烦可就来了：这是没有这个槽位，还是查找过程中真的出了错？`int` 本人对此守口如瓶。您只好去翻文档，再祈祷当初写文档的那位——大概率还是您自己——没有忘记更新。

- provenance：`C/COMPARE.md L218-222`（03-1「我的版本」）
- note：从「日志里孤零零的 failed」的具体疼痛进入本篇问题，先给现场再给追问；第三段首句原稿即带「可是，当我们把时间再往后拨掉三个月。可当咱们把时间往后拨三个月。」的重复，逐字保留。

### OP-03【forge】

> 格式引擎刚能工作那几天，改一个细节是件很折磨人的事。比如 `%08x` 少补了一个零，代码改起来不到十秒；想看看改对没有，却得重新编译内核、启动虚拟机，再盯着串口等它把那行字吐出来。几十秒倒也不长，可这种东西一天得试上几十回。等到晚上，您就会开始认真思考：少测这一次，应该也没事吧？自然下一场就是欸欸欸欸欸欸我靠我靠内核炸了内核炸了啊啊啊啊的，开始鬼叫起来了。
>
> 问题在哪里呢？问题在于我们没有进行解耦合，实际上，格式化本身根本不需要知道串口长什么样。格式化抓到一个数，欢乐的把它拆成字符，然后一个又一个把他们交出去，欸嘿！这活就干完了。至于字符最后进了内存、上了屏幕，还是从串口线上飞走，那是另一拨人的事情，我们的格式化模块，完全，也没有理由管理。如果您发现您的内核出现了这类问题，最好的办法是立刻停下手头的事情，理清代码的职责。相信我！不合理的耦合只会给您造成无穷无尽的麻烦。
>
> 所以，请让咱们先把它们拆开。格式引擎只负责排字，并且接受一个“字符出来以后交给谁”的出口。测试的时候，这个出口接一块普通缓冲区；等串口驱动真的来了，再把同一个出口接到串口上。这样为了看一个补零，终于不用每次都把整个内核叫醒了。

- provenance：`C/COMPARE.md L331-335`（04-1「我的版本」）
- note：用「改一个补零要重编内核等串口」的真实不耐烦引出解耦需求，作者示范架构动机要从工程现场长出来而不是事后补论证。

### OP-04【forge】

> 白天的时候，为了一行改动重新启动一次虚拟机，好像也没有多大事。三十秒嘛，喝口水就过去了。
>
> 到了深夜，第十次构建又跑起来，咱们的算盘可就精明了：刚才只改了一行，这次不测，大概也不会出问题吧？省下来的确实是三十秒，可 bug 最喜欢的，就是这种“大概”。它只要从中间溜过去一次，第二天就能请咱们花半天把这三十秒连本带利还回去。好一个高利贷啊！
>
> 实际上，咱们现在想验证的通常不是整个内核。格式化少补了一个零，`Result` 漏掉一种状态，这些代码在普通宿主机上就能跑。为了看它们对不对，每次都把虚拟机和串口请出来，多少有点杀鸡用虚拟化。
>
> 那就先定一个不宏大的目标：敲一条命令，几秒内把这些小块全部跑完。通过时别吵，失败时告诉我哪一个用例、哪一行出了问题。至于这个小框架需要哪些零件，咱们做到哪儿缺了，再补哪儿。

- provenance：`C/COMPARE.md L526-532`（06-1「我的版本」）
- note：「处境＋自嘲＋算账」的开场，COMPARE.md L520 明言「深夜第十次构建」最接近笔者自己的说话方式；注意尾段含单字「先」，复用须过禁词清洗。

### OP-05【qtbase】

> # 而今从头迈步从头越：从Qt6开始安装
>
> 打开 Qt 安装器，登录账号，点完许可协议，迎面就是一屏组件树：Qt 6.9.1 底下挂着 Desktop、Android、WebAssembly 一串分支，工具栏里 CMake、Ninja、Qt Creator 各自打着勾。勾哪些？这一步选歪了，要么硬盘和时间搭进去几十 GB，要么装完打开 IDE 发现连编译器都没有。然后直接红温了。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/00-qt6-install-beginner.md` 第6-8行
- note：开场零寒暄，第一句就把读者按在安装器组件树前，用屏幕实录式列举代替背景铺垫；自问「勾哪些？」替读者发问，随即把选错的赌注具象化（几十 GB 硬盘、装完没编译器）；段末「然后直接红温了」用网络口语收住情绪。标题化用「而今迈步从头越」给系列起调。

### OP-06【qtbase】

> # 现代Qt开发教程（新手篇）0.1——IDE 配置
>
> 同一个 Qt 工程，命令行里 `cmake -B build` 一把通过，进了 VS Code 满屏红色波浪线，`#include <QApplication>` 底下还画着"找不到头文件"。第一次见这场面的朋友多半会怀疑 IDE 坏了。IDE 没坏，它只是被蒙在鼓里：不知道 Qt 的头文件在哪，也不知道该用哪套编译器。把这两样告诉它，波浪线自己会退。这一篇咱们就干这件事，VS Code、CLion、Qt Creator 三家，各把工具链接上。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/01-ide-setup-beginner.md` 第6-8行
- note：开场钩子是读者马上会撞上的真实故障画面（红色波浪线、找不到头文件）；先替读者归因去恐慌（「IDE 没坏，它只是被蒙在鼓里」的拟人化），再一句话立全篇任务清单——问题、原因、任务三步走，段尾「波浪线自己会退」预支了读完的收益。

### OP-07【qtbase】

> # 现代Qt开发教程（新手篇）0.2——第一个 CMake Qt6 工程
>
> qmake 的 `.pro` 文件语法亲切，网上教程成山，为什么 Qt 6 的世界非要换 CMake？因为官方把重心整个挪了过去：Qt 6 新增的构建命令，`qt_add_executable`、`qt_add_resources` 这一族，只在 CMake 侧存在，文档示例也全线 CMake 化。qmake 还能用，但新项目再从它起步，等于逆着官方的方向游。这一篇咱们从零建一个能跑的工程，CMakeLists.txt 每一行为什么在那，讲清楚。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/02-cmake-first-project-beginner.md` 第6-8行
- note：开场先替读者问出最可能的异议（qmake 亲切为什么非换不可），答案给的是官方权力结构而非功能清单；「逆着官方的方向游」一句比喻完成定性让步（还能用，但不划算）；段末「每一行为什么在那，讲清楚」预告本篇颗粒度。

### OP-08【qtbase】

> 一个按钮被点击之后，该让谁知道这件事？最直接的写法：让按钮持有窗口的指针，点击时直接调窗口的函数。能跑，但代价是两头互相知道——按钮得清楚"谁"会响应它，窗口也得知道按钮的类型，换个窗口按钮跟着改，换个按钮窗口跟着改。
>
> Qt 的答案是信号与槽。按钮只说"我被点击了"，谁来听、听完做什么，它一概不知；窗口只说"我关心点击事件"。两边互不相识却能协作，中间的连接交给元对象系统，咱们只要告诉它"把这两个连起来"。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第12-14行
- note：开场用一个最小设计问题起手，先摆朴素解法（按钮持指针直调）再点破耦合代价（换谁都得跟着改），先让「能跑」再算账；新概念以拟人对话引入——「按钮只说‘我被点击了’」——抽象机制变成两个当事人的分工描述。

### OP-09【postwaver】

> 没有“说实话”这三个字，说的就是实话！最开始的时候，这个开头很一般，是我在项目最开始起头的时候书写的。没有想到会有一天，这个项目会变得越发的热闹！这使得我打算打起精神来，拿起我的键盘，准备好好的编写这个开头。
>
> 我不是语言神棍，说真的我一点都不想变成XX语言的忠实粉丝这种人。我**始终认为语言是用来解决问题的一种表达，只有合适和不合适的表达，没有正确不正确的表达（这也是笔者后续希望建立的教程的一个基线：说明他合适在什么地方上，并且分析他的利弊，给我们这些编程爱好者一个新的思考，而不是新的教条。即：不指望一个特性可以解决所有的工程难题）**
>
> 所以，正常而言，大部分教程会在这里向您吹捧——嘿我们的C++非常的新鲜，非常的美味，非常的牛逼。笔者偏不。我只是想从自己从事的工作，写过的代码，坐下来陪大家聊一下。为什么我们会坐在这里，谈论“Why C++?”的问题。

- provenance：`/home/charliechen/Tutorial_AwesomeModernCPP/documents/vol1-fundamentals/ch00/00-preface.md:23-27`（仓库 Tutorial_AwesomeModernCPP = post_waver 别名 awesome-modern-cpp）
- note：H1 后首三段。同文件 L33-45 已被 approved.toml 以 author_verbatim 收录（moderncpp-lunch-origin），此为同文件邻段连续散文；注意 L53-54 作者自述仓库含 LLM 辅助内容、发布前须重写抹痕，故选人称与自述密度最高的段落，最终归属仍待作者验声。

## personal_history（8 条）

### PH-01【forge】

> 第二次就是CCOperatingSystemX64，嗯，失败品，连CLI都没出，非要考古的朋友，欢迎到我的Github找找喔。

- provenance：`C/approved.toml L3-9`（id=cinux-failed-attempt，text 在 L9）
- note：失败项目直接认账并邀请读者去 Github 考古，历史陈述没有被整理成励志转折（annotations：失败经历直接承认）。

### PH-02【forge】

> 我面对我的辣椒小炒肉，就在想，有没有一种方式，既不丢掉 C 那种"贴近硬件"的控制力，又能用上更现代的语言特性来组织代码？答案当然就是 C++，而且不是那种九十年代的"C with Classes"，而是从 C++11 开始一路进化到 C++23 的现代 C++。

- provenance：`C/approved.toml L11-17`（id=moderncpp-lunch-origin，text 在 L17）
- note：抽象的语言选型从一盘辣椒小炒肉的饭桌现场里长出来，是「来路」叙事的生活锚点（annotations：从确实发生过的生活现场里长出来）。

### PH-03【forge】

> 在最当初的时候，我们的第一版断言压根就没想这么多，我们直接粗暴的包含 `<cassert>` 了，然后在该停的地方轻飘飘的写了一句 `assert(...)`。宿主机上编译、运行，全都正常。
>
> “太对了哥！代码就该这么写！” 绿绿的测试增长您满满的信心，然后准备编译内核的版本的时候，链接器冷笑一声，安静的递回来一个咱们从没实现过的名字：`__assert_fail`。“What！？”这是您的第一句话。

- provenance：`C/COMPARE.md L430-432`（05-1「我的版本」，全文至 L436）
- note：亲历翻车的复盘——「图省事包含 <cassert>」到链接器递回陌生名字，第一人称扛自己黑历史的「笔者位」叙事；后两段（L434-436）转入机制解法可一并取材。

### PH-04【qtbase】

> ## 1. 前言 / 为什么需要元对象系统
>
> 说实话，刚接触 Qt 的时候我最困惑的就是一件事：为什么写个类还要继承这个 QObject？而且还得加个 Q_OBJECT 宏？这不是给自己找麻烦吗？后来踩了一堆坑之后才发现，Qt 能做这么多神奇的事情——信号槽、属性系统、动态类型信息——全靠这个看起来有点"多余"的设计。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/01-qobject-meta-system-beginner.md` 第8-10行
- note：第一人称学艺史：连用三个问句复现新手内心戏（为什么继承？为什么加宏？不是找麻烦吗？），再用「后来踩了一堆坑之后才发现」完成从困惑到领会的翻转——作者把当年的无知摊开给读者垫脚。语域提示：此篇用「我/你」，与 00-env 系「咱们/您」并存。

### PH-05【qtbase】

> 很好，现在我们已经理解了元对象系统的基本概念。接下来看看几个常见的坑，这些都是我用血泪换来的教训。
>
> ## 4. 踩坑预防

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/01-qobject-meta-system-beginner.md` 第145-147行
- note：「我用血泪换来的教训」给整节踩坑盖上亲历者印章——坑不是文献综述而是伤疤陈列；前半句「很好，现在我们已经理解了……」的阶段性小结口吻带着师生同行的节奏感，自然引出下一节的身份转换（从讲原理的人变成交伤疤的人）。

### PH-06【qtbase】

> // 请不要这样写了，求你了
> connect(sender, SIGNAL(valueChanged(int)),
>         receiver, SLOT(onValueChanged(int)));
> ```
>
> 宏语法没有上面那道编译期检查，错误全拖到运行期才爆。比如手一抖把 valueChanged 打成 valuChanged：编译通过，运行不报错，信号发了槽永远不来，排查起来能耗掉一下午，最后发现只是少了个 e。同样的拼写错误放在函数指针语法下，编译期直接报错，定位就在那一行。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第69-74行（样本起点在代码块内部，故含收尾围栏）
- note：代码注释直接跟读者求饶（「// 请不要这样写了，求你了」）——禁用写法的态度写进代码本身；踩坑叙述落到具体笔画（valuChanged 少个 e）与真实时间成本（耗掉一下午），痛苦具象化，拒绝「难以排查」式抽象话。

### PH-07【qtbase】

> `[&]` 把 label 的引用捕进来，直观好用，但问题也出在捕获上：捕获指针或引用时，得保证信号发射那一刻对象还活着。函数里 new 了个 QLabel、Lambda 捕获它的指针连上信号，函数返回后对象在别处被删，信号再发射时，Lambda 摸到的就是野指针——不然您会收获一个非常漂亮的 segfault。这种崩溃还是偶发的，取决于信号什么时候来、对象什么时候死，排查起来相当磨人。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第147行（单段，渲染约5行）
- note：后果用反讽轻描淡写（「您会收获一个非常漂亮的 segfault」）冲淡恐怖感；偶发性讲成「信号什么时候来、对象什么时候死」的生死竞速画面；紧随其后的一段就给出根因与两条解法——惊吓与解药同段交付，吓完必须能治。

### PH-08【postwaver】

> 咱们把时间拨回第一遍。当年笔者做这个内核,断言借的就是 `<cassert>` 的壳,图省事嘛:头文件一包含、该停的地方轻飘飘写一句 `assert(...)`,正常电脑上的编译、运行全都正常,绿绿的测试一路给笔者涨着信心。
>
> 坏就坏在咱们把代码编进内核的时候。链接器呢,冷笑一声、递回来一个从没实现过的名字:`__assert_fail`。明明编译过了,凭什么它就不认呢?咱们顺着名字往回找,事情一下子就清楚了。

- provenance：`/home/charliechen/Cinux-Book/document/journey/00-armory/05-assert.md:24-26`
- note：亲历翻车叙事，人称分工与 VOICE.md「笔者扛第一遍历史/翻车、咱们桌边动手」的 2026-09-26 作者裁定吻合。同系列 01-why-now.md L40 已被 approved.toml 收录（cinux-failed-attempt），本文件尚未入库；注意 Cinux-Book 全库用半角逗号/冒号，与 AwesomeModernCPP 的全角标点不同——lint 校准时按仓库分治。

## concept_entry（10 条）

### CE-01【forge】

> 那咱们就从一个看着不起眼的需求入手吧！
>
> 请您起手写一个栈：能装 `int`；再要一个装 `double` 的，再要一个装 `std::string` 的。push、pop、top 三件事，逻辑一个字都不差。
>
> > 哦莫！建议bro您写一下，我是建议您体会一下，想一想写 C 的日子，您学完之后的内容方才深刻啊（喜）
>
> 您会怎么办？最省事的办法摆在眼前：写完 `int` 版，复制两份，把类型名换掉。

- provenance：`C/approved.toml L19-33`（id=moderncpp-stack-problem，text 在 L25-33）
- note：「从一个看着不起眼的需求入手」把读者拉进写三个栈的具体任务，块引用里怂恿读者真动手，概念进入靠任务不靠定义（annotations：先把人拉进具体任务）。

### CE-02【forge】

> 其实，可能有朋友不太了解什么叫标准库不能用，也不理解我非要用，会出现啥毛病。不着急。咱们抄起您喜欢的编辑器，走一个能多小有多小的程序吧！
>
> 我们往vector里面，直接塞上三个整数。然后抓起来就塞到g++去，您自个先享受去吧！

- provenance：`C/COMPARE.md L136、L142`（#2「我会写」块，两段 prose 之间的 L138-140 为代码占位注释块）
- note：先承认「可能有朋友不太了解」再用最小程序带人亲手撞墙，概念从实验进入而非从定义进入；尾句含单字「先」，复用须清洗。

### CE-03【forge】

> 让我们来看两个最具体的出口。测试想把字符收进数组里，所以它需要知道数组在哪、已经写到哪里；串口则简单得多，字符来了，直接写寄存器。这两边保存的状态不一样，收到字符以后做的事情也不一样。
>
> 格式引擎不该认识这些细节。它只需要两样东西：一个函数，告诉它“这个字符怎么送出去”；再带上一份原样交还给函数的数据。于是就有了这个很小的 `Sink`：

- provenance：`C/COMPARE.md L347-349`（04-2「我的版本」，后接 L351-362 Sink 代码与 L364「安心当个不问去处的送货员」收束）
- note：Sink 抽象从「测试缓冲区／串口寄存器」两个具体出口长出来，术语给已经发生的事情命名，是作者的概念引入范式。

### CE-04【qtbase】

> ## 信号与槽是什么：一个只声明，一个真干活
>
> 信号（Signal）是一句"事件声明"——按钮被点击、滑块值变、数据加载完成，声明本身不含任何实现；槽（Slot）是一个可调用目标，成员函数、静态函数、Lambda 都行。信号一响，连到它的槽全部被调用。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第20-22行
- note：标题本身就是定义（「一个只声明，一个真干活」的口语对仗）；正文两概念对称拆解：先一句话本质（事件声明/可调用目标），再各跟三四个具体实例；「信号一响，连到它的槽全部被调用」补完运行语义——定义、例子、行为三层各一句。

### CE-05【qtbase】

> ## Qt Creator：Kit 是三样凑一套
>
> Qt Creator 界面朴素，胜在它是官方 IDE，对 Qt 的支持是内置的。.ui 文件的可视化编辑、信号槽的图形化连接，到今天还是它做得最顺手，哪怕咱们主力用别家，留着它画界面不亏。
>
> 它管配置的单位叫 Kit：编译器、Qt 版本、调试器，三样凑成一个可用的构建环境。首次启动它会自动检测已装的 Qt，检测不到就手动补——Edit 菜单进 Preferences，Kits 分类下的 Qt Versions 里填上 qmake 路径（比如 `C:/Qt/6.9.1/mingw_64/bin/qmake.exe`），再回 Kits 页确认有一个 Kit 挂着这个 Qt 版本。跳过这步直接编译，得到的是一句 "No valid kit found"，缺了哪样它不细说，得咱们自己回来补。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/01-ide-setup-beginner.md` 第75-79行
- note：标题即民间定义（「Kit 是三样凑一套」）；正文先正式定义再落到具体菜单路径（Edit→Preferences→Kits→Qt Versions）；结尾带上报错的沉默性（「缺了哪样它不细说，得咱们自己回来补」）——概念、操作、故障三角齐备，先夸（朴素胜在官方）再教。

### CE-06【qtbase】

> 但这十行背后，构建系统要干三件普通 C++ 工程不用干的活。类头文件里出现 Q_OBJECT，就得先过 MOC 生成元对象代码，信号槽才有得连。图片、图标这类资源要过 RCC 编译进二进制。.ui 界面文件要过 UIC 转成 C++ 头文件。咱们看 CMakeLists.txt 时那些陌生的配置，大半就是在安排这三道工序。
>
> ## CMakeLists.txt 逐行拆

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/02-cmake-first-project-beginner.md` 第31-33行
- note：用「三道工序」的工厂比喻安放 MOC/RCC/UIC——概念不是名词背诵，而是回答「构建系统为什么比普通 C++ 多干活」；段末把 CMakeLists 里陌生配置归因到这三道工序，给下文逐行拆解埋钩，过渡零成本。

### CE-07【qtbase】

> ### 3.2 对象树与父子关系
>
> Qt 的对象树是一个自动内存管理机制。当你创建一个 QObject 时给它指定 parent，这个对象就会被加到 parent 的 children() 列表中。当 parent 被销毁时，它会自动删除所有 children。听起来很美好对吧？但这机制用不好会成为噩梦。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/01-qobject-meta-system-beginner.md` 第49-51行
- note：概念定义紧跟读者心理代言（「听起来很美好对吧？」）再急转（「用不好会成为噩梦」）——先给糖再给牙，概念引入自带态度和悬念，本节剩余内容都在兑现这句警告。语域：你/我们体。

### CE-08【qtbase】

> 这里你可能会问：QObject 的对象树机制和直接用智能指针管理内存，到底有什么区别？这个问题非常好。对象树的核心思想是"父子所有权"——每个 QObject 都有一个明确的 parent，parent 负责销毁自己的 children。而智能指针（比如 std::shared_ptr）是引用计数机制，多个 shared_ptr 可以指向同一个对象，最后一个释放时销毁。对象树的优势在于所有权非常清晰，不存在循环引用的问题（这在 shared_ptr 里是个经典坑），而且 Qt 的父子关系天然匹配 GUI 控件的层级结构。劣势呢，就是你需要手动确保 parent 活得比 child 久，不然就是野指针。两者不是互斥的，实际项目中经常混用——QObject 树管 UI 层的对象生命周期，智能指针管非 QObject 的资源。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/01-qobject-meta-system-beginner.md` 第143行（单段，渲染约9行）
- note：自问自答的深挖段式：「这里你可能会问……这个问题非常好」把读者没说出口的疑问请上台面；优劣对比讲完落到混合使用的实战结论（QObject 树管 UI、智能指针管非 QObject）——概念对比最终变成选型建议，不停留在学院式对照。

### CE-09【qtbase】

> 从回调函数过来的朋友可能会问：注册个回调不也能解耦？能，但解耦只是表层收益。信号槽真正值钱的地方是跨线程天生安全——参数打包、投递到目标线程、排队执行，这些 Qt 全包了；再加上自动断开（对象销毁连接跟着断）和一发多收。这些下文都会用到。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第16行（单段，渲染约4行）
- note：预判读者背景并主动接话：「从回调函数过来的朋友」点名另一流派，先让一步（能解耦）再翻牌（只是表层收益，跨线程才是值钱处）；「这些下文都会用到」顺手埋下全文路线图，概念引入兼做推销。

### CE-10【postwaver】

> 三个栈这种“逻辑一份、类型多种”的困境，不是咱们运气差撞上的，它普遍到有专门的名字：泛型编程（generic programming）。
>
> 这个名字是 David Musser 和 Alexander Stepanov 在 1980 年代叫响的，他们给的定义很实在：从具体、高效的算法里做抽象，得到对一整类类型都成立的算法。落到咱们这两个例子上：写 `smallest`，逻辑真正用到的只有“两个东西能比大小”；写 `Stack`，真正用到的只有“元素能拷贝”。把这些要求留下，把具体类型抽掉，一份逻辑就对所有满足要求的类型成立。泛，广泛；型，类型，名字说的就是这个意思。

- provenance：`/home/charliechen/Tutorial_AwesomeModernCPP/documents/vol1-fundamentals/ch09/00-why-generics.md:40-42`
- note：概念命名段：先复述困境再给名字，词源拆解收尾。同文件 L28-36 已被 approved.toml 收录（moderncpp-stack-problem），本段为其直接后文、同一连续散文。

## mechanism（14 条）

### ME-01【forge】

> 能跑，但栈要装几种类型，就得维护几份代码：咱们哪天发现 `pop` 少判了空栈，得挨个文件改过去，漏一个就多一个 bug；标准库要是也这么写，光是 `vector` 就得配上几百份几乎相同的代码。重复的根源其实只有一条：类型被写死在了代码里。

- provenance：`C/approved.toml L35-41`（id=moderncpp-copy-consequence，text 在 L41）
- note：从最省事的复制做法推到维护后果再收束到「类型被写死」的根因一句话，因果推进的机制段范本（annotations：从复制推到后果再得根因）。

### ME-02【forge · voice_only】

> 另一种，请各位拉高警惕，因为大家入门的时候，都写过递归。递归的确是个好东西，大问题拆成小问题，小问题拆成没问题。很符合我们处理工程问题的直觉，B! U! T! 只是符合直觉，因为很多场景下，我们面对的数据集是未知的，我们编写的递归代码，很容易接受到让递归树极端庞大的输入，他们很容易击穿一个栈。
>
> 比如说，计算阶乘时递归到 `n = 100000`，每一层函数调用，都要疯狂的消耗栈帧空间。这样的操作，很快就会把栈吃光。

- provenance：`C/approved.toml L43-53`（id=recursion-stack-overflow-lived-warning，text 在 L49-53，approval=voice_only）
- note：先接住「大家都写过递归」的共同经验再从输入规模推到击穿栈，允许 B!U!T! 式兴奋与毛边；voice_only——栈占用按最大递归深度核算等断言须按 annotations.toml 的 fact_note 核验。

### ME-03【forge · voice_only】

> 咱们可爱的编译器开始排布内存：从 `a` 这里开始。`a` 占掉偏移量 0 这 1 个字节，轮到了 `b` 这个成员，它的大小是 4 个字节，所以它就要求起始地址是 4 的倍数了，采纳偏移量 1 这个决策是不合格的，只能排到偏移量 4 上去了。您看这个中间的偏移量 1、2、3，就是编译器垫进去的 3 个填充字节。`b` 排在偏移量 4、占 4 个字节，正好收在 8 上，`sizeof(Tiny)` 就是 8。

- provenance：`C/approved.toml L55-110`（id=struct-layout-lived-walkthrough，节选 L75，approval=voice_only）
- note：「咱们可爱的编译器开始排布内存」的逐字节演算段——一次只引入一条规则马上亲手排一遍，全条目 L61-110 共三关可续挖；voice_only——示例数值必须绑定 ABI 或实测。

### ME-04【forge · voice_only】

> 比如说，当我们愉快的写下 `constexpr int kBufferSize = 256;`的时候，我们就是在告诉编译器：这个值在编译阶段就已经确定了，直接写进二进制文件里就完事了。这样的方式，可以让我们在运行的时候就直接以 CPU 产生一次立即数脉冲的时候就把数字带出来，而不用使用若干次寄存器，甚至可能是几次内存访问，我们才把 256 这个数字辛苦的憋出来（哎，真是不容易）

- provenance：`C/approved.toml L112-120`（id=constexpr-lived-explanation，text 在 L118-120，approval=voice_only；COMPARE.md L653-657 有同一文本的「OK（作者逐字样例）」副本）
- note：从「我们愉快地写下一行代码」进入，沿编译期确定、运行期取数展开，末尾允许留「（哎，真是不容易）」的不齐整情绪；voice_only——立即数/寄存器断言须看反汇编核验。

### ME-05【forge】

> 这里面的 `"[\" + level + "] " + module + ": " + detail`，每一步，都会产生新的临时 `std::string` 对象！每做一次 `+`，咱们就会再得到一个临时字符串。您想想，这多可怕！

- provenance：`C/COMPARE.md L38-40`（「我会把它松开」块，对应 L36 被批的不呼吸原句）
- note：作者亲手把一口气吞完的长句松开成带停顿、人称和「您想想，这多可怕！」的机制句，rejected.toml(breathless-temporary-string) 亦将此句引为官方修复方向。

### ME-06【forge】

> 咱们先试试最老实的写法：值放一份，错误也放一份，旁边再摆一个 `bool`，告诉我们现在该看谁。
>
> `Result` 既然只可能成功或者失败，那最好连“两边同时有效”这件事都别让人写出来。于是这里换成了 `union`：值和错误共用同一块地方。成功时让值住进去，失败时换错误来住；想把两个一起塞进去，对不起，咱们这个屋子里，没有为您提供第二张床。想来？没门！满员了！
>
> 当然，`union` 也不是白帮忙的。两个成员共用一块内存以后，编译器反而不敢替咱们决定该构造谁、又该析构谁了。想让哪一边活过来，得咱们亲手来。placement new 就是在这个时候进场的。

- provenance：`C/COMPARE.md L243、L247、L249`（03-2「我的版本」，L245 未收）
- note：先摆「最老实的写法」让非法状态现形，再引出 union 与手工生命周期，设计机制按代价层层登场；夹在中间的第二段（L245）已被 rejected.toml(reader-reaction-scripted) 摘为反例，未收；首句含「先」须清洗。

### ME-07【forge】

> `result.hpp` 头顶上的 `#include <new>`，其实是后来补上的。第一版把它漏了，偏偏整个测试照样能过。欸？这个结果就很迷惑了：placement new 的声明明明在 `<new>` 里，咱们没包含它，编译器怎么一点脾气都没有？
>
> 顺着测试文件往上翻，才看见它还包含了 `<string>`。这位热心邻居在当前这套标准库实现里，顺路把 placement new 的声明也带了进来。于是 `result.hpp` 自己缺的东西，被测试文件不声不响地补齐了。
>
> 可人家只是碰巧帮忙，并没有答应以后天天来。换一个不包含 `<string>` 的文件，问题立刻现形。所以咱们单独喂给编译器一个翻译单元，里面只准出现 `result.hpp`，看看它能不能靠自己站住：

- provenance：`C/COMPARE.md L263-267`（03-3「我的版本」）
- note：按真实排查顺序写机制——漏包含、测试仍绿、翻测试文件找到热心邻居、再起最小翻译单元验证，知识点（传递包含）在排查走完之后才得名。

### ME-08【forge】

> 咱们起手按最普通的方式编译，把异常给他打开咯！现在，让咱们的程序从一个失败的 `expected` 里硬取值，那么，聪明的大家自然知道，我们的运行库会毫不客气的终止掉我们的程序，当然，咱们的标准库还是会好心的为咱们留下 `bad_expected_access` 这个名字。至少，咱们知道它为什么死掉惹。
>
> 接着只改一个条件，把异常关掉。程序仍然会终止，只是刚才那句诊断没了。更麻烦的是，终止前写到标准输出里的内容可能还攒在缓冲区里，连那点线索也未必看得见。
>
> 到这里还不能说明它进不了内核。那咱们继续，把独立环境的开关全部加上，先只编译，不链接。嘿，它居然编过了。
>
> 别急着高兴，拿 `nm` 看一眼目标文件：里面还欠着 `abort` 和 `__stack_chk_fail`。编译器愿意替咱们生成目标文件，不代表最后真有人替它把这些名字补上。到了链接那一步，这两张欠条还是得还。
>
> 最后一组换回咱们自己的 `Result`。同样是忘记检查就取值，它会走进断言，把文件、行号和消息直接送到标准错误流。咱们现在需要比较的，已经不只是“能不能编过”，而是错误真的发生以后，还能给调试的人留下多少东西。

- provenance：`C/COMPARE.md L284-292`（03-4「我的版本」）
- note：每次只改一个条件就看一次结果的对照实验节奏，证据链逐步升级；作者 Tips（L299）自评「欠条」比喻偏怪，学节奏不随迁比喻；第三段含「先只编译」须过禁词清洗。

### ME-09【qtbase】

> ## 同步、异步与跨线程：谁来排队，Qt 说了算
>
> 发射者和接收者同线程，默认同步——发射处阻塞到槽执行完，跟直接调函数没区别。跨线程时 Qt 自动转异步：发射立即返回，参数打包送到接收者线程排队执行。想显式控制也有：

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第96-98行
- note：标题把机制结论写成判词（「谁来排队，Qt 说了算」）；机制对称展开：同线程一句、跨线程一句，每句先行为后类比（「跟直接调函数没区别」）；「想显式控制也有」一句自然引出代码，不另起过渡段。

### ME-10【qtbase】

> "自动"背后怎么认线程归属，是 [进阶篇 1.02](../../advanced/01-qtbase/02-signal-slot-advanced.md) 和 [专家篇 1.02 源码拆解](../../expert/01-qtbase/02-signal-slot-internals-expert.md) 的主菜，这里只要求会用，机制细节留给上面两篇。
>
> 异步这一端有个容易被忽视的前提：排队执行靠的是事件循环在转。空口说不直观，咱们动手看：配套示例在 `examples/beginner/01-qtbase/02-signal-slot-beginner/`（`cmake -B build && cmake --build build` 跑起来，七段输出对应七种用法）。做个实验：示例 5 里等定时器的那行 `QCoreApplication::processEvents();` 注释掉再跑，程序卡死在示例 5——事件不派发，`QTimer::singleShot` 的回调永远不来，`timerDone` 永远不翻转，while 循环空转。排队调用能不能落地，取决于事件循环在不在转，这个实验比文字描述直观。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第112-114行
- note：机制讲完立刻配可复现实验：注释掉一行重新跑，用「程序卡死在示例 5」的具体症状链（事件不派发→回调不来→标志不翻转→循环空转）代替抽象因果；「空口说不直观，咱们动手看」是作者的方法论口头禅——机制必须亲手复现才算讲清。前段「机制细节留给上面两篇」同时示范了分层 defer。

### ME-11【qtbase】

> 接着三行 `CMAKE_AUTO*` 就是给开头那三道工序派活的：AUTOMOC 盯 Q_OBJECT，AUTORCC 盯资源，AUTOUIC 盯界面文件。开关一开，哪个文件变了、要重跑哪个工具，CMake 自己盯着，不用咱们操心。漏了 AUTOMOC 的后果很典型：编译期风平浪静，链接期报上一条 `undefined reference to vtable for XXX`——类里写了 Q_OBJECT，moc 的代码却没跟上，吓人的报错背后就这么个根因。补上开关，把含 Q_OBJECT 的文件列进目标，就平了。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/02-cmake-first-project-beginner.md` 第63行（单段，渲染约6行）
- note：机制拟人成派工（「给三道工序派活」「AUTOMOC 盯 Q_OBJECT」「CMake 自己盯着」）；正常路径讲完立刻翻转故障模式——编译期风平浪静→链接期 vtable 报错，症状与根因一句话焊死，机制与坑在同一段闭环，修法一句收尾（「就平了」）。

### ME-12【qtbase】

> 您在八竿子打不着的另一个类里定义槽、连上这个信号，照样工作——声明和定义完全分开，是这套机制松耦合的根源。signals: 这个关键字本质是 public（moc 预处理时会特殊对待），Qt 用它标记"这一段是信号"，属于约定成俗。
>
> 这份约定生效有个前提：moc 真的处理了这个类。图省事不写 Q_OBJECT 的话，约定落空——不少人先写 signals 和 connect，想着"跑起来再补"，结果编译通过、信号永远连不上，因为没被 moc 处理的类，signals: 下面就是一串普通成员函数声明，怎么连都不响。继承 QObject 的第一件事就是加 Q_OBJECT，没有例外；根子（moc 的扫描机制）[1.1 篇](./01-qobject-meta-system-beginner.md) 拆过了，这里不重讲。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第46-48行
- note：先现象后根因：八竿子打不着的类也能连上，根子是声明与定义分离；再给机制的民间解释（signals: 本质是 public，「属于约定成俗」）；随后立刻给出约定失效的前提与症状链——机制、边界、故障三连，段末一句纪律（「第一件事就是加 Q_OBJECT，没有例外」）。

### ME-13【qtbase】

> 这串包名看着吓人，咱们拆开看就两拨：一拨是 `-dev` 结尾的头文件包，编译期要；一拨是 libxcb 系列的库，运行期 Qt 的 xcb 平台插件要链接它们。有人图省事只装 build-essential 和 mesa，结果编译确实能过，程序一启动就崩，终端里只有一句 `Failed to load platform plugin xcb`——头文件齐了所以编译过，xcb 插件依赖的系统库没跟上所以运行崩。编译过了、运行崩了，这种错位最迷惑人，装依赖这一步别省。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/00-qt6-install-beginner.md` 第50行（单段，渲染约6行）
- note：面对吓人的长依赖列表先给分类透镜（「拆开看就两拨」：编译期头文件/运行期库）；用「有人图省事」的反例把两类依赖的错位后果演一遍，两个「所以」并排焊死因果；「这种错位最迷惑人」替读者命名困惑并顺势给行动指令（别省）。

### ME-14【postwaver】

> GCC 的答复：
>
> ```text
> distinct.cpp:15:21: error: cannot convert ‘Stack<double>*’ to ‘Stack<int>*’ in initialization
>    15 |     Stack<int>* p = &b;
>       |                     ^~
> ```
>
> 不同意。编译器在告诉咱们：`Stack<int>` 和 `Stack<double>` 是两个互不相干的类型，跟 `int` 和 `double` 本身一个性质。它们都出自同一份 `Stack` 模板，编译器照着这份模板，给每种类型各生成了一份独立的代码。上一章的继承是让一堆类型变成一家子，`Circle` 是一种 `Shape`；模板正相反，让一份代码变成一堆类型，生成的类型之间没有任何亲缘。所以在 `Stack<int>` 里，元素就是连续排布的裸 `int`，没有虚表指针，没有间接跳转，访问它和访问普通数组没有区别。

- provenance：`/home/charliechen/Tutorial_AwesomeModernCPP/documents/vol1-fundamentals/ch09/00-why-generics.md:99-107`
- note：机制段范式：先摆真编译器输出，再逐句解读，末尾与上一章概念对偶（继承 vs 模板）。与 approved.toml 的 struct-layout-lived-walkthrough（author-conversation-example）同款「咱们排一遍」教学法，但本段来自成稿文件、有行号可复现。

## transition（11 条）

### TR-01【forge】

> 每一条，我们都见过。

- provenance：`C/COMPARE.md L20`（「命中以后，我会怎样说」表首行：`逐条见过`/`逐条讲过` 的替换句）
- note：回指前置卷经验的承接短句，把课程索引还原成作者与大伙的共同经历，是 rejected.toml(compressed-shared-history) 指定的作者改写方向。

### TR-02【forge】

> 对于咱们正常的上位机而言，甚至还要捎上我们自己写的测试框架自己，一起放到 `test/framework/` 这个目录下。
> 而我们自己手搓的Cinux，在将来，会按照“链接期选择”这个机制，纳入到我们的构建中，成为宏大内核的一部分。

- provenance：`C/COMPARE.md L195-196`（映射说明里作者松开后的第 3、4 条，对应 L190 的 AI 原句）
- note：宿主机与内核两组实现各归各家的环境切换承接，「捎上」「手搓的Cinux」「宏大内核」把目录与构建关系接住而不打断主线。

### TR-03【forge】

> `TEST("名字") { ... }` 展开以后，会出现一个测试函数和一个静态的 `Registrar` 对象。让我们先把镜头转移到这个登记器上，当然，还请您稍安勿躁，别急着管函数名是怎么拼出来的。

- provenance：`C/COMPARE.md L579`（06-3「我的版本」首行）
- note：「让我们先把镜头转移到登记器上，还请您稍安勿躁」——转场同时显式搁置副线（函数名怎么拼），不带报幕腔；含「先把」须过禁词清洗。

### TR-04【forge】

> 至于 `%p`，等真正打印地址时再说。它走无符号十六进制，并且带上 `0x`；这不是参数提升，也不是容错策略，只是咱们希望地址最终长成什么样子。

- provenance：`C/COMPARE.md L398`（04-4「我的版本」末段）
- note：「至于 %p，等真正打印地址时再说」把非当务概念显式延期，跨小节承接不欠账也不提前剧透。

### TR-05【qtbase】

> 动 IDE 之前有个顺序要立住：命令行先通。上一篇装好的 Qt 6.9.1，`qmake --version` 有输出、`cmake --version` 可用，随便一个小工程能配置成功，再进 IDE。这么排序的道理很实际——命令行通了，IDE 里再出报错，咱们就能断定问题在配置；命令行没通就先折腾 IDE，两边的报错混在一起，谁也说不清。
>
> ## VS Code：两个插件，一条路径

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/01-ide-setup-beginner.md` 第10-12行
- note：篇间过渡不靠「上一篇我们学了」套话，而是立一条工作顺序（命令行先通）并讲清排错逻辑（先固定变量，报错才可归因；否则两边报错混在一起谁也说不清）——过渡段本身在教排查方法论，兼做上一篇的验收清单。

### TR-06【qtbase】

> 咱们把 setupCommands 里那行 enable-pretty-printing 先按下不表，调试一节回收。
>
> ## CLion：填对工具链，剩下它包了

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/01-ide-setup-beginner.md` 第63-65行
- note：章回体悬念式前置引用：「先按下不表，调试一节回收」——一句话埋伏笔并明示回收地点；代码讲解中遇到超纲概念不硬讲，挂起并给出坐标，读者带着未解项读下去而非被塞满。

### TR-07【qtbase】

> Qt 的位置在 CMake profiles 里给：建一个 Profile，CMake options 填 `-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64`，Build type 选 Debug。写到这里您应该看出来了，这和 VS Code 里干的是同一件事，CMAKE_PREFIX_PATH 没变，换了个表格填而已。
>
> 调试是 CLion 的强项，QList、QHash 这类 Qt 容器展开直接看内容，咱们不用配任何东西。QML 高亮要另装插件，在 Settings 的 Plugins 页搜 QML，装上 QML Support 即可。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/01-ide-setup-beginner.md` 第71-73行
- note：跨节回连：在第三家 IDE 讲同一概念时停下指给读者看「这和 VS Code 里干的是同一件事，换了个表格填而已」——把散在各节的知识手动缝合，防读者孤立记忆；「写到这里您应该看出来了」把验收权交给读者，是对读者理解进度的信任姿态。

### TR-08【qtbase】

> 这个报错咱们在 0.1 篇见过面：`Could not find Qt6`。多半是 CMake 不知道 Qt 装在哪，也就是 CMAKE_PREFIX_PATH 没传进去，命令行下的给法：
>
> ```bash
> cmake -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64
> # Linux 换成 ~/Qt/6.9.1/gcc_64
> ```

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/02-cmake-first-project-beginner.md` 第67-72行
- note：跨篇回调把旧坑当老朋友重逢（「这个报错咱们在 0.1 篇见过面」）；先复述症状再补上一篇没给的命令行解法——过渡同时是补课，篇与篇之间形成互相偿还的知识账，新场景（命令行）顺手给平台差异注释。

### TR-09【qtbase】

> 其实笔者认为，信号槽的核心在于：发射者不需要知道接收者的存在。上一篇 [1.1 QObject 与元对象系统](./01-qobject-meta-system-beginner.md) 把 Q_OBJECT 与对象树备好了课，本篇专心讲通信本身。
>
> ## 信号与槽是什么：一个只声明，一个真干活

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第18-20行
- note：篇间分工句式：「上一篇把 Q_OBJECT 与对象树备好了课，本篇专心讲通信本身」一句话划清两篇的边界；开头「其实笔者认为」把过渡变成作者自己的取舍表态而非中立串场——过渡段也有一言之立。

### TR-10【qtbase】

> 你会发现，几乎所有的 Qt 类都继承自 QObject。这不是巧合，而是 Qt 整个框架的基石。QObject 带来的元对象系统，让 C++ 这个静态类型语言获得了类似反射的能力。我们可以运行时获取类信息、动态调用方法、在对象之间建立松耦合的通信机制。
>
> 这篇文章我们会一起搞清楚：QObject 到底是什么、对象树怎么管理内存、Q_OBJECT 宏到底做了什么。这些是理解 Qt 世界观的起点，不搞清楚后面会处处碰壁。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/01-qobject-meta-system-beginner.md` 第12-14行
- note：全篇路线图：三个「到底」问句列任务（是什么/怎么管内存/做了什么），再压一句不学的代价（「不搞清楚后面会处处碰壁」）当动力；「我们会一起搞清楚」把读者拉上同一条船。语域：你/我们体。

### TR-11【postwaver】

> 两条路不是二选一，实际工程里它们经常配合。咱们回头看上一章的 `Canvas`：它持有 `vector<unique_ptr<Shape>>`，`vector` 本身是模板，管“容器装什么类型”；装进去的 `Shape*` 走虚函数，管“画布怎么画”。一个类里，两套机制各干各的活。
>
> 接下来三篇咱们把模板的零件逐个拆开：函数模板把类型推导和几十行报错的读法讲清楚，类模板带咱们亲手实现一个泛型栈，特化则管“通用版本照顾不到的类型”该怎么单独安排。学完这三篇，`vector`、`string` 这些天天在用的东西是怎么造出来的，您就能看明白了——第 10 章的 STL，全靠这一章打底。

- provenance：`/home/charliechen/Tutorial_AwesomeModernCPP/documents/vol1-fundamentals/ch09/00-why-generics.md:160-162`
- note：章内收束+向后预告的过渡段：先回收上一章 Canvas，再点名后三篇各讲什么、勾到第 10 章 STL。注意「拆开」在此源稿中出现——现行 VOICE.md 已将「拆开/掰开」列为禁词，样本照录原文，改写侧不得复用该词。

## code_explanation（11 条）

### CX-01【forge】

> 这个是我的本地AI验证的时候，它起手的操作，可能吓到您了。在我安抚好您惊恐的心灵之后，还请您聚精会神的看看我们传递给编译器的flags，有不少您可能在前置卷见过了！他们分别是：`-ffreestanding`、`-fno-exceptions`、`-fno-rtti`、`-nostdlib`,
>
> 笔者可没骗您的！这的确是咱们将来把代码编进内核时的所采纳的flags！至于它们各管什么, 您出发之前跟我说私密马赛！我忘记了，那么您这边请：[工具链课](../../primer/01-toolchain/)逐条讲过，如果还是不理解，这里您可以大胆的当CV工程师，抄就完事了！

- provenance：`C/COMPARE.md L172-174`（#4 对照的「好的」块，前接 L167-170 的 bash 命令）
- note：bash 命令周边的讲解——先安抚「可能吓到您了」再聚精会神看 flags，「私密马赛！我忘记了」示范讲代码时顺口岔去工具链课又拉回来。

### CX-02【forge】

> 现在，让咱们拿 `%09lld` 真走一次吧！开头的 `%` 告诉解析器：后面不是普通字符了。紧跟着的 `0` 把填充字符改成零，`9` 记下总宽度，两个 `l` 再把参数类型推进到 `long long`，最后的 `d` 才决定按十进制、有符号的方式输出。
>
> 走到这里，格式串已经把要求说全了，咱们才去 `va_list` 里取那个参数。注意，是按照 `long long` 去取。可变参数不会替咱们保存一份“调用者当时传的类型说明书”，格式串写什么，取参数的人就信什么。要是调用方实际塞进来一个 `int`，这里却按照 `long long` 去读，事情不是“多出来的高 32 位有点随机”这么温和——这已经是未定义行为了。今天看着只是数字古怪，换个平台、换次优化，它爱怎么坏就怎么坏。
>
> 参数拿稳以后，剩下的才是排版：先算十进制数字本身占几位，不够 9 位的部分在左侧补零，再把数字送给 `Sink`。这样一趟走完，前面那五步才算真的落到了代码上。

- provenance：`C/COMPARE.md L378-382`（04-3「我的版本」）
- note：拿 %09lld 逐字符真走一遍解析流程，走到取参才讲类型宽度、走到排版才讲补零，代码讲解跟着执行顺序走；末段含「先算」须过禁词清洗。

### CX-03【forge】

> 真失败时，我们看到的报告最好指向这一行，而不是指向 `Check()` 函数体，否则的话，整个工程不管哪里出错，屏幕都只会反复告诉您：错在 `assert.hpp`。这情报不能说毫无用处吧，只能说约等于没有。马上您就会去掉他了。
>
> `std::source_location::current()` 因此被放进了默认参数。调用者虽然没写第三个参数，编译器却会在调用这一行把默认值补上；文件名、行号和函数名，也就在这里被抓住，然后跟着消息一起送进 `Check()`。里去，而咱们的`Check` 自己仍然是个普通函数。能打断点，能单步，参数也有类型。咱们没有为了拿调用位置，先把整个接口变成一个到处展开的宏。

- provenance：`C/COMPARE.md L454-456`（05-2「我的版本」，引子在 L448「假如说，咱们在 `AllocPage()` 里写了这样一句：」，L450-452 为代码块）
- note：先摆 Check 调用与「错在 assert.hpp 约等于没有」的后果，再让默认参数在调用处求值成为现象的答案；「送进 Check()。里去」的口语顿挫系逐字保留。

### CX-04【forge】

> 最后两行第一次看着有些重复。`list(APPEND ...)` 不是已经加进去了吗，为什么还要 `set` 一回？因为 CMake 的函数有自己的作用域。新目标确实进了 `ALL_HOST_TESTS`，但进的是函数里面那一份；函数一结束，它也跟着没了。`PARENT_SCOPE` 做的，就是把改好的清单递回外面，让最终的 `test_host` 真能依赖这些测试程序。
>
> 把这一行删掉再配置，三个测试照样各自存在，`test_host` 手里却是一张空清单。这个现象比把整份 `CMakeLists.txt` 从头解释一遍，更能说明它为什么不能少。

- provenance：`C/COMPARE.md L565-567`（06-2「我的版本」的 CMake 讲解段，前接 L544-563 的最小定义与完整函数代码）
- note：从「最后两行第一次看着有些重复」的真疑问进入作用域解释，再给「删掉这行再配置」的反证实验，配置代码讲解的问答节奏。

### CX-05【forge】

> 这里为什么非得用宏，也是在这一刻才看得出来。普通函数可以打印，也可以修改计数，可函数里的 `return` 只能退出函数自己，没办法替调用者结束当前测试体。宏展开在 `must_fail_body()` 里面，那个 `return` 才真的属于它。

- provenance：`C/COMPARE.md L616`（06-4「我的版本」，前接 L606-614 的必败测试体与失败流程）
- note：「为什么非得用宏」等 return 只能退出函数自己的现象出现之后才回答，代码讲解让结论从代码里浮出来而不是先挂墙上。

### CX-06【qtbase】

> 开头两行是每个 CMake 工程的标配。`cmake_minimum_required` 的版本咱们跟着官方写 3.16：入门示例和 qtbase 源码顶层都是这个数，机器上的 CMake 比这新当然没问题。`project()` 声明项目名、版本、语言，VERSION 会在 CMake 里生成 `PROJECT_VERSION_MAJOR` 一族变量，代码里要用版本号时直接取。
>
> `CMAKE_CXX_STANDARD` 给 17 是 Qt 6 的硬要求——它的头文件用上了 C++17 特性，标准给低，报错直接从 Qt 头文件深处冒出来，位置根本不在咱们自己的代码里。旁边的 `CMAKE_CXX_STANDARD_REQUIRED ON` 是让它硬得彻底：编译器不支持，配置阶段就报错拦下，而不是拖到编译期再炸。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/02-cmake-first-project-beginner.md` 第59-61行
- note：逐行拆解口吻样板：每行配置回答「为什么是这个值」而非「这是什么」（3.16 的出处是官方示例与 qtbase 源码顶层，顺带安抚版本更新的读者）；解释自带故障预演（报错从 Qt 头文件深处冒出来、位置不在咱们代码里）；「硬得彻底」「拖到编译期再炸」把开关语义翻成白话。

### CX-07【qtbase】

> counter 发射 valueChanged 时，app 的 quit 被调用。注意咱们没写参数类型——新式语法用函数指针，参数匹配检查交给编译器：信号带的参数槽收不下，直接编译失败。这种错在编译期报，比运行期崩友好太多。一次性的小逻辑不必单写槽函数，Lambda 直接上：

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第58行（单段，渲染约4行）
- note：「指给你看」手法：「注意咱们没写参数类型」把读者目光按到代码的空白处——讲解的亮点恰是代码里没写的东西；讲完立刻给对比评价（编译期报比运行期崩友好太多），再顺势引出下一段代码（「Lambda 直接上：」），散文与代码的交接零缝。

### CX-08【qtbase】

> 这里有几个细节值得注意。首先，构造函数通常接受一个 `QObject *parent` 参数，这个参数建立了父子关系。其次，构造函数通常用 `explicit` 修饰，避免隐式类型转换带来的意外。Q_OBJECT 宏是必须的——如果你打算使用信号槽或者元对象系统，这个宏一个都不能少。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/01-qobject-meta-system-beginner.md` 第38行（单段，渲染约4行）
- note：代码后的「细节清点」段式：首先/其次逐条点名代码里容易滑过去的修饰词（parent 参数、explicit、Q_OBJECT），每条都答「它在防什么」；「一个都不能少」用口语强调数量约束。语域：你体。

### CX-09【qtbase】

> ## 调试：pretty-printer、connect 日志、断点落点
>
> 先说 QString。GDB 默认看不见它的内容——QString 里装的是指针加隐式共享的结构，不打 pretty-printer，变量窗口里全是 `d->data` 这类代理字段，没有可读的东西。Qt Creator 自带渲染，CLion 内置，VS Code 就用咱们 launch.json 里埋的那行 enable-pretty-printing，配合 GDB 的 Python 支持把 QString、QList 打成可读的样子。三家的差别只是要不要自己动手配这一下。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/01-ide-setup-beginner.md` 第95-97行
- note：讲工具行为像讲病理：先说症状（GDB 默认看不见 QString 内容），再解剖病因（指针加隐式共享、变量窗口全是 d->data 代理字段）；「咱们 launch.json 里埋的那行」回收前文伏笔（与「按下不表」呼应）；三家的差别收束成一句（要不要自己动手配这一下）。

### CX-10【qtbase】

> 答案两处。`CMAKE_CXX_STANDARD` 给了 14，报错会从 Qt 头文件深处冒出来；AUTOMOC 没开，而 widget.cpp 里有带 Q_OBJECT 的类，链接期等着的 vtable 报错。对照上一节的逐行拆解，咱们两处都能对上号。
>
> 还想再狠一点，拿能跑的工程做实验：把 `CMAKE_CXX_STANDARD` 改成 14，重新配置构建，亲眼看编译器报的第一个错长什么样、落在哪个文件。见过一次，以后咱们在别的项目撞上同样的报错，就不会先怀疑自己的代码了。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/02-cmake-first-project-beginner.md` 第111-113行
- note：找错测验的答案讲法：先报数量（「答案两处」）再逐处对号入座，且与上一节讲解显式互指（「对照上一节的逐行拆解」）；随后「还想再狠一点」把答案升级为亲手实验，并点明练习目的——「见过一次，以后撞上同样的报错就不会先怀疑自己的代码」。

### CX-11【postwaver】

> 咱们拿一行真的排一遍。您在 `AllocPage` 里写下：
>
> ```cpp
> Check(ptr != nullptr, "分配器返回了空指针");
> ```
>
> 失败真发生的时候呢,咱们要报告指向哪一行?当然该指着您刚刚落笔的位置。可这里特别容易想歪:把 `current()` 挪进 `Check` 的函数体、每回进来取一次,看起来还挺直白呢。您也别急着点头:这么取,拿到的永远是 `Check` 自己躺在 `assert.hpp` 里的行号。咱们整个工程不管哪儿出错、看到的都只会是同一句:"错在 assert.hpp 第六十几行"。这情报咱们谁也用不上,咱们扭头就把这个写法丢掉。
>
> 默认参数把求值搬到了咱们写下调用的地方。您回头看 `AllocPage` 里的调用:第三个参数,您一个字都没写、编译器就地补上默认值。`current()` 呢,就在咱们落笔的位置当场求值:文件、行号、`AllocPage` 的函数名全抓齐,咱们装进 `loc`、跟消息一起送进 `Check`。

- provenance：`/home/charliechen/Cinux-Book/document/journey/00-armory/05-assert.md:58-66`
- note：代码讲解段范式：一行调用 → 故意走错路 → 编译器/链接器反馈 → 正解。与 approved.toml 的 struct-layout/constexpr 条目同为作者标志性的「错误答案先演一遍再丢掉」结构。半角标点为该仓库原貌。

## ending（9 条）

### EN-01【forge】

> 我们的名字表里，还会有一种错误，咱们是直接抓，抓不住的。那就是枚举顺序没变，可是咱们不小心把两个名字填反了。`switch` 也好，数组也好，他们都是无辜的关键字，自然不会替人判断字符串写得对不对。所以第一个测试很朴素，把六个错误逐个转成名字，再一个个核对。
>
> 您可能不太熟悉 `std::to_underlying`，这个可爱的小家伙来自 C++23 的 `<utility>`。这回终于轮到前面那句“语言可以往前走，库依赖仍然得看紧”落到代码上了——不是为了证明咱们紧跟潮流，单纯是这里刚好有个趁手的小工具，那就用它，哈哈！

- provenance：`C/COMPARE.md L311-313`（03-5「我的版本」）
- note：章节收尾停在「把六个错误逐个转成名字」的朴素测试动作上，只借 to_underlying 顺路岔一句哈哈，不替全文做总模型（松开方向：停在真实动作上）。

### EN-02【forge】

> 最后的最后，让我们故意只给它五个字节，再次不好意思的麻烦它输出 `abcdef`。结果缓冲区里留下的是：
>
> ```text
> abcd\0
> ```
>
> 前四格装字符，最后一格留给结尾的空字符。`e` 和 `f` 没地方住，老老实实被截掉；缓冲区外面的内容也没有遭殃。需要注意一下的是，咱们现在是在宿主机上测的。以后字符真的要往串口走，格式引擎还是这一份，换掉的只有接在后面的那个 `Sink`。

- provenance：`C/COMPARE.md L410-414 及 L416 前半`（04-5「我的版本」，尾句截断）
- note：收尾停在五字节缓冲区的真实输出与一句宿主机环境限定上（松开方向：停在实验结果）；本段尾句「到时候咱们再去折腾串口……好好睡觉才是家的美好」已被 rejected.toml(loosened-into-unrelated-warmth) 摘为反例，截断未收。

### EN-03【forge】

> 咱们现在，只需要登记用例、做几种断言、跑完以后给出失败位置。更重要的是，这套写法以后还想跟着测试进入内核。依赖越少，框架里每一步在做什么也越容易看清。

- provenance：`C/COMPARE.md L634`（06-5「我的版本」第二段，自「咱们现在」起摘）
- note：以「咱们现在只需要什么」收束工程选择而不审判现成框架；本段首句报幕（L632）与「欸嘿嘿」起句已被 rejected.toml 摘为反例故截去，L636 段尾「以后需求真的长大了，再重新评估就是，咱们又没和自己写的框架拜堂。对吧！」可作结尾语气补充。

### EN-04【qtbase】

> ---
>
> 环境就绪。下一篇 [0.1 IDE 配置](./01-ide-setup-beginner.md) 把 VS Code、CLion、Qt Creator 三家接上这套工具链。哪一步卡住的，咱们回头把这篇提过的坑再对一遍：路径空格、C++ 工作负载、xcb 依赖、DISPLAY 手写，安装期的事故大多落在这几处。第一个工程的完整构建验证，在 [0.2 第一个 CMake Qt6 工程](./02-cmake-first-project-beginner.md) 见。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/00-qt6-install-beginner.md` 第80-82行
- note：收尾三件套：两个字宣布状态（「环境就绪」）；下篇预告写成任务分派而非「敬请期待」；全篇的坑收成四词索引（路径空格、C++ 工作负载、xcb 依赖、DISPLAY 手写）供回查，并预告何处的验证才算彻底交卷——结尾是检索入口和验收标准，不抒情。

### EN-05【qtbase】

> ---
>
> 配置是不是真在工作，做个破坏性实验最放心：把 settings.json 里的 CMAKE_PREFIX_PATH 故意改错，重新配置，"Could not find Qt6" 应声而出；改回去再配，报错消失。能亲手把错造出来再消掉，这套配置就归您了。下一篇 [0.2 第一个 CMake Qt6 工程](./02-cmake-first-project-beginner.md) 从零建第一个工程，CMakeLists.txt 每一行为什么在那，讲清楚。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/01-ide-setup-beginner.md` 第119-121行
- note：收尾用破坏性实验交割所有权：故意改错看报错应声而出、改回复原——「能亲手把错造出来再消掉，这套配置就归您了」把「学会」的判据定义成能制造并消除故障，不是照着做通；下篇预告复用开篇句式，首尾同构。

### EN-06【qtbase】

> ---
>
> 环境搭建到此收官：Qt 装好、IDE 接上、第一个工程跑通。咱们想再练手的话，给 .ui 那份 XML 里塞一个 QPushButton 重新构建，看 AUTOUIC 自动重跑；下一篇 [1.1 QObject 与元对象系统](../01-qtbase/01-qobject-meta-system-beginner.md) 进 QtBase 正题，Q_OBJECT 这个到处出现的宏，就该拆开看看里面是什么了。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/00-environment-setup/02-cmake-first-project-beginner.md` 第227-229行
- note：卷级收官：三件事点名（装好、接上、跑通）收拢整个 00 卷 + 一个最小练习提议（塞个 QPushButton 看 AUTOUIC 自动重跑）+ 下篇钩子写成悬念——Q_OBJECT 是正文里反复出现却未解释的符号，「就该拆开看看里面是什么了」把好奇心引向下一卷。

### EN-07【qtbase】

> ---
>
> 日常开发里八成的信号槽场景，本篇的内容够用了。想动手的，跑配套示例：把某条 connect 的参数改错，看编译器报什么；或按上文的实验把事件循环掐断，看程序卡在哪。下一篇 [1.3 字符串与编码](./03-string-encoding-beginner.md)，讲 QString 和编码的常见陷阱。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/02-signal-slot-beginner.md` 第192-194行
- note：收尾先划适用边界（「日常开发里八成的信号槽场景，本篇的内容够用了」）——诚实声明覆盖面而非宣称全面；再给两个可执行的动手建议（改错看报错、掐事件循环看卡点），与正文实验呼应；零客套零总结腔。

### EN-08【qtbase】

> ---
>
> 到这里就大功告成了。QObject 和元对象系统是 Qt 的基础，理解了它们，后面学习信号槽、事件系统、QML 交互都会顺畅很多。如果某些地方还是有点模糊，别担心——随着我们后面的练习和实践，这些概念会越来越清晰。下一篇文章我们会深入探讨信号槽机制，那才是 Qt 真正神奇的地方。

- provenance：`/home/charliechen/Tutorial_AwesomeQt/tutorial/beginner/01-qtbase/01-qobject-meta-system-beginner.md` 第255-257行
- note：试点篇的暖收法：「如果某些地方还是有点模糊，别担心」直接安抚读者情绪并许诺后续重复，「那才是 Qt 真正神奇的地方」给下篇抬轿——与 00-env 系「状态+索引+任务」的干收尾是两种可选收法，按篇章温度选用；「大功告成」属旧口吻，新声慎用。

### EN-09【postwaver】

> 休息一下吧！大家工作，学习都不容易，喘口气，站起来走两圈，之后的日子咱们就要跟C++爆破了！
>
> 还在看，不讲知识，我们不内卷。C++ 我学到今天，下的结论是：C++是一门有深度的语言，学习曲线确实不算平缓（倒不如说平缓在哪里了），这一点笔者不忽悠您。但它也是一门回报丰厚的语言——等您真正把 RAII、模板、零开销抽象这些本事摸透，会发现写 C++ 是一件很痛快的事。
>
> 这套教程不会让您一夜之间变成 C++ 专家——**没有任何教程能做到这一点，如果有人宣称它能让你30天精通C++，你最好问问他什么是现代C++**。这条路会很艰辛，但是笔者希望这些教程可以陪您走完：从最基础的类型和变量，到面向对象的设计，再到模板和标准库的使用，耐心和动手的意愿，笔者认为，将会决定我们伟大的征程能走得多远。
>
> 休息会，咱们，要上路了。

- provenance：`/home/charliechen/Tutorial_AwesomeModernCPP/documents/vol1-fundamentals/ch00/00-preface.md:117-123`（末节「准备好了，先休息一下，然后，开始吧」）
- note：结尾四段：先劝休息、再交底学习曲线、反「30天精通」话术，末句「休息会，咱们，要上路了」是典型收束口吻。与 opening 样本同文件，作者自述此开头为本人重写（L23），可信度高于仓内均值。

---

## 入库门槛

> 以上 72 条均为**候选**：待主控逐条复核（真伪与权威等级、场景归属、清洗要求、voice_only 事实核验义务）后才可进 content_forge 语料库。未复核条目不得被写手规则、lint 校准或改写工具直接引用。复核时可参照各条 note 内的禁词/映射/标点警示与 rejected.toml 交叉核对记录。
