/// @file    section_aware_datetime.h
/// @brief   Section 感知日期时间编辑器——演示 QDateTimeEdit 的 Section 状态机。
///
/// 核心知识点：
/// - currentSection() / currentSectionChanged 信号——追踪用户正在编辑哪个区段
/// - sectionText() ——获取指定 Section 的文本内容
/// - sectionCount() / sectionAt() ——遍历所有 Section 类型
/// - QDateTimeEdit::Section 枚举（YearSection, MonthSection, DaySection 等）
/// - setDisplayFormat 与 Section 解析的关系
///
/// 对应教程：进阶层 03-QtWidgets/30-QDateTimeEdit 进阶。

#pragma once

#include <QDateTimeEdit>
#include <QWidget>

class QLabel;

/// Section 感知的日期时间演示控件。
///
/// 上方是一个 QDateTimeEdit，下方实时显示：
/// - 当前激活的 Section 名称和索引
/// - 所有 Section 的类型列表
/// - 各 Section 的当前文本
///
/// 注意：QDateTimeEdit 没有提供 currentSectionChanged 信号，
/// 因此通过事件过滤器监听鼠标点击和键盘按键来检测 Section 切换。
class SectionAwareDateTime : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit SectionAwareDateTime(QWidget* parent = nullptr);

private:
    /// @brief 将 Section 枚举值转换为可读名称。
    /// @param[in] section Section 枚举值。
    /// @return 中文名称字符串。
    static QString sectionToName(QDateTimeEdit::Section section);

    /// @brief 更新 Section 信息面板——刷新所有标签内容。
    void updateSectionInfo();

    /// @brief 重建 Section 列表（在 displayFormat 变化时调用）。
    void rebuildSectionList();

    /// @brief 事件过滤器——监听 QDateTimeEdit 上的鼠标和键盘事件以检测 Section 切换。
    bool eventFilter(QObject* watched, QEvent* event) override;

    QDateTimeEdit* m_dateTimeEdit;      // 日期时间编辑器
    QLabel* m_currentSectionLabel;      // 当前 Section 名称
    QLabel* m_sectionIndexLabel;        // 当前 Section 索引
    QLabel* m_sectionCountLabel;        // Section 总数
    QLabel* m_sectionTextsLabel;        // 各 Section 的文本内容
    QLabel* m_sectionTypesLabel;        // 所有 Section 类型列表
};
