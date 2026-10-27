// QtWidgets 入门示例 28: QFontComboBox 字体选择下拉框
// 演示：setFontFilters() 过滤字体类型（等宽/比例/全部）
//       currentFont() 获取选中字体
//       在文字编辑器中实时预览字体变化
//       字体名称的本地化显示

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFontInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

// ============================================================================
// FontPreviewTool: 字体预览工具，覆盖 QFontComboBox 核心用法
// ============================================================================
class FontPreviewTool : public QWidget
{
    Q_OBJECT

public:
    explicit FontPreviewTool(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();
    /// @brief 字体过滤器变化
    void onFilterChanged(int index);
    /// @brief 书写系统变化
    void onWritingSystemChanged(int index);
    /// @brief 根据当前所有控件的设置更新预览字体
    void updatePreviewFont();

private:
    QFontComboBox *m_fontCombo = nullptr;
    QComboBox *m_filterCombo = nullptr;
    QComboBox *m_writingSystemCombo = nullptr;
    QComboBox *m_sizeCombo = nullptr;
    QCheckBox *m_boldCheck = nullptr;
    QCheckBox *m_italicCheck = nullptr;
    QTextEdit *m_previewEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
};
