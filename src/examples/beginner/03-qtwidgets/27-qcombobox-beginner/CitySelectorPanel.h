// QtWidgets 入门示例 27: QComboBox 下拉选择框
// 演示：addItem / addItems / insertItem 添加选项
//       currentIndex() / currentText() / currentData() 获取当前值
//       setEditable(true) 可编辑组合框
//       setModel() 用自定义 Model 填充选项

#include <QComboBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QWidget>

// ============================================================================
// CitySelectorPanel: 城市信息选择器，覆盖 QComboBox 四种核心用法
// ============================================================================
class CitySelectorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CitySelectorPanel(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 为第三个 ComboBox 构建 QStandardItemModel
    void setupCustomModel();

    /// @brief 只读下拉选项变化
    void onReadonlyComboChanged(int index);

    /// @brief 可编辑下拉文本变化
    void onEditableComboTextChanged(const QString &text);

    /// @brief 自定义 Model 下拉选项变化
    void onModelComboChanged(int index);

    /// @brief 追加一条操作日志
    void appendLog(const QString &message);

private:
    QComboBox *m_readonlyCombo = nullptr;
    QLabel *m_readonlyLabel = nullptr;

    QComboBox *m_editableCombo = nullptr;
    QLabel *m_editableLabel = nullptr;

    QComboBox *m_modelCombo = nullptr;
    QLabel *m_modelLabel = nullptr;

    QPlainTextEdit *m_logEdit = nullptr;
};
