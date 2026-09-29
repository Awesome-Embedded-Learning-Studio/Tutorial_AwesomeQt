// QtWidgets 入门示例 66: QFileDialog 文件选择对话框
// 演示：getOpenFileName 打开单文件
//       getSaveFileName 保存文件
//       getOpenFileNames 批量选择
//       getExistingDirectory 选择目录
//       setNameFilter 文件类型过滤
//       QStandardPaths 默认目录

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QListWidget>
#include <QMainWindow>
#include <QStringList>
#include <QTextEdit>

// ============================================================================
// MainWindow: 演示 QFileDialog 四种静态方法
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    /// @brief 使用 QStandardPaths 获取合理的起始目录
    QString defaultStartDir() const;

    /// @brief 读取文件内容到 m_textEdit
    bool loadFile(const QString &path);

    // ====================================================================
    // 打开单个文件
    // ====================================================================
    void onOpenFile();

    // ====================================================================
    // 批量打开多个文件
    // ====================================================================
    void onBatchOpen();

    // ====================================================================
    // 点击文件列表切换显示
    // ====================================================================
    void onFileItemClicked(QListWidgetItem *item);

    // ====================================================================
    // 保存文件
    // ====================================================================
    void onSaveFile();

    // ====================================================================
    // 选择目录并统计
    // ====================================================================
    void onSelectDirectory();

    QTextEdit *m_textEdit = nullptr;
    QListWidget *m_fileList = nullptr;
    QString m_currentFilePath;
    QStringList m_filePaths;
};

#endif // MAINWINDOW_H
