/**
 * @file editable_table.h
 * @brief 可编辑表格控件 EditableTable——按列声明类型 + 委托校验 + 整表数据往返
 * @copyright Copyright (c) 2026 AwesomeQt
 *
 * 组合 QTableWidget（非自绘）。每列声明一个 ColumnType，编辑时由
 * ValidatorDelegate 选择合适的编辑器并做范围/空值校验，外部可一次性
 * 拿到/回填整表数据。
 */
#pragma once

#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVector>
#include <QWidget>

class QTableWidgetItem;

namespace AwesomeQt {

namespace detail {

/// @brief 按列类型驱动的编辑委托。createEditor 挑编辑器，setModelData 做校验。
///
/// 与 EditableTable 解耦：列规格由 columnSpecAt() 回调取，避免在委托里
/// 持有父表指针的强引用环。
class ValidatorDelegate : public QStyledItemDelegate {
    Q_OBJECT

  public:
    /// @brief 取指定模型列的规格（类型 / 范围 / 下拉项）的回调。
    ///
    /// 返回 false 表示该列无规格——退化为普通文本编辑。
    using ColumnSpecProvider =
        std::function<bool(int column, int& type, double& min, double& max, QStringList& combo)>;

    explicit ValidatorDelegate(QObject* parent = nullptr);

    /// @brief 注入列规格来源（通常指向 EditableTable 的列描述表）。
    void setColumnSpecProvider(ColumnSpecProvider provider);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

  private:
    /// @brief 拉取某列规格，失败则按文本处理。
    bool specFor(int column, int& type, double& min, double& max, QStringList& combo) const;

    ColumnSpecProvider provider_;
};

} // namespace detail

/// @brief 可编辑表格：列声明类型 + 委托校验 + 整表数据存取。
///
/// 列类型决定编辑器与校验逻辑：
/// - kText   → QLineEdit
/// - kInt    → QSpinBox，夹值到 [min,max]，空值不写入
/// - kDouble → QDoubleSpinBox，同上
/// - kCombo  → QComboBox（只能在 comboItems 里选）
/// - kCheck  → 单元勾选框（Qt::ItemIsUserCheckable），不走委托编辑器
///
/// 边界：空表 / 越界行 / 类型不符均安全夹值或忽略，不抛不崩。
class EditableTable : public QWidget {
    Q_OBJECT

    // —— Q_PROPERTY：行为开关，可在 Designer / 外部直接驱动 ——
    Q_PROPERTY(bool editable READ isEditable WRITE setEditable NOTIFY editableChanged)
    Q_PROPERTY(bool gridVisible READ gridVisible WRITE setGridVisible NOTIFY gridVisibleChanged)
    Q_PROPERTY(bool alternatingRowColors READ alternatingRowColors WRITE setAlternatingRowColors
                   NOTIFY alternatingRowColorsChanged)

  public:
    /// @brief 列类型枚举
    enum class ColumnType { kText, kInt, kDouble, kCombo, kCheck };
    Q_ENUM(ColumnType)

    explicit EditableTable(QWidget* parent = nullptr);

    /// @brief 追加一列。
    /// @param header 表头文字
    /// @param type   列类型（决定编辑器与校验）
    /// @param min    数值列下界（kInt/kDouble 用）
    /// @param max    数值列上界（kInt/kDouble 用）
    /// @param combo  下拉项（kCombo 用）
    void addColumn(const QString& header, ColumnType type, double min = 0, double max = 0,
                   const QStringList& combo = {});

    /// @brief 追加一个空行（按列类型给合理默认值）。
    void addRow();

    /// @brief 删除一行。row=-1 删除最后一行；越界忽略。
    void removeRow(int row = -1);

    /// @brief 整表回填。按列类型转换/夹值，行/列越界部分跳过。
    void setData(const QVector<QVector<QVariant>>& rows);

    /// @brief 取整表数据（按列类型还原为 QVariant）。空表返回空向量。
    QVector<QVector<QVariant>> data() const;

    /// @brief 清空所有行（保留列定义）。
    void clear();

    /// @brief 当前选中行号（无选中返回 -1）。供 demo 删行用。
    int currentRow() const;

    /// @brief 让列宽自适应内容（透传 QTableWidget）。
    void resizeColumnsToContents();

    // —— Q_PROPERTY 读写 ——
    void setEditable(bool editable);
    bool isEditable() const;

    void setGridVisible(bool visible);
    bool gridVisible() const;

    void setAlternatingRowColors(bool enabled);
    bool alternatingRowColors() const;

    QSize sizeHint() const override;

  signals:
    /// @brief 单元格数据被编辑提交后发出（含委托校验后的值）。
    void dataEdited(int row, int col, const QVariant& value);
    void editableChanged(bool editable);
    void gridVisibleChanged(bool visible);
    void alternatingRowColorsChanged(bool enabled);

  private:
    /// @brief 列规格描述。
    struct ColumnSpec {
        QString header;
        ColumnType type{ColumnType::kText};
        double min{0};
        double max{0};
        QStringList combo;
    };

    /// @brief 把类型当委托回调能用的 int 返回（detail 层不依赖枚举）。
    int columnTypeToInt(ColumnType type) const;

    /// @brief 给某行某列装上勾选框（kCheck 专用）。
    void applyCheckState(int row, int col, Qt::CheckState state);

    /// @brief cellChanged 的去重 + 转发，避免 setData 程序化填值时回灌。
    void onCellChanged(int row, int col);

    QVector<ColumnSpec> columns_; // 列定义，顺序即列序
    QTableWidget* table_{nullptr};
    detail::ValidatorDelegate* delegate_{nullptr};
    bool editable_{true};
    bool suppress_signal_{false}; // setData 回填期间屏蔽 cellChanged 回灌
};

} // namespace AwesomeQt
