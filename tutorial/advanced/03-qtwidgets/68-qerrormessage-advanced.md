---
title: "3.68 QErrorMessage 进阶"
description: "进阶篇把 QErrorMessage 的抑制机关对到源码行上：内部是一只 std::queue 加两只 QSet（qerrormessage.cpp 第 44-46 行），复选框文案是“再次显示这条消息”且默认勾着、取消勾选才记入抑制集合（第 265、439 行），抑制记录写在 done() 关闭时机（第 292-302 行），showMessage 入队与出队各查一遍（第 343-347、398-406 行），type 走另一只集合（第 45-46 行），消息载体是只读 QTextEdit 且按富文本渲染（第 94-113、356-360 行），qtHandler() 装上 jump 只放行默认类别的日志并转发原处理器（第 188-223、329-338 行），Fatal 之后收尾走 exit(1)（第 221、311-312 行），整个文件没有一行 QSettings，跨重启的持久化咱们在门外自己做，示例在 examples/advanced/03-qtwidgets/68-qerrormessage-advanced/"
---

# 现代Qt开发教程（进阶篇）3.68——QErrorMessage 进阶

上一篇 QProgressDialog 收尾时咱们约好，错误消息的抑制机制与 QSettings 持久化在这边碰头。QErrorMessage 正好把两个话题都占全了。它与 [3.66 QFileDialog 进阶](./66-qfiledialog-advanced.md) 有一处相像，两者都记用户的偏好，也都靠记忆省掉重复打扰。差别在记忆放在哪，QFileDialog 把上回逛到的目录私底下写进 QtProject 名下的 QSettings，重启还能续上，QErrorMessage 则把用户勾掉的消息全放在内存里，进程一退就清零。为什么待遇差这么多，咱们把 Qt 6.9.1 的 QErrorMessage 源码（qerrormessage.cpp）从头翻到尾，答案就在里面。

引用文件咱们交代清楚。主战场是 qerrormessage.cpp 与 qerrormessage.h（src/widgets/dialogs），全部行为拿这对文件就能对证。入门篇走过的构造与基本用法本篇不重讲，配套示例是拿 QSettings 在外面包了一层的持久化演示窗口，完整玩法放结语。

## 抑制记在哪：两只 QSet 与一次 done()

私有类的家底，咱们列得出九样（qerrormessage.cpp 第 40-48 行）。控件有四件，一枚 QPushButton，一只 QCheckBox，一块 QTextEdit，一块 QLabel。状态也有几样，一只 std::queue<Message> 的待显队列，两只 QSet<QString>，名字分别是 doNotShow 与 doNotShowType，另有 currentMessage 与 currentType，一对 QString。有些资料把它讲成单个集合装消息文本，源码里是两只集合分工，一只按消息全文记，一只按 type 记，后面咱们挨个看它们的写入时机。

复选框的方向笔者要当场摆正。构造里 again->setChecked(true)（第 265 行），文案是 “&Show this message again”（第 439 行），意思是“把消息再显示一遍”，默认是勾着的。抑制发生在反方向，用户取消勾选后关闭对话框，done() 里检查（第 292-302 行）：复选框没勾着时，type 为空就把 currentMessage 插进 doNotShow，否则把 currentType 插进 doNotShowType。有些资料把复选框写成“不再显示”，方向反了，勾上它反而保证消息还会弹。

还有一处细节咱们容易忽略，nextPending 每次翻出待显消息，都会把复选框重新勾回去（第 363 行）。用户刚对消息甲取消了勾选，乙再弹出来时，复选框又是勾着的。抑制只对当时显示的消息生效，您把它理解成按条记忆就好。

## showMessage 的两道检查：入队一遍，出队一遍

咱们从单参数的 showMessage(const QString&) 看起，它只有一行身体，转调双参版本（第 380-383 行）。双参版本（第 398-406 行）开头就把消息交给 isMessageToBeShown 过目（第 343-347 行），该函数一行管三件事，消息非空，type 为空时查 doNotShow，type 非空时查 doNotShowType。任何一件不满足，函数当场返回，消息连队都排不上。过了检查的消息 push 进 pending（第 403 行），对话框不可见时 nextPending 翻出队头再显示。

出队时的复查另有用处，笔者给您推一个场景。对话框正显示消息甲，乙丙接着入队，用户看完甲时取消了勾选，甲的全文进了 doNotShow。done() 收尾时 nextPending 逐条出队（第 349-368 行），对每条再跑一遍 isMessageToBeShown（第 355 行），如果乙与甲全文相同，乙在这一步被略过，直接轮到丙。类文档把行为写得很直白，消息排队按顺序显示，被抑制的自动跳过（第 140-144 行）。空消息连头一道关都过不了（第 345 行），您传空串它什么都不做，也不报错。

待显消息的结构也有旧说法，有些资料称 QErrorMessage 内部用链表挂待显消息。Qt 6.9.1 的源码里是 std::queue（第 44 行），文件顶上的 struct Message 也只有 content 与 type 两个 QString（第 35-38 行），咱们按眼前的代码讲，链表是老版本留下的说法。

## type 参数：另一只集合，另一把钥匙

type 的语义，isMessageToBeShown 一行写尽（第 346 行）：type 为空按消息全文匹配，type 非空按 type 匹配。两只集合各管各的（第 45-46 行），互不串门。您给三条不同的网络错误挂上同一个 type，用户取消勾选其中一条，doNotShowType 里进的是那个 type 字符串，另外两条下次直接不弹。反过来全文匹配意味着“连接失败”与“连接失败。”是两把不同的钥匙，一个句号之差就是两条独立记忆，您要是给消息文案自动拼上时间戳，抑制永远追不上它。

粒度的取舍咱们当场想清楚。type 太粗，语义不相关的错误共享一把钥匙，用户按掉一个，其余全被略过，太细则退化成全文匹配。按错误类别分 type 是比较稳的落点，示例与多数工程实践都这么干。混用会不会乱，结构上放心，两只集合各存各的，互不干扰，但哪条调用走了哪只集合要您自己心里有数，工程里统一策略比两头都用好维护。

## 富文本载体：一块只读 QTextEdit

消息显示在 QErrorMessageTextView 里（第 93-113 行），它是 QTextEdit 的子类，构造即 setReadOnly(true)（第 98 行），尺寸提示 250 乘 75（第 110-113 行）。有些资料称 QErrorMessage 用 QLabel 显示消息、靠 QLabel 自动识别富文本，源码不这么写，nextPending 直接调 setHtml，行号在第 356-360 行。只有编译时定义了 QT_NO_TEXTHTMLPARSER，它才退到 setPlainText。消息一律按富文本渲染，您要是写了 <root> 这类裸片段，它会被当成标签吃掉，显示缺一块。不想吃富文本这亏，您就在组装消息时用 toHtmlEscaped() 转义后再交进来。

qtHandler 的日志路径上倒有现成范本可学，jump 把日志原文经 Qt::convertFromPlainText 转成段落再拼富文本（第 205-206 行），换行与空白它替您料理好了。

## qtHandler()：全局实例接住 qDebug 一家

静态指针 qtMessageHandler 挂在文件里（第 154 行），咱们调 qtHandler() 时，实例还不存在就当场创建，挂上退出清理例程，把窗口标题设成 applicationName，再把 jump 装成消息处理器（第 329-338 行）。

jump 的身体值得咱们逐行看（第 190-223 行）。入口是一只 qScopeGuard，把消息转发给安装之前的处理器（第 192-195 行），文档也写明会转发（第 324 行），所以装了 qtHandler 不等于丢掉控制台输出。接着是类别过滤，只有默认类别的消息才上屏（第 200-203 行），QLoggingCategory 里自定义类别的日志照旧走老路。上屏的消息被包成富文本，前缀按消息类型取自一张五项数组（第 176-185 行），从 Debug 到 Information 共五种类型，各占一项。线程也分了岔，主线程直接调 showMessage，别的线程走 QMetaObject::invokeMethod，排队送回主线程（第 212-220 行）。Fatal 之后 metFatal 置起（第 221 行），队列走完且实例正是全局实例时，done() 收尾走 exit(1)（第 311-312 行）。析构函数另有交代，全局实例销毁时处理器被还原回原样（第 276-287 行）。

生产环境用不用它，咱们想清楚再动手。类别过滤已经替您挡掉自定义类别，可默认类别里仍会有 Qt 内部的告警，弹给最终用户看多半是打扰。调试期没有控制台的场合它很顺手，发布版里把要紧的错误换成您自己调 showMessage，选择权留在手里。

## 持久化：源码里一行 QSettings 都没有

笔者把 qerrormessage.cpp 的 include 区与全文检索过，QSettings 根本没进场（第 4-25 行），两只集合随进程消亡。66 篇的 QFileDialog 是另一副做派，构造时读 QSettings、析构时写回，目录记忆由 Qt 代劳。QErrorMessage 把整件事留给咱们，公开接口咱们数得完，构造、析构、qtHandler，加上两个 showMessage（qerrormessage.h 第 21-33 行）。抑制集合读不出来，设置入口也没有。子类化也够不着，复选框藏在私有类里，done() 虽是虚函数（qerrormessage.h 第 32 行），您重写它同样拿不到勾选状态。

所以跨重启的抑制，咱们把判定做在门外，调 showMessage 之前，用自己的键查一遍 QSettings，查过了才放行。键用什么，示例给出了答案，不用消息全文，用 err_file_not_found 这样的消息 ID 当键，文案改字不影响记忆。示例同时暴露了一个诚实的简化，它在弹出后立刻标记抑制（error_message_demo.cpp 第 121-126 行），没有等用户表态，真正的产品里您会把标记动作挪到用户明确选择之后。QErrorMessage 自带的复选框在这套方案里只是摆设，抑制判定全走外层，Qt 内部的集合与您的 QSettings 各记各的，互不打扰，外层说了算。

## 图标与原生路径：QT_CONFIG(messagebox) 分支

构造里设置图标的动作，包在 QT_CONFIG(messagebox) 的条件编译里，出处是 qerrormessage.cpp 第 259-264 行，样式给的是 SP_MessageBoxInformation，信息级图标。私有类里另有原生对话框的走法（第 62-91 行），helperPrepareShow 组装 QMessageDialogOptions 时 setStandardIcon 给的却是 Critical（第 86-88 行），主文案换成 “An error occurred”（第 86 行），真消息挪进 informativeText（第 87 行），复选框状态也一并传给平台（第 89 行）。控件路径与原生路径给的图标级别不一致，您在不同平台看到的错误对话框，模样可能对不上，移植测试时值得留个心眼。模态默认值也交给平台，构造文档写明默认 window modality 依平台而定（第 229-233 行），macOS 上构造里直接设了模态，带父窗口就 WindowModal，没带就 ApplicationModal（第 240-242 行）。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QErrorMessage](https://doc.qt.io/qt-6/qerrormessage.html) -- 错误消息对话框类，showMessage 与 qtHandler 的定义处

[Qt 文档 · QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 持久化设置类，抑制列表的存放方案

[Qt 文档 · QLoggingCategory](https://doc.qt.io/qt-6/qloggingcategory.html) -- 日志类别，qtHandler 只放行默认类别

[Qt 文档 · QDialog](https://doc.qt.io/qt-6/qdialog.html) -- 对话框基类，done() 虚函数的出处

---

标准对话框家族走到收尾，下一站 [3.69 QWizard 进阶](./69-qwizard-advanced.md)。这一篇咱们看到 QErrorMessage 靠重写 QDialog 的 done() 挂住关闭时机，向导那边换成页面重写流转的虚函数，nextId 与 validatePage 承担的跳转判定在那边等咱们，同样是以虚函数拦截流程的路数。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/68-qerrormessage-advanced/，可执行文件 68-qerrormessage-advanced）。窗口里横着三枚错误按钮加一枚 Reset Suppressed Messages，底下一条状态标签。点 File Not Found Error，错误对话框弹出来，标题是 File Error，正文是两行英文。弹出用的实例来自 qtHandler()，也就是全局单例（error_message_demo.cpp 第 118-121 行）。您再点同一枚按钮，对话框不来了，状态标签提示消息已被抑制，因为示例在弹出后立刻把 err_file_not_found 标记进集合并写回 QSettings（第 121-126 行），判定全在外层。把程序关掉重开再点，对话框仍然不来，抑制列表从 QSettings 读回来了（第 21-25 行），这正是 QErrorMessage 自己做不到的那一半。点 Reset Suppressed Messages，集合清空、键删除（第 98-105 行），三枚错误按钮全部恢复弹窗。还有个小实验留给您，重置之后弹出对话框时看一眼底部的复选框，它又是勾着的，因为 nextPending 每次都把它拨回勾选，行号在 qerrormessage.cpp 第 363 行。

完成标准照旧。两只集合各记什么、抑制写在 done() 的哪几行，您说得出。showMessage 入队与出队两道检查各挡什么，您讲得清。qtHandler 放行哪些类别、转发给谁、Fatal 之后走哪一步，您答得准。跨重启的抑制为什么必须做在门外，您有源码撑腰。
