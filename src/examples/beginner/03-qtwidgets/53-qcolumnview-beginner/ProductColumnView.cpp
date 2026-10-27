#include "ProductColumnView.h"

#include <QFrame>
#include <QLabel>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QVariantMap>
#include <QWidget>

ProductColumnView::ProductColumnView(QWidget *parent)
    : QColumnView(parent)
{
    setColumnWidths({160, 160, 180});

    // 构建预览面板
    auto *previewContainer = new QWidget;
    previewContainer->setMinimumWidth(260);

    auto *previewLayout = new QVBoxLayout(previewContainer);
    previewLayout->setContentsMargins(16, 16, 16, 16);

    // 标题
    m_previewTitle = new QLabel("选择一个产品查看详情");
    m_previewTitle->setWordWrap(true);
    m_previewTitle->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #212529;");
    previewLayout->addWidget(m_previewTitle);

    // 分隔线
    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #DEE2E6;");
    previewLayout->addWidget(separator);

    // 详细信息
    m_previewBody = new QLabel;
    m_previewBody->setWordWrap(true);
    m_previewBody->setAlignment(
        Qt::AlignTop | Qt::AlignLeft);
    m_previewBody->setStyleSheet(
        "font-size: 13px; color: #495057;"
        "line-height: 1.6;");
    previewLayout->addWidget(m_previewBody, 1);

    previewLayout->addStretch();

    setPreviewWidget(previewContainer);

    // 连接 updatePreviewWidget 信号以更新预览面板
    connect(this, &QColumnView::updatePreviewWidget,
            this, &ProductColumnView::handlePreviewUpdate);
}

/// @brief 选中条目变化时更新预览面板
void ProductColumnView::handlePreviewUpdate(const QModelIndex &index)
{
    if (!index.isValid()) {
        m_previewTitle->setText("选择一个产品查看详情");
        m_previewBody->clear();
        return;
    }

    QString name = model()->data(
        index, Qt::DisplayRole).toString();

    // 尝试读取存储在 UserRole 中的产品详情
    QVariant userData = model()->data(index, Qt::UserRole);
    if (userData.isValid()
        && userData.canConvert<QVariantMap>()) {
        QVariantMap info = userData.toMap();
        showProductDetail(name, info);
    } else {
        // 分类节点：显示基本信息
        showCategoryInfo(name, index);
    }
}

/// @brief 显示产品详细信息
void ProductColumnView::showProductDetail(const QString &name,
                                           const QVariantMap &info)
{
    m_previewTitle->setText(name);

    QString body;
    body += QString("分类: %1\n").arg(
        info.value("category", "--").toString());
    body += QString("价格: %1 元\n").arg(
        info.value("price", 0.0).toString());
    body += QString("品牌: %1\n").arg(
        info.value("brand", "--").toString());
    body += QString("\n%1").arg(
        info.value("description", "").toString());

    m_previewBody->setText(body);
}

/// @brief 显示分类基本信息
void ProductColumnView::showCategoryInfo(const QString &name,
                                          const QModelIndex &index)
{
    m_previewTitle->setText(name);

    int childCount = model()->rowCount(index);
    QString body;
    body += QString("类型: 分类节点\n");
    body += QString("子项数量: %1\n").arg(childCount);
    body += QString("深度: %1 层\n").arg(getDepth(index));

    if (childCount > 0) {
        body += "\n点击查看子分类或产品";
    }

    m_previewBody->setText(body);
}

/// @brief 计算条目在树中的深度
int ProductColumnView::getDepth(const QModelIndex &index) const
{
    int depth = 0;
    QModelIndex p = index.parent();
    while (p.isValid()) {
        ++depth;
        p = p.parent();
    }
    return depth;
}
