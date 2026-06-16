/**
 * @file image_viewer_window.h
 * @brief 图片查看器应用主窗口（骨架）——验证 app 栏 QMainWindow + 菜单 + 文件对话框范式
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

class QLabel;

/// @brief 图片查看器主窗口（骨架）。
/// 验证 app 栏范式：QMainWindow + 菜单栏 + 文件对话框 + 占位中央区。
/// 骨架阶段 onOpen 只把所选路径显示到占位 label，不真 QImage 加载/缩放/旋转（成品阶段再补）。
class ImageViewerWindow : public QMainWindow {
  public:
    explicit ImageViewerWindow(QWidget* parent = nullptr);

  private:
    void setup_menu();
    void setup_central();

    QLabel* placeholder_label_{nullptr};
};
