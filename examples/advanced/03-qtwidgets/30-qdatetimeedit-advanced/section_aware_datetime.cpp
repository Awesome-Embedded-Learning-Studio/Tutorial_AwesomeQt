/// @file    section_aware_datetime.cpp
/// @brief   SectionAwareDateTime 类实现——QDateTimeEdit Section 状态机演示。
///
/// 对应教程：进阶层 03-QtWidgets/30-QDateTimeEdit 进阶。

#include "section_aware_datetime.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

SectionAwareDateTime::SectionAwareDateTime(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    // ── 标题 ──
    auto* title = new QLabel(QStringLiteral("QDateTimeEdit Section 状态机演示"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // ── QDateTimeEdit ──
    m_dateTimeEdit = new QDateTimeEdit;
    m_dateTimeEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    // 不使用 calendarPopup，保持焦点在 Section 编辑上
    m_dateTimeEdit->setCalendarPopup(false);

    // ── Section 信息面板 ──
    auto* infoGroup = new QWidget;
    auto* infoLayout = new QVBoxLayout(infoGroup);

    m_currentSectionLabel = new QLabel;
    m_sectionIndexLabel = new QLabel;
    m_sectionCountLabel = new QLabel;
    m_sectionTypesLabel = new QLabel;
    m_sectionTextsLabel = new QLabel;

    // 文本过长时自动换行
    m_sectionTypesLabel->setWordWrap(true);
    m_sectionTextsLabel->setWordWrap(true);

    m_sectionTypesLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_sectionTextsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    infoLayout->addWidget(m_currentSectionLabel);
    infoLayout->addWidget(m_sectionIndexLabel);
    infoLayout->addWidget(m_sectionCountLabel);
    infoLayout->addWidget(m_sectionTypesLabel);
    infoLayout->addWidget(m_sectionTextsLabel);

    // ── 提示 ──
    auto* hint = new QLabel(
        QStringLiteral("提示：点击日期时间输入框中的不同部分（年/月/日/时/分/秒），\n"
                       "观察下方 Section 信息的变化。上下箭头只改变当前激活的 Section。"));
    hint->setWordWrap(true);

    // ── 组装布局 ──
    auto* editRow = new QHBoxLayout;
    auto* editLabel = new QLabel(QStringLiteral("DateTime Edit:"));
    editRow->addWidget(editLabel);
    editRow->addWidget(m_dateTimeEdit, 1);

    mainLayout->addWidget(title);
    mainLayout->addLayout(editRow);
    mainLayout->addWidget(infoGroup);
    mainLayout->addWidget(hint);
    mainLayout->addStretch();

    // ── 信号/事件连接 ──

    // QDateTimeEdit 没有提供 currentSectionChanged 信号，
    // 因此通过事件过滤器监听鼠标点击和键盘按键来检测 Section 切换。
    // 鼠标点击会改变光标位置从而切换 Section，方向键/上下箭头也会改变值。
    m_dateTimeEdit->installEventFilter(this);

    // dateTimeChanged：值变化时也刷新 sectionTexts
    connect(m_dateTimeEdit, &QDateTimeEdit::dateTimeChanged,
            this, &SectionAwareDateTime::updateSectionInfo);

    // 初始填充
    rebuildSectionList();
    updateSectionInfo();
}

// ─────────────────────────────────────────────────────────────────────────────
// Section 信息更新
// ─────────────────────────────────────────────────────────────────────────────

void SectionAwareDateTime::rebuildSectionList()
{
    // 遍历所有 section 并构建类型列表
    const int count = m_dateTimeEdit->sectionCount();
    QStringList typeList;

    for (int i = 0; i < count; ++i) {
        const auto section = m_dateTimeEdit->sectionAt(i);
        typeList << QStringLiteral("  [%1] %2 (枚举值: %3)")
                        .arg(i)
                        .arg(sectionToName(section))
                        .arg(static_cast<int>(section));
    }

    m_sectionTypesLabel->setText(
        QStringLiteral("Section 类型列表:\n%1").arg(typeList.join(QStringLiteral("\n"))));

    m_sectionCountLabel->setText(
        QStringLiteral("Section 总数: %1").arg(count));
}

void SectionAwareDateTime::updateSectionInfo()
{
    // 当前激活的 Section
    const auto current = m_dateTimeEdit->currentSection();
    m_currentSectionLabel->setText(
        QStringLiteral("当前 Section: %1").arg(sectionToName(current)));

    // 当前 Section 的索引
    m_sectionIndexLabel->setText(
        QStringLiteral("当前索引: %1").arg(m_dateTimeEdit->currentSectionIndex()));

    // 各 Section 的文本内容
    const int count = m_dateTimeEdit->sectionCount();
    QStringList textList;
    for (int i = 0; i < count; ++i) {
        const auto section = m_dateTimeEdit->sectionAt(i);
        const QString sectionText = m_dateTimeEdit->sectionText(section);
        textList << QStringLiteral("  [%1] %2 → \"%3\"")
                        .arg(i)
                        .arg(sectionToName(section))
                        .arg(sectionText);
    }
    m_sectionTextsLabel->setText(
        QStringLiteral("各 Section 文本:\n%1").arg(textList.join(QStringLiteral("\n"))));
}

// ─────────────────────────────────────────────────────────────────────────────
// 事件过滤器——检测 Section 切换
// ─────────────────────────────────────────────────────────────────────────────

bool SectionAwareDateTime::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_dateTimeEdit) {
        // 鼠标按下会改变光标位置从而切换当前 Section
        if (event->type() == QEvent::MouseButtonPress) {
            // 延迟刷新：让 QDateTimeEdit 先处理点击事件，更新 currentSection
            QMetaObject::invokeMethod(this, &SectionAwareDateTime::updateSectionInfo,
                                      Qt::QueuedConnection);
        }
        // 键盘按键（方向键等）也可能改变当前 Section 或值
        else if (event->type() == QEvent::KeyRelease) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            switch (keyEvent->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                QMetaObject::invokeMethod(this, &SectionAwareDateTime::updateSectionInfo,
                                          Qt::QueuedConnection);
                break;
            default:
                break;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

// ─────────────────────────────────────────────────────────────────────────────
// 辅助函数
// ─────────────────────────────────────────────────────────────────────────────

QString SectionAwareDateTime::sectionToName(QDateTimeEdit::Section section)
{
    // 将 Section 枚举映射到中文名称，方便理解
    switch (section) {
    case QDateTimeEdit::NoSection:       return QStringLiteral("NoSection（无）");
    case QDateTimeEdit::AmPmSection:     return QStringLiteral("AmPmSection（上午/下午）");
    case QDateTimeEdit::MSecSection:     return QStringLiteral("MSecSection（毫秒）");
    case QDateTimeEdit::SecondSection:   return QStringLiteral("SecondSection（秒）");
    case QDateTimeEdit::MinuteSection:   return QStringLiteral("MinuteSection（分）");
    case QDateTimeEdit::HourSection:     return QStringLiteral("HourSection（时）");
    case QDateTimeEdit::DaySection:      return QStringLiteral("DaySection（日）");
    case QDateTimeEdit::MonthSection:    return QStringLiteral("MonthSection（月）");
    case QDateTimeEdit::YearSection:     return QStringLiteral("YearSection（年）");
    default:
        return QStringLiteral("Unknown（未知: %1）").arg(static_cast<int>(section));
    }
}
