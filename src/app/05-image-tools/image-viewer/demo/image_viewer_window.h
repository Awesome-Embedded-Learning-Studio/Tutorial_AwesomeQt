/**
 * @file image_viewer_window.h
 * @brief 图片查看器主窗口——菜单/工具栏/状态栏 + 画布 + 同目录翻页 + 幻灯片
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>
#include <QSize>
#include <QStringList>

class QAction;
class QKeyEvent;
class QLabel;
class QScrollArea;
class QTimer;
class ImageView;

/// @brief 图片查看器主窗口（app 栏整机范式）。
///
/// QMainWindow + 菜单/工具栏/状态栏，中央区是 QScrollArea 包一个 ImageView 画布。
/// 能力：打开图片 / 缩放（滚轮 + 工具栏 ± / Fit / 100%）/ 旋转 90° / 同目录翻页 /
/// 幻灯片全屏。翻页与幻灯片共享同一套 navigate 偏移逻辑（循环）。
class ImageViewerWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit ImageViewerWindow(QWidget* parent = nullptr);

  protected:
    void keyPressEvent(QKeyEvent* event) override;

  private slots:
    void onOpen();
    void onPrevious();
    void onNext();
    void onSlideshowToggle();

  private:
    void setupActions();
    void setupMenuBar();
    void setupToolBar();
    void setupCentral();
    void setupStatusBar();
    void wireCanvas();

    void loadFile(const QString& path); // 加载 + 重建目录列表 + 首屏 fit
    void navigate(int offset);          // 同目录翻页（循环）
    void rebuildDirList(const QString& currentPath);
    void refreshNavAvailability();
    void refreshStatus(double zoom, int rotation);
    void enterSlideshow();
    void exitSlideshow();
    static QStringList imageFilters();

    ImageView* view_{nullptr};
    QScrollArea* scroll_{nullptr};
    QLabel* status_label_{nullptr};

    // 幻灯片全屏时菜单/工具栏隐藏，退出要恢复——QAction 集中持有，工具栏与菜单复用
    QAction* open_action_{nullptr};
    QAction* prev_action_{nullptr};
    QAction* next_action_{nullptr};
    QAction* zoom_in_action_{nullptr};
    QAction* zoom_out_action_{nullptr};
    QAction* fit_action_{nullptr};
    QAction* actual_action_{nullptr};
    QAction* rot_left_action_{nullptr};
    QAction* rot_right_action_{nullptr};
    QAction* slideshow_action_{nullptr};

    QStringList dir_files_; // 当前目录的可显示图片（文件名）
    int current_index_ = -1;
    QString last_open_dir_;
    QString current_path_; // 供状态栏显示文件名 / 拼翻页绝对路径
    QSize image_size_;     // 原始图像尺寸（状态栏显示）

    QTimer* slideshow_timer_{nullptr};
    bool slideshow_on_ = false;
    bool was_maximized_ = false; // 进幻灯片前的窗口态，退出时恢复
};
