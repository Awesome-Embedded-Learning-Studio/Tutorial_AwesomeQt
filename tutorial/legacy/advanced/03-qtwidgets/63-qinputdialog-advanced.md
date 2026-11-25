---
title: "3.63 QInputDialog 进阶"
description: "进阶篇拆 QInputDialog 的静态函数族。五只静态函数取消时返回值各有脾气（getText 回空 QString 第 1152-1156 行，getInt 回初值第 1245-1249 行），getItem 的 editable 默认是 true（qinputdialog.h 第 135 行），内部五只输入控件懒创建、setInputWidget 换位（第 174-183 行与第 320-359 行），数字模式 Enter 被拦、OK 键跟随合法性变灰（第 86-93 行与第 316 行），setValue 一律被 bound() 夹紧所以值在范围前设置会丢初值（qabstractspinbox.cpp 第 1760 行），setTextValue 顺手把模式切回 TextInput（第 661 行），UseListViewForComboBoxItems 只对不可编辑下拉生效（第 366 行），open() 按槽签名自动挑信号（第 46-56 行），示例在 examples/advanced/03-qtwidgets/63-qinputdialog-advanced/"
---

# 现代Qt开发教程（进阶篇）3.63——QInputDialog 进阶

62 篇咱们读过消息框的静态便捷函数族，QMessageBox::question 一行问个是非。输入对话框把同一套思路推到头，“问一句拿一个值”打包成了五只静态函数，排在头文件里（qinputdialog.h 第 125-146 行），getText 管文本，getInt、getDouble 管数字，getItem 管选择，getMultiLineText 管多行，连返回类型都替您分好了。本篇咱们把每只静态函数进门之后对话框内部做的事看个遍，再把笔者在 Qt 6.9.1 的 QInputDialog 源码（qinputdialog.cpp）里核出的几处与直觉相反的行为摆开。

引用文件咱们交代清楚。主战场是 qinputdialog.cpp 与 qinputdialog.h（src/widgets/dialogs），数字输入那头的夹紧算法下探到 qspinbox.cpp 与 qabstractspinbox.cpp（src/widgets/widgets）。配套示例是四只按钮各开一种输入模式的演示窗口，完整玩法放结语。

## 五只静态函数取消时回什么，各有各的脾气

有一件最容易想当然的事，咱们当场定下来。用户点了 Cancel，函数返回什么？答案不统一。getText 与 getMultiLineText 回一个空 QString，出处第 1152-1156 行。getInt 回初值，也就是调用方传进来的默认数，出处第 1245-1249 行。getDouble 同款回初值（第 1294-1298 行），getItem 回打开时选中的条目（第 1350 行）。这就带来一个共同点，用户取消与用户原样确认初值，回的东西一模一样，所以判断到底确认没有，ok 指针是唯一靠得住的凭据。另有一处小分寸，getItem 的 current 传越界了，items.value(current) 回空串（第 1334 行），对话框还能正常开，只是默认什么都不选。

五只函数的骨架一模一样。咱们看 getText 的实现（第 1138-1157 行），QAutoPointer 抱着一只新构造的 QInputDialog，设好标题、标签与默认值，exec 跑一圈按返回码回值，对话框随后自动销毁。静态函数没有藏魔法，它做的事您手动构造全做得出来，还能做得更细。

签名咱们逐个参数过一遍，有两处值得停。getDouble 的 decimals 默认值是 1，出处 qinputdialog.h 第 145 行。您要输两位小数，比如 3.14，decimals 得显式传成 2，默认只给到一位。更别扭的是 step，它挤在参数表末尾、排在 flags 后面（第 146 行），想给 getDouble 调步长，中间的 ok 与 flags 得一齐显式给出，参数摆在这个位置是历史演进留下的。getItem 那边，editable 的默认值是 true（第 135 行）。笔者手头的旧笔记就把它记成了 false，核对头文件才改过来。您要的效果是只能从列表里选，false 必须自己传。getText 的签名里另带了 echo 模式（第 126 行），拿密码的场景一行就够。

## setInputMode：五只输入控件按需创建，换模式只是一次布局换位

InputMode 枚举只有三个值，TextInput、IntInput、DoubleInput，出处 qinputdialog.h 第 51-54 行。那列表选择算什么模式？咱们看私有实现的判据，useComboBoxOrListView() 只问一句，combo 里有没有条目，出处 qinputdialog.cpp 第 169 行。您调 setComboBoxItems 塞进条目，chooseRightTextInputWidget 就把输入件换成下拉框或列表（第 744-745 行），inputMode() 报回来的仍是 TextInput。列表选择没有独立枚举，它是 TextInput 模式的一个分支。

私有类里的输入控件咱们挨个数，lineEdit、plainTextEdit、intSpinBox、doubleSpinBox、comboBox，外加一只 listView（第 174-183 行），出生时全是 nullptr（第 190-194 行），您用到哪只，内部才造哪只。布局在 ensureLayout 里搭起来（第 196-226 行），标签默认文案 “Enter a value:”（第 209 行），按钮排是 Ok 加 Cancel 的 QDialogButtonBox（第 215 行），输入件的位置默认由 lineEdit 占着（第 203-206 行）。

换模式的事，咱们跟 setInputWidget 走一遍（第 320-359 行）。布局里旧的输入件摘下来藏好，新的插进同一格亮出来，textValue 同步过去。您调 setInputMode(IntInput)（第 509-537 行），出现的是带校验的子类 QInputDialogSpinBox（第 66 行），QDoubleSpinBox 那边同样有子类接上，出处第 101 行。子类多做了什么，正是下一节的主角。

## 数字模式：Enter 会被拦下，OK 键跟着合法性变灰

QInputDialogSpinBox 给 QSpinBox 补了一层校验逻辑，类注释把意图写在了明处（第 60-65 行）。咱们看 keyPressEvent（第 86-93 行），用户按 Return 或 Enter 时，若当前输入不合法，事件不再往下传，一句 `setProperty("value", property("value"))` 把显示值拉回上一个合法值。您在整数框里手输越界数字再敲回车，看到数字被顶回去，就是这一句在干活。

第二个机关管 OK 键。子类把 textChanged(bool) 信号连到按钮排的 OK 上（第 312-318 行），输入不合法 OK 当场变灰，合法了再亮回来。您留意这套联动只接在两只 spinbox 上，lineEdit 没份，所以文本模式里 OK 永远亮着，合法性得您自己用 validator 或事后校验把关。

确认那一下还有一层讲究。done(int result)（第 1086-1110 行）里，三种带 Selected 后缀的信号只在 result 非零时发出，用户确认了才发，取消不发。Changed 与 Selected 两族信号的分工在这儿划清，咱们做非模态用法时，收尾靠 Selected 最稳。

## 范围与精度：setValue 一律被夹紧，小心的是调用顺序

有一种流传的说法，程序里 setIntValue 塞一个越界值，对话框会原样显示。笔者拿源码核了一遍，不成立。setIntValue 转给 QSpinBox 的 setValue，出处 qinputdialog.cpp 第 768-773 行。干活的一句在 QAbstractSpinBoxPrivate::setValue 里，头一句就是 value = bound(val)，咱们在 qabstractspinbox.cpp 第 1755-1761 行核对过，越界值在入口被夹到范围内。

顺序误判在另一处。QSpinBox 出生范围是 0 到 99（qspinbox.cpp 第 1023-1027 行），您手动构造时把 setIntValue(8080) 排在 setIntRange(1024, 65535) 前面，8080 在前一步就被夹成 99，范围后放宽也回不来，面板上显示的是 99。getInt 内部把顺序做对了，范围在前、值在后（qinputdialog.cpp 第 1238-1239 行），代价是您给静态函数传的初值越了界，会被悄悄夹到边界，函数不给任何提示。

精度那头，setDoubleDecimals 直通 QDoubleSpinBox 的 setDecimals，出处第 960-965 行。decimals 是显示与取值共用的精度，设 2 的话 value() 回的就是四舍五入到两位的数，qspinbox.cpp 第 573 行的文档注释拿 setValue(2.555) 举例，回 2.56。您要收更高精度的值，decimals 得开大，没有显示两位、内部存全精度这回事。

属性函数还有个顺手的小动作容易绊人。setTextValue 进门头一件事就是 setInputMode(TextInput)，出处第 657-671 行，setIntValue 与 setDoubleValue 也各自切回自己的模式，出处第 771、879 行。您前一句 setIntValue(8080)，后一句 setTextValue 给文本，模式已经被切回 TextInput，整数面板根本没上过场。咱们排代码时把模式类调用放在最前面，这些小动作就影响不到结果。

## TextInput 的定制：validator 走 findChild，看清它摸到的是谁

静态 getText 暴露的参数里有 echo 模式与输入法提示，没有 validator 的位置。想要正则校验，您得手动构造，再自己摸到内部的 QLineEdit。示例走的正是同一条路（examples/advanced/03-qtwidgets/63-qinputdialog-advanced/，input_dialog_demo.cpp 的 openTextDialog），设好模式与默认值之后，findChild<QLineEdit*>() 把验证器挂上去。

findChild 的一步您得知道它在摸谁。ensureLayout 保证对话框自家的 lineEdit 总是最早出生（第 203-206 行），所以哪怕当前是别的模式在场，findChild<QLineEdit*>() 摸到的也是藏起来的自家 lineEdit，验证器设上去不起作用。纯文本模式下没这个问题，但拿 findChild 摸内部控件，Qt 没给跨版本承诺，笔者的底线是能用属性接口就不摸内部，前缀后缀这类需求，直接继承 QDialog 自己搭面板更踏实。

多行文本咱们顺带收进来。UsePlainTextEditForTextInput 选项，出处 qinputdialog.h 第 46 行，把输入件换成 QPlainTextEdit，getMultiLineText 的实现靠的就是设它，出处 qinputdialog.cpp 第 1190-1191 行，没有为多行单开模式。选项集里另有一只 NoButtons，文档注明可以拿来做“活对话框”（第 582 行），按钮排藏起来，值一变就发 Changed 信号，配后文的 open 正好成套。

## getItem：可编辑下拉与列表视图，谁上场由一处分支决定

下拉与列表共用一份数据。咱们看 ensureListView，造列表时直接 setModel(comboBox->model())，出处第 282 行，单选、禁编辑（第 280-281 行）。您不选任何条目，OK 是灰的（第 356-357 行），选中一行后，currentRowChanged 把 OK 点亮，出处第 438-443 行。

谁上场，chooseRightTextInputWidget 里一个分支说了算，出处第 361-387 行。第 366 行的条件咱们原样看，UseListViewForComboBoxItems 开着且下拉框不可编辑，才轮到列表上场，否则一律下拉框。两样同时设置时，赢的是可编辑下拉框，被跳过的是列表视图。文档把行为写明白了，枚举说明原文就是 “rather than a non-editable QComboBox”（第 583-584 行），列表视图天生只服务不可编辑的场景。

可编辑模式还带出返回值上的一处细节。setComboBoxText 找不到列表里的对应项时，只有可编辑才 setEditText（第 389-397 行），用户敲进来的自定义文本原样进了 textValue。您拿返回值去 indexOf 查索引，列表外的值查回来是 -1，后续拿 -1 当索引用就出事了。咱们老实用字符串本身，或者查到 -1 就单走一条自定义分支。

## open()：按槽函数签名自动挑信号，关窗自动断连

静态函数用 exec 堵着等结果，QInputDialog 另备了一条非模态的路。open(receiver, member)（第 1028-1035 行）连接前要拿 signalForMember 挑信号（第 46-56 行），槽的第一个参数是 QString 就接 textValueSelected，是 int 接 intValueSelected，是 double 接 doubleValueSelected，没有参数就接 accepted。对话框关闭时，done 会自动拆掉连接（第 1104-1109 行）。咱们做工具栏上点一下弹窗改一个值的交互，挑信号与断连两件手工活，全都省了。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QInputDialog](https://doc.qt.io/qt-6/qinputdialog.html) -- 输入对话框类，五只静态函数与属性接口的定义处

[Qt 文档 · QValidator](https://doc.qt.io/qt-6/qvalidator.html) -- 输入校验基类，三种校验状态的定义

[Qt 文档 · QRegularExpressionValidator](https://doc.qt.io/qt-6/qregularexpressionvalidator.html) -- 正则校验器，示例文本面板用的验证器

[Qt 文档 · QSpinBox](https://doc.qt.io/qt-6/qspinbox.html) -- 整数输入框，范围与步长接口

[Qt 文档 · QDoubleSpinBox](https://doc.qt.io/qt-6/qdoublespinbox.html) -- 浮点输入框，decimals 精度语义

---

标准对话框的取值部分到这里收队，下一站 [3.64 QColorDialog 进阶](./64-qcolordialog-advanced.md)，从取值转入取色，静态函数族与自定义选项在那边等咱们。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/63-qinputdialog-advanced/，可执行文件 63-qinputdialog-advanced）。第一只按钮开邮箱面板，咱们试着敲一个空格，输入框根本不收，验证器把非法字符拦在按键这一层，把地址删到只剩半截再点 OK，半截照样进了结果标签，文本模式没有 OK 联动。第二只按钮是静态 getInt 的端口面板，范围 1024 到 65535，手输 80，OK 当场变灰，敲回车数字被顶回上一个合法值。第三只是对象式构造的浮点面板，范围 -273.15 到 1000，精度两位，输 -300 敲回车，看着它被顶回 36.5。第四只列表面板给 getItem 传了 false，您想打字也打不进去，只能从五个模块里选，把 false 改成 true 重编，自定义模块名敲得进去，indexOf 查不到的细节当场能试。再动一处代码，把 openTextDialog 里的 setInputMode 那行注释掉，面板一模一样，因为 setTextValue 进门就替您把模式切好了。

完成标准照旧。五只静态函数取消时各回什么，您说得出。getInt 内部范围在前还是值在前，顺序反了会怎样，您讲得明。数字模式下 OK 键的亮灰由哪根连接决定，您数得清。可编辑与列表视图同时设置时谁上场，判定条件在哪一行，您答得上。
