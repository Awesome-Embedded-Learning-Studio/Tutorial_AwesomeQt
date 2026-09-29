/**
 * @file image_viewer_window.cpp
 * @brief ImageViewerWindow 实现——菜单/工具栏装配 + 同目录翻页 + 幻灯片
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "image_viewer_window.h"

#include "image_view.h"

#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>

ImageViewerWindow::ImageViewerWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Image Viewer");
    resize(900, 650);

    setupCentral(); // 先建画布/滚动区——setupActions 的 lambda 触发时会解引用 view_/scroll_
    setupActions();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    wireCanvas();

    slideshow_timer_ = new QTimer(this);
    slideshow_timer_->setInterval(3000); // 3 秒一张
    connect(slideshow_timer_, &QTimer::timeout, this, [this]() { navigate(+1); });
}

// ============================================================================
// QAction 装配：一个 action 复用到菜单 + 工具栏，图标走 QStyle 标准图标（无外部资源）
// ============================================================================
void ImageViewerWindow::setupActions() {
    open_action_ = new QAction("&Open...", this);
    open_action_->setShortcut(QKeySequence::Open);
    open_action_->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    connect(open_action_, &QAction::triggered, this, &ImageViewerWindow::onOpen);

    prev_action_ = new QAction("&Previous", this);
    prev_action_->setShortcut(Qt::ALT | Qt::Key_Left);
    prev_action_->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    connect(prev_action_, &QAction::triggered, this, &ImageViewerWindow::onPrevious);

    next_action_ = new QAction("&Next", this);
    next_action_->setShortcut(Qt::ALT | Qt::Key_Right);
    next_action_->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    connect(next_action_, &QAction::triggered, this, &ImageViewerWindow::onNext);

    zoom_in_action_ = new QAction("Zoom &In", this);
    zoom_in_action_->setShortcut(QKeySequence::ZoomIn);
    connect(zoom_in_action_, &QAction::triggered, this, [this]() { view_->zoomIn(); });

    zoom_out_action_ = new QAction("Zoom &Out", this);
    zoom_out_action_->setShortcut(QKeySequence::ZoomOut);
    connect(zoom_out_action_, &QAction::triggered, this, [this]() { view_->zoomOut(); });

    fit_action_ = new QAction("&Fit to Window", this);
    fit_action_->setShortcut(Qt::CTRL | Qt::Key_F);
    connect(fit_action_, &QAction::triggered, this,
            [this]() { view_->zoomToFit(scroll_->viewport()->size()); });

    actual_action_ = new QAction("&Actual Size (100%)", this);
    actual_action_->setShortcut(Qt::CTRL | Qt::Key_0);
    connect(actual_action_, &QAction::triggered, this, [this]() { view_->setZoom(1.0); });

    rot_left_action_ = new QAction("Rotate &Left", this);
    rot_left_action_->setShortcut(Qt::CTRL | Qt::Key_L);
    connect(rot_left_action_, &QAction::triggered, this, [this]() { view_->rotateBy(-90); });

    rot_right_action_ = new QAction("Rotate &Right", this);
    rot_right_action_->setShortcut(Qt::CTRL | Qt::Key_R);
    connect(rot_right_action_, &QAction::triggered, this, [this]() { view_->rotateBy(90); });

    slideshow_action_ = new QAction("&Slideshow", this);
    slideshow_action_->setShortcut(Qt::Key_F5);
    connect(slideshow_action_, &QAction::triggered, this, &ImageViewerWindow::onSlideshowToggle);
}

void ImageViewerWindow::setupMenuBar() {
    auto* file_menu = menuBar()->addMenu("&File");
    file_menu->addAction(open_action_);
    file_menu->addSeparator();
    file_menu->addAction(prev_action_);
    file_menu->addAction(next_action_);
    file_menu->addSeparator();
    file_menu->addAction(slideshow_action_);
    file_menu->addSeparator();
    file_menu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

    auto* view_menu = menuBar()->addMenu("&View");
    view_menu->addAction(zoom_in_action_);
    view_menu->addAction(zoom_out_action_);
    view_menu->addAction(fit_action_);
    view_menu->addAction(actual_action_);
    view_menu->addSeparator();
    view_menu->addAction(rot_left_action_);
    view_menu->addAction(rot_right_action_);

    auto* help_menu = menuBar()->addMenu("&Help");
    help_menu->addAction("&About", this, [this]() {
        QMessageBox::about(this, "About",
                           "AwesomeQt Image Viewer\n"
                           "app/ 栏整机成品：加载 / 缩放 / 旋转 / 翻页 / 幻灯片");
    });
}

void ImageViewerWindow::setupToolBar() {
    auto* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->addAction(open_action_);
    tb->addSeparator();
    tb->addAction(prev_action_);
    tb->addAction(next_action_);
    tb->addSeparator();
    tb->addAction(zoom_out_action_);
    tb->addAction(zoom_in_action_);
    tb->addAction(fit_action_);
    tb->addAction(rot_left_action_);
    tb->addAction(rot_right_action_);
    tb->addSeparator();
    tb->addAction(slideshow_action_);
}

void ImageViewerWindow::setupCentral() {
    scroll_ = new QScrollArea(this);
    scroll_->setBackgroundRole(QPalette::Dark);
    scroll_->setAlignment(Qt::AlignCenter);
    // widgetResizable=false：画布保持自己的 sizeHint（= 变换后图像尺寸），
    // 比视口大就出滚动条，小就按 alignment 居中。
    scroll_->setWidgetResizable(false);

    view_ = new ImageView;
    scroll_->setWidget(view_);
    setCentralWidget(scroll_);
}

void ImageViewerWindow::setupStatusBar() {
    status_label_ = new QLabel("No image");
    statusBar()->addWidget(status_label_);
}

// ============================================================================
// 画布 ↔ 窗口：变换后 resize 画布（让滚动范围跟着变）+ 状态栏刷新 + 失败提示
// ============================================================================
void ImageViewerWindow::wireCanvas() {
    // 画布尺寸随 zoom/rotation 变 → 显式 resize 到新 sizeHint，QScrollArea 才更新滚动范围
    connect(view_, &ImageView::transformChanged, this, [this](double zoom, int rotation) {
        view_->resize(view_->sizeHint());
        refreshStatus(zoom, rotation);
    });
    connect(view_, &ImageView::imageLoaded, this, [this](const QString& path, const QSize& size) {
        current_path_ = path;
        image_size_ = size;
        refreshNavAvailability();
        refreshStatus(view_->zoom(), view_->rotation());
    });
    connect(view_, &ImageView::loadFailed, this, [this](const QString& message) {
        status_label_->setText("Load failed: " + message);
        // 幻灯片全屏时不能弹模态框（打断循环 + 阻塞 timer）；只刷状态栏，由 navigate 静默跳过坏图
        if (!slideshow_on_) {
            QMessageBox::warning(this, "Open Image", "Failed to load image:\n" + message);
        }
    });
}

// ============================================================================
// 打开 / 加载
// ============================================================================
void ImageViewerWindow::onOpen() {
    const QString start_dir = last_open_dir_.isEmpty() ? QDir::homePath() : last_open_dir_;
    const QString path = QFileDialog::getOpenFileName(
        this, "Open Image", start_dir,
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.svg *.webp *.ppm *.pgm *.pbm);;"
        "All Files (*)");
    if (path.isEmpty()) {
        return;
    }
    last_open_dir_ = QFileInfo(path).absolutePath();
    loadFile(path);
}

void ImageViewerWindow::loadFile(const QString& path) {
    if (!view_->loadImage(path)) {
        return; // loadFailed 已弹框
    }
    rebuildDirList(path);
    // 首屏 fit to window——延一帧等布局稳定，viewport 尺寸才准
    QTimer::singleShot(0, this, [this]() { view_->zoomToFit(scroll_->viewport()->size()); });
}

// ============================================================================
// 同目录翻页（循环）
// ============================================================================
void ImageViewerWindow::rebuildDirList(const QString& currentPath) {
    const QFileInfo info(currentPath);
    QDir dir = info.absoluteDir();
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name);
    dir.setNameFilters(imageFilters());
    dir_files_ = dir.entryList();
    current_index_ = dir_files_.indexOf(info.fileName());
    refreshNavAvailability();
}

QStringList ImageViewerWindow::imageFilters() {
    return {"*.png", "*.jpg",  "*.jpeg", "*.bmp", "*.gif",
            "*.svg", "*.webp", "*.ppm",  "*.pgm", "*.pbm"};
}

void ImageViewerWindow::navigate(int offset) {
    if (dir_files_.isEmpty() || current_index_ < 0) {
        return;
    }
    const int n = static_cast<int>(dir_files_.size());
    const QString dir_path = QFileInfo(current_path_).absolutePath();
    // 沿 offset 方向找下一张能加载的图，跳过坏图——坏图不污染 current_index_，
    // 否则会「游标指向坏图、画面停在旧图」脱钩，幻灯片也会卡在坏图反复弹框。
    for (int step = 1; step <= n; ++step) {
        int idx = (current_index_ + offset * step) % n;
        if (idx < 0) {
            idx += n;
        }
        if (idx == current_index_) {
            break; // 绕一圈回到当前，整圈都加载不了
        }
        const QString path = QDir(dir_path).filePath(dir_files_[idx]);
        if (view_->loadImage(path)) {
            current_index_ = idx; // 加载成功才提交游标（翻页保持当前 zoom/rotation）
            return;
        }
        // 坏图：loadFailed 已触发（幻灯片态静默），继续找下一张
    }
}

void ImageViewerWindow::onPrevious() {
    if (slideshow_on_) {
        slideshow_timer_->start(); // 手动翻页重置幻灯片倒计时
    }
    navigate(-1);
}

void ImageViewerWindow::onNext() {
    if (slideshow_on_) {
        slideshow_timer_->start();
    }
    navigate(+1);
}

void ImageViewerWindow::refreshNavAvailability() {
    // 单张或空目录：前后翻页都禁用（循环到自己也无意义）
    const bool can = dir_files_.size() > 1;
    prev_action_->setEnabled(can);
    next_action_->setEnabled(can);
}

void ImageViewerWindow::refreshStatus(double zoom, int rotation) {
    if (!view_->hasImage()) {
        status_label_->setText("No image");
        return;
    }
    status_label_->setText(QString("%1   ·   %2×%3   ·   %4%   ·   %5°")
                               .arg(QFileInfo(current_path_).fileName())
                               .arg(image_size_.width())
                               .arg(image_size_.height())
                               .arg(static_cast<int>(zoom * 100))
                               .arg(rotation));
}

// ============================================================================
// 幻灯片：全屏 + 藏菜单/工具栏 + 定时翻页；Esc / Space 退出
// ============================================================================
void ImageViewerWindow::onSlideshowToggle() {
    if (slideshow_on_) {
        exitSlideshow();
    } else {
        enterSlideshow();
    }
}

void ImageViewerWindow::enterSlideshow() {
    if (dir_files_.isEmpty() || current_index_ < 0) {
        return; // 没图或当前未加载，不放
    }
    slideshow_on_ = true;
    slideshow_action_->setText("Exit &Slideshow");
    was_maximized_ = isMaximized(); // 记录前置窗口态，退出时恢复（showNormal 会丢最大化）
    showFullScreen();
    menuBar()->hide();
    for (auto* tb : findChildren<QToolBar*>()) {
        tb->hide();
    }
    slideshow_timer_->start();
}

void ImageViewerWindow::exitSlideshow() {
    slideshow_timer_->stop();
    slideshow_on_ = false;
    slideshow_action_->setText("&Slideshow");
    if (was_maximized_) {
        showMaximized(); // 恢复进幻灯片前的最大化态
    } else {
        showNormal();
    }
    menuBar()->show();
    for (auto* tb : findChildren<QToolBar*>()) {
        tb->show();
    }
}

void ImageViewerWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            if (slideshow_on_) {
                exitSlideshow();
                return;
            }
            break;
        case Qt::Key_Space:
            if (slideshow_on_) {
                exitSlideshow();
                return;
            }
            break;
        case Qt::Key_Left:
            onPrevious();
            return;
        case Qt::Key_Right:
            onNext();
            return;
        default:
            break;
    }
    QMainWindow::keyPressEvent(event);
}
