/**
 * @file proxy-model_window.h
 * @brief Proxy-Model 演示主窗口——源模型 + 代理层 + 表格 + 搜索框 + 排序/过滤控件
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include "proxy-model.h"

#include <QMainWindow>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QStandardItemModel;
class QTableView;

/// @brief 演示窗口：QStandardItemModel 源数据 → AwesomeQt::SortFilterProxyModel 代理 → QTableView
/// 展示。
///
/// 顶部控件实时驱动代理：
/// - 搜索框：setFilterFixedString（代理自动刷新）；
/// - 排序方向开关：setSortOrder + 按年龄列数值排序；
/// - 过滤范围下拉：kActiveColumn（仅当前列）/ kAllColumns（全列扫描）。
class ProxyModelWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit ProxyModelWindow(QWidget* parent = nullptr);

  private:
    void setup_source_model();
    void setup_proxy();
    void setup_widgets();

    /// @brief 响应搜索框文本变化，把关键词投喂给代理。
    /// @param[in] text 当前搜索关键词。
    void onSearchChanged(const QString& text);
    /// @brief 切换排序方向（升序 / 降序）。
    /// @param[in] ascending true=升序，false=降序。
    void onSortDirectionChanged(bool ascending);
    /// @brief 切换过滤匹配范围。
    /// @param[in] index 下拉索引：0=kActiveColumn，1=kAllColumns。
    void onFilterScopeChanged(int index);

    QStandardItemModel* source_model_{nullptr};
    AwesomeQt::SortFilterProxyModel* proxy_model_{nullptr};

    QLineEdit* search_edit_{nullptr};
    QCheckBox* ascending_check_{nullptr};
    QComboBox* scope_combo_{nullptr};
    QTableView* table_view_{nullptr};
};
