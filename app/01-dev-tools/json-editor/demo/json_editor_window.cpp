/**
 * @file json_editor_window.cpp
 * @brief JsonEditorWindow 实现——菜单/工具栏装配 + 校验 + 树递归填充
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "json_editor_window.h"

#include <cmath>
#include <limits>

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTextCursor>
#include <QTextStream>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>

JsonEditorWindow::JsonEditorWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("JSON Editor");
    resize(960, 640);

    setupCentral(); // 先建编辑区/树——构造末尾 onValidate() 会解引用它们
    setupActions();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    setSampleJson();
    onValidate(); // 启动即展示树（用默认示例）
}

// ============================================================================
// 中央区：QSplitter 水平分两栏——左编辑区，右树视图
// ============================================================================
void JsonEditorWindow::setupCentral() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    editor_ = new QPlainTextEdit(splitter);
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    editor_->setFont(mono);
    editor_->setPlaceholderText("在此粘贴或编辑 JSON...");

    tree_ = new QTreeWidget(splitter);
    tree_->setHeaderLabels({"Key / Index", "Value", "Type"});
    tree_->setAlternatingRowColors(true);

    splitter->addWidget(editor_);
    splitter->addWidget(tree_);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    setCentralWidget(splitter);
}

// ============================================================================
// QAction 装配：一个 action 复用到菜单 + 工具栏，图标走 QStyle 标准图标（无外部资源）
// ============================================================================
void JsonEditorWindow::setupActions() {
    open_action_ = new QAction("&Open...", this);
    open_action_->setShortcut(QKeySequence::Open);
    open_action_->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    connect(open_action_, &QAction::triggered, this, &JsonEditorWindow::onOpen);

    save_action_ = new QAction("&Save...", this);
    save_action_->setShortcut(QKeySequence::Save);
    save_action_->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(save_action_, &QAction::triggered, this, &JsonEditorWindow::onSave);

    format_action_ = new QAction("&Format", this);
    format_action_->setShortcut(Qt::CTRL | Qt::Key_I);
    format_action_->setToolTip("格式化为缩进（Indented）");
    connect(format_action_, &QAction::triggered, this, &JsonEditorWindow::onFormat);

    compact_action_ = new QAction("&Compact", this);
    compact_action_->setShortcut(Qt::CTRL | Qt::Key_M);
    compact_action_->setToolTip("压缩为紧凑（无空白）");
    connect(compact_action_, &QAction::triggered, this, &JsonEditorWindow::onCompact);

    validate_action_ = new QAction("&Validate", this);
    validate_action_->setShortcut(Qt::CTRL | Qt::Key_Return);
    validate_action_->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    connect(validate_action_, &QAction::triggered, this, &JsonEditorWindow::onValidate);
}

void JsonEditorWindow::setupMenuBar() {
    auto* file_menu = menuBar()->addMenu("&File");
    file_menu->addAction(open_action_);
    file_menu->addAction(save_action_);
    file_menu->addSeparator();
    file_menu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

    auto* json_menu = menuBar()->addMenu("&JSON");
    json_menu->addAction(format_action_);
    json_menu->addAction(compact_action_);
    json_menu->addSeparator();
    json_menu->addAction(validate_action_);

    auto* help_menu = menuBar()->addMenu("&Help");
    help_menu->addAction("&About", this, [this]() {
        QMessageBox::about(this, "About",
                           "AwesomeQt JSON Editor\n"
                           "app/ 栏整机成品：编辑 / 校验 / 格式化 / 紧凑 / 树视图");
    });
}

void JsonEditorWindow::setupToolBar() {
    auto* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->addAction(open_action_);
    tb->addAction(save_action_);
    tb->addSeparator();
    tb->addAction(format_action_);
    tb->addAction(compact_action_);
    tb->addSeparator();
    tb->addAction(validate_action_);
}

void JsonEditorWindow::setupStatusBar() {
    status_label_ = new QLabel("Ready");
    statusBar()->addWidget(status_label_);
}

// ============================================================================
// 解析：QJsonDocument::fromJson(bytes, &parseError)
// ============================================================================
bool JsonEditorWindow::parse(const QByteArray& bytes, QJsonDocument* out) {
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        // offset 是从 bytes 起始算的字节偏移（UTF-8），含中文时与编辑器字符列不等——标 @byte 区分
        showStatusError(
            QString("%1 @byte offset %2").arg(parseError.errorString()).arg(parseError.offset));
        return false;
    }
    *out = doc;
    return true;
}

// ============================================================================
// Validate：解析成功清空树并递归填充，状态栏绿字 OK；失败红字 + 清空树
// ============================================================================
void JsonEditorWindow::onValidate() {
    const QByteArray bytes = editor_->toPlainText().toUtf8();
    QJsonDocument doc;
    if (!parse(bytes, &doc)) {
        tree_->clear();
        return;
    }
    populateTree(doc);
    showStatusOk();
}

// ============================================================================
// 树递归填充：顶层一个虚拟根代表整篇文档，下挂 object 键 / array 元素 / scalar
// ============================================================================
void JsonEditorWindow::populateTree(const QJsonDocument& doc) {
    tree_->clear();

    if (doc.isObject()) {
        auto* root = new QTreeWidgetItem({"(root)", "", "object"});
        tree_->addTopLevelItem(root);
        const QJsonObject obj = doc.object();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            fillValue(root, it.key(), it.value());
        }
        root->setExpanded(true);
    } else if (doc.isArray()) {
        auto* root = new QTreeWidgetItem({"(root)", "", "array"});
        tree_->addTopLevelItem(root);
        const QJsonArray arr = doc.array();
        int idx = 0;
        for (const QJsonValue& v : arr) {
            // 数组元素用索引当 key 列
            fillValue(root, QString("[%1]").arg(idx), v);
            ++idx;
        }
        root->setExpanded(true);
    } else {
        // 文档为空或仅一个 scalar（顶层非容器）
        auto* root = new QTreeWidgetItem({"(root)", "(empty)", "null"});
        tree_->addTopLevelItem(root);
    }
}

/// @brief 递归填充单个值到 parent 下。递归终止于 QJsonValue 不是 Object/Array。
void JsonEditorWindow::fillValue(QTreeWidgetItem* parent, const QString& key,
                                 const QJsonValue& value) {
    switch (value.type()) {
        case QJsonValue::Object: {
            auto* node = new QTreeWidgetItem({key, "", "object"});
            parent->addChild(node);
            const QJsonObject obj = value.toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                fillValue(node, it.key(), it.value());
            }
            break;
        }
        case QJsonValue::Array: {
            auto* node = new QTreeWidgetItem({key, "", "array"});
            parent->addChild(node);
            const QJsonArray arr = value.toArray();
            int idx = 0;
            for (const QJsonValue& v : arr) {
                fillValue(node, QString("[%1]").arg(idx), v);
                ++idx;
            }
            break;
        }
        case QJsonValue::Bool: {
            new QTreeWidgetItem(parent, {key, value.toBool() ? "true" : "false", "bool"});
            break;
        }
        case QJsonValue::Double: {
            // Qt 的 JSON 数字统一是 double；整数（无小数部分）不带小数点。
            // 注意：qFuzzyCompare 对 |d|>=1e12 量级失效（带小数的大数会被误判整数），
            // 必须用精确判定 d==floor(d)；且 static_cast<qint64> 对越界值是 UB，要先做范围检查。
            const double d = value.toDouble();
            QString text;
            if (std::isfinite(d) && d == std::floor(d) &&
                d >= static_cast<double>(std::numeric_limits<qint64>::min()) &&
                d <= static_cast<double>(std::numeric_limits<qint64>::max())) {
                text = QString::number(static_cast<qint64>(d));
            } else {
                text = QString::number(d);
            }
            new QTreeWidgetItem(parent, {key, text, "number"});
            break;
        }
        case QJsonValue::String: {
            new QTreeWidgetItem(parent, {key, value.toString(), "string"});
            break;
        }
        case QJsonValue::Null: {
            new QTreeWidgetItem(parent, {key, "null", "null"});
            break;
        }
        default:
            new QTreeWidgetItem(parent, {key, "(unknown)", "unknown"});
            break;
    }
}

// ============================================================================
// Format / Compact：解析成功后用 QJsonDocument::toJson 写回编辑区（保光标位）
// ============================================================================
void JsonEditorWindow::writeDocument(const QJsonDocument& doc, QJsonDocument::JsonFormat format) {
    // setPlainText 会把光标重置到开头——记位置再恢复（仅防越界，缩进/压缩后语义点会偏）
    const int cursor_pos = editor_->textCursor().position();
    editor_->setPlainText(QString::fromUtf8(doc.toJson(format)));
    QTextCursor c = editor_->textCursor();
    c.setPosition(qMin(cursor_pos, editor_->toPlainText().size()));
    editor_->setTextCursor(c);
}

void JsonEditorWindow::onFormat() {
    QJsonDocument doc;
    if (!parse(editor_->toPlainText().toUtf8(), &doc)) {
        return; // parse 已报错
    }
    writeDocument(doc, QJsonDocument::Indented);
    showStatusOk();
}

void JsonEditorWindow::onCompact() {
    QJsonDocument doc;
    if (!parse(editor_->toPlainText().toUtf8(), &doc)) {
        return;
    }
    writeDocument(doc, QJsonDocument::Compact);
    showStatusOk();
}

// ============================================================================
// Open / Save：QFileDialog 读写 .json（UTF-8）
// ============================================================================
void JsonEditorWindow::onOpen() {
    const QString start_dir = last_open_dir_.isEmpty() ? QDir::homePath() : last_open_dir_;
    const QString path = QFileDialog::getOpenFileName(this, "Open JSON", start_dir,
                                                      "JSON Files (*.json);;All Files (*)");
    if (path.isEmpty()) {
        return;
    }
    last_open_dir_ = QFileInfo(path).absolutePath();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        showStatusError("Cannot open: " + file.errorString());
        return;
    }
    const QByteArray bytes = file.readAll();
    file.close();

    editor_->setPlainText(QString::fromUtf8(bytes));
    onValidate();
}

void JsonEditorWindow::onSave() {
    const QString start_dir = last_open_dir_.isEmpty() ? QDir::homePath() : last_open_dir_;
    QString path = QFileDialog::getSaveFileName(this, "Save JSON", start_dir,
                                                "JSON Files (*.json);;All Files (*)");
    if (path.isEmpty()) {
        return;
    }
    if (QFileInfo(path).suffix().isEmpty()) {
        path += ".json"; // getSaveFileName 不自动补后缀，手动补
    }
    last_open_dir_ = QFileInfo(path).absolutePath();

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        showStatusError("Cannot save: " + file.errorString());
        return;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << editor_->toPlainText();
    out.flush(); // 必须在 close 前 flush——QFile::close 不 flush QTextStream 缓冲，否则截断
    file.close();

    showStatusOk();
}

// ============================================================================
// 状态栏：绿字 OK / 红字错误
// ============================================================================
void JsonEditorWindow::showStatusOk() {
    status_label_->setStyleSheet("color: green;");
    status_label_->setText("OK");
}

void JsonEditorWindow::showStatusError(const QString& message) {
    status_label_->setStyleSheet("color: red;");
    status_label_->setText("Error: " + message);
}

// ============================================================================
// 默认示例：覆盖 object / array / 嵌套 / scalar / 中文
// ============================================================================
void JsonEditorWindow::setSampleJson() {
    const QString sample = QStringLiteral(R"({
  "name": "AwesomeQt",
  "version": 6.11,
  "active": true,
  "tags": ["qt", "json", "编辑器"],
  "author": {
    "name": "Charlie",
    "level": 1,
    "verified": false
  },
  "empty": null
})");
    editor_->setPlainText(sample);
}

// ============================================================================
// 外部驱动接口（offscreen harness / 自动化）
// ============================================================================
bool JsonEditorWindow::loadTextAndValidate(const QString& text) {
    editor_->setPlainText(text);
    const QByteArray bytes = text.toUtf8();
    QJsonDocument doc;
    if (!parse(bytes, &doc)) {
        tree_->clear();
        return false;
    }
    populateTree(doc);
    showStatusOk();
    return true;
}

int JsonEditorWindow::topLevelNodeCount() const {
    return tree_->topLevelItemCount();
}

QString JsonEditorWindow::editorText() const {
    return editor_->toPlainText();
}
