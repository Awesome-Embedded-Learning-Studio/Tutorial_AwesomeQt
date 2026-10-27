// QtWidgets 入门示例 40: QTabBar 独立标签栏
// 演示：QTabBar 与 QTabWidget 的区别（可独立使用）
//       自定义标签栏 + 自定义内容区域组合
//       tabCloseRequested 信号实现可关闭标签页
//       setMovable(true) 可拖动标签排序

#include <QLabel>
#include <QStackedWidget>
#include <QTabBar>
#include <QWidget>

// ============================================================================
// TabBarDemoWidget: QTabBar 综合演示窗口
// ============================================================================
class TabBarDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabBarDemoWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 为指定文档名创建编辑器和预览页
    void createDocumentContent(const QString &name,
                               const QString &content);

    /// @brief 移除指定索引的文档（标签 + 编辑器 + 预览）
    void removeDocument(int index);

    /// @brief 更新底部状态栏
    void updateStatus();

private:
    QTabBar *m_tabBar = nullptr;
    QStackedWidget *m_editorStack = nullptr;
    QStackedWidget *m_previewStack = nullptr;
    QLabel *m_statusLabel = nullptr;
};
