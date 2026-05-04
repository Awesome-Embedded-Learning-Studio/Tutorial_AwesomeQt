# 现代Qt开发教程（新手篇）3.69——QWizard：向导对话框

## 1. 前言 / 安装向导、配置向导、导入向导——它们全是同一个东西

如果你用过任何桌面软件的安装程序，那你一定见过那种"下一步……下一步……完成"的多步骤对话框序列。Visual Studio 的安装向导、Inno Setup 打包出来的安装程序、Chrome 首次运行时的导入设置向导、甚至 Qt Creator 的新建项目向导——它们的交互形态高度一致：顶部一个标题，中间一块内容区域，底部一排"上一步""下一步""取消""完成"按钮。这种模式叫做"向导对话框"（Wizard Dialog），它把一个复杂的配置流程拆分成多个步骤，引导用户一步一步完成。

QWizard 就是 Qt 提供的向导对话框框架。它管理着一组 QWizardPage（向导页）的导航逻辑——用户点"下一步"切换到后一页，点"上一步"回到前一页，点"取消"放弃整个流程，走完最后一页点"完成"提交所有数据。听起来简单，但实际工程中向导的难点不在导航本身，而在页面之间的数据传递和页面状态的动态变化。比方说一个"新建项目向导"——第一步选项目类型（C++ / Python / Rust），第二步根据类型展示不同的配置选项，第三步汇总所有信息让用户确认。第二步的内容完全取决于第一步的选择，这意味着每次用户切换到第二步时，页面的内容需要根据前一步的数据重新初始化。

QWizard 为这类需求提供了完整的解决方案：addPage 注册向导页、initializePage / validatePage 控制页面的生命周期、registerField / field 实现页面间的数据传递、setButtonText / setPixmap 自定义向导的外观。今天我们从这四个方面展开，最后用一个完整的"用户注册向导"练习来串联所有知识。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QWizard 和 QWizardPage 都在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QWizard、QWizardPage、QLineEdit、QComboBox、QLabel、QVBoxLayout、QGridLayout、QCheckBox、QApplication 和 QDebug。

## 3. 核心概念讲解

### 3.1 addPage / setPage：添加向导页

QWizard 的页面管理方式很简单：你创建一组 QWizardPage 子类实例，通过 addPage 把它们依次注册到 QWizard 中。QWizard 内部维护一个页面 ID 列表，按照 addPage 的调用顺序分配递增的 ID（0, 1, 2, ...）。用户点"下一步"时，QWizard 根据 ID 顺序确定下一个要显示的页面。

先来看一个最简单的三页向导：

```cpp
class IntroPage : public QWizardPage
{
public:
    IntroPage(QWidget *parent = nullptr)
        : QWizardPage(parent)
    {
        setTitle("欢迎使用配置向导");
        setSubTitle("本向导将引导你完成初始配置");
        auto *label = new QLabel(
            "点击\"下一步\"继续。");
        label->setWordWrap(true);
        auto *layout = new QVBoxLayout;
        layout->addWidget(label);
        setLayout(layout);
    }
};

class ConfigPage : public QWizardPage
{
public:
    ConfigPage(QWidget *parent = nullptr)
        : QWizardPage(parent)
    {
        setTitle("基本配置");
        setSubTitle("请填写以下配置信息");
        // ... 配置项控件 ...
    }
};

class ConclusionPage : public QWizardPage
{
public:
    ConclusionPage(QWidget *parent = nullptr)
        : QWizardPage(parent)
    {
        setTitle("配置完成");
        setSubTitle("请确认你的配置");
        // ... 汇总信息展示 ...
    }
};
```

然后在主函数或者调用处把这些页面组装起来：

```cpp
QWizard wizard;
wizard.setWindowTitle("初始配置向导");
wizard.addPage(new IntroPage);
wizard.addPage(new ConfigPage);
wizard.addPage(new ConclusionPage);

if (wizard.exec() == QWizard::Accepted) {
    qDebug() << "用户完成了向导";
} else {
    qDebug() << "用户取消了向导";
}
```

addPage 返回分配的页面 ID。如果你的向导需要非线性导航——比如根据第一步的选择跳到不同的后续页面——你需要用 setPage(id, page) 手动指定 ID，然后在 QWizardPage::nextId() 中返回你想跳转的目标页面 ID。但线性向导（绝大多数场景）直接用 addPage 就够了。

setTitle 和 setSubTitle 是 QWizardPage 提供的方法，用于设置向导页的标题和副标题。QWizard 默认会在页面内容区域的上方显示这两个文字。如果 QWizard 设置了 WizardStyle（比如 AeroStyle 或 ModernStyle），标题和副标题的显示方式会随风格变化。

这里有一个需要注意的地方：QWizard 默认使用 QWizard::ClassicStyle，在 Windows 上看起来可能有点老气。你可以调用 setWizardStyle(QWizard::AeroStyle) 让它在 Windows Vista 及以上版本使用 Aero 风格的水印和标题布局，或者在 Linux 上用 ModernStyle。不同风格下标题、水印、banner 图片的位置和大小都不一样——如果你设置了自定义的 pixmap，可能需要针对不同风格准备不同尺寸的图片。

### 3.2 initializePage / validatePage：页面生命周期

当用户导航到某一页时，QWizard 会调用该页面的 initializePage() 方法。这个方法的作用是"在页面即将显示之前初始化它的内容"。最典型的应用场景是：根据前面页面收集到的数据来动态设置当前页面的控件状态。

比如一个"新建项目向导"——第一步让用户选择项目类型，第二步根据类型展示不同的配置选项。如果用户选了"C++ 项目"，第二步就显示 C++ 相关的编译器选项；如果选了"Python 项目"，第二步就显示 Python 解释器路径选择。这种"后一页依赖前一页数据"的场景正是 initializePage 的用武之地：

```cpp
class ProjectConfigPage : public QWizardPage
{
public:
    ProjectConfigPage(QWidget *parent = nullptr)
        : QWizardPage(parent)
    {
        setTitle("项目配置");
        setSubTitle("根据项目类型配置选项");
        // 创建所有可能用到的控件
        m_compilerCombo = new QComboBox;
        m_compilerCombo->addItems(
            {"GCC", "Clang", "MSVC"});
        m_pythonPathEdit = new QLineEdit;

        m_compilerLabel = new QLabel("编译器:");
        m_pythonLabel = new QLabel("Python 路径:");

        auto *layout = new QGridLayout;
        layout->addWidget(m_compilerLabel, 0, 0);
        layout->addWidget(m_compilerCombo, 0, 1);
        layout->addWidget(m_pythonLabel, 1, 0);
        layout->addWidget(m_pythonPathEdit, 1, 1);
        setLayout(layout);
    }

    void initializePage() override
    {
        // 从第一页获取用户选择的项目类型
        const QString projectType =
            field("projectType").toString();

        if (projectType == "C++") {
            m_compilerLabel->show();
            m_compilerCombo->show();
            m_pythonLabel->hide();
            m_pythonPathEdit->hide();
        } else if (projectType == "Python") {
            m_compilerLabel->hide();
            m_compilerCombo->hide();
            m_pythonLabel->show();
            m_pythonPathEdit->show();
        }
    }

private:
    QComboBox *m_compilerCombo;
    QLineEdit *m_pythonPathEdit;
    QLabel *m_compilerLabel;
    QLabel *m_pythonLabel;
};
```

initializePage 在每次页面即将显示时都会被调用——不仅是第一次进入，用户点"上一步"再点"下一步"回来时也会重新调用。这意味着你不需要在 initializePage 中判断"是不是第一次初始化"——每次都完整设置就行。

与 initializePage 对应的是 validatePage()。当用户点击"下一步"或"完成"时，QWizard 会调用当前页面的 validatePage() 方法。如果这个方法返回 true，导航继续；如果返回 false，向导停在当前页不动。validatePage 的用途是验证用户输入——如果当前页的必填字段没填完或者格式不对，拒绝让用户继续。

```cpp
class UserInfoPage : public QWizardPage
{
    // ...
    bool validatePage() override
    {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            m_nameEdit->setFocus();
            return false;
        }
        if (!m_emailEdit->text().contains('@')) {
            m_emailEdit->setFocus();
            return false;
        }
        return true;
    }
};
```

QWizardPage 还有一个相关的方法 isComplete()，它和 validatePage 的区别需要理解清楚。isComplete 返回当前页面的数据是否"完整"——QWizard 会根据它的返回值来决定"下一步"按钮是否可用（灰色/可点击）。QWizardPage 默认的实现是根据 registerField 注册的 mandatory 字段来判断——所有标记为 mandatory 的字段都有非空值时，isComplete 返回 true。你也可以重写 isComplete 来实现自定义的完整性逻辑，并在此逻辑发生变化时发射 completeChanged 信号通知 QWizard 重新评估按钮状态。

总结一下这两个方法的分工：isComplete 控制"下一步"按钮是否可点击（实时），validatePage 控制用户点击"下一步"后是否真的放行（点的时候）。如果你只是做简单的必填字段检查，用 registerField 的 mandatory 标记就够了，不需要重写 isComplete 和 validatePage。如果验证逻辑比较复杂（比如交叉验证、异步校验），就需要手动实现。

### 3.3 registerField / field：页面间数据传递

向导最核心的需求之一就是页面之间的数据共享——第一页用户输入的名字，第三页的汇总页面要能读到。QWizard 提供了一套字段注册机制来解决这个问题。

在 QWizardPage 中调用 registerField 把某个控件的属性注册为一个全局字段。注册之后，任何页面都可以通过 field(name) 来读取这个字段的值，就像访问一个全局的键值存储一样。

registerField 有两种常见的用法。第一种是注册控件的某个属性（最常用的是用户输入属性）：

```cpp
class UserInfoPage : public QWizardPage
{
public:
    UserInfoPage(QWidget *parent = nullptr)
        : QWizardPage(parent)
    {
        setTitle("用户信息");
        m_nameEdit = new QLineEdit;
        m_emailEdit = new QLineEdit;

        auto *layout = new QGridLayout;
        layout->addWidget(new QLabel("姓名:"), 0, 0);
        layout->addWidget(m_nameEdit, 0, 1);
        layout->addWidget(new QLabel("邮箱:"), 1, 0);
        layout->addWidget(m_emailEdit, 1, 1);
        setLayout(layout);

        // 注册字段：名称, 控件, 属性名, 信号
        // "name" 字段绑定到 m_nameEdit 的 text 属性
        registerField("name*", m_nameEdit);
        registerField("email", m_emailEdit);
    }

private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_emailEdit;
};
```

注意看 "name*" 这个字段名末尾的星号。在 QWizard 的约定中，字段名以星号结尾表示这是一个 mandatory（必填）字段。对于 mandatory 字段，QWizard 会监控它的值——当值为空时，"下一步"按钮保持灰色不可点击；当值非空时，"下一步"按钮变为可用。这个约定省去了手动重写 isComplete 的麻烦。字段名在存储时星号会被去掉，所以 field("name") 就能读到对应的值，不需要带星号。

registerField 的完整签名是 registerField(const QString &name, QWidget *widget, const char *property = nullptr, const char *changedSignal = nullptr)。property 指定要绑定到控件的哪个 Qt 属性。对于 QLineEdit，默认属性就是 "text"；对于 QComboBox，默认属性是 "currentText"；对于 QCheckBox，默认属性是 "checked"。所以大部分情况下不需要显式指定 property 和 changedSignal——QWizard 会根据控件类型自动选择合理的默认值。但如果你绑定的控件不是常见类型，或者想绑定一个非默认属性（比如 QSpinBox 的 value 而不是 text），就需要显式指定：

```cpp
// QSpinBox 的 value 属性，通过 valueChanged 信号跟踪变化
registerField("port", m_spinBox, "value",
               SIGNAL(valueChanged(int)));
```

在另一个页面中读取这些字段值的方式很简单——调用 field(name) 返回一个 QVariant：

```cpp
class SummaryPage : public QWizardPage
{
public:
    // ...
    void initializePage() override
    {
        const QString name = field("name").toString();
        const QString email = field("email").toString();
        m_summaryLabel->setText(
            QString("姓名: %1\n邮箱: %2")
                .arg(name).arg(email));
    }
};
```

field() 可以在任何 QWizardPage 中调用——不需要知道字段是在哪个页面注册的，只要字段名对得上就能读到值。这背后的实现是 QWizard 维护了一个全局的字段映射表，registerField 时往表里写，field 时从表里读。当用户点"上一步"回到之前的页面修改了某个字段的值，后续页面再次进入时通过 initializePage 读取到的就是最新值。

有一种常见的错误用法需要提醒一下：不要在构造函数里调用 field()。因为页面在构造的时候还没有被添加到 QWizard 中，字段映射表还不存在，field() 会返回一个空的 QVariant。field() 的调用应该在 initializePage()、validatePage() 或者用户交互的槽函数中进行——这些时机都在页面被添加到向导之后。

### 3.4 自定义按钮文字与样式

默认情况下 QWizard 的按钮文字是 "Back"、"Next"、"Finish"、"Cancel"（如果系统语言是中文则显示对应翻译）。但很多场景下你希望自定义这些文字——比如把 "Next" 改成 "下一步，选择文件"，或者把 "Cancel" 改成 "放弃所有设置"。

QWizard 提供了 setButtonText 方法来修改按钮文字。你可以在 QWizard 级别设置（对所有页面生效），也可以在 QWizardPage 级别设置（只对该页面生效，优先级更高）：

```cpp
QWizard wizard;
wizard.setButtonText(
    QWizard::NextButton, "下一步");
wizard.setButtonText(
    QWizard::BackButton, "上一步");
wizard.setButtonText(
    QWizard::FinishButton, "完成配置");
wizard.setButtonText(
    QWizard::CancelButton, "取消");
```

如果你想让某个特定页面的按钮文字不一样——比如最后一步的 "Finish" 改成 "开始使用"——可以在该页面的 initializePage 中调用 wizard()->setButtonText：

```cpp
void ConclusionPage::initializePage()
{
    wizard()->setButtonText(
        QWizard::FinishButton, "开始使用");
}
```

除了按钮文字，QWizard 还支持设置各种位置的图片来定制外观。setPixmap 可以设置的图片位置包括：WatermarkPixmap（左侧水印，只在 ClassicStyle/ModernStyle 下显示）、BannerPixmap（顶部横幅，只在 ModernStyle 下显示）、LogoPixmap（页面标题旁的 Logo）、BackgroundPixmap（整个向导的背景图片，只在 MacStyle 下显示）。不同风格下可用的图片位置不同，设置之前最好确认你使用的 WizardStyle 支持哪个位置。

```cpp
wizard.setWizardStyle(QWizard::ModernStyle);
wizard.setPixmap(
    QWizard::BannerPixmap,
    QPixmap(":/images/banner.png"));
wizard.setPixmap(
    QWizard::LogoPixmap,
    QPixmap(":/images/logo.png"));
```

如果你想要更彻底的样式控制，可以通过 QWizard 的 stylesheet 来覆盖默认的视觉表现。QWizard 内部的控件都有对应的 object name（比如 "qt_wizard_title" 是标题 QLabel，"qt_wizard_subtitle" 是副标题 QLabel），你可以用这些 object name 在 QSS 中精确控制每个元素的样式。

还有一个有用的选项是 setOption(QWizard::HaveHelpButton, true)，它会在向导底部添加一个"帮助"按钮。点击帮助按钮时发射 helpRequested 信号，你可以连接这个信号来显示上下文相关的帮助内容。对于配置项比较多的向导来说，帮助按钮能显著降低用户的认知负担。

## 4. 踩坑预防

第一个坑是 registerField 的 mandatory 星号只对控件的默认属性有效。星号机制通过检查属性的 toString() 是否为空来判断完整性。如果你注册的是一个自定义属性或者控件的数值属性（比如 QSpinBox 的 value），即使加了星号也不会按预期工作——因为数值属性的 toString() 永远不为空（QSpinBox 的 value 默认是 0）。对于这类情况，你需要手动重写 isComplete() 方法来实现完整性判断。

第二个坑是 validatePage 和 isComplete 的混淆。isComplete 控制按钮的启用/禁用状态，validatePage 控制点击后是否放行。如果你在 validatePage 中做了复杂的校验逻辑，但忘记在 isComplete 中反映校验结果的变化，用户会看到一个可点击的"下一步"按钮——但点了之后没反应。这时候用户会很困惑。所以如果验证逻辑有延迟或者需要手动触发重新评估，记得在状态变化时 emit completeChanged()。

第三个坑是非线性导航的实现。如果你的向导不是简单的线性流程（第一步->第二步->第三步），而是有分支跳转，必须重写 QWizardPage::nextId() 来返回正确的目标页面 ID。同时要确保用 setPage 手动管理 ID，而不是依赖 addPage 的自动分配——因为分支跳转需要你知道目标页面的确切 ID。还有一个细节：非线性向导的"上一步"行为由 QWizard 内部的历史栈管理，用户点"上一步"会回到上一页（不是 ID-1 那一页），所以不需要手动处理"回退到哪里"的问题。

第四个坑是字段值的生命周期。当用户点"上一步"回到之前的页面修改了某个字段，后续页面的 initializePage 会被重新调用，此时 field() 读取的是修改后的最新值。但如果你在 initializePage 之外的地方缓存了字段值（比如把 field("name") 存到了一个成员变量里），这个缓存值不会自动更新——你需要在 initializePage 中重新读取。所以尽量不要缓存 field 的返回值，每次需要的时候直接调用 field()。

第五个坑是 QWizard::exec() 的模态行为。exec() 是阻塞调用——它会启动一个局部事件循环，直到用户点击"完成"或"取消"才返回。这意味着在 exec() 返回之前，主窗口的事件循环被阻塞。如果你的向导需要和主窗口交互（比如实时预览配置效果），不能使用 exec()，应该用 show() 以非模态方式运行向导，然后连接 finished 信号来处理结果。

## 5. 练习项目

我们来做一个综合练习：创建一个"用户注册向导"，包含四个页面。第一步是欢迎页，显示一段介绍文字和一个用户协议复选框（勾选后才能进入下一步）。第二步是用户信息页，包含姓名输入框（必填）和邮箱输入框（必填），使用 registerField 注册为 mandatory 字段。第三步是偏好设置页，根据第二步填写的邮箱域名动态显示推荐内容——initializePage 中读取 field("email")，如果邮箱是 gmail.com 就显示"推荐使用 Google 服务"的提示，如果是 outlook.com 就显示"推荐使用 Microsoft 服务"。第四步是确认页，汇总显示前面所有步骤填写的信息，validatePage 中打印所有字段值到 qDebug。

向导底部的按钮文字自定义为"上一步""下一步""完成注册""放弃"。第三步的"下一步"按钮文字改为"确认偏好"。向导设置 HaveHelpButton 选项，点击帮助按钮时在状态栏显示当前页的帮助提示。

提示：复选框的 checked 属性注册为字段时需要指定第三个参数 "checked" 和第四个参数 SIGNAL(toggled(bool))。邮箱域名的提取可以用 QString::split('@').last()。

## 6. 官方文档参考链接

[Qt 文档 -- QWizard](https://doc.qt.io/qt-6/qwizard.html) -- 向导对话框类

[Qt 文档 -- QWizardPage](https://doc.qt.io/qt-6/qwizardpage.html) -- 向导页面类

[Qt 文档 -- QWizardPage::registerField](https://doc.qt.io/qt-6/qwizardpage.html#registerField) -- 字段注册方法

[Qt 文档 -- QWizardPage::initializePage](https://doc.qt.io/qt-6/qwizardpage.html#initializePage) -- 页面初始化回调

[Qt 文档 -- QWizardPage::validatePage](https://doc.qt.io/qt-6/qwizardpage.html#validatePage) -- 页面验证回调

---

到这里，QWizard 的核心用法就全部讲完了。addPage 组装线性向导流程，initializePage / validatePage 分别在页面显示前和离开前执行初始化与校验逻辑，registerField / field 构成页面间的数据传递通道，setButtonText 和 setPixmap 则让你可以定制向导的外观。掌握了这套机制，无论是安装向导、配置向导还是导入向导，你都能用 QWizard 框架快速搭建出来——不用每次都从 QDialog + QStackedWidget 手搓一套导航逻辑了。
