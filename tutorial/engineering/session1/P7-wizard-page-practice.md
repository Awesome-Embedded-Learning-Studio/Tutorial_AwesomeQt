# 实战练习 · WizardPage — 用 QWizard 搞定多步流程

## 前言：别自己造向导了，Qt 已经给你造好了

安装程序、数据导入向导、首次运行设置——这些多步骤流程在桌面应用里太常见了。如果你要自己从头实现，得处理一堆事情：页面堆栈、前进/后退按钮的状态管理、页面间数据传递、最后一步的"完成"逻辑。自己写一遍你会发现，这些东西的套路性极强，每做一个向导都是在重复劳动。

Qt 的 QWizard 就是为了消灭这种重复劳动而存在的。它帮你管好了页面导航、按钮状态、甚至页面间的数据共享机制（registerField/field）。你要做的只是继承 QWizardPage，写每一页的内容，然后 addPage 到 QWizard 里就完事了。这篇练习我们就来把 QWizard 的核心流程走一遍。

---

## 出发前的装备清单

- **QWizard** — 向导框架，管理页面导航、按钮、标题区域。继承自 QDialog，所以用 exec() 启动。
- **QWizardPage** — 单个向导页面的基类。你可以通过 setTitle/setSubTitle 设置标题，通过 registerField 注册字段供后续页面读取。
- **QWizard 的字段系统** — registerField() + field() 实现跨页面数据传递。这是 QWizard 最强大的功能，不需要你手动传递数据。
- **QWizardPage::isComplete()** — 虚函数，控制"下一步"按钮是否可用。返回 true 才能翻页。

---

## 我们的目标长什么样

一个三页向导：

```
第 1 页 — 欢迎               第 2 页 — 配置               第 3 页 — 确认
┌─────────────────────┐      ┌─────────────────────┐      ┌─────────────────────┐
│ 欢迎                 │      │ 配置信息             │      │ 确认                 │
│                     │      │                     │      │                     │
│ 欢迎使用数据导入向导 │      │ 项目名称: [____]    │      │ 您的配置如下：       │
│                     │      │ 描述:     [____]    │      │ 名称: MyProject     │
│ 点击"下一步"开始    │      │                     │      │ 描述: 测试项目       │
│                     │      │                     │      │                     │
│    [下一步] [取消]   │      │   [上一步] [下一步]  │      │   [上一步] [完成]    │
└─────────────────────┘      └─────────────────────┘      └─────────────────────┘
```

完成标准：三页导航正确，第二页的输入字段为空时"下一步"按钮禁用，第三页展示第二页填写的数据。

---

## 第一步 — 创建 QWizard 实例

### 思考题

QWizard 有三种内置风格：ClassicStyle（Windows 传统）、ModernStyle（带侧边栏的）、MacStyle（macOS 原生）。在 Linux 上默认用哪种？你觉得哪种最适合这个练习？

### 动手写

在 main.cpp 里创建 QWizard：

```cpp
#include <QApplication>
#include <QWizard>
// TODO: #include 你的三个页面头文件

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWizard wizard;
    wizard.setWindowTitle("数据导入向导");
    wizard.setWizardStyle(QWizard::ModernStyle);

    // TODO: wizard.addPage(new WelcomePage);
    // TODO: wizard.addPage(new ConfigPage);
    // TODO: wizard.addPage(new SummaryPage);

    wizard.exec();
    return 0;
}
```

addPage 的顺序决定了页面的导航顺序——用户点"下一步"就会从第 1 页到第 2 页到第 3 页。

### 检查点

先不写页面类，直接编译会报错。没关系，下一步我们就开始写页面。

---

## 第二步 — 欢迎页：最简单的 QWizardPage

### 动手写

欢迎页什么都不做，只显示欢迎文本：

```cpp
#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QWizardPage>

class WelcomePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit WelcomePage(QWidget *parent = nullptr)
        : QWizardPage(parent)
    {
        // TODO: setTitle("欢迎")
        //       setSubTitle("欢迎使用数据导入向导")

        // TODO: 创建 QLabel 放欢迎文本
        //       添加到 QVBoxLayout
        //       setLayout(layout)
    }
};
```

QWizardPage 的 setTitle 和 setSubtitle 会在 QWizard 的标题区域自动渲染——你不需要自己画标题。你只需要在中央区域放内容。

### 检查点

编译运行。你应该能看到一个向导窗口，标题区域显示"欢迎"，中间有欢迎文本，底部有"下一步"和"取消"按钮。点"下一步"暂时没反应（因为还没加第二页）。

---

## 第三步 — 配置页：输入字段 + registerField + isComplete

### 思考题

这是整个练习最关键的一步。QWizard 的字段系统让页面间共享数据变得很简单：你在配置页用 `registerField("fieldName*", widget)` 注册字段（注意那个星号），然后在摘要页用 `field("fieldName")` 读取值。问题是：那个星号 `*` 是什么意思？去 Qt 文档查一下 `registerField` 的说明。提示：星号和"下一步"按钮的启用状态有关。

### 动手写

```cpp
#pragma once
#include <QLineEdit>
#include <QFormLayout>
#include <QWizardPage>

class ConfigPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ConfigPage(QWidget *parent = nullptr)
        : QWizardPage(parent)
    {
        setTitle("配置信息");
        setSubTitle("请填写项目基本信息");

        // TODO: 创建 QLineEdit，比如 m_nameEdit 和 m_descEdit
        // TODO: 用 QFormLayout 排列
        // TODO: setLayout(formLayout)

        // TODO: 注册字段
        //       registerField("projectName*", m_nameEdit);
        //       registerField("projectDesc", m_descEdit);
        //       注意 projectName 后面的星号！
        //       星号表示这个字段是"mandatory"（必填），
        //       为空时"下一步"按钮自动禁用。
    }

private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_descEdit;
};
```

关于那个星号：`registerField("projectName*", widget)` 中的 `*` 告诉 QWizard，这个字段是必填的。当字段为空时，QWizard 会自动禁用"下一步"按钮。这个机制是通过 QWizardPage::isComplete() 实现的——默认的 isComplete() 检查所有带星号的字段是否非空。

如果你想自定义验证逻辑（比如邮箱格式），可以重写 isComplete()。但别忘了：当你觉得完整状态发生变化时，必须 `emit completeChanged()`，否则 QWizard 不会重新检查——这是一个超级经典的坑。

### 你可能会遇到的坑

registerField 的第一个参数是字段名，是一个字符串。QWizard 内部用 QHash 存这些字段。如果你在两个不同的页面用相同的字段名注册，后注册的会覆盖前面的——不会报错，但数据会串。命名的时候注意不要撞车。

### 检查点

现在运行向导应该有三页了。第二页有两个输入框，项目名称为空时"下一步"按钮是灰色不可点的。输入一些文字后"下一步"变为可用。

---

## 第四步 — 摘要页：读取前页字段

### 思考题

摘要页需要在初始化时读取前页的字段值。但有个问题：摘要页的 initializePage() 被调用时，前页的控件还存在吗？字段值已经注册了吗？提示：QWizard 保证在显示某一页之前调用该页的 initializePage()，此时前页的所有字段都已经在 QWizard 的字段表里了。

### 动手写

```cpp
#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QWizardPage>

class SummaryPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit SummaryPage(QWidget *parent = nullptr)
        : QWizardPage(parent)
    {
        setTitle("确认");
        setSubTitle("请确认以下信息");

        // TODO: 创建 QLabel 用于展示摘要
        //       m_summaryLabel = new QLabel
        //       m_summaryLabel->setWordWrap(true)
        //       setLayout(new QVBoxLayout with the label)
    }

    // 重写 initializePage —— 每次显示这一页时调用
    void initializePage() override
    {
        // TODO: 读取字段
        //       QString name = field("projectName").toString();
        //       QString desc = field("projectDesc").toString();
        //
        //       field() 返回 QVariant，需要 .toString() 转换
        //
        // TODO: 格式化并设置到 m_summaryLabel
    }

private:
    QLabel *m_summaryLabel;
};
```

`field("projectName")` 返回的是 QVariant，你需要用 `.toString()` 把它转成 QString。对于 QLineEdit 注册的字段，QVariant 里存的值就是 QLineEdit 的 text()。如果你注册的是 QSpinBox，那 field() 返回的是 int，需要 `.toInt()`。类型对应关系在 Qt 文档的 registerField 说明里有完整列表。

### 检查点

在配置页填写项目名称和描述，点下一步到摘要页。摘要页应该正确显示你填写的内容。如果显示为空，检查字段名是否和 registerField 的第一个参数完全一致（区分大小写）。

---

## 最终组装

main.cpp 在第一步已经写好了。现在编译运行完整流程：欢迎页 → 配置页（必填验证）→ 摘要页（展示数据）→ 点完成。向导关闭后 exec() 返回 QDialog::Accepted。

你可以加一个检查：

```cpp
if (wizard.exec() == QDialog::Accepted) {
    // 向导完成后可以读取所有字段
    qDebug() << "项目名:" << wizard.field("projectName").toString();
    qDebug() << "描述:" << wizard.field("projectDesc").toString();
}
```

注意 QWizard 本身也可以调用 field()——因为字段是注册在 QWizard 的字段表里的，不是某个页面的私有数据。

---

## 验收标准

向导三页导航正确：下一步、上一步、取消都能正常工作。配置页项目名称为空时下一步按钮禁用，填写后启用。摘要页正确显示配置页填写的数据。点完成返回 Accepted，点取消返回 Rejected。从摘要页点"上一步"回到配置页时，之前填写的数据应该还在（QWizard 自动保存控件状态）。

---

## 进阶挑战

重写 isComplete() 做更复杂的验证——比如项目名称必须是 3 个字符以上，或者不能包含特殊字符。注意：改变验证状态后必须 emit completeChanged()。或者加一个第四页"进度页"，模拟一个耗时操作（用 QTimer 模拟进度条），完成后自动跳到"完成"按钮。再或者实现条件跳转：配置页加一个 QCheckBox "跳过摘要页"，勾选后直接从配置页跳到完成，跳过摘要页——提示：重写 QWizardPage::nextId()。

---

## 踩坑预防清单

> **坑 #1：isComplete() 改变后不 emit completeChanged()**
> 如果你重写了 isComplete() 但忘记在验证状态变化时 emit completeChanged()，QWizard 不会重新调用 isComplete()，"下一步"按钮的启用状态不会更新。这是 QWizard 使用中最常见的坑，Qt 文档里有写但很多人不读。

> **坑 #2：registerField 字段名拼写错误**
> field("projectName") 和 registerField("projectName*") 的字段名必须完全一致。多一个空格、大小写不对、忘记星号，都会导致 field() 返回空 QVariant。而且这种错误没有任何编译期或运行时警告。

> **坑 #3：initializePage 的调用时机**
> initializePage() 在页面**即将显示**时被调用，不是在构造时。所以你不能在构造函数里调 field()——那时候前页的数据还不存在。如果你需要根据前页数据初始化界面，必须放在 initializePage() 里。

---

## 官方文档参考

- [QWizard Class](https://doc.qt.io/qt-6/qwizard.html) — 向导框架
- [QWizardPage Class](https://doc.qt.io/qt-6/qwizardpage.html) — registerField、field、isComplete

到这里就大功告成了。QWizard 的字段系统虽然 API 不多，但一旦掌握了 registerField + field + isComplete 这个三角组合，几乎任何多步流程都能优雅地实现。接下来我们做两个实用的工具应用，从 Base64 编解码开始。
