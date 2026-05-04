# 现代Qt开发教程（新手篇）3.22——QLineEdit：单行文本输入

## 1. 前言 / QLineEdit 远不止一个输入框

QLineEdit 大概是 Qt 里仅次于 QPushButton 的高频控件。任何需要用户输入一行文字的地方——登录框、搜索栏、IP 地址配置、密码输入——背后都是 QLineEdit 在工作。也正因为它的使用频率太高，很多开发者把它当作"拖上去、连个信号、取个 text"就完事的控件，从来没有系统了解过 QLineEdit 的完整能力集。

但实际上 QLineEdit 有不少被低估的功能：`setPlaceholderText` 的灰字提示、`setEchoMode` 做密码框和无回显、`setValidator` 配合 QIntValidator 和 QRegularExpressionValidator 做输入限制、以及三个容易混淆的信号 textChanged / textEdited / editingFinished 各自的触发时机和适用场景。其中信号的区别是一个让很多开发者踩过坑的地方——你明明只是想"用户输入完之后做点什么"，结果选错了信号，要么在每次按键时都触发了一遍，要么在程序自己 setText 的时候也跟着触发了。这篇文章我们就把 QLineEdit 的四个核心维度讲透：基础输入控制、回显模式与密码框、输入验证器、以及信号机制的区别。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QLineEdit 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。本篇涉及的 QIntValidator、QRegularExpressionValidator 定义在 Qt6::Gui 中，但因为 QtWidgets 依赖 Gui 模块，实际使用中只需链接 Qt6::Widgets 就够了。QRegularExpression 是 Qt 对 C++ 标准库 std::regex 的高效替代，在正则验证器中广泛使用。QLineEdit 的行为在所有桌面平台上一致，视觉差异由各平台的 QStyle 控制。

## 3. 核心概念讲解

### 3.1 基础输入控制：placeholder / maxLength / readOnly

QLineEdit 有三个最基础的输入控制属性，它们分别从不同角度限制了用户的输入行为。

`setPlaceholderText(const QString &)` 设置占位文字——当 QLineEdit 的内容为空时，会以灰色显示这段提示文字。它不会成为 QLineEdit 实际内容的一部分，调用 `text()` 不会返回 placeholder 文字。这个功能在搜索框、登录框等需要提示用户"这里应该输入什么"的场景中几乎是标配。

```cpp
auto *searchEdit = new QLineEdit();
searchEdit->setPlaceholderText("输入关键词搜索...");

auto *emailEdit = new QLineEdit();
emailEdit->setPlaceholderText("example@domain.com");
```

`setMaxLength(int)` 限制用户能输入的最大字符数。当输入的字符数达到上限后，用户继续按键不会有任何效果——既不会追加字符也不会有错误提示。需要注意的是，这个限制是"字符数"而不是"字节数"，一个中文字符和一个英文字母都算一个字符。如果你没有调用 `setMaxLength()`，QLineEdit 的默认最大长度是 32767 个字符——这在实际使用中等同于"无限制"，因为单行输入框里输入 32767 个字符本身就是一件很极端的事情。

```cpp
auto *usernameEdit = new QLineEdit();
usernameEdit->setMaxLength(20);  // 最多 20 个字符

auto *codeEdit = new QLineEdit();
codeEdit->setMaxLength(6);       // 验证码 6 位
```

`setReadOnly(bool)` 把 QLineEdit 切换为只读模式。只读模式下用户无法编辑内容，但可以选择和复制文本。视觉上只读的 QLineEdit 通常会以灰色背景显示，具体样式取决于 QStyle。`setReadOnly(true)` 和 `setEnabled(false)` 的区别在于：只读模式下文本仍然可以被选择和复制，而禁用状态下文本连选择都做不到——用户无法与之交互，文本看起来也是灰色的。所以如果你只是不希望用户修改内容但仍然允许他们复制，用 `setReadOnly(true)`；如果你完全不希望用户跟这个输入框有任何交互，用 `setEnabled(false)`。

```cpp
auto *pathEdit = new QLineEdit();
pathEdit->setText("/usr/local/bin/app");
pathEdit->setReadOnly(true);  // 显示路径但不允许手动修改
```

这三个属性是 QLineEdit 最基础的控制手段。在实际项目中，placeholder 和 maxLength 通常在界面初始化时就设置好，而 readOnly 则可能在运行时根据业务状态动态切换——比如"编辑"和"查看"模式之间的切换。

### 3.2 回显模式：setEchoMode 与密码框

`setEchoMode(QLineEdit::EchoMode)` 控制 QLineEdit 如何显示用户输入的字符。QLineEdit 提供了四种回显模式，其中两种在日常开发中使用频率最高。

`QLineEdit::Normal` 是默认值——用户输入什么就显示什么，所见即所得。绝大多数 QLineEdit 都使用这个模式。

`QLineEdit::Password` 是密码框模式——用户输入的每个字符都被替换成一个圆点（或星号，取决于 QStyle），实际内容保存在 QLineEdit 内部但不在屏幕上显示。用户仍然可以正常输入、删除、选择和复制——只是看到的都是圆点而不是实际字符。

`QLineEdit::PasswordEchoOnEdit` 是一个比较特殊的模式——当用户正在输入时（也就是焦点在这个 QLineEdit 上并且用户正在编辑），新输入的字符会短暂地显示为明文，然后过一小段时间（通常是 1-2 秒）或者用户输入下一个字符时，之前的字符会被替换成圆点。这个模式在移动设备的密码输入中特别常见——Android 和 iOS 的密码输入框默认就是这种行为。它兼顾了安全性（最终都变成圆点）和输入准确性（用户能看到自己刚才按了什么键）。

`QLineEdit::NoEcho` 是完全不回显——用户输入的内容既不显示明文也不显示圆点，屏幕上什么都没有。这个模式用于最高安全级别的场景，比如输入 PIN 码或加密密钥——你甚至不希望旁观者通过圆点的数量来猜测密码长度。

```cpp
// 普通输入框
auto *nameEdit = new QLineEdit();
nameEdit->setEchoMode(QLineEdit::Normal);

// 密码框——输入即显示为圆点
auto *passwordEdit = new QLineEdit();
passwordEdit->setEchoMode(QLineEdit::Password);
passwordEdit->setPlaceholderText("请输入密码");

// 编辑时短暂明文——类似手机密码输入
auto *pinEdit = new QLineEdit();
pinEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
pinEdit->setMaxLength(6);

// 无回显——最高安全性
auto *keyEdit = new QLineEdit();
keyEdit->setEchoMode(QLineEdit::NoEcho);
```

使用 Password 模式时有一个需要注意的点：Qt 6 默认不会在密码框上启用操作系统的密码管理器集成。如果你需要浏览器式的"记住密码"弹窗功能，那不是 QLineEdit 自身的能力——你需要自己实现或者使用第三方库。QLineEdit 的 Password 模式做的唯一一件事就是把显示的字符替换成圆点，仅此而已。

另一个细节是 `setClearButtonEnabled(bool)`。这个方法会在 QLineEdit 的右侧显示一个清除按钮（一个灰色的小叉号），当输入框有内容时出现，点击后清空所有内容。这个功能在搜索框和任何需要频繁清空重输的场景中非常实用，建议在搜索框上始终开启。

```cpp
auto *searchEdit = new QLineEdit();
searchEdit->setPlaceholderText("搜索...");
searchEdit->setClearButtonEnabled(true);
```

### 3.3 输入验证器：setValidator + QIntValidator / QRegularExpressionValidator

QLineEdit 的 `setValidator(QValidator *)` 机制允许你在用户输入的同时就进行格式验证——不符合验证规则的字符直接被拒绝，根本不会出现在输入框中。这比"用户输入完之后检查格式、弹出错误提示"的体验好得多，因为用户不会花时间输入一段无效的内容然后被告知从头再来。

Qt 提供了三个内置的验证器类。`QIntValidator` 限制只能输入整数，可以设置允许的最小值和最大值。`QDoubleValidator` 限制只能输入浮点数，同样支持范围设置和精度控制。`QRegularExpressionValidator` 是最灵活的一个——你给它一个正则表达式，只有匹配正则的输入才会被接受。

QIntValidator 的使用非常直观。假设你有一个端口号输入框，合法的端口号范围是 1 到 65535：

```cpp
auto *portEdit = new QLineEdit();
portEdit->setValidator(new QIntValidator(1, 65535, portEdit));
portEdit->setPlaceholderText("1 - 65535");
```

这段代码创建了一个限制范围 1-65535 的整数验证器。用户在输入框中只能输入数字字符——如果按了字母键，验证器会拒绝这个字符，输入框不会发生任何变化。但这里有一个很容易踩的坑：QIntValidator 验证的是整个字符串是否构成一个合法的整数，而不是逐字符验证。比如当输入框为空时，用户输入"0"，验证器检查"0"是否在 1-65535 范围内——"0"不在范围内，但 QIntValidator 会返回 `QValidator::Intermediate`（中间状态），允许用户继续输入，因为用户可能正在输入"80"或者"443"。但如果用户最终输入完了离开输入框，而内容仍然是"0"（一个不合法的值），QIntValidator 不会自动阻止焦点离开——验证器只在输入过程中过滤非法字符，不负责最终的提交验证。

这意味着 setValidator 只是一个"输入过程中的辅助过滤"，你不能完全依赖它来保证最终值的合法性。在提交数据之前，你还是需要在业务逻辑中做一次完整的验证。

QRegularExpressionValidator 是处理复杂格式输入的利器。比如你需要一个 IPv4 地址输入框：

```cpp
// IPv4 地址正则（简化版，不完全严谨但够用）
auto *ipEdit = new QLineEdit();
ipEdit->setValidator(new QRegularExpressionValidator(
    QRegularExpression(
        R"(^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$)"),
    ipEdit));
ipEdit->setPlaceholderText("192.168.1.1");
```

或者一个只允许输入字母和数字的用户名输入框：

```cpp
auto *usernameEdit = new QLineEdit();
usernameEdit->setValidator(new QRegularExpressionValidator(
    QRegularExpression(R"(^[a-zA-Z0-9_]{1,20}$)"),
    usernameEdit));
usernameEdit->setPlaceholderText("字母、数字、下划线");
```

这里有一个重要的细节需要理解：QRegularExpressionValidator 的验证逻辑是"当前输入是否有可能通过正则匹配"。它的 `validate()` 方法返回三种状态：`QValidator::Acceptable`（完全匹配正则）、`QValidator::Intermediate`（当前输入不完整，但继续输入可能变成合法的）、`QValidator::Invalid`（完全不合法）。用户每次按键时，QLineEdit 把当前完整的文本传给验证器的 `validate()` 方法，如果返回 `Invalid`，这次按键就被拒绝。这意味着你的正则表达式需要考虑"中间状态"——如果正则要求整个字符串以 `@` 结尾，但用户还没输入到 `@` 的时候就已经被拒绝了，那用户永远无法输入完整的地址。

所以实际使用中，QRegularExpressionValidator 的正则通常写得比较宽松，只排除明显非法的字符，而不是试图一次性验证完整的格式。完整格式的验证放在提交时做。

### 3.4 信号机制：textChanged vs textEdited vs editingFinished

QLineEdit 有三个跟文本内容变化相关的信号，它们的触发时机和适用场景各不相同，搞混了会导致各种莫名其妙的问题。

`textChanged(const QString &)` 在文本内容发生任何变化时触发——无论是用户通过键盘输入、粘贴、删除，还是程序通过 `setText()` 修改。这个信号是最"敏感"的，它会在每一次内容变化时都触发。使用场景：实时搜索（用户每输入一个字符就过滤一次结果）、实时字符计数、实时格式预览。

`textEdited(const QString &)` 只在用户手动编辑文本时触发——程序通过 `setText()` 修改内容不会触发这个信号。使用场景：标记"文档已修改"状态（只有用户的操作才算修改，程序初始化时的 setText 不算）、在用户输入时做实时建议但不影响程序设置默认值的逻辑。

`editingFinished()` 在用户完成编辑时触发——具体来说就是用户按了 Enter/Return 键，或者 QLineEdit 失去了焦点。这个信号不带参数，你需要手动调用 `text()` 来获取当前内容。使用场景：表单提交（用户填完一项后按 Enter 或 Tab 跳到下一项时触发验证）、地址栏输入（用户输完 URL 后按 Enter 开始加载）。

```cpp
auto *edit = new QLineEdit();

// 任何内容变化都触发（包括 setText）
connect(edit, &QLineEdit::textChanged, this, [](const QString &text) {
    qDebug() << "textChanged:" << text;
});

// 只有用户手动编辑才触发
connect(edit, &QLineEdit::textEdited, this, [](const QString &text) {
    qDebug() << "textEdited:" << text;
});

// 按回车或失去焦点时触发
connect(edit, &QLineEdit::editingFinished, this, [edit]() {
    qDebug() << "editingFinished:" << edit->text();
});
```

我们来模拟一下用户的操作序列，看看三个信号分别在什么时候触发。假设用户在空输入框中输入了"a"然后"b"，然后按了 Enter，然后程序调用了 `edit->setText("hello")`：

输入"a"时，textChanged 触发（参数 "a"），textEdited 触发（参数 "a"）。输入"b"时，textChanged 触发（参数 "ab"），textEdited 触发（参数 "ab"）。按 Enter 时，editingFinished 触发。程序调用 setText("hello") 时，textChanged 触发（参数 "hello"），textEdited 不触发，editingFinished 不触发。

从这个序列可以清楚地看到：textChanged 是"内容变了就触发"，textEdited 是"用户动了才触发"，editingFinished 是"用户编辑完了才触发"。

实际项目中最常见的错误是在应该用 textEdited 的地方用了 textChanged。假设你有一个"设置已修改"的标志位，用户修改了输入框内容后需要保存。如果你用 textChanged 来设置标志位，那么程序初始化时调用 setText() 也会触发 textChanged，导致标志位被错误地设为 true——程序还没让用户看到界面，"已修改"就变成 true 了。用 textEdited 就没有这个问题，因为 setText() 不会触发 textEdited。

另一个常见错误是在应该用 editingFinished 的地方用了 textChanged。假设你有一个网络请求，用户在地址栏输入 URL 后需要发起请求。如果你用 textChanged 来触发请求，用户每输入一个字符就会发一次请求——输入 "h""t""t""p" 就已经发了四次请求。用 editingFinished 的话，只有用户按 Enter 或输入框失去焦点时才会发一次请求。

## 4. 踩坑预防

第一个坑是 textChanged 和 textEdited 的混淆。textChanged 在程序调用 setText() 时也会触发，textEdited 不会。如果你需要区分"用户操作"和"程序设置"，用 textEdited。如果你需要无论什么原因导致内容变化都做出响应，用 textChanged。

第二个坑是 QIntValidator 允许中间状态输入。当范围下限大于 0 时（比如 1-65535），用户可以先输入"0"（中间状态），验证器不会阻止。但如果用户最终提交了"0"，这个值不在合法范围内。所以 setValidator 只是一个输入辅助，最终的提交验证不能省略。

第三个坑是 QRegularExpressionValidator 的正则写得太严格导致用户无法正常输入。如果你的正则要求完整匹配一个邮箱地址，用户在输入过程中（比如只输入了"user"）就会被拒绝，因为"user"不是一个合法的邮箱。解决方案是让正则宽松一些，只排除明显非法的字符（比如空格），完整格式验证放在 editingFinished 或提交时做。

第四个坑是 Password 模式下 `text()` 返回的是明文密码。这个是理所当然的行为，但有些开发者会误以为 Password 模式下 text() 返回的是圆点字符串——不是的，text() 始终返回用户实际输入的明文。如果你需要把密码保存到配置文件或日志中，务必做加密处理，不要直接存储明文。

第五个坑是 setMaxLength 对 setText 和 paste 的影响。当 setText 的内容超过了 maxLength，超出部分会被静默截断。当用户粘贴一段超过 maxLength 的文字时，同样会被截断到 maxLength 长度——这个过程没有任何提示。如果你需要提示用户"输入内容被截断了"，需要在 textChanged 中比较当前 text 长度和预期长度。

## 5. 练习项目

我们来做一个综合练习：创建一个模拟"用户注册表单"的窗口，展示 QLineEdit 的各项能力。表单包含五行输入：用户名（限制字母数字下划线，最大 20 字符）、邮箱地址（正则验证基本格式）、年龄（整数验证，范围 1-150）、密码（Password 回显模式）、确认密码（Password 回显模式，失焦时检查与密码是否一致）。每个输入框都有合适的 placeholder 提示。窗口右上角有一个 QLabel 实时显示"用户名"输入框的字符计数（textChanged 信号驱动）。窗口底部有一个"注册"按钮（QPushButton），点击后检查所有输入框的内容合法性——如果通过则在窗口底部的 QLabel 上显示"注册成功"，否则显示具体的错误信息（比如"两次密码不一致""年龄不在合法范围内"）。

几个提示：用户名验证器用 QRegularExpressionValidator 限制为字母数字下划线；年龄验证器用 QIntValidator(1, 150)；密码一致性检查在确认密码框的 editingFinished 信号中做；字符计数用 textChanged 信号配合 text().length()；注册按钮的完整验证逻辑不依赖 setValidator，而是在提交时手动检查每个字段的合法性。

## 6. 官方文档参考链接

[Qt 文档 · QLineEdit](https://doc.qt.io/qt-6/qlineedit.html) -- 单行文本输入框

[Qt 文档 · QValidator](https://doc.qt.io/qt-6/qvalidator.html) -- 输入验证器基类

[Qt 文档 · QIntValidator](https://doc.qt.io/qt-6/qintvalidator.html) -- 整数验证器

[Qt 文档 · QRegularExpressionValidator](https://doc.qt.io/qt-6/qregularexpressionvalidator.html) -- 正则表达式验证器

[Qt 文档 · QRegularExpression](https://doc.qt.io/qt-6/qregularexpression.html) -- 正则表达式类

---

到这里，QLineEdit 的四个核心维度我们就全部讲完了。placeholder/maxLength/readOnly 是最基础的输入控制手段；setEchoMode 提供了从明文到完全无回显的四种模式，密码框只是其中一种应用；setValidator 配合 QIntValidator 和 QRegularExpressionValidator 可以在输入过程中就过滤掉非法内容，但最终验证不能省略；textChanged / textEdited / editingFinished 三个信号的触发时机决定了你应该在什么场景下使用哪一个——搞混它们是 QLineEdit 最常见的踩坑来源。QLineEdit 看起来就是"一个输入框"，但用好它需要理解的细节比你想象的多。
