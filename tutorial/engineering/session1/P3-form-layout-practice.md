# 实战练习 · FormLayout — 摆控件的正确姿势

## 前言：最不起眼但最常用的布局

如果你做过任何带"设置"或"注册"或"新建记录"界面的应用，你一定见过这种界面结构：左边一列标签（"用户名"、"邮箱"、"密码"），右边一列输入控件，上下对齐，整整齐齐。这种模式在各种 GUI 框架里都无处不在，以至于 Qt 专门提供了一个布局类来干这件事——QFormLayout。

你可能会说，QHBoxLayout 不也能做吗？左边放个 QLabel 右边放个 QLineEdit，每行一个 QHBoxLayout。技术上确实可以，但 QFormLayout 替你处理了一堆琐碎的事：标签和输入控件的默认间距、行间距、标签的对齐方式、统一的标签宽度，还有一个很实用的功能——`addRow` 可以一行代码同时创建标签和输入控件。用 QHBoxLayout 手动拼要写三倍以上的代码，而且稍微改一下间距就得每行都改。所以这个练习不只是练 QFormLayout 本身，更重要的是练"用正确的工具做正确的事"这个思维。

---

## 出发前的装备清单

- **QFormLayout** — 专门为标签-输入对设计的布局管理器，核心方法 `addRow(QString label, QWidget *field)`。它就是你今天的主角。
- **QLineEdit / QTextEdit / QComboBox / QSpinBox / QCheckBox / QRadioButton / QDateEdit** — 各种常见的输入控件。这篇练习你会把它们全部用一遍，感受一下每种控件返回数据的方式有什么不同。
- **QDialogButtonBox** — 标准对话框按钮容器，自带 Ok/Cancel/Apply 等标准按钮，自动处理平台差异。
- **QMessageBox** — 用来展示收集到的数据。

---

## 我们的目标长什么样

我们要做一个信息录入表单，效果大概是这样：

```
┌──────────────────────────────────────┐
│  信息录入                             │
│                                      │
│  姓名:    [________________]         │
│  邮箱:    [________________]         │
│  描述:    [                    ]      │
│           [                    ]      │
│  类别:    [▼ 请选择        ]         │
│  数量:    [↑ 1            ]          │
│  启用:    ☑                           │
│  类型:    ○ 类型A  ○ 类型B           │
│  日期:    [2026-04-21    📅]         │
│                                      │
│         [提交]    [重置]              │
└──────────────────────────────────────┘
```

完成标准：填写所有字段后点"提交"，弹出一个 QMessageBox 展示所有字段的值。点"重置"，所有字段恢复到默认状态。两个单选按钮是互斥的（选 A 时 B 自动取消）。

---

## 第一步 — 创建对话框和主布局

### 思考题

我们这次用的是 QDialog 而不是 QWidget。想一想为什么？QDialog 比 QWidget 多了哪些功能？提示：exec()、accept()、reject() 这三个方法和模态对话框有关。不过我们这个练习其实不需要这些高级功能，用 QDialog 纯粹是因为"表单通常出现在对话框里"这个惯例。用 QWidget 也完全能做，选择权在你。

### 动手写

我们来搭一个基本的对话框类，骨架如下：

```cpp
#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextEdit>

class FormDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FormDialog(QWidget *parent = nullptr);

private slots:
    // TODO: 提交按钮的槽函数
    // TODO: 重置按钮的槽函数

private:
    void setupUi();

    // TODO: 声明所有输入控件的成员变量指针
    //       你需要为每种输入类型各一个：
    //       QLineEdit* m_nameEdit
    //       QLineEdit* m_emailEdit
    //       QTextEdit* m_descEdit
    //       QComboBox* m_categoryCombo
    //       QSpinBox* m_quantitySpin
    //       QCheckBox* m_enabledCheck
    //       QRadioButton* m_typeA
    //       QRadioButton* m_typeB
    //       QDateEdit* m_dateEdit
};
```

在 setupUi() 里，创建一个 QFormLayout 作为主布局，然后逐个 addRow：

```cpp
void FormDialog::setupUi()
{
    setWindowTitle("信息录入");

    // TODO: 创建 QFormLayout，设为对话框的主布局
    //       QFormLayout *form = new QFormLayout(this);

    // TODO: 创建所有输入控件
    //       然后用 form->addRow("标签文本", widget) 逐个添加
    //       例如：form->addRow("姓名:", m_nameEdit = new QLineEdit);
    //       注意 addRow 有多种重载——有的只接受 QWidget，
    //       有的可以同时传入 QString 作为标签。
    //       查一下文档，选最简洁的那种。
}
```

这里有一个很棒的细节：`addRow` 的 QString 版本会自动帮你创建一个 QLabel 作为标签，你不需要手动 new QLabel。一个参数搞定标签+输入控件+布局关系，这就是 QFormLayout 存在的意义。

### 检查点

到这里先编译运行。你应该能看到一个空的对话框——因为我们还没添加任何控件。如果你已经有了 QFormLayout 的基本框架（哪怕还没有 addRow），至少说明头文件和构造函数没写错。

---

## 第二步 — 逐个添加字段：八种输入控件一次用遍

### 思考题

每种输入控件"返回数据"的方式都不同。QLineEdit 用 `text()`，QSpinBox 用 `value()`，QCheckBox 用 `isChecked()`，QComboBox 用 `currentText()` 或者 `currentIndex()`。在动手之前，先去文档里快速扫一眼这些方法，心里有个数。后面提交数据的时候你会用到它们。

### 动手写

现在来把所有控件都加上。下面是每个控件的要点，但具体代码你需要自己写：

**QLineEdit（姓名、邮箱）** — 最基本的单行输入。可以设置 `setPlaceholderText()` 来显示占位提示。邮箱那个还可以考虑用 `setValidator()` 加一个简单的格式验证，但先不做，后面进阶挑战再加。

**QTextEdit（描述）** — 多行文本输入，注意它获取文本的方法不是 `text()` 而是 `toPlainText()`。这是个经典的"第一次一定会查文档"的方法名。

**QComboBox（类别）** — 下拉选择框。你需要用 `addItem()` 预设几个选项。想想应该放在 setupUi 的哪个位置——当然是创建控件之后、addRow 之前。

**QSpinBox（数量）** — 数字步进器。可以设置范围 `setRange()` 和步长 `setSingleStep()`。默认范围是 0-99，你可能需要改一下。

**QCheckBox（启用）** — 复选框。QFormLayout 的 addRow 如果传 QCheckBox 作为 field，标签那一列会自动留空（因为 checkbox 本身就带文字）。你也可以把 QCheckBox 作为 label 参数传给 addRow，效果略有不同——试试两种方式看哪个更符合你的预期。

**QRadioButton（类型A/类型B）** — 单选按钮需要互斥，也就是选了 A 就不能同时选 B。Qt 的互斥机制需要用 QButtonGroup 来实现，而不是 QRadioButton 本身的什么属性。创建一个 QButtonGroup，把两个 QRadioButton 都 `addButton()` 进去，它们就自动互斥了。然后把两个按钮放在一个 QWidget 里（用 QHBoxLayout 横排），再把这个 QWidget 作为 addRow 的 field。

**QDateEdit（日期）** — 日期选择器。默认显示当前日期。可以用 `setCalendarPopup(true)` 开启日历弹窗选择模式，比纯手输友好得多。

```cpp
// 组装单选按钮的示例结构：
// QWidget *typeWidget = new QWidget;
// QHBoxLayout *typeLayout = new QHBoxLayout(typeWidget);
// typeLayout->addWidget(m_typeA = new QRadioButton("类型A"));
// typeLayout->addWidget(m_typeB = new QRadioButton("类型B"));
// typeLayout->setContentsMargins(0, 0, 0, 0);
// QButtonGroup *group = new QButtonGroup(this);
// group->addButton(m_typeA);
// group->addButton(m_typeB);
// form->addRow("类型:", typeWidget);
```

### 你可能会遇到的坑

QButtonGroup 这一步真的很容易忘。如果你不放 QButtonGroup，两个 QRadioButton 各自独立，你就能同时选中 A 和 B——这完全违背了"单选"的语义，而且不会有任何编译警告或运行时报错。这个坑的隐蔽性极高，属于那种"你觉得逻辑很对但就是不对"的经典 case。

另一个容易忽视的地方是 QTextEdit 在 QFormLayout 里的表现。QTextEdit 默认有一个比较大的 sizeHint，可能会撑得整个表单很高。你可以用 `setMaximumHeight()` 限制一下它的高度。

### 检查点

编译运行。你应该能看到一个完整的表单，所有字段整齐排列。点一下 ComboBox 下拉能看到选项，SpinBox 上下箭头能用，两个 RadioButton 互斥，DateEdit 点击能弹出日历。如果布局看起来乱，检查一下有没有遗漏 `setLayout` 或者布局的 parent 设置。

---

## 第三步 — 按钮行：提交和重置

### 思考题

按钮有三种放法：手动 new QPushButton 放到 QHBoxLayout 里；用 QDialogButtonBox 自动管理标准按钮；或者用 QFormLayout 的 addRow 直接放。你觉得哪种最适合这个场景？提示：QDialogButtonBox 是"标准对话框"的惯例写法，它的好处是自动适配平台风格（Windows 上按钮顺序和 macOS 不同）。

### 动手写

我建议用 QDialogButtonBox，因为它确实省心：

```cpp
// 在 setupUi 的最后：
// TODO: 创建 QDialogButtonBox
//       添加 Ok 和 Reset 角色（注意是 Reset 不是 Cancel）
//       握住 accepted() 和 clicked(button) 信号

// 提交：连接 QDialogButtonBox::accepted 到 accept 槽
// 重置：连接 clicked 信号，检查被点击的按钮是否是 ResetButton
//        如果是，调用 resetFields()
```

另一种做法是直接创建两个 QPushButton，手动放进 QHBoxLayout。这样更灵活但需要自己管理按钮顺序。对于学习来说两种都试一下也没坏处。

### 检查点

你能看到两个按钮在表单底部。点击它们暂时没反应（因为槽函数还没实现），但至少说明布局是对的。

---

## 第四步 — 收集数据：提交功能

### 动手写

提交按钮的槽函数要做的事情很直接：把每个控件的当前值读出来，拼成一段可读的文本，用 QMessageBox 显示。

```cpp
void FormDialog::onSubmit()
{
    // TODO: 从每个控件读取值：
    //   m_nameEdit->text()
    //   m_emailEdit->text()
    //   m_descEdit->toPlainText()    ← 注意不是 text()
    //   m_categoryCombo->currentText()
    //   m_quantitySpin->value()      ← 返回 int，需要转成 QString
    //   m_enabledCheck->isChecked()  ← 返回 bool
    //   m_typeA->isChecked() ? "类型A" : "类型B"
    //   m_dateEdit->date().toString("yyyy-MM-dd")

    // TODO: 用 QString 拼接或者 QString::asprintf 格式化
    // TODO: QMessageBox::information(this, "提交结果", summaryText);
}
```

注意 `QString::number()` 用来把 int 转 QString，或者用 `QString::arg()` 的方式格式化。两种都可以，选你顺手的那种。

### 检查点

填写所有字段，点提交。QMessageBox 应该弹出并显示你填的所有数据。如果某个字段显示为空，检查一下是不是读了错误的方法（比如对 QTextEdit 调了 `text()` 而不是 `toPlainText()`）。

---

## 第五步 — 重置功能

### 思考题

重置意味着把所有控件恢复到初始状态。你能想到几种实现方式？最直接的是逐个调用每个控件的 reset 方法。有没有更优雅的批量方式？提示：Qt 没有内置的"表单重置"功能——你得自己写。

### 动手写

```cpp
void FormDialog::onReset()
{
    // TODO: 逐个重置每个控件
    //   m_nameEdit->clear()
    //   m_emailEdit->clear()
    //   m_descEdit->clear()
    //   m_categoryCombo->setCurrentIndex(0)
    //   m_quantitySpin->setValue(1)
    //   m_enabledCheck->setChecked(false)
    //   m_typeA->setChecked(true)      ← 恢复默认选中
    //   m_dateEdit->setDate(QDate::currentDate())
}
```

没什么技巧，就是老老实实一个个重置。如果你控件很多，这个函数会很长，但逻辑很直。

### 检查点

填写一些数据后点重置。所有字段应该恢复到初始值。再点提交，弹出的 QMessageBox 应该显示的是默认值而不是之前填的数据。

---

## 最终组装

main.cpp 很简单，创建 FormDialog 并 exec()：

```cpp
#include <QApplication>
#include "form_dialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    FormDialog dialog;
    return dialog.exec();
}
```

注意 QDialog::exec() 会启动一个局部事件循环，对话框关闭时 exec() 才返回。这就是"模态对话框"的含义——在对话框打开期间，主窗口（如果有的话）是无法操作的。

---

## 验收标准

所有八种输入控件都能正常使用：QLineEdit 可以输入文字，QTextEdit 可以输入多行文字，QComboBox 可以选择选项，QSpinBox 可以调数字，QCheckBox 可以勾选，QRadioButton 互斥且只能选一个，QDateEdit 可以选日期。点提交后 QMessageBox 正确展示所有字段的值。点重置后所有字段回到默认状态。窗口缩放时布局保持对齐，标签列宽度一致。

---

## 进阶挑战

给邮箱字段加一个正则验证器，格式不对时输入框边框变红——查一下 QRegularExpressionValidator 和 QLineEdit 的 setValidator。或者加一个"导出到 JSON"按钮，用 QJsonDocument 和 QJsonObject 把表单数据序列化输出——正好练一下 JSON API。再或者，实现一个"保存草稿"功能：表单内容自动保存到 QSettings，下次打开时自动恢复。

---

## 踩坑预防清单

> **坑 #1：忘记 QButtonGroup 导致单选不互斥**
> QRadioButton 不像 HTML 的 radio button 那样通过 name 属性自动互斥。Qt 里你必须显式创建 QButtonGroup 并把按钮加进去。忘了这步，两个 radio 可以同时选中，而且没有任何报错提醒你。

> **坑 #2：QTextEdit 的方法名不一样**
> QLineEdit 用 `text()`，QTextEdit 用 `toPlainText()`。如果你对 QTextEdit 调了 `text()`……它确实有这个方法，但返回的是富文本 HTML，不是用户看到的纯文本。这个坑会让你 debug 半天。

> **坑 #3：QFormLayout 的 addRow 传 QCheckBox 时标签列留空**
> 如果你 `addRow("启用:", checkBox)`，你会发现标签列的"启用:"显示了但 QCheckBox 旁边也有文字，看起来冗余。更自然的做法是 `addRow(checkBox)` 只传一个参数，让 checkbox 自己的文本充当标签。

---

## 官方文档参考

- [QFormLayout Class](https://doc.qt.io/qt-6/qformlayout.html) — addRow 的各种重载
- [QDialogButtonBox Class](https://doc.qt.io/qt-6/qdialogbuttonbox.html) — 标准按钮容器
- [QButtonGroup Class](https://doc.qt.io/qt-6/qbuttongroup.html) — 按钮分组和互斥
- [QMessageBox Class](https://doc.qt.io/qt-6/qmessagebox.html) — 消息框

到这里就大功告成了。这篇练习虽然技术难度不高，但它覆盖了一个你在实际工程中几乎每天都会用到的布局模式。下一篇我们开始做完整应用了——一个秒表，这是你从"控件练习"跨入"应用练习"的第一步。
