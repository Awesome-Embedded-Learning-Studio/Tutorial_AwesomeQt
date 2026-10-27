// QtWidgets 入门示例 17: QPushButton 最常用的按钮
// 演示：setDefault / setAutoDefault 与对话框回车键关联
//       按钮带菜单：setMenu(QMenu*) 下拉按钮
//       图标按钮：setIcon + setIconSize
//       扁平按钮 setFlat(true) 与 QSS 美化

#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H

#include <QDialog>

class QLabel;

// ============================================================================
// FileDialog: 模拟文件操作对话框，继承 QDialog 以演示 setDefault
// ============================================================================
class FileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileDialog(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 更新操作结果展示
    void setResult(const QString &text);

private:
    QLabel *m_resultLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
};

#endif // FILE_DIALOG_H
