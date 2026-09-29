/// @file    filtered_font_combo.cpp
/// @brief   FilteredFontCombo 类实现——Writing System 过滤 + 字体预览面板。
///
/// 对应教程：进阶层 03-QtWidgets/28-QFontComboBox 进阶。

#include "filtered_font_combo.h"
#include "font_preview_delegate.h"

#include <QComboBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 书写系统到示例文本的映射
// ─────────────────────────────────────────────────────────────────────────────

namespace {

/// 书写系统枚举值到示例预览文本的映射表。
/// 用于在选中字体后，根据当前书写系统显示对应的示例文字。
struct WritingSystemSample
{
    QFontDatabase::WritingSystem ws;
    const char* text;
};

// clang-format off
static const WritingSystemSample kWritingSystemSamples[] = {
    { QFontDatabase::SimplifiedChinese,  "你好世界 Hello"   },
    { QFontDatabase::TraditionalChinese, "你好世界 Hello"   },
    { QFontDatabase::Japanese,           "こんにちは Hello"  },
    { QFontDatabase::Korean,             "안녕하세요 Hello"  },
    { QFontDatabase::Cyrillic,           "Привет Hello"     },
    { QFontDatabase::Arabic,             "مرحبا Hello"      },
    { QFontDatabase::Hebrew,             "שלום Hello"       },
    { QFontDatabase::Greek,              "Γειά Hello"       },
    { QFontDatabase::Thai,               "สวัสดี Hello"     },
    { QFontDatabase::Latin,              "Hello World 123"  },
};
// clang-format on

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

FilteredFontCombo::FilteredFontCombo(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);

    // 标题
    auto* title =
        new QLabel(QStringLiteral("QFontComboBox 进阶演示——Writing System 过滤 + 字体预览"));
    title->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 14px;"));
    mainLayout->addWidget(title);

    // --- 书写系统选择行 ---
    auto* wsRow = new QHBoxLayout;
    auto* wsLabel = new QLabel(QStringLiteral("书写系统:"));
    m_writingSystemCombo = new QComboBox;
    wsRow->addWidget(wsLabel);
    wsRow->addWidget(m_writingSystemCombo, 1);
    mainLayout->addLayout(wsRow);

    // --- 字体选择行 ---
    auto* fontRow = new QHBoxLayout;
    auto* fontLabel = new QLabel(QStringLiteral("选择字体:"));
    m_fontCombo = new QFontComboBox;
    m_fontCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    // 安装自定义 delegate——在下拉列表中用对应字体渲染预览
    m_fontCombo->setItemDelegate(new FontPreviewDelegate(m_fontCombo));

    fontRow->addWidget(fontLabel);
    fontRow->addWidget(m_fontCombo, 1);
    mainLayout->addLayout(fontRow);

    // --- 信息行 ---
    m_infoLabel = new QLabel;
    mainLayout->addWidget(m_infoLabel);

    // --- 预览区域 ---
    auto* previewTitle = new QLabel(QStringLiteral("字体预览:"));
    previewTitle->setStyleSheet(QStringLiteral("font-weight: bold;"));
    mainLayout->addWidget(previewTitle);

    m_previewLabel = new QLabel;
    m_previewLabel->setFrameShape(QFrame::Box);
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setMinimumHeight(80);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_previewLabel);

    mainLayout->addStretch();

    // --- 信号槽连接 ---
    connect(m_writingSystemCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &FilteredFontCombo::applyWritingSystemFilter);

    connect(m_fontCombo, &QFontComboBox::currentFontChanged, this,
            &FilteredFontCombo::updatePreview);

    // 初始化
    populateWritingSystems();
    updatePreview();

    setWindowTitle(QStringLiteral("QFontComboBox Advanced Demo"));
    resize(550, 350);
}

// ─────────────────────────────────────────────────────────────────────────────
// 填充书写系统下拉框
// ─────────────────────────────────────────────────────────────────────────────

void FilteredFontCombo::populateWritingSystems()
{
    // 在最前面插入 "Any" 选项表示不过滤
    m_writingSystemCombo->addItem(QStringLiteral("Any（不过滤）"),
                                  static_cast<int>(QFontDatabase::Any));

    // 遍历系统支持的所有书写系统
    for (auto ws : QFontDatabase::writingSystems()) {
        // QFontDatabase::Any 已经手动添加为第一项，跳过
        if (ws == QFontDatabase::Any) {
            continue;
        }
        m_writingSystemCombo->addItem(QFontDatabase::writingSystemName(ws),
                                      static_cast<int>(ws));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 根据书写系统过滤字体列表
// ─────────────────────────────────────────────────────────────────────────────

void FilteredFontCombo::applyWritingSystemFilter()
{
    auto ws = static_cast<QFontDatabase::WritingSystem>(
        m_writingSystemCombo->currentData().toInt());

    // setWritingSystem 触发 QFontComboBox 内部模型重建
    m_fontCombo->setWritingSystem(ws);

    // 重建后选中项可能变化，更新预览
    updatePreview();
}

// ─────────────────────────────────────────────────────────────────────────────
// 更新预览标签
// ─────────────────────────────────────────────────────────────────────────────

void FilteredFontCombo::updatePreview()
{
    QFont font = m_fontCombo->currentFont();
    QString sample = sampleTextForCurrentWritingSystem();

    // 设置预览标签的字体和文本
    font.setPointSize(20);
    m_previewLabel->setFont(font);
    m_previewLabel->setText(sample);

    // 更新信息标签
    m_infoLabel->setText(
        QStringLiteral("当前字体: %1 | 书写系统: %2")
            .arg(font.family(), m_writingSystemCombo->currentText()));
}

// ─────────────────────────────────────────────────────────────────────────────
// 获取当前书写系统对应的示例文本
// ─────────────────────────────────────────────────────────────────────────────

QString FilteredFontCombo::sampleTextForCurrentWritingSystem() const
{
    auto ws = static_cast<QFontDatabase::WritingSystem>(
        m_writingSystemCombo->currentData().toInt());

    for (const auto& entry : kWritingSystemSamples) {
        if (entry.ws == ws) {
            return QString::fromUtf8(entry.text);
        }
    }

    // 默认文本（Any 或未匹配的书写系统）
    return QStringLiteral("Hello World 你好世界 123");
}
