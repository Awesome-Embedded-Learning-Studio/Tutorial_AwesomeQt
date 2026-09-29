// QtWidgets 入门示例 64: QColorDialog 颜色选择对话框
// 演示：getColor 模态选择
//       ShowAlphaChannel 透明度通道
//       currentColorChanged 实时预览
//       棋盘格可视化透明度效果

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColorDialog>
#include <QLabel>
#include <QMainWindow>

#include "colorpreviewwidget.h"

// ============================================================================
// MainWindow: 演示 QColorDialog 的三种用法
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // ====================================================================
    // 模态选择颜色（无 Alpha）
    // ====================================================================
    void onPickColor();

    // ====================================================================
    // 模态选择颜色（带 Alpha 通道）
    // ====================================================================
    void onPickAlphaColor();

    // ====================================================================
    // 非模态实时预览
    // ====================================================================
    void onLivePreview();

    void onLiveColorChanged(const QColor &color);

    // ====================================================================
    // 应用颜色到预览区域并更新信息标签
    // ====================================================================
    void applyColor(const QColor &color);

    ColorPreviewWidget *m_preview = nullptr;
    QLabel *m_infoLabel = nullptr;
    QColorDialog *m_liveDialog = nullptr;
};

#endif // MAINWINDOW_H
