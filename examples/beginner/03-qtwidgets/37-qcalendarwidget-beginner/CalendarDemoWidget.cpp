#include "CalendarDemoWidget.h"

#include <QApplication>
#include <QDate>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextCharFormat>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// CalendarDemoWidget: QCalendarWidget 综合演示窗口
// ============================================================================
CalendarDemoWidget::CalendarDemoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QCalendarWidget 综合演示 — 日历控件");
    resize(720, 480);
    initUi();
}

/// @brief 初始化界面
void CalendarDemoWidget::initUi()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ================================================================
    // 左侧: 日历控件
    // ================================================================
    m_calendar = new QCalendarWidget;
    m_calendar->setMinimumDate(QDate(2020, 1, 1));
    m_calendar->setMaximumDate(QDate(2030, 12, 31));
    m_calendar->setGridVisible(true);
    m_calendar->setSelectionMode(
        QCalendarWidget::SingleSelection);
    mainLayout->addWidget(m_calendar, 3);

    // 自定义表头：红色加粗
    QTextCharFormat weekendHeaderFormat;
    weekendHeaderFormat.setForeground(QColor("#D32F2F"));
    weekendHeaderFormat.setFontWeight(QFont::Bold);
    m_calendar->setHeaderTextFormat(weekendHeaderFormat);

    // 全局周末格式：浅灰色
    QTextCharFormat weekendFormat;
    weekendFormat.setForeground(QColor("#757575"));
    m_calendar->setWeekdayTextFormat(Qt::Saturday,
                                     weekendFormat);
    m_calendar->setWeekdayTextFormat(Qt::Sunday,
                                     weekendFormat);

    // ================================================================
    // 右侧: 信息面板
    // ================================================================
    auto *rightPanel = new QVBoxLayout;
    rightPanel->setSpacing(12);

    // 选中日期显示
    auto *selectedGroup = new QGroupBox("当前选中日期");
    auto *selectedLayout = new QVBoxLayout(selectedGroup);

    m_selectedLabel = new QLabel("未选择");
    m_selectedLabel->setAlignment(Qt::AlignCenter);
    m_selectedLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold;"
        "padding: 12px;"
        "background-color: #E3F2FD;"
        "border: 1px solid #90CAF9;"
        "border-radius: 6px;"
        "color: #1565C0;");
    selectedLayout->addWidget(m_selectedLabel);

    // 日期手动输入
    auto *dateEditRow = new QHBoxLayout;
    auto *dateEditLabel = new QLabel("手动选择:");
    m_dateEdit = new QDateEdit;
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setDate(QDate::currentDate());
    dateEditRow->addWidget(dateEditLabel);
    dateEditRow->addWidget(m_dateEdit, 1);
    selectedLayout->addLayout(dateEditRow);

    rightPanel->addWidget(selectedGroup);

    // 日期范围约束
    auto *rangeGroup = new QGroupBox("可选日期范围");
    auto *rangeLayout = new QVBoxLayout(rangeGroup);

    auto *minRow = new QHBoxLayout;
    auto *minLabel = new QLabel("最小日期:");
    m_minDateEdit = new QDateEdit;
    m_minDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_minDateEdit->setDate(QDate(2020, 1, 1));
    minRow->addWidget(minLabel);
    minRow->addWidget(m_minDateEdit, 1);
    rangeLayout->addLayout(minRow);

    auto *maxRow = new QHBoxLayout;
    auto *maxLabel = new QLabel("最大日期:");
    m_maxDateEdit = new QDateEdit;
    m_maxDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_maxDateEdit->setDate(QDate(2030, 12, 31));
    maxRow->addWidget(maxLabel);
    maxRow->addWidget(m_maxDateEdit, 1);
    rangeLayout->addLayout(maxRow);

    auto *applyRangeBtn = new QPushButton("应用范围约束");
    applyRangeBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 6px 16px; font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #1565C0; }");
    rangeLayout->addWidget(applyRangeBtn);

    rightPanel->addWidget(rangeGroup);

    // 高亮操作
    auto *highlightGroup = new QGroupBox("日期高亮");
    auto *highlightLayout = new QVBoxLayout(highlightGroup);

    auto *highlightWeekendBtn =
        new QPushButton("高亮当前月份周末");
    highlightWeekendBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #388E3C; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 6px 16px; font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #2E7D32; }");

    auto *highlightTodayBtn =
        new QPushButton("高亮今天");
    highlightTodayBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #F57C00; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 6px 16px; font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #EF6C00; }");

    auto *clearHighlightBtn =
        new QPushButton("清除所有高亮");
    clearHighlightBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #757575; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 6px 16px; font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #616161; }");

    highlightLayout->addWidget(highlightWeekendBtn);
    highlightLayout->addWidget(highlightTodayBtn);
    highlightLayout->addWidget(clearHighlightBtn);

    rightPanel->addWidget(highlightGroup);

    rightPanel->addStretch();

    // 状态标签
    m_statusLabel = new QLabel("点击日历选择日期");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "color: #888; font-size: 11px;"
        "padding: 6px;"
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    rightPanel->addWidget(m_statusLabel);

    mainLayout->addLayout(rightPanel, 2);

    // ================================================================
    // 信号连接
    // ================================================================

    // 日历选择变化 → 更新右侧标签
    connect(m_calendar, &QCalendarWidget::selectionChanged,
            this, &CalendarDemoWidget::updateSelectedLabel);

    // 日历选择变化 → 同步到 QDateEdit
    connect(m_calendar, &QCalendarWidget::selectionChanged,
            this, [this]() {
        m_dateEdit->setDate(m_calendar->selectedDate());
    });

    // QDateEdit 手动输入 → 同步到日历
    connect(m_dateEdit, &QDateEdit::dateChanged, this,
            [this](const QDate &date) {
        m_calendar->setSelectedDate(date);
    });

    // 月份切换 → 更新状态
    connect(m_calendar,
            &QCalendarWidget::currentPageChanged, this,
            [this](int year, int month) {
        m_statusLabel->setText(
            QString("当前页面: %1 年 %2 月")
                .arg(year)
                .arg(month, 2, 10, QChar('0')));
    });

    // 应用范围约束
    connect(applyRangeBtn, &QPushButton::clicked, this,
            [this]() {
        QDate minDate = m_minDateEdit->date();
        QDate maxDate = m_maxDateEdit->date();

        if (minDate > maxDate) {
            m_statusLabel->setText(
                "错误: 最小日期不能大于最大日期");
            m_statusLabel->setStyleSheet(
                "color: #D32F2F; font-size: 11px;"
                "padding: 6px;"
                "background-color: #FFEBEE;"
                "border: 1px solid #EF9A9A;"
                "border-radius: 4px;");
            return;
        }

        m_calendar->setMinimumDate(minDate);
        m_calendar->setMaximumDate(maxDate);
        m_statusLabel->setText(
            QString("范围已设为 %1 ~ %2")
                .arg(minDate.toString("yyyy-MM-dd"))
                .arg(maxDate.toString("yyyy-MM-dd")));
        m_statusLabel->setStyleSheet(
            "color: #388E3C; font-size: 11px;"
            "padding: 6px;"
            "background-color: #E8F5E9;"
            "border: 1px solid #A5D6A7;"
            "border-radius: 4px;");
    });

    // 高亮周末
    connect(highlightWeekendBtn, &QPushButton::clicked,
            this, [this]() {
        highlightWeekends(m_calendar->yearShown(),
                          m_calendar->monthShown());
    });

    // 高亮今天
    connect(highlightTodayBtn, &QPushButton::clicked,
            this, [this]() {
        QTextCharFormat todayFormat;
        todayFormat.setBackground(QColor("#FFF176"));
        todayFormat.setFontWeight(QFont::Bold);
        todayFormat.setForeground(QColor("#333"));

        m_calendar->setDateTextFormat(
            QDate::currentDate(), todayFormat);

        m_statusLabel->setText(
            "今天已高亮: " +
            QDate::currentDate().toString("yyyy-MM-dd"));
    });

    // 清除高亮
    connect(clearHighlightBtn, &QPushButton::clicked,
            this, [this]() {
        // 清除所有自定义日期格式
        // 遍历当前可见月份范围并清除
        int year = m_calendar->yearShown();
        int month = m_calendar->monthShown();
        QDate start(year, month, 1);
        QDate end = start.addMonths(1).addDays(-1);
        QDate date = start;

        QTextCharFormat emptyFormat;
        while (date <= end) {
            m_calendar->setDateTextFormat(date,
                                          emptyFormat);
            date = date.addDays(1);
        }

        m_statusLabel->setText("已清除当前月份高亮");
    });

    // 初始化选中日期显示
    updateSelectedLabel();
}

/// @brief 更新选中日期标签
void CalendarDemoWidget::updateSelectedLabel()
{
    QDate selected = m_calendar->selectedDate();
    QString text = selected.toString("yyyy-MM-dd");

    // 获取星期几的中文名称
    static const QStringList kWeekDays = {
        "周一", "周二", "周三", "周四",
        "周五", "周六", "周日"};
    int dayOfWeek = selected.dayOfWeek();
    text += " " + kWeekDays.at(dayOfWeek - 1);

    m_selectedLabel->setText(text);
}

/// @brief 高亮指定月份的所有周末
void CalendarDemoWidget::highlightWeekends(int year, int month)
{
    QTextCharFormat weekendFormat;
    weekendFormat.setBackground(QColor("#FFCDD2"));
    weekendFormat.setForeground(QColor("#C62828"));
    weekendFormat.setFontWeight(QFont::Bold);

    QDate date(year, month, 1);
    QDate endDate = date.addMonths(1).addDays(-1);

    while (date <= endDate) {
        // Qt: Monday=1 ... Saturday=6, Sunday=7
        if (date.dayOfWeek() >= 6) {
            m_calendar->setDateTextFormat(date,
                                          weekendFormat);
        }
        date = date.addDays(1);
    }

    m_statusLabel->setText(
        QString("已高亮 %1 年 %2 月的周末")
            .arg(year)
            .arg(month, 2, 10, QChar('0')));
}
