---
title: "卡住怎么办"
description: "按症状查：编译报 QFontDatabase、等宽字体不生效、Info 行某主题下不可读、裁旧行数不对、append 不滚底、新行黏一起、setMaxLines 调小不生效——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/log-viewer/`，对照着看。

## cpp 编译报 `expected type-specifier before 'QFontDatabase'` / `'QFontDatabase' has not been declared`

- 构造里用了 `QFontDatabase::systemFont(...)` 设等宽字体，但 cpp 顶部**漏引 `<QFontDatabase>`**？Q_OBJECT 类的 cpp 不像头文件那样顺手引字体类。→ 顶部补 `#include <QFontDatabase>`，对照 `src/log_viewer.cpp:9`
- 是不是误把 `QFontDatabase` 当成只读 `.h` 里需要的，忘在 cpp 引？设字体的代码在 cpp，就得 cpp 引。

## 等宽字体没生效，日志列还是参差不齐

- 是不是用了 `QFont("Monospace")` 这种按名字取字体的写法？**系统上未必有这个名字的字体**。改用 `QFontDatabase::systemFont(QFontDatabase::FixedFont)` 取系统首选等宽字体。→ `src/log_viewer.cpp:23`
- 是不是干脆没设字体？默认比例字体下时间戳 / 级别列宽不一，看着乱。
- 进阶排查：[QFontDatabase](https://doc.qt.io/qt-6/qfontdatabase.html)

## Info 行在深色主题下看不见 / 浅色主题下太刺眼

- `colorForLevel` 是不是给 Info 也**硬编码了黑或白**？这会在某个主题下不可读。让 Info 返回默认构造的 `QColor()`（`isValid()==false`），`append` 里只对有效色 `setForeground`，Info 行用 view 默认前景色跟着主题走。→ `src/log_viewer.cpp:129` / `src/log_viewer.cpp:47`
- 主题切换不熟？查 QPalette / `QGuiApplication::palette()`。

## 裁旧行数对不上（删多了 / 删不干净 / 行数显示漂移）

- 是不是**自己维护了一个行计数器**？clear 之后没清零、或和 document 实际块数漂移就会出错。改用 `view_->blockCount()` 当唯一真相源。→ `src/log_viewer.cpp:155`
- 删除循环是不是漏了把行尾 `\n` 带进选区？只 `movePosition(NextBlock, KeepAnchor)` 不够，还得 `movePosition(NextCharacter, KeepAnchor)` 带上换行，否则删不干净。→ `src/log_viewer.cpp:163-169`

## append 后没滚到底，最新行看不到

- `autoScroll` 默认开，但滚底代码**是不是只调了 `moveCursor(End)` 没调 `ensureCursorVisible()`**？前者挪光标、后者才让视口跟着滚，两件套缺一不可。→ `src/log_viewer.cpp:60-61`
- 是不是在裁旧之前滚的底？得先裁后滚，滚到的才是最终末尾。→ `src/log_viewer.cpp:56` 裁旧在 `src/log_viewer.cpp:58` 滚底之前
- 进阶排查：[QPlainTextEdit](https://doc.qt.io/qt-6/qplaintextedit.html)

## 新行没独立成块，裁旧时把两行黏一起删

- insertText 前**是不是没补 `\n`**？文末非空时直接 insertText，新行会和旧行黏在一块，按块裁旧就错乱。判断 `!cursor.atStart()` 时先补一个 `\n`。→ `src/log_viewer.cpp:51`
- 这步和 step 6 裁旧直接挂钩：`\n` 保证每条日志独立成块，按块删才干净。

## `setMaxLines` 调小后旧行没被裁，要等下次 append

- setter **是不是只改了 `max_lines_`、emit 了信号，没主动裁旧**？不裁旧行就残留在 document 里，控件状态不自洽。clamp 完、emit 前先调一次 `trimOldBlocks()`。→ `src/log_viewer.cpp:93`
- 进阶排查：[属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)

## moc 报错（Q_PROPERTY / Q_ENUM 不认识）

- 头文件**有没有 `Q_OBJECT`**？→ `include/log_viewer.h:26`
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- Q_ENUM 的 Level 枚举**是不是在类里**、Q_ENUM 紧跟其后？→ `include/log_viewer.h:36-37`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
