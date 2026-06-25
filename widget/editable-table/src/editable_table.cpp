/**
 * @file editable_table.cpp
 * @brief EditableTable 控件实现——委托编辑器 + 校验 + 整表数据往返
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "editable_table.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QSpinBox>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace AwesomeQt {

namespace detail {

// ============================================================================
// ValidatorDelegate —— 按列规格挑编辑器、写回时做范围/空值校验
// ============================================================================

ValidatorDelegate::ValidatorDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void ValidatorDelegate::setColumnSpecProvider(ColumnSpecProvider provider) {
    provider_ = std::move(provider);
}

bool ValidatorDelegate::specFor(int column, int& type, double& min, double& max,
                                QStringList& combo) const {
    if (!provider_) {
        return false;
    }
    return provider_(column, type, min, max, combo);
}

QWidget* ValidatorDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/,
                                         const QModelIndex& index) const {
    int type = static_cast<int>(EditableTable::ColumnType::kText);
    double min = 0, max = 0;
    QStringList combo;
    if (!specFor(index.column(), type, min, max, combo)) {
        type = static_cast<int>(EditableTable::ColumnType::kText);
    }

    switch (type) {
        case static_cast<int>(EditableTable::ColumnType::kInt): {
            auto* box = new QSpinBox(parent);
            // 用 double 存范围，整型列截断为 int
            box->setRange(static_cast<int>(min), static_cast<int>(max));
            return box;
        }
        case static_cast<int>(EditableTable::ColumnType::kDouble): {
            auto* box = new QDoubleSpinBox(parent);
            box->setRange(min, max);
            box->setDecimals(3);
            return box;
        }
        case static_cast<int>(EditableTable::ColumnType::kCombo): {
            auto* combo_box = new QComboBox(parent);
            combo_box->addItems(combo);
            return combo_box;
        }
        case static_cast<int>(EditableTable::ColumnType::kCheck):
            // 勾选列由 setCellWidget(flags) 处理，不弹编辑器
            return nullptr;
        case static_cast<int>(EditableTable::ColumnType::kText):
        default: {
            auto* edit = new QLineEdit(parent);
            return edit;
        }
    }
}

void ValidatorDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    if (!editor) {
        return;
    }
    const QString value = index.data(Qt::EditRole).toString();

    if (auto* box = qobject_cast<QSpinBox*>(editor)) {
        bool ok = false;
        int parsed = value.toInt(&ok);
        box->setValue(ok ? parsed : box->minimum()); // 空值落回最小值，避免留空
        return;
    }
    if (auto* box = qobject_cast<QDoubleSpinBox*>(editor)) {
        bool ok = false;
        double parsed = value.toDouble(&ok);
        box->setValue(ok ? parsed : box->minimum());
        return;
    }
    if (auto* combo = qobject_cast<QComboBox*>(editor)) {
        int idx = combo->findText(value);
        combo->setCurrentIndex(idx >= 0 ? idx : 0); // 不在列表里则回到首项
        return;
    }
    if (auto* edit = qobject_cast<QLineEdit*>(editor)) {
        edit->setText(value);
        return;
    }
}

void ValidatorDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                     const QModelIndex& index) const {
    if (!editor || !model) {
        return;
    }

    if (auto* box = qobject_cast<QSpinBox*>(editor)) {
        // 数值列空值已被 setEditorData 兜成最小值，这里直接写——保证始终合法
        model->setData(index, box->value(), Qt::EditRole);
        return;
    }
    if (auto* box = qobject_cast<QDoubleSpinBox*>(editor)) {
        model->setData(index, box->value(), Qt::EditRole);
        return;
    }
    if (auto* combo = qobject_cast<QComboBox*>(editor)) {
        model->setData(index, combo->currentText(), Qt::EditRole);
        return;
    }
    if (auto* edit = qobject_cast<QLineEdit*>(editor)) {
        const QString text = edit->text();
        // 文本列允许空串；委托不再额外拦截，空值是否非法由业务决定
        model->setData(index, text, Qt::EditRole);
        return;
    }
}

void ValidatorDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                             const QModelIndex& /*index*/) const {
    if (editor) {
        editor->setGeometry(option.rect);
    }
}

} // namespace detail

// ============================================================================
// EditableTable —— 组合 QTableWidget，列类型驱动委托
// ============================================================================

EditableTable::EditableTable(QWidget* parent) : QWidget(parent) {
    table_ = new QTableWidget(this); // parent=this，对象树托管
    table_->setColumnCount(0);
    table_->setRowCount(0);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 委托按列规格挑编辑器；规格回调把列定义喂给 detail 层（避免环引用）
    delegate_ = new detail::ValidatorDelegate(this);
    delegate_->setColumnSpecProvider(
        [this](int column, int& type, double& min, double& max, QStringList& combo) {
            if (column < 0 || column >= columns_.size()) {
                return false;
            }
            const auto& spec = columns_.at(column);
            type = columnTypeToInt(spec.type);
            min = spec.min;
            max = spec.max;
            combo = spec.combo;
            return true;
        });
    table_->setItemDelegate(delegate_);

    // 用函数指针语法连信号槽（禁 SIGNAL/SLOT 宏）
    connect(table_, &QTableWidget::cellChanged, this, &EditableTable::onCellChanged);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(table_);
}

int EditableTable::columnTypeToInt(ColumnType type) const {
    return static_cast<int>(type);
}

void EditableTable::addColumn(const QString& header, ColumnType type, double min, double max,
                              const QStringList& combo) {
    ColumnSpec spec;
    spec.header = header;
    spec.type = type;
    spec.min = min;
    spec.max = max;
    spec.combo = combo;
    columns_.append(spec);

    const int col = table_->columnCount();
    table_->setColumnCount(col + 1);

    // 列标题（QTableWidget 要求先有列再设 header item）
    auto* header_item = new QTableWidgetItem(header);
    table_->setHorizontalHeaderItem(col, header_item);
}

void EditableTable::applyCheckState(int row, int col, Qt::CheckState state) {
    QTableWidgetItem* item = table_->item(row, col);
    if (!item) {
        item = new QTableWidgetItem;
        table_->setItem(row, col, item);
    }
    // 勾选列：可用户勾选 + 文本空，让复选框居中显示
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(state);
}

void EditableTable::addRow() {
    suppress_signal_ = true; // 程序化建项，屏蔽 cellChanged 回灌
    const int row = table_->rowCount();
    table_->setRowCount(row + 1);

    for (int col = 0; col < columns_.size(); ++col) {
        const auto& spec = columns_.at(col);
        switch (spec.type) {
            case ColumnType::kCheck:
                applyCheckState(row, col, Qt::Unchecked);
                break;
            case ColumnType::kCombo:
                if (!spec.combo.isEmpty()) {
                    auto* item = new QTableWidgetItem(spec.combo.first());
                    table_->setItem(row, col, item);
                }
                break;
            case ColumnType::kInt:
            case ColumnType::kDouble: {
                // 默认值取范围下界，保证一建行就合法
                const double default_val = (spec.min <= spec.max) ? spec.min : 0.0;
                auto* item = new QTableWidgetItem(QString::number(default_val, 'f', 3));
                table_->setItem(row, col, item);
                break;
            }
            case ColumnType::kText:
            default:
                table_->setItem(row, col, new QTableWidgetItem(QString()));
                break;
        }
    }
    suppress_signal_ = false;
}

void EditableTable::removeRow(int row) {
    if (table_->rowCount() == 0) {
        return; // 空表，忽略
    }
    if (row < 0 || row >= table_->rowCount()) {
        row = table_->rowCount() - 1; // 越界/默认 → 删最后一行
    }
    table_->removeRow(row);
}

void EditableTable::setData(const QVector<QVector<QVariant>>& rows) {
    suppress_signal_ = true;
    table_->setRowCount(0); // 清行但留列

    const int col_count = columns_.size();
    if (col_count == 0) {
        suppress_signal_ = false;
        return; // 没列就没法填
    }

    table_->setRowCount(rows.size());
    for (int r = 0; r < rows.size(); ++r) {
        const auto& row_data = rows.at(r);
        for (int c = 0; c < col_count && c < row_data.size(); ++c) {
            const auto& spec = columns_.at(c);
            const QVariant& v = row_data.at(c);

            switch (spec.type) {
                case ColumnType::kCheck: {
                    const bool checked =
                        (v.typeId() == QMetaType::Bool) ? v.toBool() : (v.toInt() != 0);
                    applyCheckState(r, c, checked ? Qt::Checked : Qt::Unchecked);
                    break;
                }
                case ColumnType::kCombo: {
                    // 不在下拉项里则取首项兜底
                    QString text = v.toString();
                    if (!spec.combo.contains(text) && !spec.combo.isEmpty()) {
                        text = spec.combo.first();
                    }
                    table_->setItem(r, c, new QTableWidgetItem(text));
                    break;
                }
                case ColumnType::kInt: {
                    bool ok = false;
                    int parsed = v.toInt(&ok);
                    if (!ok) {
                        parsed = static_cast<int>(spec.min);
                    }
                    const int lo = static_cast<int>(spec.min);
                    const int hi = static_cast<int>(spec.max);
                    parsed = std::clamp(parsed, lo, hi); // 越界夹值
                    table_->setItem(r, c, new QTableWidgetItem(QString::number(parsed)));
                    break;
                }
                case ColumnType::kDouble: {
                    bool ok = false;
                    double parsed = v.toDouble(&ok);
                    if (!ok) {
                        parsed = spec.min;
                    }
                    const double lo = std::min(spec.min, spec.max);
                    const double hi = std::max(spec.min, spec.max);
                    parsed = std::clamp(parsed, lo, hi);
                    table_->setItem(r, c, new QTableWidgetItem(QString::number(parsed, 'f', 3)));
                    break;
                }
                case ColumnType::kText:
                default:
                    table_->setItem(r, c, new QTableWidgetItem(v.toString()));
                    break;
            }
        }
        // 该行某列没给数据（row_data 比 col_count 短）——填占位项，避免 item==null
        for (int c = row_data.size(); c < col_count; ++c) {
            table_->setItem(r, c, new QTableWidgetItem(QString()));
        }
    }
    suppress_signal_ = false;
}

QVector<QVector<QVariant>> EditableTable::data() const {
    QVector<QVector<QVariant>> result;
    const int row_count = table_->rowCount();
    const int col_count = columns_.size();
    if (col_count == 0 || row_count == 0) {
        return result; // 空表 / 无列 → 空
    }

    for (int r = 0; r < row_count; ++r) {
        QVector<QVariant> row;
        row.reserve(col_count);
        for (int c = 0; c < col_count; ++c) {
            const auto& spec = columns_.at(c);
            const QTableWidgetItem* item = table_->item(r, c);

            if (spec.type == ColumnType::kCheck) {
                const Qt::CheckState state = item ? item->checkState() : Qt::Unchecked;
                row.append(state == Qt::Checked);
                continue;
            }

            // 其余类型按 EditRole 文本还原
            const QString text = item ? item->text() : QString();
            switch (spec.type) {
                case ColumnType::kInt: {
                    bool ok = false;
                    const int parsed = text.toInt(&ok);
                    row.append(ok ? QVariant(parsed) : QVariant());
                    break;
                }
                case ColumnType::kDouble: {
                    bool ok = false;
                    const double parsed = text.toDouble(&ok);
                    row.append(ok ? QVariant(parsed) : QVariant());
                    break;
                }
                case ColumnType::kCombo:
                case ColumnType::kText:
                default:
                    row.append(text);
                    break;
            }
        }
        result.append(row);
    }
    return result;
}

void EditableTable::clear() {
    suppress_signal_ = true;
    table_->setRowCount(0); // 留列定义
    suppress_signal_ = false;
}

int EditableTable::currentRow() const {
    return table_->currentRow();
}

void EditableTable::resizeColumnsToContents() {
    table_->resizeColumnsToContents();
}

void EditableTable::setEditable(bool editable) {
    if (editable_ == editable) {
        return;
    }
    editable_ = editable;
    // 切编辑触发：只读 vs 双击/选中/回车均可编辑
    table_->setEditTriggers(
        editable_ ? (QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked |
                     QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed)
                  : QAbstractItemView::NoEditTriggers);
    emit editableChanged(editable_);
}

bool EditableTable::isEditable() const {
    return editable_;
}

void EditableTable::setGridVisible(bool visible) {
    if (table_->showGrid() == visible) {
        return;
    }
    table_->setShowGrid(visible);
    emit gridVisibleChanged(visible);
}

bool EditableTable::gridVisible() const {
    return table_->showGrid();
}

void EditableTable::setAlternatingRowColors(bool enabled) {
    if (table_->alternatingRowColors() == enabled) {
        return;
    }
    table_->setAlternatingRowColors(enabled);
    emit alternatingRowColorsChanged(enabled);
}

bool EditableTable::alternatingRowColors() const {
    return table_->alternatingRowColors();
}

QSize EditableTable::sizeHint() const {
    return QSize(360, 240);
}

void EditableTable::onCellChanged(int row, int col) {
    if (suppress_signal_) {
        return; // setData/addRow 程序化改动，不回灌
    }
    if (row < 0 || row >= table_->rowCount() || col < 0 || col >= columns_.size()) {
        return; // 边界 clamp
    }
    const QTableWidgetItem* item = table_->item(row, col);
    if (!item) {
        return;
    }
    const auto& spec = columns_.at(col);
    QVariant value;
    if (spec.type == ColumnType::kCheck) {
        value = item->checkState() == Qt::Checked;
    } else {
        value = item->text();
    }
    emit dataEdited(row, col, value);
}

} // namespace AwesomeQt
