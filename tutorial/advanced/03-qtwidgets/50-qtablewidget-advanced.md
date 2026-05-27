---
title: "3.50 QTableWidget 进阶"
description: "入门篇我们用 QTableWidget 搭了二维表格界面，掌握了 setRowCount/setColumnCount/setItem/cellChanged 信号、水平/垂直表头设置，以及单元格中嵌入 QLineEdit/QComboBox 等 widget 的方法。"
---

# 现代Qt开发教程（进阶篇）3.50——QTableWidget 进阶

## 1. 前言 / 当表格不再只是"格子填文字"

入门篇我们用 QTableWidget 搭了二维表格界面，掌握了 setRowCount / setColumnCount / setItem / cellChanged 信号、水平/垂直表头设置，以及单元格中嵌入 QLineEdit/QComboBox 等 widget 的方法。对于一个几十行几列的简单数据表格，入门篇那一套完全够用。但当你需要做"合并单元格"（比如表头跨列合并）、"冻结首行首列"（滚动时表头和首列固定不动）这类 Excel 级别的表格功能时，QTableWidget 的基础接口就不够了——这些功能需要组合使用多个 QTableWidget 的特性，甚至需要多个 QTableWidget 配合。

今天我们把 QTableWidget 的两个高级能力拆透：单元格合并和冻结首行首列。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QTableWidget 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。冻结首行首列涉及多视图和同步滚动。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 单元格合并——setSpan

QTableWidget::setSpan(int row, int col, int rowSpan, int colSpan) 将从 (row, col) 开始的 rowSpan 行、colSpan 列的单元格合并为一个。合并后只有左上角的单元格保留内容和格式，其余被合并的单元格变为不可见。

```cpp
// 合并第一行的前三列作为一个大表头
tableWidget->setSpan(0, 0, 1, 3);

// 合并一个 2x2 的区域
tableWidget->setSpan(2, 1, 2, 2);
```

setSpan 的参数中 row 和 col 是合并起始位置（左上角），rowSpan 和 colSpan 是跨几行/列。设 rowSpan=1, colSpan=3 表示合并同一行的 3 列。设 rowSpan=2, colSpan=2 表示合并 2x2 的区域。

合并后的单元格的内容由左上角的 QTableWidgetItem 决定。被合并的单元格（非左上角的那些）的 item 虽然存在但不可见——调用 item(row, col) 仍然能拿到它们的数据，但它们不会被渲染。

```cpp
// 设置合并单元格的内容
auto *merged_item = tableWidget->item(0, 0);
merged_item->setText("2024 年度销售汇总");
merged_item->setTextAlignment(Qt::AlignCenter);
merged_item->setFont(QFont("", 14, QFont::Bold));
```

取消合并用 setSpan(row, col, 1, 1)——把 span 重置为 1x1 即可。也可以用 clearSpans() 一次性取消所有合并。

setSpan 有几个需要注意的行为。第一，合并范围不能重叠——如果你先 setSpan(0, 0, 1, 3) 合并了前三列，再 setSpan(0, 2, 1, 2) 试图从第二列开始再合并两列，第二个 setSpan 的行为是未定义的——你可能看到合并区域混乱。所以在设计合并逻辑时要先规划好整个合并方案，确保不重叠。

第二，合并后行高和列宽由左上角单元格的内容和大小决定。如果合并了一个 1x3 的区域但内容文字很短，合并后的单元格看起来就是左边有一大块文字、右边全是空白。你需要设置合并项的 textAlignment 为 AlignCenter 并且适当调宽列。

第三，排序（setSortingEnabled）和合并不能很好地配合。当用户点击列头排序时，行的顺序会变化，但 span 是绑定在 (row, col) 位置上的——排序后 span 的位置不变，但行内容变了，导致合并区域和内容错位。如果你的表格同时需要合并和排序，排序时需要先 clearSpans()，排序完成后再重新设置 span。

### 3.2 冻结首行首列

QTableWidget 本身没有"冻结行/列"的接口。但 Qt 官方示例（Frozen Column Example）展示了一个经典的实现方案：用两个 QTableView（QTableWidget 继承自 QTableView）叠在一起，一个显示完整表格（底层），一个只显示冻结区域（顶层），两者同步滚动。

实现冻结首行的核心思路是：创建一个 QTableWidget 作为主表格，再创建一个 QTableWidget（或者 QHeaderView）作为冻结的行表头。冻结表头覆盖在主表格上方，位置始终在视口顶部。当主表格垂直滚动时，冻结表头不跟随滚动——它保持在固定位置。

```cpp
class FrozenTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    FrozenTableWidget(int rows, int cols, QWidget *parent = nullptr)
        : QTableWidget(rows, cols, parent)
    {
        // 冻结的首行：创建一个独立的 QTableWidget
        m_frozen = new QTableWidget(1, columnCount(), this);
        m_frozen->setFocusPolicy(Qt::NoFocus);
        m_frozen->setVerticalScrollMode(ScrollPerPixel);
        m_frozen->setHorizontalScrollMode(ScrollPerPixel);
        m_frozen->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_frozen->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_frozen->setEditTriggers(NoEditTriggers);
        m_frozen->setSelectionModel(selectionModel());

        // 复制首行数据到冻结表
        for (int col = 0; col < columnCount(); ++col) {
            auto *src = item(0, col);
            if (src) {
                m_frozen->setItem(0, col, src->clone());
            }
            m_frozen->setColumnWidth(col, columnWidth(col));
        }

        viewport()->stackUnder(m_frozen);
        update_frozen_geometry();

        connect(horizontalHeader(), &QHeaderView::sectionResized,
                this, &FrozenTableWidget::update_section_width);
        connect(verticalHeader(), &QHeaderView::sectionResized,
                this, &FrozenTableWidget::update_section_height);
        connect(horizontalScrollBar(), &QScrollBar::valueChanged,
                m_frozen->horizontalScrollBar(), &QScrollBar::setValue);
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QTableWidget::resizeEvent(event);
        update_frozen_geometry();
    }

    void scrollContentsBy(int dx, int dy) override
    {
        QTableWidget::scrollContentsBy(dx, dy);
        update_frozen_geometry();
    }

private:
    void update_frozen_geometry()
    {
        // 冻结行始终在视口顶部
        int row_height = rowHeight(0);
        int header_height = horizontalHeader()->height();
        int x = verticalHeader()->width();
        int w = viewport()->width();
        m_frozen->setGeometry(x, header_height, w, row_height);
    }

    void update_section_width(int col, int, int width)
    {
        m_frozen->setColumnWidth(col, width);
    }

    void update_section_height(int, int, int)
    {
        update_frozen_geometry();
    }

    QTableWidget *m_frozen;
};
```

这个方案的核心是两个表共享同一个 SelectionModel——这样选中主表格的某个单元格时，冻结表也会显示对应的高亮。horizontalScrollBar 的 valueChanged 信号连接到冻结表的滚动条——水平滚动时两者同步，垂直滚动时只有主表格滚动（冻结表不接收垂直滚动）。

冻结首列的思路完全一样，只是冻结表显示的是第一列而不是第一行，垂直滚动时同步，水平滚动时不同步。

同时冻结首行和首列需要三个 QTableWidget：完整表格（底层）、冻结行（右上）、冻结列（左下），以及一个冻结左上角交叉区域的小方块。复杂度会明显上升。

## 4. 踩坑预防

第一个坑是 setSpan 和排序的冲突。开启 setSortingEnabled(true) 后用户点击列头排序，行的顺序会变，但 span 的位置是按 (row, col) 索引绑定的。排序后 span 还在原来的索引位置，但那一行的内容已经不是合并时对应的行了。后果是合并区域出现在错误的内容上——比如"2024 年度汇总"出现在某一行数据上而不是表头区域。解决方案是排序前 clearSpans()，排序后重新 setSpan。

第二个坑是冻结表的 selectionModel 同步。如果冻结表和主表不共享 selectionModel，用户在冻结表中点击的选中和主表不联动——看起来像是两套独立的表格。必须在构造时把主表的 selectionModel() 传给冻结表。

第三个坑是冻结表的列宽和主表不同步。如果用户拖动列边界调整了主表的列宽，冻结表的对应列宽没有跟着变——冻结表和主表的列错位。解决方案是连接 horizontalHeader 的 sectionResized 信号，在信号槽中同步冻结表的列宽。

## 5. 练习项目

练习项目：带冻结行和单元格合并的成绩单。我们要实现一个专业级的成绩展示表格。

完成标准是：QTableWidget 显示学生成绩表，共 6 行（1 行表头 + 5 行数据）、5 列（姓名、语文、数学、英语、总分）。表头行的前三个列（语文、数学、英语）合并为一个单元格，显示"科目成绩"。使用 setSpan 合并。水平表头固定（不随垂直滚动），使用 QTableWidget 自带的 horizontalHeader（QTableWidget 默认就冻结了水平表头，不需要额外实现）。首列（姓名）冻结——创建一个独立的 QTableWidget 只显示第一列，垂直滚动时保持固定。总分列自动计算（语文+数学+英语），在单元格内容变化时更新。

提示几个关键点：setSpan(0, 1, 1, 3) 合并科目区域；cellChanged 信号中读取三科成绩计算总分；冻结列使用独立 QTableWidget + 共享 selectionModel + 同步垂直滚动。

## 6. 官方文档参考链接

[Qt 文档 · QTableWidget](https://doc.qt.io/qt-6/qtablewidget.html) -- 表格控件，包含 setSpan/clearSpans/setCellWidget 等接口

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- 表格视图基类，scrollContentsBy 可用于冻结行/列

[Qt 文档 · QTableWidgetItem](https://doc.qt.io/qt-6/qtablewidgetitem.html) -- 表格项，clone() 用于复制到冻结表

[Qt 文档 · Frozen Column Example](https://doc.qt.io/qt-6/qtwidgets-itemviews-frozencolumn-example.html) -- Qt 官方冻结列示例

---

到这里，QTableWidget 的进阶内容就拆完了。setSpan 做合并单元格很直接，但和排序不兼容——排序前必须 clearSpans。冻结首行首列的方案是多个 QTableView 叠加 + 同步滚动 + 共享 SelectionModel，虽然代码量不小但思路清晰。掌握了这两项能力后，你就能做出接近 Excel 体验的表格界面——而不只是"能填数据的格子"。
