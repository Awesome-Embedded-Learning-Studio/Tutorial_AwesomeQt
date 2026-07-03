---
title: "Step 4-5：格式化/紧凑 + 文件读写"
description: "Format/Compact 解析后 toJson 写回编辑区并保光标位置；Open/Save 走 QFileDialog，UTF-8 编码读写，记上次打开目录。"
---

# Step 4-5：格式化/紧凑 + 文件读写

← [手册首页](./index.md) · 上一步 [Step 2-3 解析与树填充](./02-parse-and-validate.md) →

## Step 4：Format / Compact——解析后写回，保光标

### 目标

`Format`（Ctrl+I）把编辑区 JSON 美化成缩进，`Compact`（Ctrl+M）压成无空白紧凑。两者都先解析、再写回——**写回时光标位置不能丢**，用户在中段编辑后格式化，光标得留在原位附近。

### 提示

- 抽 `writeDocument(const QJsonDocument& doc, QJsonDocument::JsonFormat format)`：
  - 重写前记光标 `const int pos = editor_->textCursor().position();`
  - `editor_->setPlainText(QString::fromUtf8(doc.toJson(format)));`
  - 拿新光标 `QTextCursor c = editor_->textCursor();`，`c.setPosition(qMin(pos, editor_->toPlainText().size()));`——qMin 兜底格式化后文本变短、原 pos 越界
  - `editor_->setTextCursor(c);`
- `onFormat`：parse 编辑区文本，失败 return（parse 已报错），成功 `writeDocument(doc, QJsonDocument::Indented)` + `showStatusOk()`
- `onCompact`：同理，格式传 `QJsonDocument::Compact`
- 关键认知：Format/Compact 本质是「解析 → toJson 重新序列化」，所以**非法 JSON 无法格式化**（parse 直接拒绝），这是对的——不合法的文本不该被「美化」掩盖错误

### 检查点

贴一段单行紧凑 JSON，按 Ctrl+I 变成多行缩进、按 Ctrl+M 变回单行；在中段编辑后格式化，光标停在原位附近（不飞回开头）= 写回链通了。故意写非法 JSON 按 Ctrl+I，应被拒绝、状态栏报错。

> 文本编辑与光标看 [QPlainTextEdit](../../../../../beginner/03-qtwidgets/24-qplaintextedit-beginner.md)，JSON 序列化看 [JSON 与 XML 解析](../../../../../beginner/01-qtbase/16-json-xml-beginner.md)。

### 对照答案

- writeDocument（记光标 → setPlainText → qMin 兜底拉回）：`demo/json_editor_window.cpp:266-273`
- onFormat / onCompact：`demo/json_editor_window.cpp:275-291`

---

## Step 5：Open / Save——QFileDialog + UTF-8 + 记目录

### 目标

`Open`（Ctrl+O）弹文件对话框选 `.json`，读出来填进编辑区并自动 Validate；`Save`（Ctrl+S）弹保存对话框，把编辑区文本以 UTF-8 写盘。两者都**记住上次打开的目录**，下次默认从那开始。

### 提示

- 成员 `QString last_open_dir_;`，Open/Save 起始目录：`last_open_dir_.isEmpty() ? QDir::homePath() : last_open_dir_`
- `QFileDialog::getOpenFileName(this, "Open JSON", start_dir, "JSON Files (*.json);;All Files (*)")`，返回空串=用户取消，直接 return
- 选中后 `last_open_dir_ = QFileInfo(path).absolutePath();` 记目录
- 读：`QFile file(path); file.open(QIODevice::ReadOnly | QIODevice::Text);` 失败 `showStatusError("Cannot open: " + file.errorString()); return;`；`file.readAll()` 拿字节，`file.close()`，`editor_->setPlainText(QString::fromUtf8(bytes))`，最后 `onValidate()` 自动校验
- Save：`getSaveFileName` 同理记目录，`QFile file(path); file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);`
- **`getSaveFileName` 不自动补后缀**：用户输了 `data` 不带 `.json`，就真写出无后缀文件。手动 `if (QFileInfo(path).suffix().isEmpty()) path += ".json";` 补上
- **写必须用 QTextStream 并显式设编码**：`QTextStream out(&file); out.setEncoding(QStringConverter::Utf8); out << editor_->toPlainText();`——不设编码平台默认可能非 UTF-8，中文会乱码
- **close 前必须 flush**：`out.flush(); file.close();`——`QFile::close()` 不会 flush QTextStream 的缓冲，不 flush 直接 close 会截断缓冲区里没落盘的字节，文件末尾数据丢失
- `file.close()` 后 `showStatusOk()`

### 检查点

Open 选一个含中文的 `.json`，编辑区/树正常显示中文；Save 存盘后用外部编辑器（或重新 Open）确认中文不乱码；关掉重开，Open 对话框默认停在上次目录 = IO 链通了。

> 文件 IO 看 [文件与 IO](../../../../../beginner/01-qtbase/08-file-io-beginner.md)，QFileDialog 看 [对话框系统](../../../../../beginner/03-qtwidgets/06-dialog-system-beginner.md)，字符串编码看 [字符串与编码](../../../../../beginner/01-qtbase/03-string-encoding-beginner.md)。

### 对照答案

- onOpen（getOpenFileName + 记目录 + ReadOnly 读 + 自动 Validate）：`demo/json_editor_window.cpp:296-315`
- onSave（getSaveFileName + 手动补后缀 + 记目录 + WriteOnly + QTextStream Utf8）：`demo/json_editor_window.cpp:317-341`
- 手动补 `.json` 后缀（getSaveFileName 不自动补）：`demo/json_editor_window.cpp:324-325`
- 显式 UTF-8 编码：`demo/json_editor_window.cpp:335`
- close 前 flush（防 QTextStream 缓冲被截断）：`demo/json_editor_window.cpp:337`

---

恭喜，五个 Step 搓完整机 JSON 编辑器。成品导览的「[怎么读这份 code](../#怎么读这份-code)」给了一份对照阅读顺序，把成品按那条链再过一遍，把这个骨架吃透——配置文件查看器、API 响应检查器、序列化调试器都能换皮复用。卡住的地方翻 [卡住怎么办](./troubleshooting.md)。
