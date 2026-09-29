/// @file    dynamic_radio_group.cpp
/// @brief   DynamicRadioGroup 类实现——QButtonGroup exclusive、动态增删、信号对比。
///
/// 对应教程：进阶层 03-QtWidgets/19-QRadioButton 进阶。

#include "dynamic_radio_group.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

DynamicRadioGroup::DynamicRadioGroup(QWidget* parent)
    : QWidget(parent)
    , m_nextDeviceId(0)
    , m_deviceGroupExclusive(true)
    , m_idClickedCount(0)
    , m_buttonClickedCount(0)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 三个演示区域依次排列
    mainLayout->addWidget(createDeviceSection());
    mainLayout->addWidget(createDualGroupSection());
    mainLayout->addWidget(createSignalCompareSection());
    mainLayout->addStretch();

    setWindowTitle(QStringLiteral("QRadioButton Advanced Demo"));
    resize(600, 600);
}

// ─────────────────────────────────────────────────────────────────────────────
// 设备选择区——动态增删 QRadioButton
// ─────────────────────────────────────────────────────────────────────────────

QWidget* DynamicRadioGroup::createDeviceSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle =
        new QLabel(QStringLiteral("1. 动态设备选择器（运行时增删按钮）"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // QButtonGroup 管理 exclusive 互斥关系
    // QRadioButton 的 autoExclusive 是基于同一 parent 的独立机制
    // 要真正关闭互斥，需要同时 setExclusive(false) 和 setAutoExclusive(false)
    m_deviceGroup = new QButtonGroup(this);
    m_deviceGroup->setExclusive(true);

    // 设备按钮容器
    auto* deviceContainer = new QWidget;
    m_deviceLayout = new QVBoxLayout(deviceContainer);
    m_deviceLayout->setContentsMargins(0, 0, 0, 0);

    // 预设两个初始设备
    auto* radio1 = new QRadioButton(QStringLiteral("传感器 A"), deviceContainer);
    m_deviceGroup->addButton(radio1, m_nextDeviceId++);
    m_deviceLayout->addWidget(radio1);
    radio1->setChecked(true);

    auto* radio2 = new QRadioButton(QStringLiteral("传感器 B"), deviceContainer);
    m_deviceGroup->addButton(radio2, m_nextDeviceId++);
    m_deviceLayout->addWidget(radio2);

    // 操作区：输入框 + 添加/移除按钮
    auto* controlRow = new QHBoxLayout;
    m_deviceNameInput =
        new QLineEdit(QStringLiteral("新传感器"));
    m_deviceNameInput->setPlaceholderText(
        QStringLiteral("输入设备名称"));
    m_addDeviceBtn = new QPushButton(QStringLiteral("添加设备"));
    m_removeDeviceBtn = new QPushButton(QStringLiteral("移除选中设备"));
    m_toggleExclusiveBtn =
        new QPushButton(QStringLiteral("切换 Exclusive（当前：ON）"));

    controlRow->addWidget(m_deviceNameInput, 1);
    controlRow->addWidget(m_addDeviceBtn);
    controlRow->addWidget(m_removeDeviceBtn);
    controlRow->addWidget(m_toggleExclusiveBtn);

    // 选中状态显示
    m_deviceInfoLabel = new QLabel;
    m_deviceInfoLabel->setFrameShape(QFrame::Box);

    layout->addWidget(sectionTitle);
    layout->addWidget(deviceContainer);
    layout->addLayout(controlRow);
    layout->addWidget(m_deviceInfoLabel);

    // 信号连接
    // idClicked 只在用户实际点击时触发，程序 setChecked 不会触发
    connect(m_deviceGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DynamicRadioGroup::updateDeviceInfo);
    connect(m_addDeviceBtn, &QPushButton::clicked, this,
            &DynamicRadioGroup::addDevice);
    connect(m_removeDeviceBtn, &QPushButton::clicked, this,
            &DynamicRadioGroup::removeDevice);
    connect(m_toggleExclusiveBtn, &QPushButton::clicked, this,
            &DynamicRadioGroup::toggleExclusive);

    // 初始状态
    updateDeviceInfo();

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 两个独立 exclusive 组互不干扰
// ─────────────────────────────────────────────────────────────────────────────

QWidget* DynamicRadioGroup::createDualGroupSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle =
        new QLabel(QStringLiteral("2. 两个独立的 Exclusive 组（互不干扰）"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    // 组 A：颜色选择
    // QButtonGroup 的 exclusive 范围仅限于组内按钮，不同组互不影响
    m_groupA = new QButtonGroup(this);
    m_groupA->setExclusive(true);

    auto* groupALayout = new QHBoxLayout;
    auto* groupALabel = new QLabel(QStringLiteral("颜色:"));
    groupALabel->setStyleSheet(QStringLiteral("font-weight: bold;"));

    const QStringList kColors = {
        QStringLiteral("红色"),
        QStringLiteral("绿色"),
        QStringLiteral("蓝色"),
    };

    for (int i = 0; i < kColors.size(); ++i) {
        auto* radio = new QRadioButton(kColors[i]);
        m_groupA->addButton(radio, i);
        groupALayout->addWidget(radio);
        if (i == 0) {
            radio->setChecked(true);
        }
    }

    m_groupALabel = new QLabel;
    m_groupALabel->setFrameShape(QFrame::StyledPanel);

    auto* groupASection = new QVBoxLayout;
    groupASection->addLayout(groupALayout);
    groupASection->addWidget(m_groupALabel);

    // 组 B：尺寸选择
    // 即使按钮的 parent 相同，QButtonGroup 的 exclusive 也不会跨组生效
    m_groupB = new QButtonGroup(this);
    m_groupB->setExclusive(true);

    auto* groupBLayout = new QHBoxLayout;
    auto* groupBLabel = new QLabel(QStringLiteral("尺寸:"));
    groupBLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));

    const QStringList kSizes = {
        QStringLiteral("小"),
        QStringLiteral("中"),
        QStringLiteral("大"),
    };

    for (int i = 0; i < kSizes.size(); ++i) {
        auto* radio = new QRadioButton(kSizes[i]);
        m_groupB->addButton(radio, i);
        groupBLayout->addWidget(radio);
        if (i == 0) {
            radio->setChecked(true);
        }
    }

    m_groupBLabel = new QLabel;
    m_groupBLabel->setFrameShape(QFrame::StyledPanel);

    auto* groupBSection = new QVBoxLayout;
    groupBSection->addLayout(groupBLayout);
    groupBSection->addWidget(m_groupBLabel);

    layout->addWidget(sectionTitle);
    layout->addLayout(groupASection);
    layout->addLayout(groupBSection);

    // 信号连接
    connect(m_groupA, QOverload<int>::of(&QButtonGroup::idClicked), this,
            [this](int id) {
                m_groupALabel->setText(
                    QStringLiteral("选中颜色 ID: %1").arg(id));
            });
    connect(m_groupB, QOverload<int>::of(&QButtonGroup::idClicked), this,
            [this](int id) {
                m_groupBLabel->setText(
                    QStringLiteral("选中尺寸 ID: %1").arg(id));
            });

    // 初始状态
    m_groupALabel->setText(QStringLiteral("选中颜色 ID: 0"));
    m_groupBLabel->setText(QStringLiteral("选中尺寸 ID: 0"));

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// idClicked vs buttonClicked 对比
// ─────────────────────────────────────────────────────────────────────────────

QWidget* DynamicRadioGroup::createSignalCompareSection()
{
    auto* container = new QWidget;
    auto* layout = new QVBoxLayout(container);

    auto* sectionTitle =
        new QLabel(QStringLiteral("3. idClicked vs buttonClicked 信号对比"));
    sectionTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));

    auto* hint = new QLabel(
        QStringLiteral("idClicked 只在用户点击时触发，程序 setChecked 不触发。"
                       "点击按钮观察计数差异。"));
    hint->setWordWrap(true);

    // 三个测试按钮
    m_signalGroup = new QButtonGroup(this);
    m_signalGroup->setExclusive(true);

    auto* buttonsRow = new QHBoxLayout;
    const QStringList kOptions = {
        QStringLiteral("选项 X"),
        QStringLiteral("选项 Y"),
        QStringLiteral("选项 Z"),
    };

    for (int i = 0; i < kOptions.size(); ++i) {
        auto* radio = new QRadioButton(kOptions[i]);
        m_signalGroup->addButton(radio, i);
        buttonsRow->addWidget(radio);
        if (i == 0) {
            radio->setChecked(true);
        }
    }

    // 信号计数标签
    m_idClickedLog = new QLabel(QStringLiteral("idClicked 触发次数: 0"));
    m_buttonClickedLog =
        new QLabel(QStringLiteral("buttonClicked 触发次数: 0"));

    auto* logRow = new QHBoxLayout;
    logRow->addWidget(m_idClickedLog);
    logRow->addWidget(m_buttonClickedLog);

    layout->addWidget(sectionTitle);
    layout->addWidget(hint);
    layout->addLayout(buttonsRow);
    layout->addLayout(logRow);

    // 信号连接
    // idClicked(int) 只在用户实际点击时触发
    connect(m_signalGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int id) {
                ++m_idClickedCount;
                m_idClickedLog->setText(
                    QStringLiteral("idClicked 触发次数: %1 (最后 ID: %2)")
                        .arg(m_idClickedCount)
                        .arg(id));
            });

    // buttonClicked(QAbstractButton*) 也只在用户实际点击时触发
    // 与 idClicked 的区别是参数类型：按钮指针 vs 整数 ID
    connect(m_signalGroup,
            QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, [this](QAbstractButton* button) {
                ++m_buttonClickedCount;
                m_buttonClickedLog->setText(
                    QStringLiteral("buttonClicked 触发次数: %1 (按钮: %2)")
                        .arg(m_buttonClickedCount)
                        .arg(button->text()));
            });

    return container;
}

// ─────────────────────────────────────────────────────────────────────────────
// 槽函数实现
// ─────────────────────────────────────────────────────────────────────────────

void DynamicRadioGroup::addDevice()
{
    const QString name = m_deviceNameInput->text().trimmed();
    if (name.isEmpty()) {
        return;
    }

    // 动态创建 QRadioButton，加入 QButtonGroup 和布局
    // 使用 addButton(radio, id) 分配递增 ID，便于后续通过 button(id) 查找
    auto* radio = new QRadioButton(name);
    m_deviceGroup->addButton(radio, m_nextDeviceId++);
    m_deviceLayout->addWidget(radio);

    updateDeviceInfo();
}

void DynamicRadioGroup::removeDevice()
{
    // 获取当前选中按钮的 ID
    const int checkedId = m_deviceGroup->checkedId();
    if (checkedId < 0) {
        // 没有选中任何按钮或组内没有按钮
        return;
    }

    auto* btn = m_deviceGroup->button(checkedId);
    if (btn == nullptr) {
        return;
    }

    // removeButton 不会改变按钮的 checked 状态
    // 移除后 checkedId() 将返回 -1，需要手动处理
    m_deviceGroup->removeButton(btn);
    m_deviceLayout->removeWidget(btn);
    btn->deleteLater();

    // 移除选中项后，自动选中剩余的第一个按钮
    // checkedId() 在 removeButton 后返回 -1，需要遍历查找
    const auto buttons = m_deviceGroup->buttons();
    if (!buttons.isEmpty()) {
        buttons.first()->setChecked(true);
    }

    updateDeviceInfo();
}

void DynamicRadioGroup::toggleExclusive()
{
    m_deviceGroupExclusive = !m_deviceGroupExclusive;
    m_deviceGroup->setExclusive(m_deviceGroupExclusive);

    if (m_deviceGroupExclusive) {
        m_toggleExclusiveBtn->setText(
            QStringLiteral("切换 Exclusive（当前：ON）"));
    } else {
        m_toggleExclusiveBtn->setText(
            QStringLiteral("切换 Exclusive（当前：OFF）"));

        // 注意：setExclusive(false) 不会取消按钮的 autoExclusive
        // QRadioButton 的 autoExclusive 默认为 true，基于同一 parent 生效
        // 要实现完全非互斥，还需手动关闭 autoExclusive
    }

    updateDeviceInfo();
}

void DynamicRadioGroup::updateDeviceInfo()
{
    const int checkedId = m_deviceGroup->checkedId();
    const bool exclusive = m_deviceGroup->exclusive();
    const int count = m_deviceGroup->buttons().size();

    QString info =
        QStringLiteral("设备数量: %1 | Exclusive: %2 | checkedId: %3")
            .arg(count)
            .arg(exclusive ? QStringLiteral("ON") : QStringLiteral("OFF"))
            .arg(checkedId);

    if (checkedId >= 0) {
        auto* checkedBtn = m_deviceGroup->button(checkedId);
        if (checkedBtn != nullptr) {
            info += QStringLiteral(" | 选中: %1").arg(checkedBtn->text());
        }
    }

    m_deviceInfoLabel->setText(info);
}
