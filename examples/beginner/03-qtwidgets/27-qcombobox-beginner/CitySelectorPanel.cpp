// QtWidgets 入门示例 27: QComboBox 下拉选择框
// 演示：addItem / addItems / insertItem 添加选项
//       currentIndex() / currentText() / currentData() 获取当前值
//       setEditable(true) 可编辑组合框
//       setModel() 用自定义 Model 填充选项

#include "CitySelectorPanel.h"

#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QStandardItemModel>
#include <QTime>
#include <QVBoxLayout>

// ============================================================================
// CitySelectorPanel: 城市信息选择器，覆盖 QComboBox 四种核心用法
// ============================================================================
CitySelectorPanel::CitySelectorPanel(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QComboBox 综合演示 — 城市信息选择器");
    resize(600, 520);
    initUi();
}

/// @brief 初始化界面
void CitySelectorPanel::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ================================================================
    // 区域 1: addItem + userData — 只读城市选择
    // ================================================================
    auto *group1 = new QGroupBox("只读下拉 (addItem + userData)");
    auto *layout1 = new QHBoxLayout(group1);

    m_readonlyCombo = new QComboBox();
    m_readonlyCombo->addItem("北京", "BJ");
    m_readonlyCombo->addItem("上海", "SH");
    m_readonlyCombo->addItem("广州", "GZ");
    m_readonlyCombo->addItem("深圳", "SZ");
    m_readonlyCombo->addItem("杭州", "HZ");
    m_readonlyCombo->addItem("成都", "CD");

    // 在索引 0 位置前面插入"全部"选项（演示 insertItem）
    m_readonlyCombo->insertItem(0, "全部城市", "ALL");
    m_readonlyCombo->setCurrentIndex(0);

    m_readonlyLabel = new QLabel("城市编码: ALL");
    m_readonlyLabel->setStyleSheet("color: #1976D2; font-weight: bold;");

    layout1->addWidget(m_readonlyCombo, 1);
    layout1->addWidget(m_readonlyLabel, 1);

    connect(m_readonlyCombo, &QComboBox::currentIndexChanged,
            this, &CitySelectorPanel::onReadonlyComboChanged);

    mainLayout->addWidget(group1);

    // ================================================================
    // 区域 2: addItems + setEditable — 可编辑城市选择
    // ================================================================
    auto *group2 = new QGroupBox("可编辑下拉 (addItems + setEditable)");
    auto *layout2 = new QHBoxLayout(group2);

    m_editableCombo = new QComboBox();
    m_editableCombo->addItems(
        QStringList{"北京", "上海", "广州", "深圳", "杭州", "成都"});
    m_editableCombo->setEditable(true);
    m_editableCombo->setInsertPolicy(QComboBox::NoInsert);
    m_editableCombo->lineEdit()->setPlaceholderText("输入或选择城市...");

    m_editableLabel = new QLabel("当前输入: 北京");
    m_editableLabel->setStyleSheet("color: #333; font-weight: bold;");

    layout2->addWidget(m_editableCombo, 1);
    layout2->addWidget(m_editableLabel, 1);

    connect(m_editableCombo, &QComboBox::currentTextChanged,
            this, &CitySelectorPanel::onEditableComboTextChanged);

    mainLayout->addWidget(group2);

    // ================================================================
    // 区域 3: QStandardItemModel — 多角色数据
    // ================================================================
    auto *group3 = new QGroupBox("自定义 Model (QStandardItemModel)");
    auto *layout3 = new QHBoxLayout(group3);

    m_modelCombo = new QComboBox();
    setupCustomModel();

    m_modelLabel = new QLabel("编码: —  |  地区: —");
    m_modelLabel->setStyleSheet("color: #333; font-weight: bold;");

    layout3->addWidget(m_modelCombo, 1);
    layout3->addWidget(m_modelLabel, 1);

    connect(m_modelCombo, &QComboBox::currentIndexChanged,
            this, &CitySelectorPanel::onModelComboChanged);

    mainLayout->addWidget(group3);

    // ================================================================
    // 区域 4: 操作日志
    // ================================================================
    auto *logGroup = new QGroupBox("操作日志");
    auto *logLayout = new QVBoxLayout(logGroup);

    m_logEdit = new QPlainTextEdit();
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(140);
    m_logEdit->setPlaceholderText("操作日志将在此显示...");
    logLayout->addWidget(m_logEdit);

    mainLayout->addWidget(logGroup, 1);
}

/// @brief 为第三个 ComboBox 构建 QStandardItemModel
void CitySelectorPanel::setupCustomModel()
{
    auto *model = new QStandardItemModel(this);

    struct CityInfo
    {
        QString name;
        QString code;
        QString region;
    };

    QList<CityInfo> cities = {
        {"北京", "BJ", "华北"},
        {"上海", "SH", "华东"},
        {"广州", "GZ", "华南"},
        {"深圳", "SZ", "华南"},
        {"杭州", "HZ", "华东"},
        {"成都", "CD", "西南"},
    };

    for (const auto &city : cities) {
        auto *item = new QStandardItem(city.name);
        item->setData(city.code, Qt::UserRole);       // 城市编码
        item->setData(city.region, Qt::UserRole + 1); // 所在地区
        model->appendRow(item);
    }

    m_modelCombo->setModel(model);
}

/// @brief 只读下拉选项变化
void CitySelectorPanel::onReadonlyComboChanged(int index)
{
    if (index < 0) return;

    QString code = m_readonlyCombo->currentData().toString();
    QString name = m_readonlyCombo->currentText();
    m_readonlyLabel->setText("城市编码: " + code);

    appendLog("只读选择框 → " + name + " (编码: " + code + ")");
}

/// @brief 可编辑下拉文本变化
void CitySelectorPanel::onEditableComboTextChanged(const QString &text)
{
    int idx = m_editableCombo->findText(text);
    if (idx >= 0) {
        m_editableLabel->setText("当前输入: " + text);
        m_editableLabel->setStyleSheet("color: #333; font-weight: bold;");
    } else {
        m_editableLabel->setText("自定义输入: " + text);
        m_editableLabel->setStyleSheet("color: #D32F2F; font-weight: bold;");
    }

    appendLog("可编辑框文本变化 → " + text);
}

/// @brief 自定义 Model 下拉选项变化
void CitySelectorPanel::onModelComboChanged(int index)
{
    if (index < 0) return;

    QString code = m_modelCombo->currentData(Qt::UserRole).toString();
    QString region = m_modelCombo->currentData(Qt::UserRole + 1).toString();
    QString name = m_modelCombo->currentText();

    m_modelLabel->setText(
        "编码: " + code + "  |  地区: " + region);

    appendLog("Model 选择框 → " + name
              + " (编码: " + code + ", 地区: " + region + ")");
}

/// @brief 追加一条操作日志
void CitySelectorPanel::appendLog(const QString &message)
{
    QString timestamp = QTime::currentTime().toString("HH:mm:ss");
    m_logEdit->appendPlainText(
        "[" + timestamp + "] " + message);
}
