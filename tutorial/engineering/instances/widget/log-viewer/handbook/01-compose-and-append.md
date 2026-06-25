---
title: "Step 1-3：组合骨架 + append 染色 + 时间戳"
description: "把 QPlainTextEdit 组合进 LogViewer（只读 / 等宽 / NoWrap），加 Level 枚举，用 QTextCursor 按级别套前景色 insertText，补时间戳前缀与便捷重载。"
---

# Step 1-3：组合骨架 + append 染色 + 时间戳

← [手册首页](./index.md) · 下一步 [Step 4-5 属性化 + 自动滚底](./02-autoscroll-and-trim.md) →

## Step 1：LogViewer 骨架（组合 QPlainTextEdit）

### 目标

做一个 `AwesomeQt::LogViewer : public QWidget`，内部持有一个只读 QPlainTextEdit，弹出来是一只等宽字体、不换行的只读文本框。这一步还不要染色，先让骨架立住。

### 提示

- 继承 `QWidget`，成员 `QPlainTextEdit* view_`
- 构造里 `view_ = new QPlainTextEdit(this)`——parent 设 this，让对象树托管生命周期
- `view_->setReadOnly(true)`；`view_->setLineWrapMode(QPlainTextEdit::NoWrap)`——日志一行一行往下堆，不要自动换行
- 等宽字体用 `QFontDatabase::systemFont(QFontDatabase::FixedFont)`，别按名字 `QFont("Monospace")` 取（系统不一定有）。引 `<QFontDatabase>` 别忘了
- 一个 `QVBoxLayout(this)`，`setContentsMargins(0,0,0,0)`，`addWidget(view_)`

### 检查点

把 LogViewer 放进窗口 show 出来：一只只读、等宽、不换行的文本框，光标进去改不了字 = 骨架对了。

> 自定义控件 / 对象树不熟？[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)、[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)。

### 对照答案

- 构造组合 view：`src/log_viewer.cpp:18`
- 等宽字体 + NoWrap：`src/log_viewer.cpp:23` / `src/log_viewer.cpp:24`

---

## Step 2：Level 枚举 + append 染色

### 目标

定义 `enum class Level { Info, Warning, Error }`，加 `Q_ENUM(Level)`。实现 `void append(Level level, const QString& message)`，每条 append 时按级别套前景色：Info 用默认前景色、Warning 暗黄、Error 红。

### 提示

- 枚举放类里，紧跟 `Q_ENUM(Level)`——moc 要认得这个枚举（后面 Q_PROPERTY / 外部信号槽会用到）
- 写私有 `QColor colorForLevel(Level) const`：Info 返回一个默认构造的 `QColor()`（`isValid()` 为 false，表示「用默认」），Warning 返回 `QColor(180,120,0)`，Error 返回 `QColor(200,0,0)`
- `append` 里：`QTextCursor cursor(view_->document()); cursor.movePosition(QTextCursor::End);`
- 取色后**只对有效色** `QTextCharFormat::setForeground`，Info 不调 setForeground——这样 Info 行跟着 view 默认前景色走，深浅主题都不出错
- `cursor.insertText(line, fmt)` 一步把文字和颜色写进 document

关键认知——**为什么 Info 要返回无效色**：硬编码黑或白都会在某个主题下不可读。让 Info 不设格式、用 view 默认前景色，控件在任何配色下都不崩。

### 检查点

调 `append(Level::Warning, "配置缺失")` 后看到一行暗黄字；`append(Level::Error, "连接失败")` 看到一行红字；`append(Level::Info, "启动完成")` 看到一行默认前景色字。切换系统深浅主题，Info 行始终可读 = 着色对了。

> 富文本操纵不熟？[QPlainTextEdit 入门](../../../../../beginner/03-qtwidgets/24-qplaintextedit-beginner.md)、[属性系统深度拆解](../../../../../advanced/01-qtbase/01-qobject-property-system-advanced.md)。

### 对照答案

- Level 枚举 + Q_ENUM：`include/log_viewer.h:36-37`
- colorForLevel 三态色：`src/log_viewer.cpp:129`
- append 着色插入（只对有效色 setForeground）：`src/log_viewer.cpp:45-54`

---

## Step 3：时间戳前缀 + 便捷重载

### 目标

每条 append 的行格式是 `[HH:MM:SS] [LEVEL] message`，时间戳用 `QTime::currentTime().toString("HH:mm:ss")`。再加三个便捷重载 `appendInfo / appendWarning / appendError`，省得每次手写 `Level::`。时间戳可关（这一步先写死开，开关留到 step 4 升级 Q_PROPERTY）。

### 提示

- 拼行：`QString line;` 先按开关决定要不要加 `[HH:mm:ss]` + 空格前缀（这一步先恒加），再拼 `[LEVEL] message`
- 写一个静态 `levelLabel(Level)`：Info→`INFO`、Warning→`WARN`、Error→`ERROR`
- 文末非空时先补一个 `\n`：`if (!cursor.atStart()) cursor.insertText("\n");`——保证新行独立成块，和后面 step 6 的「按块裁旧」直接挂钩
- `appendInfo` 等就是一行 `append(Level::Info, message)` 的转发

### 检查点

连续 append 三条不同级别，每行开头都有正确的时间戳和级别标签，三行换行分明不黏一起。空 message 也能 append（只显示时间戳 + 级别）= 格式对了。

> QString / 时间格式化不熟？[QString 进阶](../../../../../advanced/01-qtbase/03-qstring-advanced.md)。

### 对照答案

- 拼行 + 时间戳 + 级别标签：`src/log_viewer.cpp:34-39`
- 文末补 `\n`：`src/log_viewer.cpp:51`
- 便捷重载转发：`src/log_viewer.cpp:65-75`

---

下一步：[Step 4-5 把行为开关升级成 Q_PROPERTY + 自动滚底](./02-autoscroll-and-trim.md)。
