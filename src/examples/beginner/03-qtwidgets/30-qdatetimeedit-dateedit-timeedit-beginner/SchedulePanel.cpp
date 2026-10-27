// QtWidgets 入门示例 30: QDateEdit / QTimeEdit / QDateTimeEdit 日期时间输入
// 演示：setMinimumDate / setMaximumDate 限制范围
//       setCalendarPopup(true) 弹出日历选择器
//       QDate / QTime / QDateTime 数据类型互转
//       valueChanged 信号驱动摘要更新

#include "SchedulePanel.h"

#include <QApplication>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTimeEdit>
#include <QVBoxLayout>

// ============================================================================
// SchedulePanel: 日程安排面板
// 覆盖 QDateEdit / QTimeEdit / QDateTimeEdit 的核心用法
// ============================================================================
SchedulePanel::SchedulePanel(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(
        "QDateEdit / QTimeEdit / QDateTimeEdit 综合演示 — 日程安排");
    resize(720, 540);
    initUi();
}

/// @brief 初始化界面
void SchedulePanel::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ================================================================
    // 上方：日程信息输入区
    // ================================================================
    auto *inputGroup = new QGroupBox("日程信息");
    auto *formLayout = new QFormLayout(inputGroup);
    formLayout->setSpacing(10);

    // ---- 标题输入 ----
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("输入日程标题...");
    formLayout->addRow("标题:", m_titleEdit);

    // ---- 开始日期（弹出日历，范围从今天到一年后）----
    m_startDateEdit = new QDateEdit();
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("yyyy 年 MM 月 dd 日");
    m_startDateEdit->setMinimumDate(QDate::currentDate());
    m_startDateEdit->setMaximumDate(QDate::currentDate().addYears(1));
    m_startDateEdit->setDate(QDate::currentDate());
    formLayout->addRow("开始日期:", m_startDateEdit);

    // ---- 开始时间（范围 06:00-23:00）----
    m_startTimeEdit = new QTimeEdit();
    m_startTimeEdit->setDisplayFormat("HH:mm");
    m_startTimeEdit->setMinimumTime(QTime(6, 0, 0));
    m_startTimeEdit->setMaximumTime(QTime(23, 0, 0));
    m_startTimeEdit->setTime(QTime(9, 0, 0));
    formLayout->addRow("开始时间:", m_startTimeEdit);

    // ---- 截止日期时间（弹出日历）----
    m_endDateTimeEdit = new QDateTimeEdit();
    m_endDateTimeEdit->setCalendarPopup(true);
    m_endDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_endDateTimeEdit->setMinimumDateTime(
        QDateTime(QDate::currentDate(), QTime(6, 0, 0)));
    m_endDateTimeEdit->setMaximumDateTime(
        QDateTime(QDate::currentDate().addYears(1), QTime(23, 0, 0)));
    // 默认截止时间为当天 18:00
    m_endDateTimeEdit->setDateTime(
        QDateTime(QDate::currentDate(), QTime(18, 0, 0)));
    formLayout->addRow("截止时间:", m_endDateTimeEdit);

    mainLayout->addWidget(inputGroup);

    // ================================================================
    // 中间：摘要信息
    // ================================================================
    auto *summaryGroup = new QGroupBox("时长摘要");
    auto *summaryLayout = new QVBoxLayout(summaryGroup);

    m_summaryLabel = new QLabel();
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setAlignment(Qt::AlignCenter);
    m_summaryLabel->setStyleSheet(
        "background-color: #E8F5E9;"
        "border: 1px solid #A5D6A7;"
        "border-radius: 6px;"
        "padding: 14px;"
        "font-size: 14px;"
        "color: #2E7D32;");
    summaryLayout->addWidget(m_summaryLabel);

    mainLayout->addWidget(summaryGroup);

    // ================================================================
    // 下方：添加按钮 + 日程列表
    // ================================================================
    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(12);

    // ---- 添加按钮 ----
    auto *addButton = new QPushButton("添加日程");
    addButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 8px 20px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #1565C0;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #0D47A1;"
        "}");

    auto *buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(addButton);
    buttonLayout->addStretch();
    bottomLayout->addLayout(buttonLayout);

    // ---- 日程列表 ----
    auto *listGroup = new QGroupBox("已添加的日程");
    auto *listLayout = new QVBoxLayout(listGroup);

    m_scheduleList = new QListWidget();
    m_scheduleList->setStyleSheet(
        "QListWidget {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 4px;"
        "  font-size: 13px;"
        "}"
        "QListWidget::item {"
        "  padding: 6px;"
        "  border-bottom: 1px solid #EEE;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1565C0;"
        "}");
    listLayout->addWidget(m_scheduleList);

    bottomLayout->addWidget(listGroup, 1);
    mainLayout->addLayout(bottomLayout, 1);

    // ================================================================
    // 信号连接
    // ================================================================
    connect(m_startDateEdit, &QDateEdit::dateChanged,
            this, &SchedulePanel::updateSummary);
    connect(m_startTimeEdit, &QTimeEdit::timeChanged,
            this, &SchedulePanel::updateSummary);
    connect(m_endDateTimeEdit, &QDateTimeEdit::dateTimeChanged,
            this, &SchedulePanel::updateSummary);
    connect(addButton, &QPushButton::clicked,
            this, &SchedulePanel::onAddSchedule);

    // 初始化摘要
    updateSummary();
}

/// @brief 更新时长摘要
void SchedulePanel::updateSummary()
{
    // 从 QDateEdit 和 QTimeEdit 构建 QDateTime
    QDateTime startDateTime(m_startDateEdit->date(),
                            m_startTimeEdit->time());
    QDateTime endDateTime = m_endDateTimeEdit->dateTime();

    QString startStr = startDateTime.toString("yyyy-MM-dd HH:mm");
    QString endStr = endDateTime.toString("yyyy-MM-dd HH:mm");

    if (endDateTime <= startDateTime) {
        m_summaryLabel->setText(
            QString("从 %1 到 %2\n截止时间必须晚于开始时间！")
                .arg(startStr, endStr));
        return;
    }

    // 计算时间差
    qint64 totalSecs = startDateTime.secsTo(endDateTime);
    qint64 days = totalSecs / 86400;
    qint64 hours = (totalSecs % 86400) / 3600;
    qint64 minutes = (totalSecs % 3600) / 60;

    QString durationText;
    if (days > 0) {
        durationText += QString("%1 天 ").arg(days);
    }
    if (hours > 0) {
        durationText += QString("%1 小时 ").arg(hours);
    }
    if (minutes > 0) {
        durationText += QString("%1 分钟").arg(minutes);
    }
    if (durationText.isEmpty()) {
        durationText = "不足 1 分钟";
    }

    m_summaryLabel->setText(
        QString("从 %1 到 %2\n持续时长: %3")
            .arg(startStr, endStr, durationText));
}

/// @brief 添加日程到列表
void SchedulePanel::onAddSchedule()
{
    QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入日程标题！");
        return;
    }

    QDateTime startDateTime(m_startDateEdit->date(),
                            m_startTimeEdit->time());
    QDateTime endDateTime = m_endDateTimeEdit->dateTime();

    if (endDateTime <= startDateTime) {
        QMessageBox::warning(this, "提示",
                             "截止时间必须晚于开始时间！");
        return;
    }

    qint64 totalSecs = startDateTime.secsTo(endDateTime);
    qint64 hours = totalSecs / 3600;
    qint64 minutes = (totalSecs % 3600) / 60;

    QString entry = QString("[%1] %2 | %3 → %4 | %5h %6m")
                        .arg(startDateTime.toString("yyyy-MM-dd"))
                        .arg(title)
                        .arg(startDateTime.toString("HH:mm"))
                        .arg(endDateTime.toString("MM-dd HH:mm"))
                        .arg(hours)
                        .arg(minutes);

    m_scheduleList->addItem(entry);
    m_titleEdit->clear();
}
