/**
 * @file tetris_window.cpp
 * @brief TetrisWindow 实现——棋盘 + 副面板装配 + 键盘 + 菜单 + 最高分持久化
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "tetris_window.h"

#include "tetris_board.h"

#include <QAction>
#include <QCloseEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

// ============================================================================
// NextPreview：把下一个方块形态画成一个 4×4 小方格组（居中）
// ============================================================================
NextPreview::NextPreview(QWidget* parent) : QWidget(parent) {
    // 固定大小：4 格 × 小格边长 + 边距
    constexpr int kCell = 18;
    setFixedSize(kPieceMatrix * kCell + 8, kPieceMatrix * kCell + 8);
}

void NextPreview::setPiece(int type) {
    type_ = type;
    update();
}

QSize NextPreview::sizeHint() const {
    constexpr int kCell = 18;
    return {kPieceMatrix * kCell + 8, kPieceMatrix * kCell + 8};
}

void NextPreview::paintEvent(QPaintEvent* event) {
    (void)event;
    QPainter p(this);
    p.fillRect(rect(), QColor(24, 24, 28));

    constexpr int kCell = 18;
    // 先算 4×4 矩阵的包围盒，再居中——避免不同方块在框里位置跳。
    const auto& m = TetrisBoard::shapeOf(type_).rotations[0];
    int min_r = kPieceMatrix, max_r = -1, min_c = kPieceMatrix, max_c = -1;
    for (int r = 0; r < kPieceMatrix; ++r) {
        for (int c = 0; c < kPieceMatrix; ++c) {
            if (m[r][c] != 0) {
                min_r = std::min(min_r, r);
                max_r = std::max(max_r, r);
                min_c = std::min(min_c, c);
                max_c = std::max(max_c, c);
            }
        }
    }
    if (max_r < 0) {
        return; // 空形态（不应发生）
    }
    const int bw = (max_c - min_c + 1) * kCell;
    const int bh = (max_r - min_r + 1) * kCell;
    const int ox = (width() - bw) / 2 - min_c * kCell;
    const int oy = (height() - bh) / 2 - min_r * kCell;

    const QColor col = TetrisBoard::shapeOf(type_).color;
    for (int r = 0; r < kPieceMatrix; ++r) {
        for (int c = 0; c < kPieceMatrix; ++c) {
            if (m[r][c] == 0)
                continue;
            const QRect rect(ox + c * kCell + 1, oy + r * kCell + 1, kCell - 2, kCell - 2);
            p.fillRect(rect, col);
            p.setPen(col.lighter(160));
            p.drawLine(rect.left(), rect.top(), rect.right(), rect.top());
            p.drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());
            p.setPen(col.darker(160));
            p.drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());
            p.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
        }
    }
}

// ============================================================================
// TetrisWindow
// ============================================================================
TetrisWindow::TetrisWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("AwesomeQt Tetris");
    resize(460, 600);

    setupCentral();
    setupMenuBar();

    // board_ 构造时 restart() 把 high_score_ 清零，这里把持久化的历史最高分回填。
    const int saved_high = loadHighScore();
    board_->setHighScore(saved_high);

    // 棋盘分数变化 → 只刷副面板（softDrop 高频，每次写 QSettings 是无谓 I/O）；
    // 最高分落盘推迟到 gameOver / closeEvent（内存里 board_->highScore() 是真相源）
    connect(board_, &TetrisBoard::statsChanged, this, [this]() { refreshSidePanel(); });
    connect(board_, &TetrisBoard::gameOver, this, [this]() {
        saveHighScore();
        refreshSidePanel();
    });

    refreshSidePanel();
}

void TetrisWindow::setupCentral() {
    auto* central = new QWidget(this);
    auto* main_layout = new QHBoxLayout(central);
    main_layout->setContentsMargins(12, 12, 12, 12);
    main_layout->setSpacing(16);

    // 左：棋盘（stretch=1，占主区）
    board_ = new TetrisBoard(central);
    main_layout->addWidget(board_, 1);

    // 右：副面板
    auto* side = new QWidget(central);
    auto* side_layout = new QVBoxLayout(side);
    side_layout->setSpacing(10);

    auto title_font = []() {
        QFont f;
        f.setBold(true);
        f.setPointSize(11);
        return f;
    };

    auto* next_title = new QLabel("NEXT", side);
    next_title->setFont(title_font());
    side_layout->addWidget(next_title);

    next_preview_ = new NextPreview(side);
    side_layout->addWidget(next_preview_);
    side_layout->addSpacing(8);

    auto make_row = [side, &title_font](const QString& title, QLabel*& value) {
        auto* t = new QLabel(title, side);
        t->setFont(title_font());
        value = new QLabel("0", side);
        QFont vf = value->font();
        vf.setPointSize(14);
        value->setFont(vf);
        auto* box = new QVBoxLayout();
        box->setSpacing(0);
        box->addWidget(t);
        box->addWidget(value);
        return box;
    };

    side_layout->addLayout(make_row("SCORE", score_label_));
    side_layout->addLayout(make_row("LEVEL", level_label_));
    side_layout->addLayout(make_row("LINES", lines_label_));
    side_layout->addLayout(make_row("BEST", high_score_label_));
    side_layout->addStretch(1);

    // 操作说明
    auto* help_title = new QLabel("CONTROLS", side);
    help_title->setFont(title_font());
    side_layout->addWidget(help_title);
    const QStringList controls = {
        "← →   move",      "↑     rotate", "↓     soft drop",
        "SPACE hard drop", "P     pause",  "R     restart",
    };
    for (const QString& c : controls) {
        auto* lbl = new QLabel(c, side);
        QFont f = lbl->font();
        f.setPointSize(9);
        lbl->setFont(f);
        side_layout->addWidget(lbl);
    }

    main_layout->addWidget(side);

    setCentralWidget(central);

    // 状态栏
    status_label_ = new QLabel("Playing");
    statusBar()->addWidget(status_label_);

    // 棋盘拿到键盘焦点（StrongFocus 已设）；窗口显示后确保焦点在棋盘
    board_->setFocus();
}

void TetrisWindow::setupMenuBar() {
    restart_action_ = new QAction("&Restart", this);
    restart_action_->setShortcut(Qt::Key_R);
    connect(restart_action_, &QAction::triggered, this, &TetrisWindow::onRestart);

    pause_action_ = new QAction("&Pause", this);
    pause_action_->setShortcut(Qt::Key_P);
    connect(pause_action_, &QAction::triggered, this, &TetrisWindow::onPauseToggle);

    auto* game_menu = menuBar()->addMenu("&Game");
    game_menu->addAction(restart_action_);
    game_menu->addAction(pause_action_);
    game_menu->addSeparator();
    game_menu->addAction("&Quit", QKeySequence::Quit, this, &QWidget::close);

    auto* help_menu = menuBar()->addMenu("&Help");
    help_menu->addAction("&About", this, [this]() {
        QMessageBox::about(this, "About",
                           "AwesomeQt Tetris\n"
                           "app/ 栏整机成品：自绘棋盘 / 7 形态方块 / 旋转碰撞 / 消行计分 / "
                           "等级加速 / 最高分");
    });
}

void TetrisWindow::refreshSidePanel() {
    score_label_->setText(QString::number(board_->score()));
    level_label_->setText(QString::number(board_->level()));
    lines_label_->setText(QString::number(board_->lines()));
    high_score_label_->setText(QString::number(board_->highScore()));
    next_preview_->setPiece(board_->nextPiece());
    pause_action_->setText(board_->isPaused() ? "&Resume" : "&Pause");

    if (board_->isGameOver()) {
        status_label_->setText("GAME OVER — press R to restart");
    } else if (board_->isPaused()) {
        status_label_->setText("Paused");
    } else {
        status_label_->setText("Playing");
    }
}

int TetrisWindow::loadHighScore() {
    QSettings settings;
    return settings.value(kHighScoreKey_, 0).toInt();
}

void TetrisWindow::saveHighScore() {
    QSettings settings;
    settings.setValue(kHighScoreKey_, board_->highScore());
}

void TetrisWindow::onRestart() {
    board_->restart();
    refreshSidePanel();
    board_->setFocus();
}

void TetrisWindow::onPauseToggle() {
    board_->setPaused(!board_->isPaused());
    refreshSidePanel();
    board_->setFocus();
}

void TetrisWindow::keyPressEvent(QKeyEvent* event) {
    // R/P 由 Game 菜单 QAction 快捷键统一处理（WindowShortcut 先命中并 accept），
    // 此处不重复绑定；方向键由棋盘 keyPressEvent（StrongFocus）独占。
    QMainWindow::keyPressEvent(event);
}

void TetrisWindow::closeEvent(QCloseEvent* event) {
    // 退出时落盘最高分（statsChanged 已不再每次写，避免狂按 ↓ 高频落盘）
    saveHighScore();
    QMainWindow::closeEvent(event);
}
