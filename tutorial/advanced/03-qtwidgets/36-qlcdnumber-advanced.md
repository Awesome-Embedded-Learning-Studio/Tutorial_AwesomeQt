---
title: "3.36 QLCDNumber 进阶"
description: "入门篇用 QLCDNumber 过了 display 三重载、setDigitCount、setMode 与仪表盘场景，进阶篇对着 6.9.1 源码把行为边界重新量了一遍，溢出时 digitStr 原样保留、画面停在最近一次成功显示的内容上、只发 overflow() 信号而 d->val 已换新值，display(double) 的溢出判定在 double2string 里按含小数点的字符串长度算，%g 降精度把 12.34 显示成 12.3，-99.9 这类四舍五入后指数不小于精度的值落进科学计数法判溢，smallDecimalPoint 的省位只对 display(QString) 生效、点号从字符流剥进 points 位图，int 路径的负号嵌进右对齐的前导空格，setMode 只是换 base 再拿 val 重显、intValue() 的 qRound 与显示用的 (int) 截断在 255.99 上分家，SegmentStyle 落成 fill 与 shadow 两个标志位、两个构造的默认都是 Filled 而无参构造的文档写 Outline，paintEvent 不经 QStyle，drawString 的格距在 smallDecimalPoint 下从 segLen*6/5 放宽到 segLen*7/5，drawDigit 的差分更新表是 Sinclair Spectrum 年代的思路且 newString 为真的路径无人调用，sizeHint 每格按 9 像素粗估、高固定 23，配套示例在 examples/advanced/03-qtwidgets/36-qlcdnumber-advanced/"
---

# 现代Qt开发教程（进阶篇）3.36——QLCDNumber 进阶

[3.35 QProgressBar 进阶](./35-qprogressbar-advanced.md) 把显示家族送进第三站。QProgressBar 难在把画笔全交给 QStyle，自己只把值与范围记准，QLCDNumber 正相反，paintEvent 里 QStyle 一次都没出现，格子坐标、七段多边形、明暗描边全由自己画。官方文档说它是 Qt 里最老的部分，来源能追到 Sinclair Spectrum 上的一段 BASIC 程序，出处 qlcdnumber.cpp 第 69-70 行。咱们对着源码把行为边界重新量一遍，源稿在溢出表现和 smallDecimalPoint 的占位上偏得最远，正文按源码改准。

默认值照 35 篇的样子记下。不带参数构造直接委托 QLCDNumber(5, parent)，出处第 295-298 行，init() 里落定的状态是 val 为 0、base 十进制、smallPoint 假、段样式 Filled、边框 Box 加 Raised，出处第 320-331 行，digitCount 默认 5，属性文档在第 351 行。正文引用以 qlcdnumber.cpp 为主，源稿的断言笔者逐条对着源码核过，三处推翻、几处补边界，都在下面各节里。

## 溢出时画面停在旧内容上，value() 已经换了新值

display(double) 的实现咱们原样摆出来看：

```cpp
// qlcdnumber.cpp 第 502-512 行
void QLCDNumber::display(double num)
{
    Q_D(QLCDNumber);
    d->val = num;
    bool of;
    QString s = double2string(d->val, d->base, d->ndigits, &of);
    if (of)
        emit overflow();
    else
        d->internalSetString(s);
}
```

d->val = num 排在溢出判断前面。您要是拿 value() 做联动，溢出那一刻读到的已经是塞不进去的新值，屏幕上却还挂着旧字符串。溢出分支只发 overflow() 信号，internalSetString 没被调用，digitStr 原样保留，画面停在最近一次成功显示的内容上。源稿说超出范围会 “显示全横线”，6.9.1 里找不到依据，推翻。display(int) 同款结构，出处第 539-549 行。溢出信号的文档另有说明，它从不因 setDigitCount() 而发出，出处第 108 行，调位数这个动作本身不会误报。

display(QString) 不做溢出检查，超长的字符串在 internalSetString 里被静默截断，smallPoint 关着取右端 ndigits 位，开着装满即停，出处第 685-689 与 705-706 行，overflow 信号在字符串路径上一次都不会响。您用字符串重载显示动态内容时，长度得自己把关。

## digitCount 反过来决定精度，smallDecimalPoint 的省位只认字符串

display(double) 的字符串来自 double2string，降精度的循环值得咱们盯着看：

```cpp
// qlcdnumber.cpp 第 181-189 行（double2string 内）
        int nd = ndigits;
        do {
            s = QString::asprintf("%*.*g", ndigits, nd, num);
            qsizetype i = s.indexOf(u'e');
            if (i > 0 && s[i+1]==u'+') {
                s[i] = u' ';
                s[i+1] = u'e';
            }
        } while (nd-- && (int)s.size() > ndigits);
```

咱们看 %g 的精度参数，它从 ndigits 起步，字符串超长就减一位重试，所以 digitCount 不只管放得下几个字符，还反过来决定浮点数留几位有效数字，源稿讲的双向关系是真的。降法有两条路，定点压位数，压不动就落科学计数法，“e+” 被换成空格加 e，出处第 184-188 行。

咱们拿三个值推一遍。display(12.34) 配 digitCount(4)，“12.34” 五个字符压不进四格，精度降到三位有效数字得 “12.3”，四个字符放下，不溢出，屏幕显示 12.3。display(-99.9) 配 digitCount(4) 进的是另一条支线：“-99.9” 五字符超，降到两位有效数字，四舍五入得 -100，可指数不小于精度，%g 就改走科学计数法，-100 是 1.0 乘 10 的平方，指数 2 等于精度 2，出来的是 “-1e+02”，经 e+ 替换成 “-1 e02” 六字符，出处第 184-188 行，还是放不下，判溢，画面停在旧内容上，digitCount 给 5 才放得下原样的 “-99.9”。源稿按负号加三个数字共四格的算术说它刚好放下，算式看着对上，%g 的科学计数法支线根本没让它走到那一步。display(-999.9) 配 digitCount(4) 同样溢出，%4.3g 直接给 “-1e+03”，替换成 “-1 e03” 六字符，判溢发信号。

smallDecimalPoint 的省位只对字符串路径生效。display(QString) 直进 internalSetString，smallPoint 为真时点号从字符流里剥出来记进 points 位图，槽位只数数字，出处第 690-724 行，您拿 “12.34” 走字符串重载、digitCount 给 4，就靠它省下一位。display(double) 的溢出判定发生在 double2string，那里不知道 smallPoint 的存在，判的是含小数点的字符串总长，您开了 smallDecimalPoint，digitCount 的需求一位都没省下来。源稿说开了它 display(12.34) 只需 digitCount(4)，照源码跑出来是显示 12.3，同样推翻。

int 路径的负号另有一套嵌位法。int2string 把 num 取过绝对值再格式化，负号嵌进右对齐的前导空格里，出处第 151-161 行。您按 int 重载喂 -999、digitCount 给 4，“ 999” 嵌成 “-999”，四个字符。digitCount(3) 时前导空格没了，insert 把总长顶到五，判溢。负号要多要一位，这一点成立，机制是借前导空格，跟格子算术不相干。

digitCount 自身带着钳位，大于 99 警告并压到 99，小于 0 压到 0，出处第 360-372 行。警告文案里还留着 setNumDigits 这个改名前的老 API 名，您在控制台看到它，别去找一个当前版本里不存在的函数。

## setMode 两行完成换制，intValue 与屏幕各算各的

咱们把 setMode 的实现看全，一共两行，d->base = m，接着 display(d->val)，出处第 470-475 行。base 换掉，拿存的 val 重显一遍，val 没动，您切完 Hex 再读 value()，报的还是 255.0。格式化交给 int2string，十六、十、八进制分别走 %*x、%*i、%*o，二进制分支自己拿缓冲区从低位往高位拼，出处第 122-150 行。

非十进制下的 double 要过一道 int 域检查，num 超出 int 范围直接判溢返回空串，出处第 172-179 行，检查过了用 (int) 强转截断，出处第 179 行。intValue() 的取整方式跟这里不一致，它返回 qRound(d->val)，四舍五入，出处第 527-531 行。255.99 在 Hex 模式下屏幕显示 FF，截断成 255，intValue() 却报 256。两处各算各的，您在联动代码里别混用。

display(QString) 喂进来的值走另一套语义，s.toDouble(&ok) 转得动就存，转不动存 0，出处第 565-574 行。您拿 “0xFF” 走 display(QString)，digitStr 里存的是原文，接着调 setMode，重显的是 val 里的 0，字符串画面被数值化显示顶掉。文档对这个重载写着 `disregards mode() and smallDecimalPoint()`，出处第 555-556 行，前半句与实现一致，后半句要打折扣，布局照样受 smallPoint 影响，点号照旧剥进位图。查溢出有现成的 checkOverflow 两重载，内部把 int2string 或 double2string 预演一遍看长度，出处第 427-449 行，您显示大数之前可以拿它探路。

## SegmentStyle 落成两个标志位，默认值以代码为准

setSegmentStyle 的实现只有两行赋值，咱们直接看，d->fill 在 Flat 或 Filled 时为真，d->shadow 在 Outline 或 Filled 时为真，出处第 1142-1148 行。三档样式落成两个位的组合，Outline 只描边，Filled 填充加描边，Flat 只填充。绘制侧 drawSegment 里，fill 位管拿 fgColor 填多边形，出处第 863-987 行，shadow 位管拿 palette 的 light 与 dark 画明暗边线，出处第 999 行起，取色集中在第 846-856 行。

Outline 因此是只描边不填充的空心段，段内露出控件背景。QSS 的 color 改的是 foregroundRole，只影响填充，对 Outline 的可见度帮不上忙，深背景上想看清轮廓得动 palette 的 light 与 dark。您配深色工业风的仪表盘，Filled 或 Flat 配亮色 color 才走得通，示例第一区三只并排的 LCD 就是这么配的，出处 segment_style_demo.cpp 第 73-75 行。

默认值有个文档与实现不一致的地方。无参构造的文档说段样式设成 Outline，出处第 288 行，可无参构造实际委托 QLCDNumber(5, parent)，出处第 295-298 行，init() 里设的是 Filled，出处第 329 行。您不带参数 new 出来的 LCD 是 Filled 的，文档那句当不了 6.9.1 的依据。

## paintEvent 里没有 QStyle，段是从多边形画起的

咱们从 paintEvent 顺着往下走。起手画边框、开抗锯齿，shadow 位为真时把画笔平移半个像素，一像素宽的描边线才落在像素中心，出处第 661-674 行。往下 drawString 算格子，格距公式里装着 smallDecimalPoint 的全部布局影响，digitSpace 在 smallPoint 时取 2 否则取 1，每格步进是 segLen*(5+digitSpace)/5，出处第 745-750 行。开了 smallDecimalPoint，整排格距加宽，每格步进从 segLen*6/5 放宽到 segLen*7/5，段长上限跟着变小，单个数字并不挪位。属性文档也只说数字之间的空隙变宽，出处第 636-637 行。

小数点自己是段号 7，smallPoint 时画在两格之间的下方，否则画在本格底部中央，出处第 947-950 行。冒号是段 8 与 9 的上下两点。全部字形排在 getSegments 的表里，三十项，数字之外还有 A 到 F、h、H、L、o、P、r、u、U、Y、度数符号和空格，非法字符落到空表，画出来是空格，出处第 197-281 行。您想让 LCD 显示 “HELLO” 没问题，H、E、L、O 都在表里，想显示 “W” 就只剩一排空格。

drawDigit 收新旧两个字符，旧字符的段标成擦除、新字符的段标成绘制、重合的标成不动，只处理有变化的段，出处第 782-824 行。这套增量更新是省重绘年代的思路。可 paintEvent 调 drawString 时把 newString 显式传了 false，出处第 670-673 行，旧字符一律按空格算，差分逻辑空转，每次全量重画。笔者把 qlcdnumber.cpp 里 drawString 的调用者数了一遍，newString 为真的路径当前没有任何人走，差分表留在源码里没有调用者。

sizeHint 宽按每格 9 像素乘位数再加 10，smallDecimalPoint 关着时位数还要加一给点号，高固定 23，出处第 1164-1167 行。每格 9 像素是粗估，您要精确排版别指望它，自己 setMinimumSize。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QLCDNumber 在 QtWidgets，自绘话题涉及的 QPainter 在 QtGui，链接 Qt6::Widgets 即可。

[Qt 文档 · QLCDNumber](https://doc.qt.io/qt-6/qlcdnumber.html) -- 液晶数字显示控件，digitCount、mode、segmentStyle、smallDecimalPoint 属性说明

[Qt 文档 · QFrame](https://doc.qt.io/qt-6/qframe.html) -- QLCDNumber 的父类，frameShape 与 frameShadow 影响边框外观

[Qt 文档 · QPalette](https://doc.qt.io/qt-6/qpalette.html) -- 调色板，light 与 dark 角色决定 Outline 样式的描边色

[Qt 文档 · QPainter](https://doc.qt.io/qt-6/qpainter.html) -- 绘图引擎，drawPolygon 画段与 translate 半像素平移

[Qt 文档 · QBitArray](https://doc.qt.io/qt-6/qbitarray.html) -- 位图容器，smallDecimalPoint 模式下点号的存储

---

数码管到这就收住，显示家族的下一程换日期方向，等咱们跟 QCalendarWidget 把整月日历翻起来，入口在 [3.37 QCalendarWidget 进阶](./37-qcalendarwidget-advanced.md)。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/36-qlcdnumber-advanced/，可执行文件 36-qlcdnumber-advanced）。第一区三种风格并排，同一数值喂三只 LCD。三只依次设成 Outline、Filled、Flat，出处 segment_style_demo.cpp 第 63、73-75、85-87 行。第二区管溢出，digitCount 与显示值都做成旋钮，默认 4 位配 12.34，出处第 127、137 行，checkOverflow 里设好位数再 display，出处第 251-252 行。第三区切进制，按进制换 digitCount，十进制 5 位、十六进制 4 位、八进制 6 位、二进制 16 位，出处第 287-308 行，设好 digitCount 再 setMode 再 display 的次序在第 311-313 行，下方标签实时报 value()，出处第 316-320 行。

实验补两步，都在第二区。您第一步把值敲成 12.00、digitCount 留 4：提示标签说需要五个槽位，屏幕却显示 12。标签用 QString::number(value, 'f', 2) 按两位小数算长度，出处 segment_style_demo.cpp 第 255-256 行，真实路径是 %g 把 12.00 压成 “12”，标签在 .00 结尾的值上会误报。接着敲 -99.9、digitCount 仍留 4，这一回真溢出，屏幕停在旧内容 12 上，提示标签报文本 -99.90 需要 6 个槽位，数字比真实需求多算一位，但溢出与画面停旧内容这次报对了，巧在 %g 的科学计数法支线，四舍五入出的 -100 被支线送进科学计数法，落不到屏幕上。第二步把值敲成 99999、digitCount 留 2，触发真溢出，画面停在旧内容上，overflow 信号发了、屏幕没换，出处 qlcdnumber.cpp 第 508-511 行。

完成标准照旧。溢出时画面停在什么上、value() 报什么，您说得出来。display(12.34) 配 digitCount(4)，屏幕出什么、-99.9 出什么，您不跑程序就能推。setMode 之后 value() 变不变，intValue() 与屏幕在 255.99 上差在哪，您指得出两处行号。三种 SegmentStyle 各对应 fill 与 shadow 的哪种组合，您答得上来。
