---
title: "卡住怎么办"
description: "按症状查：校验没反应、报错没 offset、树不出现/不递归、整数带 .0、Format 光标飞回开头、保存中文乱码、构造崩——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `app/01-dev-tools/json-editor/`，对照着看。

## 点 Validate 没反应 / 树不更新

- onValidate 里**有没有调 populateTree**？parse 成功后只 showStatusOk 不填充，树就空。→ `demo/json_editor_window.cpp:162-171`
- populateTree 有没有 `tree_->clear()` 在前、`addTopLevelItem(root)` 在后？漏了根，递归没起点。→ `demo/json_editor_window.cpp:176-203`
- 进阶排查：[QTreeWidget](../../../../../beginner/03-qtwidgets/48-qtreewidget-beginner.md)

## 报错只说「非法 token」，找不到在哪

- parse 拼 `errorString` 时**有没有带 `QJsonParseError::offset`**？offset 是 UTF-8 字节偏移（含中文时与编辑器字符列不等），定位语法错误就靠它——状态栏标 `@byte offset` 区分字符列。→ `demo/json_editor_window.cpp:151-152`
- 进阶排查：[JSON 与 XML 解析](../../../../../beginner/01-qtbase/16-json-xml-beginner.md)

## 树只填了顶层，没递归展开

- fillValue 的 Object/Array 分支里**有没有继续递归调 fillValue**？只 `addChild` 不递归就只铺一层。→ `demo/json_editor_window.cpp:209-228`
- Object/Array 分支**漏了 break**？switch 穿透到下一个 case，节点构造乱了。→ `demo/json_editor_window.cpp:216`、`:227`
- Object/Array 分支是不是**先建 node 再用 node 当 parent 递归**？直接用 parent 递归会层级错乱。→ `demo/json_editor_window.cpp:209-215`

## 整数在树里显示成 `1.0` 或 `42.0`

- Double 分支**有没有做整数判定**？Qt JSON 数字统一是 double，`QString::number(1.0)` 会给 `1.0`。→ `demo/json_editor_window.cpp:233-242`
- 判定用精确比较 `std::isfinite(d) && d == std::floor(d)`（别用 `qFuzzyCompare`——对 \|d\|>=1e12 量级会失效，带小数的大数被误判整数），是整数转 `qint64` 再 `QString::number`。→ `demo/json_editor_window.cpp:239-241`
- 进阶排查：[变体与类型系统](../../../../../beginner/01-qtbase/05-variant-type-beginner.md)

## Format / Compact 后光标飞回文档开头

- writeDocument 重写文本前**有没有记光标 position**？重写后用 `qMin(原位, 新长度)` 拉回。→ `demo/json_editor_window.cpp:268-272`
- 拉回时**有没有 qMin 兜底**？Compact 后文本变短，原 pos 可能越界。→ `demo/json_editor_window.cpp:271`
- 进阶排查：[QPlainTextEdit](../../../../../beginner/03-qtwidgets/24-qplaintextedit-beginner.md)

## 保存后中文变乱码（重开读不回来）

- Save 用 QTextStream 写时**有没有显式 setEncoding(Utf8)**？平台默认编码可能非 UTF-8。→ `demo/json_editor_window.cpp:335`
- 读的时候用的是 `QString::fromUtf8(bytes)` 而不是 `QString(bytes)`？后者也依赖默认编码。→ `demo/json_editor_window.cpp:313`
- 进阶排查：[字符串与编码](../../../../../beginner/01-qtbase/03-string-encoding-beginner.md)

## 保存的文件末尾数据缺失 / 截断（重开解析失败）

- Save 在 `file.close()` 前**有没有 `out.flush()`**？`QFile::close()` 不 flush QTextStream 的缓冲，缓冲区里还没落盘的字节会被截断丢弃。→ `demo/json_editor_window.cpp:337`
- 进阶排查：[文件与 IO](../../../../../beginner/01-qtbase/08-file-io-beginner.md)

## 保存的文件没有 `.json` 后缀

- `getSaveFileName` **不自动补后缀**，用户输 `data` 就真写出无后缀文件。手动 `if (QFileInfo(path).suffix().isEmpty()) path += ".json";` 补。→ `demo/json_editor_window.cpp:324-325`
- 进阶排查：[对话框系统](../../../../../beginner/03-qtwidgets/06-dialog-system-beginner.md)

## 构造函数里崩 / actions 的槽解引用空指针

- 构造顺序**是不是 setupCentral 在 setupActions 之前**？先建 editor_/tree_，actions 的槽才能安全解引用它们。→ `demo/json_editor_window.cpp:42-46`
- 进阶排查：[QMainWindow 主窗口](../../../../../beginner/03-qtwidgets/55-qmainwindow-beginner.md)

## 想实时校验，挂 textChanged 后大文件卡顿

- 不要每按一键就 parse + 重建整棵树。要么走显式 Validate（决策②），要么加 300ms 防抖（进阶挑战）。→ 对比 `demo/json_editor_window.cpp:162` 的 onValidate 是按钮触发
- 进阶排查：[事件系统](../../../../../beginner/01-qtbase/07-event-system-beginner.md)、[定时器](../../../../../beginner/01-qtbase/11-timer-beginner.md)

## moc 报错（Q_OBJECT / 信号槽不认）

- JsonEditorWindow 头里**有没有 Q_OBJECT**？
- CMake **开了 AUTOMOC 吗**（app 顶层 `set(CMAKE_AUTOMOC ON)` 或 qt_add_executable 自动处理）？
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
