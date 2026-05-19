/// @file    rich_text_navigator.h
/// @brief   RichTextNavigator 类声明——QTextDocument 操控与 QTextCursor 导航演示。
///
/// 对应教程：进阶层 03-QtWidgets/23-QTextEdit 进阶。

#pragma once

#include <QTextEdit>

class QLabel;
class QLineEdit;
class QPushButton;

/// QTextDocument 底层操控与 QTextCursor 高级导航演示控件。
///
/// 展示四个核心知识点：
/// - QTextDocument 的 blockCount / characterCount 等统计信息
/// - QTextCursor::movePosition 与 select block/line/word 的导航操作
/// - insertHtml 在光标位置插入富文本片段
/// - QTextDocument::find 进行文本搜索并用 ExtraSelection 高亮
class RichTextNavigator : public QTextEdit
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化界面布局与信号槽连接。
    /// @param[in] parent 父控件指针。
    explicit RichTextNavigator(QWidget* parent = nullptr);

    /// @brief 创建顶部工具栏：光标导航按钮与搜索框。
    /// @return 包含工具栏控件的 QWidget 指针。
    /// @note 由 main.cpp 调用，返回的控件由调用者负责布局。
    QWidget* createToolBar();

    /// @brief 创建底部状态栏：显示光标位置与文档统计。
    /// @return 包含状态信息的 QWidget 指针。
    /// @note 由 main.cpp 调用，返回的控件由调用者负责布局。
    QWidget* createStatusBar();

    /// @brief 更新状态栏显示（光标位置、block 号、文档统计）。
    void updateStatusBar();

private:
    /// @brief 初始化编辑器，加载一段演示用的富文本内容。
    void loadSampleContent();

    /// @brief 将光标移动到文档起始位置。
    void moveToStart();

    /// @brief 将光标移动到文档末尾位置。
    void moveToEnd();

    /// @brief 选中光标所在的整个段落（block）。
    void selectBlock();

    /// @brief 选中光标所在的行。
    void selectLine();

    /// @brief 选中光标所在的单词。
    void selectWord();

    /// @brief 在光标位置插入一段富文本 HTML 片段。
    void insertRichText();

    /// @brief 执行搜索：高亮所有匹配项并跳转到第一个。
    void performSearch();

private:
    // --- 工具栏控件 ---
    QPushButton* m_btnStart;       // 跳到文档起始
    QPushButton* m_btnEnd;         // 跳到文档末尾
    QPushButton* m_btnSelectBlock; // 选中当前段落
    QPushButton* m_btnSelectLine;  // 选中当前行
    QPushButton* m_btnSelectWord;  // 选中当前单词
    QPushButton* m_btnInsert;      // 插入富文本
    QLineEdit*   m_searchInput;    // 搜索输入框
    QPushButton* m_btnSearch;      // 搜索按钮

    // --- 状态栏控件 ---
    QLabel* m_cursorPosLabel;      // 光标位置
    QLabel* m_blockInfoLabel;      // 当前 block 编号
    QLabel* m_docStatsLabel;       // 文档统计（block 数 / 字符数）
};
