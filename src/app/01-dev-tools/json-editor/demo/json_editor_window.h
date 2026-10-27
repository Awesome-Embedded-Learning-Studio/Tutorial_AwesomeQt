/**
 * @file json_editor_window.h
 * @brief JSON 编辑器主窗口——编辑区 + 树视图 + 校验/格式化/紧凑/打开/保存
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QJsonDocument>
#include <QMainWindow>
#include <QString>

class QAction;
class QLabel;
class QPlainTextEdit;
class QTreeWidget;
class QTreeWidgetItem;

/// @brief JSON 编辑器主窗口（app 栏整机范式）。
///
/// QMainWindow + 菜单/工具栏/状态栏，中央区是 QSplitter 水平分两栏：
/// 左 QPlainTextEdit 编辑区，右 QTreeWidget 树视图。
/// 能力：Open(Ctrl+O) / Save(Ctrl+S) / Format(缩进) / Compact(紧凑) /
/// Validate（解析成功清空树并递归填充 object/array/scalar 节点，失败红字报错）。
///
/// 设计要点：解析与重绘由用户点 Validate 触发，不在 textChanged 实时校验——
/// 大 JSON 实时解析会卡编辑器。
class JsonEditorWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit JsonEditorWindow(QWidget* parent = nullptr);

    /// 供 offscreen harness / 调用方驱动：填文本 + 校验。返回是否解析成功。
    bool loadTextAndValidate(const QString& text);

    /// 树顶层节点数（offscreen ground truth 用，公式复算不靠视觉模型）。
    int topLevelNodeCount() const;

    /// 编辑区当前文本（offscreen harness 粗校验 Compact 后是否含换行）。
    QString editorText() const;

  private slots:
    void onOpen();
    void onSave();
    void onFormat();  // Indented
    void onCompact(); // Compact
    void onValidate();

  private:
    void setupActions();
    void setupMenuBar();
    void setupToolBar();
    void setupCentral();
    void setupStatusBar();

    bool parse(const QByteArray& bytes, QJsonDocument* out); // 解析，失败写状态栏
    void writeDocument(const QJsonDocument& doc, QJsonDocument::JsonFormat format);
    void populateTree(const QJsonDocument& doc);
    void fillValue(QTreeWidgetItem* parent, const QString& key, const class QJsonValue& value);
    void showStatusOk();
    void showStatusError(const QString& message);
    void setSampleJson();

    QPlainTextEdit* editor_{nullptr};
    QTreeWidget* tree_{nullptr};
    QLabel* status_label_{nullptr};

    // 工具栏与菜单复用同一组 QAction
    QAction* open_action_{nullptr};
    QAction* save_action_{nullptr};
    QAction* format_action_{nullptr};
    QAction* compact_action_{nullptr};
    QAction* validate_action_{nullptr};

    QString last_open_dir_;
};
