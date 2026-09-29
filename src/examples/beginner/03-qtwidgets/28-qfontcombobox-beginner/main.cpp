// QtWidgets 入门示例 28: QFontComboBox 字体选择下拉框
// 演示：setFontFilters() 过滤字体类型（等宽/比例/全部）
//       currentFont() 获取选中字体
//       在文字编辑器中实时预览字体变化
//       字体名称的本地化显示

#include <QApplication>

#include "FontPreviewTool.h"

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
        "QFontComboBox, QComboBox {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "}"
        "QFontComboBox:hover, QComboBox:hover {"
        "  border-color: #1976D2;"
        "}"
        "QTextEdit {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 12px;"
        "  font-size: 14px;"
        "}"
    );

    FontPreviewTool tool;
    tool.show();

    return app.exec();
}
