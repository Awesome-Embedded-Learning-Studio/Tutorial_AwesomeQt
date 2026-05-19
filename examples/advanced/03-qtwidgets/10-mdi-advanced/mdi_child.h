/// @file    mdi_child.h
/// @brief   MDI 子窗口内容控件——内含 QPlainTextEdit 的简易文本编辑器。
///
/// 对应教程：进阶层 03-QtWidgets/10-MDI 进阶。

#pragma once

#include <QWidget>

class QPlainTextEdit;
class QLabel;

/// MDI 子窗口内容控件。
///
/// 作为 QMdiSubWindow 的内容 widget，提供：
/// - 基于 QPlainTextEdit 的文本编辑区域
/// - document()->isModified() 脏标记查询接口
/// - 文本内容获取接口
/// - 关闭前检查未保存修改的 closeEvent 拦截
class MdiChild : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针。
    explicit MdiChild(QWidget* parent = nullptr);

    /// @brief 设置子窗口标题。
    /// @param[in] title 标题文本。
    void setDocumentTitle(const QString& title);

    /// @brief 获取当前文本内容。
    /// @return 编辑器中的全部文本。
    QString textContent() const;

    /// @brief 检查文档是否有未保存的修改。
    /// @return true 表示文档被修改过。
    bool isModified() const;

protected:
    /// @brief 重写关闭事件——有未保存修改时弹出确认对话框。
    /// @param[in] event 关闭事件。
    void closeEvent(QCloseEvent* event) override;

private:
    QPlainTextEdit* m_editor;  // 文本编辑器
    QLabel* m_modLabel;        // 修改状态指示标签
};
