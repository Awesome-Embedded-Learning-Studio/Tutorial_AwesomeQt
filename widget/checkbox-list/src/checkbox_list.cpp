/**
 * @file checkbox_list.cpp
 * @brief 勾选列表控件 CheckboxList 实现——扁平勾选 + 状态汇总
 * @copyright Copyright (c) 2026 AwesomeQt. Licensed under MIT.
 */

#include "checkbox_list.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

namespace AwesomeQt {

CheckboxList::CheckboxList(QWidget* parent) : QWidget(parent) {
    // 内含一个 QListWidget，构造期 new、parent=this 由对象树托管，放进布局。
    list_ = new QListWidget(this);
    list_->setUniformItemSizes(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(list_);

    // 函数指针语法连接，避免 SIGNAL/SLOT 宏。
    connect(list_, &QListWidget::itemChanged, this, &CheckboxList::onItemChanged);
}

QListWidgetItem* CheckboxList::addItem(const QString& text, bool checked) {
    auto* item = new QListWidgetItem(text, list_);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

    // setCheckState 会触发 itemChanged；这里是初始化，守卫防回灌。
    const bool was_blocked = list_->blockSignals(true);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    list_->blockSignals(was_blocked);

    list_->addItem(item);
    return item;
}

void CheckboxList::addItems(const QStringList& texts) {
    // 批量逐项初始化，整段 blockSignals 守卫，避免每个 setCheckState 都回灌。
    const bool was_blocked = list_->blockSignals(true);
    for (const QString& text : texts) {
        auto* item = new QListWidgetItem(text, list_);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        list_->addItem(item);
    }
    list_->blockSignals(was_blocked);
}

void CheckboxList::setItemChecked(QListWidgetItem* item, Qt::CheckState state) {
    if (item == nullptr) {
        return; // 边界：空指针安全返回。
    }

    // setCheckState 会触发 itemChanged → onItemChanged → checkedChanged；
    // 单项操作允许信号透传（外部就靠它知道状态变了），不守卫。
    item->setCheckState(state);
}

void CheckboxList::checkAll() {
    if (list_ == nullptr) {
        return;
    }
    // 批量逐项 setCheckState，每项都触发 itemChanged，blockSignals 守卫防雪崩。
    const bool was_blocked = list_->blockSignals(true);
    for (int i = 0; i < list_->count(); ++i) {
        QListWidgetItem* item = list_->item(i);
        if (item != nullptr) {
            item->setCheckState(Qt::Checked);
        }
    }
    list_->blockSignals(was_blocked);
}

void CheckboxList::uncheckAll() {
    if (list_ == nullptr) {
        return;
    }
    const bool was_blocked = list_->blockSignals(true);
    for (int i = 0; i < list_->count(); ++i) {
        QListWidgetItem* item = list_->item(i);
        if (item != nullptr) {
            item->setCheckState(Qt::Unchecked);
        }
    }
    list_->blockSignals(was_blocked);
}

void CheckboxList::invertChecked() {
    if (list_ == nullptr) {
        return;
    }
    // 反选读旧态再写新态：若不守卫，写 Checked 会触发 itemChanged，逻辑无害但噪音大。
    const bool was_blocked = list_->blockSignals(true);
    for (int i = 0; i < list_->count(); ++i) {
        QListWidgetItem* item = list_->item(i);
        if (item == nullptr) {
            continue;
        }
        const bool on = item->checkState() == Qt::Checked;
        item->setCheckState(on ? Qt::Unchecked : Qt::Checked);
    }
    list_->blockSignals(was_blocked);
}

QStringList CheckboxList::checkedTexts() const {
    QStringList result;
    if (list_ == nullptr) {
        return result;
    }
    const int n = list_->count();
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
        QListWidgetItem* item = list_->item(i);
        if (item != nullptr && item->checkState() == Qt::Checked) {
            result.append(item->text());
        }
    }
    return result;
}

QList<QListWidgetItem*> CheckboxList::checkedItems() const {
    QList<QListWidgetItem*> result;
    if (list_ == nullptr) {
        return result;
    }
    const int n = list_->count();
    for (int i = 0; i < n; ++i) {
        QListWidgetItem* item = list_->item(i);
        if (item != nullptr && item->checkState() == Qt::Checked) {
            result.append(item);
        }
    }
    return result;
}

QListWidget* CheckboxList::listView() const {
    return list_;
}

bool CheckboxList::alternatingRowColors() const {
    return list_ != nullptr && list_->alternatingRowColors();
}

void CheckboxList::setAlternatingRowColors(bool enabled) {
    if (list_ == nullptr) {
        return;
    }
    if (list_->alternatingRowColors() == enabled) {
        return; // 无变化不发 NOTIFY。
    }
    list_->setAlternatingRowColors(enabled);
    emit alternatingRowColorsChanged(enabled);
}

int CheckboxList::spacing() const {
    return list_ != nullptr ? list_->spacing() : 0;
}

void CheckboxList::setSpacing(int pixels) {
    if (list_ == nullptr || pixels < 0) {
        return; // 边界：负值无意义，clamp 掉。
    }
    if (list_->spacing() == pixels) {
        return;
    }
    list_->setSpacing(pixels);
    emit spacingChanged(pixels);
}

QSize CheckboxList::sizeHint() const {
    return {200, 240};
}

void CheckboxList::onItemChanged(QListWidgetItem* item) {
    if (item == nullptr) {
        return; // 边界：空指针安全返回。
    }
    // 转发为更易用的 checkedChanged(item, bool)；批量操作的雪崩已由调用方 blockSignals 挡住。
    emit checkedChanged(item, item->checkState() == Qt::Checked);
}

} // namespace AwesomeQt
