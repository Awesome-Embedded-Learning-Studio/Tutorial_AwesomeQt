// QtWidgets 入门示例 52: QHeaderView 表头控件
// 演示：setSectionResizeMode 固定/自适应/拉伸
//       setSortIndicator 排序指示
//       hideSection / showSection 隐藏列
//       自定义表头绘制（继承 QHeaderView + paintSection）

#include <QHeaderView>
#include <QList>

class QPainter;
class QRect;

// ============================================================================
// ColoredHeaderView: 自定义表头 —— 带彩色方块标记 + 渐变背景
// ============================================================================
class ColoredHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit ColoredHeaderView(Qt::Orientation orientation,
                                QWidget *parent = nullptr);

protected:
    void paintSection(QPainter *painter, const QRect &rect,
                      int logicalIndex) const override;

private:
    /// @brief 绘制排序方向箭头
    void drawSortArrow(QPainter *painter, const QRect &rect) const;

    QList<QColor> kColors;
};
