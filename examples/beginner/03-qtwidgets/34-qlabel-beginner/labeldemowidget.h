// QtWidgets 入门示例 34: QLabel 文本与图像显示
// 演示：显示文本/HTML 富文本/图片
//       setAlignment 对齐与 setWordWrap 自动换行
//       setBuddy 关联快捷键到伙伴控件
//       linkActivated 信号处理超链接点击

#include <QLabel>
#include <QWidget>

// ============================================================================
// LabelDemoWidget: QLabel 综合演示窗口
// ============================================================================
class LabelDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LabelDemoWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

private:
    QLabel *m_infoLabel = nullptr;
};
