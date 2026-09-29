// QtWidgets 入门示例 65: QFontDialog 字体选择对话框
// 演示：getFont 模态选择
//       setCurrentFont 初始预选
//       currentFontChanged 实时预览
//       QFontDatabase 过滤等宽字体

#ifndef MONOFONTDIALOG_H
#define MONOFONTDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

// ============================================================================
// MonoFontDialog: 自定义等宽字体选择对话框
// ============================================================================
class MonoFontDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MonoFontDialog(const QFont &currentFont,
                            QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("选择等宽字体");
        setMinimumWidth(400);

        auto *layout = new QVBoxLayout(this);

        // ---- 表单区域 ----
        auto *formLayout = new QFormLayout;

        m_familyCombo = new QComboBox;
        populateMonoFonts(currentFont.family());
        formLayout->addRow("字体族:", m_familyCombo);

        m_sizeSpin = new QSpinBox;
        m_sizeSpin->setRange(8, 72);
        m_sizeSpin->setValue(currentFont.pointSize());
        formLayout->addRow("字号:", m_sizeSpin);

        auto *styleLayout = new QHBoxLayout;
        m_boldCheck = new QCheckBox("粗体");
        m_italicCheck = new QCheckBox("斜体");
        m_boldCheck->setChecked(currentFont.bold());
        m_italicCheck->setChecked(currentFont.italic());
        styleLayout->addWidget(m_boldCheck);
        styleLayout->addWidget(m_italicCheck);
        styleLayout->addStretch();
        formLayout->addRow("样式:", styleLayout);

        layout->addLayout(formLayout);

        // ---- 预览标签 ----
        auto *previewLabel = new QLabel("预览:");
        layout->addWidget(previewLabel);

        m_previewText = new QLabel(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
            "abcdefghijklmnopqrstuvwxyz\n"
            "0123456789 !@#$%^&*()\n"
            "int main() { return 0; }");
        m_previewText->setFrameStyle(
            QFrame::StyledPanel | QFrame::Sunken);
        m_previewText->setAlignment(Qt::AlignCenter);
        m_previewText->setMinimumHeight(80);
        m_previewText->setWordWrap(true);
        layout->addWidget(m_previewText);

        // ---- 按钮 ----
        auto *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);
        layout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted,
                this, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected,
                this, &QDialog::reject);

        // ---- 实时预览更新 ----
        connect(m_familyCombo, &QComboBox::currentTextChanged,
                this, &MonoFontDialog::updatePreview);
        connect(m_sizeSpin,
                QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MonoFontDialog::updatePreview);
        connect(m_boldCheck, &QCheckBox::checkStateChanged,
                this, &MonoFontDialog::updatePreview);
        connect(m_italicCheck, &QCheckBox::checkStateChanged,
                this, &MonoFontDialog::updatePreview);

        updatePreview();
    }

    QFont selectedFont() const
    {
        QFont font(m_familyCombo->currentText(),
                   m_sizeSpin->value());
        font.setBold(m_boldCheck->isChecked());
        font.setItalic(m_italicCheck->isChecked());
        return font;
    }

private:
    /// @brief 填充等宽字体列表并尝试预选当前字体
    void populateMonoFonts(const QString &currentFamily)
    {
        QFontDatabase fontDb;
        const QStringList allFamilies = fontDb.families();

        // 只保留等宽字体
        QStringList monoFamilies;
        int currentIdx = -1;
        for (const QString &family : allFamilies) {
            if (fontDb.isFixedPitch(family)) {
                monoFamilies.append(family);
                if (family == currentFamily) {
                    currentIdx =
                        monoFamilies.size() - 1;
                }
            }
        }

        m_familyCombo->addItems(monoFamilies);

        // 尝试预选当前字体
        if (currentIdx >= 0) {
            m_familyCombo->setCurrentIndex(currentIdx);
        }
    }

    void updatePreview()
    {
        m_previewText->setFont(selectedFont());
    }

    QComboBox *m_familyCombo = nullptr;
    QSpinBox *m_sizeSpin = nullptr;
    QCheckBox *m_boldCheck = nullptr;
    QCheckBox *m_italicCheck = nullptr;
    QLabel *m_previewText = nullptr;
};

#endif // MONOFONTDIALOG_H
