/// @file    font_dialog_demo.cpp
/// @brief   FontDialogDemo 实现——字体过滤与预览。
///
/// 对应教程：进阶层 03-QtWidgets/65-QFontDialog 进阶。

#include "font_dialog_demo.h"

#include <QCheckBox>
#include <QFontDatabase>
#include <QFontDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

FontDialogDemo::FontDialogDemo(QWidget* parent)
    : QWidget(parent)
    , m_previewEdit(nullptr)
    , m_fontInfoLabel(nullptr)
    , m_staticBtn(nullptr)
    , m_objectBtn(nullptr)
    , m_monospaceOnly(false)
{
    setupUI();
}

void FontDialogDemo::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);

    // 预览区域
    m_previewEdit = new QPlainTextEdit(this);
    m_previewEdit->setPlainText(
        tr("The quick brown fox jumps over the lazy dog.\n"
           "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
           "abcdefghijklmnopqrstuvwxyz\n"
           "0123456789 !@#$%^&*()"));
    m_previewEdit->setMinimumHeight(120);
    mainLayout->addWidget(m_previewEdit);

    // 字体信息
    m_fontInfoLabel = new QLabel(this);
    m_fontInfoLabel->setWordWrap(true);
    mainLayout->addWidget(m_fontInfoLabel);

    // 等宽字体过滤复选框
    auto* monoCheck = new QCheckBox(tr("Monospace fonts only"), this);
    connect(monoCheck, &QCheckBox::toggled,
            this, &FontDialogDemo::toggleMonospaceOnly);
    mainLayout->addWidget(monoCheck);

    // 两种选择方式的按钮
    m_staticBtn = new QPushButton(tr("Pick Font (Static Method)"), this);
    m_objectBtn = new QPushButton(tr("Pick Font (Dialog Object)"), this);

    connect(m_staticBtn, &QPushButton::clicked,
            this, &FontDialogDemo::pickFontStatic);
    connect(m_objectBtn, &QPushButton::clicked,
            this, &FontDialogDemo::pickFontObject);

    mainLayout->addWidget(m_staticBtn);
    mainLayout->addWidget(m_objectBtn);

    // 显示初始字体信息
    applyFont(font());

    setWindowTitle(tr("QFontDialog Advanced Demo"));
    resize(450, 400);
}

void FontDialogDemo::pickFontStatic()
{
    bool ok = false;
    // @note getFont 静态方法是最简单的用法，返回选中的字体
    QFont selectedFont = QFontDialog::getFont(
        &ok, m_previewEdit->font(), this, tr("Select Font"));

    if (ok) {
        applyFont(selectedFont);
    }
}

void FontDialogDemo::pickFontObject()
{
    // @note 使用 QFontDialog 对象可以访问更多选项
    QFontDialog dialog(this);
    dialog.setWindowTitle(tr("Select Font"));
    dialog.setCurrentFont(m_previewEdit->font());

    // @note 如果启用了等宽过滤，通过 QFontDialogOptions 限制
    if (m_monospaceOnly) {
        // QFontDialog 没有直接的字体过滤 API，
        // 但可以通过设置 options 来影响行为
        dialog.setOption(QFontDialog::MonospacedFonts);
    }

    if (dialog.exec() == QDialog::Accepted) {
        applyFont(dialog.currentFont());
    }
}

void FontDialogDemo::toggleMonospaceOnly(bool checked)
{
    m_monospaceOnly = checked;
}

void FontDialogDemo::applyFont(const QFont& font)
{
    m_previewEdit->setFont(font);

    // @note QFontInfo 提供实际使用的字体信息（可能和请求的不完全一致）
    const QFontInfo info(font);

    QFontDatabase db;
    const QString styles = db.styles(info.family()).join(QStringLiteral(", "));

    m_fontInfoLabel->setText(
        tr("Family: %1\n"
           "Style: %2\n"
           "Size: %3 pt\n"
           "Weight: %4\n"
           "Available styles: %5")
            .arg(info.family())
            .arg(info.styleName())
            .arg(info.pointSize())
            .arg(info.weight())
            .arg(styles));
}
