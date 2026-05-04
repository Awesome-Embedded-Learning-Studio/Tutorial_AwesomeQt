# 现代Qt开发教程（新手篇）3.63——QInputDialog：输入对话框

## 1. 前言 / 每个应用都需要问用户要一点数据

我们在前一篇讲 QMessageBox 的时候说过，对话框的本质是应用和用户之间的一次短暂交互。QMessageBox 负责的是"应用告诉用户一些事情"，而 QInputDialog 负责的是"应用问用户要一些数据"。这两个类在功能上是互补的：一个向用户输出信息，一个从用户获取输入。

你可能在很多桌面应用中见过这样的场景——点击"重命名"后弹出一个带文本框的小窗口，让你输入新文件名；点击"调整大小"后弹出一个带数字输入框的小窗口，让你输入新的分辨率；点击"选择月份"后弹出一个带下拉列表的小窗口，让你从预定义的选项中选择一个。这些场景的共同特征是：输入单一、交互短暂、不需要复杂的表单布局。Qt 把这类场景抽象成了 QInputDialog，提供了四个静态方法来分别获取文本、整数、浮点数和列表选项。

如果只是弹个框让用户敲几个字或者选一个数字，QInputDialog 的静态方法一行代码就够了。但如果你需要对输入内容做校验——比如 IP 地址格式检查、端口号范围限制——QInputDialog 也支持传入 QRegExpValidator 或者设置 min/max 范围。而当你的对话框需要同时收集多个字段（用户名 + 密码 + 记住密码勾选框）时，QInputDialog 就不够用了，这时候你应该自己写一个 QDialog。

今天我们从四个方面来展开。先看 getText / getInt / getDouble / getItem 四个静态方法的用法和各自适用的场景，然后讨论如何通过验证器和范围限制来阻止用户提交无效输入，接着研究 QInputDialog 在需要定制时的非静态用法，最后谈谈什么情况下该放弃 QInputDialog 改用自定义 QDialog。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QInputDialog 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QInputDialog、QApplication、QMainWindow、QPushButton、QLabel、QVBoxLayout、QHBoxLayout、QIntValidator、QDoubleValidator、QDialog、QLineEdit、QDialogButtonBox、QFormLayout 和 QCheckBox。

## 3. 核心概念讲解

### 3.1 getText：让用户输入一段文本

QInputDialog::getText() 是最常用的静态方法，它弹出一个包含 QLineEdit 的输入对话框，用户输入文本后点"确定"返回输入的内容。返回值是一个 QPair<QString, bool>——准确说是一个 std::pair<QString, bool>，其中第一个元素是用户输入的文本，第二个元素是用户是否点击了 OK（true 表示用户确认了输入，false 表示用户取消了）。

```cpp
bool ok = false;
QString text = QInputDialog::getText(
    this,                           // 父窗口
    "重命名文件",                    // 对话框标题
    "请输入新文件名:",               // 标签文本
    QLineEdit::Normal,              // 输入框模式
    "old_name.txt",                 // 默认文本
    &ok);                           // 接收确认状态

if (ok && !text.isEmpty()) {
    renameFile(text);
}
```

getText 的第四个参数是 QLineEdit::EchoMode，决定了输入框的显示模式。Normal 是正常显示输入内容，Password 是用圆点遮盖输入内容（适合密码输入），NoEcho 是完全不显示输入内容（适合高安全性场景）。绝大多数情况下用 Normal 就够了。

如果你需要对输入文本做格式校验——比如只允许输入合法的 IP 地址——可以通过第六个参数传入 Qt::InputMethodHints 或者干脆放弃静态方法，改用 QInputDialog 的实例化方式手动设置 QRegularExpressionValidator。静态方法本身不直接支持 validator 参数，这是一个需要留意的限制。

有一点值得注意：getText 返回的 QString 可能是空字符串。用户在输入框里什么都不填直接点 OK，ok 为 true 但 text 为空。如果你的业务逻辑不允许空输入，需要手动检查。

### 3.2 getInt / getDouble：让用户输入一个数字

QInputDialog::getInt() 弹出一个包含 QSpinBox 的输入对话框，用于获取整数。getInt 允许你设置最小值、最大值和步长，用户只能在这个范围内输入——这是它比 getText + QIntValidator 更方便的地方，因为 QSpinBox 本身就限制了输入范围，用户根本无法输入越界的值。

```cpp
bool ok = false;
int fontSize = QInputDialog::getInt(
    this,
    "设置字号",
    "字号大小:",
    12,         // 默认值
    1,          // 最小值
    72,         // 最大值
    1,          // 步长
    &ok);

if (ok) {
    setFontSize(fontSize);
}
```

getInt 的参数依次是父窗口、标题、标签文本、默认值、最小值、最大值、步长和确认状态指针。其中步长参数决定了用户点击上下箭头时数值变化的幅度。对于字号设置这种场景，步长为 1 就够了；如果是调整分辨率或者缩放比例，步长可能需要设为 10 或者 100。

QInputDialog::getDouble() 的行为和 getInt 几乎一样，区别在于它弹出一个 QDoubleSpinBox，用于获取浮点数。getDouble 多了一个 decimals 参数，控制小数位数。

```cpp
bool ok = false;
double scale = QInputDialog::getDouble(
    this,
    "缩放比例",
    "请输入缩放比例 (0.1 ~ 5.0):",
    1.0,        // 默认值
    0.1,        // 最小值
    5.0,        // 最大值
    2,          // 小数位数
    &ok);

if (ok) {
    applyScale(scale);
}
```

getDouble 的 decimals 参数默认值是 1。如果你需要更高精度（比如坐标输入需要小数点后 4 位），把这个参数设大就行了。QDoubleSpinBox 会自动处理小数点的输入和显示格式。

 getInt 和 getDouble 的一个共同优势是：它们通过 QSpinBox / QDoubleSpinBox 硬性限制了输入范围，用户不可能输入超出 min ~ max 的值。这比"弹一个文本框让用户随便输，提交后再校验"的用户体验好得多——用户在输入的时候就知道合法范围是什么，不会填了半天提交后才被告知"数值超出范围"。

### 3.3 getItem：让用户从列表中选择

QInputDialog::getItem() 弹出一个包含 QComboBox 的输入对话框，用户只能从预定义的选项中选择一个。返回值是用户选中的那个 QString。

```cpp
const QStringList months = {
    "一月", "二月", "三月", "四月",
    "五月", "六月", "七月", "八月",
    "九月", "十月", "十一月", "十二月"
};

bool ok = false;
QString month = QInputDialog::getItem(
    this,
    "选择月份",
    "请选择一个月份:",
    months,         // 选项列表
    0,              // 默认选中第 0 项
    false,          // 是否允许用户编辑输入
    &ok);

if (ok) {
    setFilterByMonth(month);
}
```

getItem 的第五个参数 editable 是一个容易忽略但非常重要的开关。当 editable 为 true 时，QComboBox 变成可编辑模式——用户既可以从下拉列表中选择，也可以自己输入不在列表中的文本。当 editable 为 false（默认）时，用户只能从列表中选择。

editable 的选择取决于你的业务需求。如果你的选项是枚举性质（月份、性别、国家），editable 应该设为 false，因为用户不应该输入"第十三个月"。如果你的选项是建议性质（邮箱后缀、常用城市），editable 可以设为 true，允许用户输入列表中没有的值。

### 3.4 验证用户输入：阻止无效提交

前面提到，getText 的静态方法不直接支持 validator。如果你需要验证用户的文本输入，有两种方式。第一种是使用 QInputDialog 的实例化方式，手动设置 validator。

```cpp
QInputDialog dialog(this);
dialog.setWindowTitle("设置端口号");
dialog.setLabelText("请输入端口号 (1 ~ 65535):");
dialog.setInputMode(QInputDialog::IntInput);
dialog.setIntRange(1, 65535);
dialog.setIntValue(8080);

if (dialog.exec() == QDialog::Accepted) {
    int port = dialog.intValue();
    setServerPort(port);
}
```

当你把 inputMode 设为 QInputDialog::IntInput 后，QInputDialog 内部会使用 QSpinBox 来限制输入范围——效果和 getInt 一样，但你可以做更多的定制，比如通过 setOkButtonText 和 setCancelButtonText 修改按钮文字。

对于文本输入的格式校验，可以配合 QRegularExpressionValidator 使用。下面是一个 IP 地址输入的例子：

```cpp
QInputDialog dialog(this);
dialog.setWindowTitle("服务器地址");
dialog.setLabelText("请输入 IP 地址:");
dialog.setTextValue("192.168.1.1");

// IP 地址格式校验
QRegularExpression ipRegex(
    "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
auto *validator = new QRegularExpressionValidator(ipRegex, &dialog);
dialog.setTextEchoMode(QLineEdit::Normal);
// 注意：QInputDialog 没有直接的 setTextInputValidator 方法
// 需要找到内部的 QLineEdit 并设置
```

这里有一个让人比较头疼的限制：QInputDialog 的静态方法 getText 没有提供设置 validator 的参数。实例化方式也没有提供 setTextInputValidator 这样的直接接口。如果你的输入校验需求比较复杂——比如"用户名只能是字母开头、包含字母和数字、长度 4~20 个字符"——直接用 QInputDialog 可能会让你感到力不从心。这时候正确的做法不是硬凑，而是自己写一个 QDialog，里面放一个 QLineEdit 和一个 QDialogButtonBox，然后给 QLineEdit 设置 validator。

### 3.5 多输入字段：改用自定义 QDialog

QInputDialog 最大的局限性在于它只能收集一个输入值。当你的对话框需要同时收集多个字段时——比如"新建用户"对话框需要用户名、密码和"记住密码"三个字段——QInputDialog 就完全无能为力了。这不是 bug，而是设计上的刻意取舍：QInputDialog 追求的是极简的单输入场景，多字段表单本来就不在它的职责范围内。

遇到多字段输入的场景，我们应该直接使用 QDialog 手动搭建表单。QFormLayout 是 Qt 提供的表单布局管理器，非常适合这种"标签 + 输入控件"一行一行的排列方式。

```cpp
class UserLoginDialog : public QDialog
{
public:
    explicit UserLoginDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("用户登录");
        setMinimumWidth(320);

        auto *formLayout = new QFormLayout(this);

        m_usernameEdit = new QLineEdit;
        m_usernameEdit->setPlaceholderText("请输入用户名");
        formLayout->addRow("用户名:", m_usernameEdit);

        m_passwordEdit = new QLineEdit;
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_passwordEdit->setPlaceholderText("请输入密码");
        formLayout->addRow("密码:", m_passwordEdit);

        m_rememberCheck = new QCheckBox("记住密码");
        formLayout->addRow(m_rememberCheck);

        // 按钮区域
        auto *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
        formLayout->addRow(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted,
                this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected,
                this, &QDialog::reject);
    }

    QString username() const
    {
        return m_usernameEdit->text();
    }

    QString password() const
    {
        return m_passwordEdit->text();
    }

    bool rememberPassword() const
    {
        return m_rememberCheck->isChecked();
    }

private:
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QCheckBox *m_rememberCheck = nullptr;
};
```

使用时就像使用 QDialog 一样：

```cpp
UserLoginDialog dialog(this);
if (dialog.exec() == QDialog::Accepted) {
    qDebug() << "用户名:" << dialog.username()
             << "记住密码:" << dialog.rememberPassword();
}
```

这种自定义对话框的优势在于完全可控：你可以给每个输入控件设置独立的 validator，可以在 accept() 中做跨字段校验（比如"确认密码"必须和"密码"一致），可以在输入不符合要求时阻止对话框关闭——通过重写 QDialog::accept() 方法，在条件不满足时不调用 QDialog::accept() 就行了。

```cpp
void accept() override
{
    if (m_usernameEdit->text().length() < 4) {
        m_usernameEdit->setStyleSheet(
            "border: 1px solid red;");
        return;  // 阻止关闭
    }
    if (m_passwordEdit->text().length() < 6) {
        m_passwordEdit->setStyleSheet(
            "border: 1px solid red;");
        return;
    }
    QDialog::accept();
}
```

这就是我们说的"验证用户输入并阻止无效提交"的核心思路：在 accept() 里做校验，不通过就不调用基类的 accept()，对话框就不会关闭。用户被"困"在对话框里，必须修正输入才能继续操作。这种模式比"先关闭对话框，校验失败再重新弹出来"的用户体验好太多了。

## 4. 踩坑预防

第一个坑是忽略 getText 返回空字符串的情况。用户在输入框里什么都不填直接点 OK，ok 变量为 true，text 为空字符串。如果你的业务不允许空输入——比如文件名不能为空——必须同时检查 ok 和 text.isEmpty()。

第二个坑是 getItem 的 editable 参数。默认为 false，用户只能从列表中选择。如果你不小心把它设成了 true，用户可能会输入你列表中根本不存在的值，后续代码拿到这个值可能会出问题。反过来，如果你的场景确实需要用户自由输入，别忘了把 editable 设为 true。

第三个坑是在 getInt / getDouble 中设置了过于狭窄的范围。min 和 max 的默认值分别是 0 和 99（getInt），如果你的业务需要的值超出这个范围（比如端口号最大 65535，文件大小可能上 GB），务必手动设置合理的范围，不然用户输入不了正确的值。

第四个坑是在 QInputDialog 的实例化方式中试图访问内部的 QLineEdit 或 QSpinBox。QInputDialog 没有提供公开的接口来获取这些内部控件——它们是实现细节，不是 API 的一部分。如果你需要对 QLineEdit 设置 validator 或 placeholderText，正确的做法是放弃 QInputDialog，改用自定义 QDialog。

第五个坑是在自定义 QDialog 的 accept() 中忘记调用 QDialog::accept()。如果你在子类中重写了 accept() 做校验，校验通过后一定要调用 QDialog::accept()，否则对话框不会关闭，result() 也不会变成 QDialog::Accepted。这是一个很容易犯的低级错误——校验逻辑写完了，忘了一行 QDialog::accept()，结果对话框怎么都关不掉。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，主窗口上有四个按钮和一个结果显示区域。"输入文件名"按钮调用 getText 弹出文本输入对话框，让用户输入文件名，默认文本为 "untitled.txt"，用户确认后将输入的文件名显示在结果区域。"设置字号"按钮调用 getInt 弹出整数输入对话框，范围 1~72，默认 12，步长 1，用户确认后显示选择的字号。"输入缩放比例"按钮调用 getDouble 弹出浮点数输入对话框，范围 0.1~5.0，默认 1.0，保留 2 位小数，用户确认后显示缩放比例。"选择编码"按钮调用 getItem 弹出列表选择对话框，选项为 "UTF-8 / GBK / ISO-8859-1 / ASCII"，默认选中 "UTF-8"，不可编辑，用户确认后显示选择的编码。

另外实现一个"用户登录"按钮，点击后弹出一个自定义的 QDialog，包含用户名输入框、密码输入框和"记住密码"复选框。用户名校验规则为至少 4 个字符，密码校验规则为至少 6 个字符。校验不通过时输入框边框变红，对话框不关闭；校验通过后关闭对话框，在结果区域显示用户名和是否勾选了记住密码。

提示：自定义登录对话框使用 QFormLayout 布局，通过重写 accept() 方法做校验，校验不通过时不调用 QDialog::accept()。

## 6. 官方文档参考链接

[Qt 文档 -- QInputDialog](https://doc.qt.io/qt-6/qinputdialog.html) -- 输入对话框类

[Qt 文档 -- QInputDialog::getText](https://doc.qt.io/qt-6/qinputdialog.html#getText) -- 文本输入静态方法

[Qt 文档 -- QInputDialog::getInt](https://doc.qt.io/qt-6/qinputdialog.html#getInt) -- 整数输入静态方法

[Qt 文档 -- QInputDialog::getDouble](https://doc.qt.io/qt-6/qinputdialog.html#getDouble) -- 浮点数输入静态方法

[Qt 文档 -- QInputDialog::getItem](https://doc.qt.io/qt-6/qinputdialog.html#getItem) -- 列表选择静态方法

[Qt 文档 -- QFormLayout](https://doc.qt.io/qt-6/qformlayout.html) -- 表单布局管理器

---

到这里，QInputDialog 的全部用法就讲完了。四个静态方法 getText / getInt / getDouble / getItem 覆盖了文本、整数、浮点数和列表选择四种最常见的单值输入场景，实例化方式提供了更多的定制空间，而自定义 QDialog 则是多字段输入和复杂校验的终极方案。掌握这三层递进——静态方法解决简单场景、实例化解决中等复杂度场景、自定义 QDialog 解决复杂场景——你就能在任何输入需求面前选择最合适的工具。
