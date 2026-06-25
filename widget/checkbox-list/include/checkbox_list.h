/**
 * @file checkbox_list.h
 * @brief 勾选列表控件 CheckboxList——封装 QListWidget，每项带复选框的扁平勾选列表
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 *
 * 对照 CheckboxTree：本控件扁平无层级，重点在「勾选 API + 状态汇总」，
 * 不做父子联动（那是 CheckboxTree 的事）。组合 QListWidget，默认不重写
 * paintEvent，让 view 自己画。
 */
#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>

class QListWidget;
class QListWidgetItem;

namespace AwesomeQt {

/// @brief 扁平勾选列表：每项一个复选框，提供勾选/批量操作/状态汇总 API。
///
/// 设计要点：
/// - 内含一个 `QListWidget*`（构造期 new，parent=this 由对象树托管），本控件不自绘，
///   只负责「数据 + 交互逻辑」；
/// - 勾选状态变化靠 `QListWidget::itemChanged` 驱动，转发为更易用的 `checkedChanged`；
/// - `checkAll`/`uncheckAll`/`invertChecked` 等批量改写会逐项 `setCheckState`，每改一项
///   都触发 `itemChanged`，必须 `blockSignals` 守卫防信号雪崩（同 CheckboxTree 教训，
///   列表版更简单——没有层级递归）。
class CheckboxList : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：alternatingRowColors / spacing，可被 Designer / 外部驱动 ——
    Q_PROPERTY(bool alternatingRowColors READ alternatingRowColors WRITE setAlternatingRowColors
                   NOTIFY alternatingRowColorsChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)

  public:
    explicit CheckboxList(QWidget* parent = nullptr);

    /// @brief 追加一项并装上复选框。
    /// @param text    显示文本。
    /// @param checked 初始是否勾选，默认未勾选。
    /// @return 新建并已挂入列表的项指针。
    QListWidgetItem* addItem(const QString& text, bool checked = false);

    /// @brief 批量追加多项（默认均未勾选）。空列表忽略。
    void addItems(const QStringList& texts);

    /// @brief 设置某项勾选状态（程序化入口，内部守卫信号）。
    /// @param item 目标项，nullptr 安全返回。
    /// @param state 目标勾选状态。
    void setItemChecked(QListWidgetItem* item, Qt::CheckState state);

    /// @brief 全部勾选。
    void checkAll();

    /// @brief 全部取消勾选。
    void uncheckAll();

    /// @brief 反选：勾选的变不勾、不勾的变勾。
    void invertChecked();

    /// @brief 取所有已勾选项的文本（按列表顺序）。空列表返回空列表。
    QStringList checkedTexts() const;

    /// @brief 取所有处于 Checked 状态的项指针（按列表顺序）。空列表返回空列表。
    QList<QListWidgetItem*> checkedItems() const;

    /// @brief 暴露内部 view，供外部只读访问（设置表头 / 读选中项等）。
    /// @return 内部 QListWidget 指针（所有权仍归本控件）。
    QListWidget* listView() const;

    // —— Q_PROPERTY 读写 ——
    bool alternatingRowColors() const;
    void setAlternatingRowColors(bool enabled);

    int spacing() const;
    void setSpacing(int pixels);

    QSize sizeHint() const override;

  signals:
    /// @brief 某项勾选状态变化（用户点击或 setItemChecked 触发，批量操作不发逐项信号）。
    void checkedChanged(QListWidgetItem* item, bool checked);
    void alternatingRowColorsChanged(bool enabled);
    void spacingChanged(int pixels);

  private:
    /// @brief itemChanged 槽：去重 + 边界，转发为 checkedChanged。
    void onItemChanged(QListWidgetItem* item);

    QListWidget* list_{nullptr};
};

} // namespace AwesomeQt
