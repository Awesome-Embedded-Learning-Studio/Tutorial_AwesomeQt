---
title: "3.27 QComboBox 进阶"
description: "入门篇我们学会了 addItem 往 QComboBox 里塞选项、currentIndex 拿当前值、setEditable 开可编辑模式、setModel 挂自定义数据源。但当你真正在产品级界面里用 QComboBox 时，入门篇那些知识很快就会撞墙——下拉弹窗被屏幕边缘截断、自定义 delegate 的 paint 在弹窗里不生效、setModelColumn 切换列后索引全部错位、QCompleter 的弹出列表和 QComboBox 自身的弹窗互相打架。"
---

# 现代Qt开发教程（进阶篇）3.27——QComboBox 进阶

## 1. 前言 / QComboBox 的下拉弹窗远没有看上去那么简单

入门篇我们把 QComboBox 当一个"下拉选择器"来用——addItem 塞选项，currentIndex 拿值，setEditable 开输入，setModel 挂数据源，标准流程走完收工。做内部工具确实够用，但在正式的产品 UI 里——需要自定义外观、挂复杂 Model、弹窗里显示多列数据——你会发现入门篇的 API 突然不好使了。下拉弹窗被屏幕边缘截断，自定义 delegate 在弹窗里不生效，setModelColumn 切列后数据全错，QCompleter 弹窗和 QComboBox 弹窗互相打架。我们今天把这三块拆透：弹窗定位与 showPopup 覆写、自定义委托在弹窗中的绘制、Model 驱动的 QComboBox 与 QCompleter 集成。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QComboBox 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。自定义 delegate 涉及 QStyledItemDelegate，QCompleter 涉及 Qt Widgets 中的 QCompleter 类，均在 QtWidgets 模块内，无需额外链接。

## 3. 核心概念讲解

### 3.1 弹窗定位机制——showPopup 覆写与屏幕边缘处理

QComboBox 的下拉弹窗本质上是一个 QFrame 派生的内部容器，里面嵌了一个 QListView 来显示选项列表。弹窗的定位逻辑在 showPopup() 中实现——默认算法让弹窗出现在 QComboBox 正下方，左对齐，高度等于内容高度。

这个默认算法在大多数情况下工作正常，但当 QComboBox 位于窗口底部或者靠近屏幕边缘时就会出问题。弹窗可能超出屏幕可见区域被截断，用户看不到下面的选项。默认实现确实有屏幕边界检测——它会检查弹窗是否会超出屏幕底部，如果超出就尝试把弹窗改到 QComboBox 上方弹出。但这个检测只覆盖了垂直方向，水平方向的截断处理非常有限。

覆写 showPopup() 可以完全控制弹窗位置。核心思路是先调用父类的 showPopup() 让弹窗创建并显示，然后通过 `view()` 获取内部 QListView，通过容器窗口调整弹窗的 geometry：

```cpp
class SmartComboBox : public QComboBox
{
protected:
    void showPopup() override
    {
        QComboBox::showPopup();  // 先让默认实现创建弹窗

        // 获取弹窗容器
        auto *popup = findChild<QFrame*>();
        if (!popup) return;

        // 计算屏幕可用区域
        auto screen = screen() ? screen()->availableGeometry()
                               : QGuiApplication::primaryScreen()->availableGeometry();

        // 如果弹窗底部超出屏幕，移到上方
        if (popup->geometry().bottom() > screen.bottom()) {
            int top = mapToGlobal(QPoint(0, 0)).y() - popup->height();
            if (top >= screen.top()) {
                popup->move(popup->x(), top);
            }
        }

        // 如果弹窗右侧超出屏幕，左移
        if (popup->geometry().right() > screen.right()) {
            int left = screen.right() - popup->width();
            popup->move(left, popup->y());
        }
    }
};
```

这里有一个细节值得注意：弹窗的宽度。默认情况下 QComboBox 的弹窗宽度等于 QComboBox 自身的宽度。如果你的选项文本很长，超出的部分会被省略号截断。你可以通过 `setSizeAdjustPolicy(QComboBox::AdjustToContents)` 让弹窗自动调整宽度以适应最长的选项文本，或者通过 `setMinimumContentsLength(int)` 设置最小字符宽度。但 AdjustToContents 的计算是基于 Model 中所有项的 sizeHint，如果 Model 很大（几千项），每次显示弹窗都要遍历计算，可能导致弹窗弹出时有明显延迟。折中方案是用 AdjustToMinimumContentsLengthWithIcon 策略加上一个合理的 minimumContentsLength。

### 3.2 自定义委托渲染——弹窗与编辑框的绘制差异

QComboBox 在两个地方调用 delegate 的 paint：弹窗中的 QListView 和 QComboBox 自身的编辑区域。这两个地方的绘制行为有关键差异：弹窗中的 QListView 使用 delegate 的 paint 正常绘制，但非可编辑模式下编辑区域的绘制走的是 QComboBox 自己的内部逻辑——不调用 delegate 的 paint。

这意味着如果你设置了一个自定义 delegate 来实现特殊的选项外观（比如带图标、带颜色标记、带进度条），弹窗中的选项会正确应用你的自定义绘制，但 QComboBox 编辑区域显示的当前选中项仍然是默认样式。这个差异在非可编辑模式下尤其明显。

解决这个差异的一种方式是把 QComboBox 设为可编辑模式（setEditable(true)），然后通过 lineEdit() 获取内部 QLineEdit 设为只读——绕过 QComboBox 的内部绘制，转而使用 delegate。但副作用是交互行为变了（点击文字区域弹出光标而不是下拉弹窗）。

更干净的方案是覆写 QComboBox 的 paintEvent，自己绘制当前选中项：

```cpp
class CustomCombo : public QComboBox
{
protected:
    void paintEvent(QPaintEvent *) override
    {
        auto painter = QStylePainter(this);
        painter.setPen(palette().color(QPalette::Text));

        // 绘制 QComboBox 的基本外观（边框、下拉箭头）
        QStyleOptionComboBox opt;
        initStyleOption(&opt);
        painter.drawComplexControl(QStyle::CC_ComboBox, opt);

        // 自己绘制当前选中项的内容
        if (currentIndex() >= 0) {
            auto index = model()->index(currentIndex(), modelColumn());
            // 使用 delegate 绘制当前项
            QStyleOptionViewItem itemOpt;
            itemOpt.rect = style()->subElementRect(QStyle::SE_ComboBoxFocusRect, &opt, this);
            itemOpt.state = QStyle::State_Enabled;
            itemOpt.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
            itemDelegate()->paint(&painter, itemOpt, index);
        }
    }
};
```

这种做法让你完全控制编辑区域的绘制内容，同时保持弹窗使用 delegate 的标准绘制流程。注意 initStyleOption 必须调用，它负责填充 QStyleOptionComboBox 的各种状态信息（enabled、editable、hasFocus 等），drawComplexControl 依赖这些信息来绘制正确的控件外观。

还有一个和委托相关的坑：如果你在 QComboBox 上调了 `setItemDelegate()`，它会同时影响弹窗 QListView 的 delegate。但 QComboBox 内部还有一个单独的"编辑器 delegate"（通过 setItemDelegateForColumn 在内部 QListView 上设置）。如果你发现弹窗的绘制不符合预期，检查一下是不是被内部的 delegate 覆盖了。

### 3.3 Model 驱动的 QComboBox——setModel + setModelColumn + QCompleter

入门篇我们讲了 setModel 挂 QStandardItemModel 的基本用法。进阶篇我们要处理 setModelColumn 和 QCompleter 集成这两个更复杂的场景。

`setModelColumn(int)` 告诉 QComboBox 从 Model 的哪一列取数据作为"显示列"。默认第 0 列。但这里有一个特别坑的行为：setModelColumn 改变显示列后，currentIndex 不变，但 currentData 的含义变了。currentData(int role) 内部通过 `model()->index(currentIndex(), modelColumn())` 获取——modelColumn() 充当了列索引。所以如果你在 column=0 存了城市编码，column=1 存了地区名称，切换 modelColumn 后 currentData(Qt::UserRole) 取出来的就不是你期望的数据。

```cpp
// Model 结构: 列 0 = 城市名, 列 1 = 城市编码, 列 2 = 地区
auto *model = new QStandardItemModel(this);
model->setHorizontalHeaderLabels({"城市", "编码", "地区"});

// 添加数据...
combo->setModel(model);
combo->setModelColumn(0);  // 显示城市名

// 用户选择了"北京"（index 0）
combo->currentData(Qt::DisplayRole);  // "北京"——来自列 0
combo->model()->index(0, 1).data().toString();  // "BJ"——需要手动指定列

// 切换显示列
combo->setModelColumn(1);  // 现在显示编码
combo->currentData(Qt::DisplayRole);  // "BJ"——来自列 1
```

安全使用 setModelColumn 的方式是：永远通过 `model()->index(combo->currentIndex(), column)` 来获取其他列的数据，不要依赖 currentData 的隐含列。currentData 只取当前 modelColumn 指定列的数据，如果你需要跨列访问，直接操作 Model 的 index。

接下来是 QCompleter 集成。QComboBox 在可编辑模式下自动内置了一个 QCompleter，用于输入时自动补全。默认的补全行为是大小写不敏感的前缀匹配，从 QComboBox 的 Model 中查找匹配项。你可以通过 `completer()` 获取这个内置的 QCompleter 并自定义它的行为：

```cpp
combo->setEditable(true);
auto *c = combo->completer();
c->setCaseSensitivity(Qt::CaseInsensitive);
c->setFilterMode(Qt::MatchContains);  // 包含匹配，不只是前缀
c->setCompletionMode(QCompleter::PopupCompletion);
```

但这里有一个交互层面的问题：QComboBox 自己有一个下拉弹窗，QCompleter 也有一个弹窗。两个弹窗可能同时出现，视觉上非常混乱。处理方案是监听 QCompleter 的信号来控制 QComboBox 的弹窗——当 QCompleter 弹窗显示时隐藏 QComboBox 的下拉，或者更简单地设 insertPolicy 为 NoInsert，把自动补全完全交给 QCompleter 处理。

现在有一道调试题给大家。下面这段代码在 setModel 之后调用 setModelColumn，但 currentData 始终返回无效的 QVariant，为什么？

```cpp
auto *model = new QStandardItemModel(this);
// ... 填充 model 数据 ...
combo->setModel(model);
combo->setModelColumn(1);  // 切到第二列
int idx = combo->currentIndex();
auto data = combo->currentData(Qt::UserRole);  // 返回无效 QVariant
```

问题出在 setModelColumn(1) 之后，QComboBox 从 Model 的第二列取数据。如果第二列的数据没有被设置 Qt::UserRole 角色的值（只有 Qt::DisplayRole 有值），currentData(Qt::UserRole) 当然返回无效 QVariant。setModelColumn 改变了 QComboBox "看"的列，你必须确保那一列确实有你需要的 role 数据。

## 4. 踩坑预防

第一个坑是弹窗被屏幕边缘截断。默认的 showPopup 只做简单的垂直方向边界检测，水平方向的处理非常有限。后果是靠近屏幕右边缘的 QComboBox 弹窗可能被截掉一半，用户无法看到完整选项。解决方案是覆写 showPopup，在父类实现之后手动检查弹窗 geometry 是否超出屏幕可用区域，超出就调整位置。

第二个坑是自定义 delegate 的 paint 在非可编辑 QComboBox 的编辑区域不被调用。弹窗里的选项绘制正常，但 QComboBox 自身显示的当前选中项仍然是默认样式。后果是用户选中某个选项后，弹窗关闭，QComboBox 显示的内容和你精心设计的自定义绘制不一致。解决方案是覆写 paintEvent 手动绘制当前选中项，或者将 QComboBox 设为可编辑但 lineEdit 只读。

第三个坑是 setModelColumn 之后 currentData 的隐含列变了。currentData 总是从 modelColumn() 指定的列取数据，如果你在其他列存了关联数据，切换 modelColumn 后 currentData 取出来的就不是你期望的数据。后果是业务逻辑用到 currentData 时拿到错误的值，导致后续处理出错。解决方案是通过 model()->index(currentIndex(), targetColumn) 显式指定列来获取数据，不依赖 currentData 的隐含列语义。

## 5. 练习项目

练习项目：多列数据选择器。实现一个 QComboBox，挂载三列 QStandardItemModel（城市名称、编码、地区），弹窗显示三列完整信息，编辑区域只显示名称。用 QStyledItemDelegate 子类实现三列布局绘制（名称左对齐、编码居中、地区右对齐），覆写 showPopup 防截断，setModelColumn(0) 控制显示列，通过 model()->index() 显式获取数据。完成标准是弹窗三列正确显示不被截断，选中后编辑区域只显示名称，切换选项后能正确获取编码和地区。提示：delegate 的 paint 中用 rect 分三段绘制；showPopup 中用 availableGeometry 做边界检查；数据获取用 model()->index(combo->currentIndex(), column) 替代 currentData。

## 6. 官方文档参考链接

[Qt 文档 · QComboBox](https://doc.qt.io/qt-6/qcombobox.html) -- 下拉选择框，包含 showPopup、setModelColumn、setSizeAdjustPolicy 等进阶 API

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 标准项委托，用于自定义选项绘制

[Qt 文档 · QCompleter](https://doc.qt.io/qt-6/qcompleter.html) -- 自动补全控件，与 QComboBox 的可编辑模式集成

[Qt 文档 · QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 通用标准项模型，多列数据管理

---

到这里，QComboBox 的进阶内容我们就过了一遍。showPopup 覆写解决了屏幕边缘截断问题，自定义 delegate 的绘制差异通过 paintEvent 覆写统一了，setModelColumn 的隐含列语义搞明白了，QCompleter 集成的弹窗冲突也有了处理方案。下一篇我们继续 QtWidgets 的进阶之旅。
