#include "check_box_demo.h"

#include <QApplication>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

CheckBoxDemo::CheckBoxDemo(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QCheckBox 综合演示 — 三态 & 层级复选");
    resize(640, 560);
    initUi();
}

void CheckBoxDemo::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("QCheckBox 综合演示");
    titleLabel->setFont(QFont("Arial", 15, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ================================================================
    // 上半部分: 全选/全不选 — 三态复选框
    // ================================================================
    auto *selectGroup = new QGroupBox("全选 / 全不选 (三态复选框)");
    auto *selectLayout = new QVBoxLayout(selectGroup);

    // 三态的"全选"复选框
    m_selectAllCheck = new QCheckBox("全选");
    m_selectAllCheck->setTristate(true);
    selectLayout->addWidget(m_selectAllCheck);

    // 五个子项复选框
    const QStringList items = {
        "选项 A — 基础功能",
        "选项 B — 高级功能",
        "选项 C — 实验性功能",
        "选项 D — 网络模块",
        "选项 E — 调试工具"
    };

    for (const auto &text : items) {
        auto *check = new QCheckBox(text);
        m_childChecks.push_back(check);
        selectLayout->addWidget(check);
    }

    // checkStateChanged(Qt::CheckState) 信号监听 — 展示三态值
    connect(m_selectAllCheck, &QCheckBox::checkStateChanged, this,
            [this](Qt::CheckState state) {
                if (m_updating) return;
                m_updating = true;

                // 全选/全不选 — 同步子项
                if (state == Qt::Checked) {
                    for (auto *c : m_childChecks) {
                        c->setChecked(true);
                    }
                } else if (state == Qt::Unchecked) {
                    for (auto *c : m_childChecks) {
                        c->setChecked(false);
                    }
                }
                // PartiallyChecked 只由 updateSelectAllState() 设置

                m_updating = false;
                updateStatus();
            });

    // 子项变化 → 更新"全选"状态
    for (auto *child : m_childChecks) {
        connect(child, &QCheckBox::checkStateChanged, this,
                [this](Qt::CheckState /*state*/) {
                    if (!m_updating) {
                        updateSelectAllState();
                        updateStatus();
                    }
                });
    }

    mainLayout->addWidget(selectGroup);

    // ================================================================
    // 下半部分: QTreeWidget 层级复选
    // ================================================================
    auto *treeGroup = new QGroupBox("层级复选 (QTreeWidget)");
    auto *treeLayout = new QVBoxLayout(treeGroup);

    m_treeWidget = new QTreeWidget();
    m_treeWidget->setHeaderLabel("项目文件");
    m_treeWidget->setColumnCount(1);

    // 连接 itemChanged 信号（在构建树之前连接，用 m_updating 防循环）
    connect(m_treeWidget, &QTreeWidget::itemChanged,
            this, [this](QTreeWidgetItem *item, int column) {
                if (m_updating || column != 0) return;

                m_updating = true;
                // 向下传播
                updateTreeChildren(item, item->checkState(0));
                m_updating = false;

                // 向上冒泡
                updateTreeParent(item->parent());
                updateStatus();
            });

    // 构建树形结构
    buildTree();

    treeLayout->addWidget(m_treeWidget);
    mainLayout->addWidget(treeGroup, 1);

    // ================================================================
    // 底部状态栏
    // ================================================================
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet(
        "background-color: #F5F5F5;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;"
        "padding: 8px;"
        "color: #555;"
        "font-size: 12px;");
    mainLayout->addWidget(m_statusLabel);

    // 初始状态
    updateStatus();
}

void CheckBoxDemo::buildTree()
{
    m_updating = true;

    const QStringList groupNames = {"项目组 Alpha", "项目组 Beta"};
    const QStringList fileNames = {"main.cpp", "config.json", "README.md"};

    for (const auto &group : groupNames) {
        auto *parentItem = new QTreeWidgetItem({group});
        parentItem->setCheckState(0, Qt::Unchecked);
        parentItem->setExpanded(true);

        for (const auto &file : fileNames) {
            auto *childItem = new QTreeWidgetItem({file});
            childItem->setCheckState(0, Qt::Unchecked);
            parentItem->addChild(childItem);
        }

        m_treeWidget->addTopLevelItem(parentItem);
    }

    m_updating = false;
}

void CheckBoxDemo::updateSelectAllState()
{
    m_updating = true;

    int checkedCount = 0;
    for (const auto *child : m_childChecks) {
        if (child->isChecked()) {
            ++checkedCount;
        }
    }

    if (checkedCount == 0) {
        m_selectAllCheck->setCheckState(Qt::Unchecked);
    } else if (checkedCount == static_cast<int>(m_childChecks.size())) {
        m_selectAllCheck->setCheckState(Qt::Checked);
    } else {
        m_selectAllCheck->setCheckState(Qt::PartiallyChecked);
    }

    m_updating = false;
}

void CheckBoxDemo::updateTreeChildren(QTreeWidgetItem *item, Qt::CheckState state)
{
    for (int i = 0; i < item->childCount(); ++i) {
        auto *child = item->child(i);
        child->setCheckState(0, state);
        updateTreeChildren(child, state);
    }
}

void CheckBoxDemo::updateTreeParent(QTreeWidgetItem *parent)
{
    if (parent == nullptr) return;

    int checkedCount = 0;
    int partialCount = 0;
    int total = parent->childCount();

    for (int i = 0; i < total; ++i) {
        auto state = parent->child(i)->checkState(0);
        if (state == Qt::Checked) {
            ++checkedCount;
        } else if (state == Qt::PartiallyChecked) {
            ++partialCount;
        }
    }

    m_updating = true;

    if (checkedCount == total) {
        parent->setCheckState(0, Qt::Checked);
    } else if (checkedCount == 0 && partialCount == 0) {
        parent->setCheckState(0, Qt::Unchecked);
    } else {
        parent->setCheckState(0, Qt::PartiallyChecked);
    }

    m_updating = false;

    // 继续向上冒泡
    updateTreeParent(parent->parent());
}

void CheckBoxDemo::updateStatus()
{
    // 统计复选框组
    int childChecked = 0;
    for (const auto *c : m_childChecks) {
        if (c->isChecked()) ++childChecked;
    }

    // 统计 QTreeWidget
    int treeChecked = 0;
    int treeTotal = 0;
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        auto *parent = m_treeWidget->topLevelItem(i);
        for (int j = 0; j < parent->childCount(); ++j) {
            ++treeTotal;
            if (parent->child(j)->checkState(0) == Qt::Checked) {
                ++treeChecked;
            }
        }
    }

    m_statusLabel->setText(
        QString("复选框组: %1/%2 已选中 | 树形结构: %3/%4 已选中")
            .arg(childChecked)
            .arg(static_cast<int>(m_childChecks.size()))
            .arg(treeChecked)
            .arg(treeTotal));
}
