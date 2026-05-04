// QtWidgets 入门示例 23: QTextEdit 富文本多行编辑器
// 演示：纯文本 vs 富文本模式切换
//       setHtml / toHtml / toPlainText 内容读写
//       光标操作：QTextCursor 插入/选中/格式化
//       document()->setModified() 追踪修改状态

#include <QApplication>

#include "MiniEditor.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  margin-top: 8px;"
        "  padding-top: 16px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 4px;"
        "}"
        "QPushButton {"
        "  padding: 5px 14px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #DDD;"
        "}"
        "QTextEdit {"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "  line-height: 1.5;"
        "}");

    MiniEditor editor;
    editor.show();

    return app.exec();
}
