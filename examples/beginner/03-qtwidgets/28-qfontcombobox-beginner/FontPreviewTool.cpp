// QtWidgets 入门示例 28: QFontComboBox 字体选择下拉框
// 演示：setFontFilters() 过滤字体类型（等宽/比例/全部）
//       currentFont() 获取选中字体
//       在文字编辑器中实时预览字体变化
//       字体名称的本地化显示

#include "FontPreviewTool.h"

// ============================================================================
// FontPreviewTool: 字体预览工具，覆盖 QFontComboBox 核心用法
// ============================================================================
FontPreviewTool::FontPreviewTool(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QFontComboBox 综合演示 — 字体预览工具");
    resize(720, 480);
    initUi();
}

/// @brief 初始化界面
void FontPreviewTool::initUi()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ================================================================
    // 左侧：控制面板
    // ================================================================
    auto *controlGroup = new QGroupBox("字体设置");
    auto *formLayout = new QFormLayout(controlGroup);
    formLayout->setSpacing(10);

    // ---- 字体选择 ----
    m_fontCombo = new QFontComboBox();
    m_fontCombo->setWritingSystem(QFontDatabase::Any);
    formLayout->addRow("字体:", m_fontCombo);

    // ---- 字体过滤器 ----
    m_filterCombo = new QComboBox();
    m_filterCombo->addItem("全部字体", QVariant::fromValue(
        static_cast<int>(QFontComboBox::AllFonts)));
    m_filterCombo->addItem("等宽字体", QVariant::fromValue(
        static_cast<int>(QFontComboBox::MonospacedFonts)));
    m_filterCombo->addItem("比例字体", QVariant::fromValue(
        static_cast<int>(QFontComboBox::ProportionalFonts)));
    m_filterCombo->addItem("可缩放字体", QVariant::fromValue(
        static_cast<int>(QFontComboBox::ScalableFonts)));
    formLayout->addRow("过滤器:", m_filterCombo);

    // ---- 书写系统 ----
    m_writingSystemCombo = new QComboBox();
    m_writingSystemCombo->addItem("任意",
        static_cast<int>(QFontDatabase::Any));
    m_writingSystemCombo->addItem("简体中文",
        static_cast<int>(QFontDatabase::SimplifiedChinese));
    m_writingSystemCombo->addItem("日文",
        static_cast<int>(QFontDatabase::Japanese));
    m_writingSystemCombo->addItem("韩文",
        static_cast<int>(QFontDatabase::Korean));
    m_writingSystemCombo->addItem("拉丁文",
        static_cast<int>(QFontDatabase::Latin));
    formLayout->addRow("书写系统:", m_writingSystemCombo);

    // ---- 字体大小 ----
    m_sizeCombo = new QComboBox();
    for (int s = 8; s <= 72; s += 2) {
        m_sizeCombo->addItem(QString::number(s), s);
    }
    m_sizeCombo->setCurrentText("14");
    formLayout->addRow("大小:", m_sizeCombo);

    // ---- 粗体 / 斜体 ----
    auto *styleLayout = new QHBoxLayout();
    m_boldCheck = new QCheckBox("粗体");
    m_italicCheck = new QCheckBox("斜体");
    styleLayout->addWidget(m_boldCheck);
    styleLayout->addWidget(m_italicCheck);
    styleLayout->addStretch();
    formLayout->addRow("样式:", styleLayout);

    // ---- 状态栏 ----
    m_statusLabel = new QLabel("当前字体信息将在此显示");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet(
        "color: #555; font-size: 11px;"
        "background-color: #F5F5F5;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px; padding: 8px;");
    formLayout->addRow(m_statusLabel);

    controlGroup->setFixedWidth(280);
    mainLayout->addWidget(controlGroup);

    // ================================================================
    // 右侧：预览区域
    // ================================================================
    auto *previewGroup = new QGroupBox("字体预览");
    auto *previewLayout = new QVBoxLayout(previewGroup);

    m_previewEdit = new QTextEdit();
    m_previewEdit->setPlainText(
        "The quick brown fox jumps over the lazy dog.\n"
        "0123456789 !@#$%^&*()\n\n"
        "敏捷的棕色狐狸跳过了懒狗。天地玄黄，宇宙洪荒。\n"
        "寒蝉效应，蛙鸣蝉噪，春风又绿江南岸。\n\n"
        "ABCDEFGHIJKLM\n"
        "abcdefghijklm\n\n"
        "春眠不觉晓，处处闻啼鸟。\n"
        "夜来风雨声，花落知多少。");
    previewLayout->addWidget(m_previewEdit);

    mainLayout->addWidget(previewGroup, 1);

    // ================================================================
    // 信号连接
    // ================================================================
    connect(m_fontCombo, &QFontComboBox::currentFontChanged,
            this, &FontPreviewTool::updatePreviewFont);
    connect(m_filterCombo, &QComboBox::currentIndexChanged,
            this, &FontPreviewTool::onFilterChanged);
    connect(m_writingSystemCombo, &QComboBox::currentIndexChanged,
            this, &FontPreviewTool::onWritingSystemChanged);
    connect(m_sizeCombo, &QComboBox::currentIndexChanged,
            this, &FontPreviewTool::updatePreviewFont);
    connect(m_boldCheck, &QCheckBox::toggled,
            this, &FontPreviewTool::updatePreviewFont);
    connect(m_italicCheck, &QCheckBox::toggled,
            this, &FontPreviewTool::updatePreviewFont);

    // 初始化预览
    updatePreviewFont();
}

/// @brief 字体过滤器变化
void FontPreviewTool::onFilterChanged(int index)
{
    if (index < 0) return;

    auto filter = static_cast<QFontComboBox::FontFilter>(
        m_filterCombo->currentData().toInt());
    m_fontCombo->setFontFilters(filter);
    updatePreviewFont();
}

/// @brief 书写系统变化
void FontPreviewTool::onWritingSystemChanged(int index)
{
    if (index < 0) return;

    auto ws = static_cast<QFontDatabase::WritingSystem>(
        m_writingSystemCombo->currentData().toInt());
    m_fontCombo->setWritingSystem(ws);
    updatePreviewFont();
}

/// @brief 根据当前所有控件的设置更新预览字体
void FontPreviewTool::updatePreviewFont()
{
    QFont font = m_fontCombo->currentFont();

    int size = m_sizeCombo->currentData().toInt();
    font.setPointSize(size);
    font.setBold(m_boldCheck->isChecked());
    font.setItalic(m_italicCheck->isChecked());

    m_previewEdit->setFont(font);

    // 用 QFontInfo 获取实际渲染信息
    QFontInfo info(font);
    m_statusLabel->setText(
        QString("family: %1\nstyleName: %2\n"
                "pointSize: %3 | bold: %4 | italic: %5")
            .arg(info.family())
            .arg(info.styleName())
            .arg(info.pointSize())
            .arg(info.bold() ? "true" : "false")
            .arg(info.italic() ? "true" : "false"));
}
