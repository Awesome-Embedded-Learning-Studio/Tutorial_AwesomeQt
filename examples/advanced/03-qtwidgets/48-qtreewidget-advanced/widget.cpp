/// @file    widget.cpp
/// @brief   演示 QTreeWidget 延迟加载子节点（懒加载）的实现。
///
/// 对应教程：进阶层 03-QtWidgets/48-qtreewidget-advanced。

#include "widget.h"

// ---------------------------------------------------------------------------
// Widget
// ---------------------------------------------------------------------------

Widget::Widget(QWidget* parent)
    : QWidget(parent),
      m_treeWidget(new QTreeWidget(this)),
      m_statusLabel(new QLabel(QStringLiteral("点击展开目录节点加载子项"), this))
{
    auto* mainLayout = new QVBoxLayout(this);

    // -- 配置 QTreeWidget --
    m_treeWidget->setHeaderLabel(QStringLiteral("文件系统（模拟）"));
    // @note 设置缩进让层级结构更清晰
    m_treeWidget->setIndentation(24);

    // -- 添加顶层目录项 --
    createTopLevelItem(QStringLiteral("Documents"), QStringLiteral("docs"));
    createTopLevelItem(QStringLiteral("Pictures"), QStringLiteral("pics"));
    createTopLevelItem(QStringLiteral("Music"), QStringLiteral("music"));
    createTopLevelItem(QStringLiteral("Projects"), QStringLiteral("projects"));
    createTopLevelItem(QStringLiteral("Downloads"), QStringLiteral("downloads"));

    // -- 连接信号 --
    // @note itemExpanded 在用户展开节点时触发，用于实现懒加载
    connect(m_treeWidget, &QTreeWidget::itemExpanded, this, &Widget::onItemExpanded);
    connect(m_treeWidget, &QTreeWidget::currentItemChanged, this,
            &Widget::onCurrentItemChanged);

    // -- 组装布局 --
    mainLayout->addWidget(m_treeWidget, 1);
    mainLayout->addWidget(m_statusLabel);

    setWindowTitle(QStringLiteral("QTreeWidget 延迟加载子节点"));
    resize(400, 500);
}

QTreeWidgetItem* Widget::createTopLevelItem(const QString& name, const QString& type)
{
    auto* item = new QTreeWidgetItem(m_treeWidget);
    item->setText(0, name);
    // @note 通过 Qt::UserRole 存储目录类型，用于后续生成不同类型的子项
    item->setData(0, Qt::UserRole, type);

    // @note 添加一个占位子项，使顶层项显示展开箭头
    // 真实的子项在首次展开时才加载
    auto* placeholder = new QTreeWidgetItem(item);
    placeholder->setText(0, QStringLiteral("Loading..."));
    // 用 UserRole 标记这是占位项
    placeholder->setData(0, Qt::UserRole, QString(kPlaceholder));

    return item;
}

void Widget::loadChildren(QTreeWidgetItem* item)
{
    if (!item) {
        return;
    }

    // 检查是否已经有真实子项（非占位）
    if (item->childCount() > 0) {
        auto* firstChild = item->child(0);
        if (firstChild->data(0, Qt::UserRole).toString() != kPlaceholder) {
            // @note 已经加载过真实子项，跳过重复加载
            return;
        }
    }

    // @note 先移除占位子项，再填充真实子节点
    // takeChildren 返回所有子项的列表，我们需要清空再重新填充
    QList<QTreeWidgetItem*> oldChildren = item->takeChildren();
    // Qt 对象树管理子项生命周期，手动删除旧的占位项
    qDeleteAll(oldChildren);

    // 获取目录类型，用于模拟不同内容
    QString dirType = item->data(0, Qt::UserRole).toString();
    QStringList entries = simulateChildEntries(item->text(0), dirType);

    for (const QString& entry : entries) {
        auto* child = new QTreeWidgetItem(item);
        child->setText(0, entry);

        // @note 如果子项本身可能是目录（含 "folder" 字样），也加占位项
        if (entry.contains(QStringLiteral("folder"), Qt::CaseInsensitive)) {
            auto* subPlaceholder = new QTreeWidgetItem(child);
            subPlaceholder->setText(0, QStringLiteral("Loading..."));
            subPlaceholder->setData(0, Qt::UserRole, QString(kPlaceholder));
        }
    }
}

QStringList Widget::simulateChildEntries(const QString& dirName,
                                         const QString& dirType) const
{
    QStringList entries;

    // @note 模拟不同目录类型的文件内容
    // 真实应用中应使用 QDir::entryList 读取实际文件
    if (dirType == QStringLiteral("docs")) {
        entries << QStringLiteral("report_final.docx")
                << QStringLiteral("notes.txt")
                << QStringLiteral("work_folder")
                << QStringLiteral("resume.pdf")
                << QStringLiteral("meeting_notes.md");
    } else if (dirType == QStringLiteral("pics")) {
        entries << QStringLiteral("vacation")
                << QStringLiteral("screenshots")
                << QStringLiteral("photo_001.jpg")
                << QStringLiteral("photo_002.png")
                << QStringLiteral("avatar.bmp");
    } else if (dirType == QStringLiteral("music")) {
        entries << QStringLiteral("playlist.m3u")
                << QStringLiteral("album_folder")
                << QStringLiteral("track01.mp3")
                << QStringLiteral("track02.flac")
                << QStringLiteral("podcast.mp3");
    } else if (dirType == QStringLiteral("projects")) {
        entries << QStringLiteral("project_alpha")
                << QStringLiteral("project_beta")
                << QStringLiteral("README.md")
                << QStringLiteral("CMakeLists.txt")
                << QStringLiteral("src_folder");
    } else if (dirType == QStringLiteral("downloads")) {
        entries << QStringLiteral("archive.zip")
                << QStringLiteral("installer.exe")
                << QStringLiteral("document.pdf")
                << QStringLiteral("temp_folder")
                << QStringLiteral("data.csv");
    } else {
        entries << QStringLiteral("file_1.dat") << QStringLiteral("file_2.dat");
    }

    return entries;
}

void Widget::onItemExpanded(QTreeWidgetItem* item)
{
    // @note 懒加载核心：在首次展开时才填充子节点
    loadChildren(item);
    m_statusLabel->setText(
        QStringLiteral("已展开: %1  |  子项数: %2")
            .arg(item->text(0))
            .arg(item->childCount()));
}

void Widget::onCurrentItemChanged(QTreeWidgetItem* current,
                                  QTreeWidgetItem* /*previous*/)
{
    if (current) {
        m_statusLabel->setText(
            QStringLiteral("选中: %1").arg(current->text(0)));
    }
}
