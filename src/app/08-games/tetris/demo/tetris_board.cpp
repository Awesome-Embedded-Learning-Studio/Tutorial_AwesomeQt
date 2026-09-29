/**
 * @file tetris_board.cpp
 * @brief TetrisBoard 实现——7 形态方块 + 旋转/碰撞/锁定/消行 + 自绘渲染
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "tetris_board.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>
#include <algorithm>

// ============================================================================
// 7 种方块（I/O/T/S/Z/J/L）：每种 4 个旋转态，4×4 包络矩阵（1=占）。
// 旋转态全部预排好——不用运行时转置，避免方向/翻转错位 bug；颜色按经典约定。
// 4×4 包络统一边长，旋转包络不变（I 用满宽，其余居中 3×3 但仍按 4×4 索引）。
// ============================================================================
const std::array<TetrisBoard::PieceShape, 7>& TetrisBoard::pieceShapes() {
    // helper：把 4 行字符串字面量（# = 占）翻成 4×4 int 矩阵，可读性远胜手填 0/1
    auto mk = [](const char* r0, const char* r1, const char* r2, const char* r3) {
        std::array<std::array<int, kPieceMatrix>, kPieceMatrix> m{};
        const char* rows[kPieceMatrix] = {r0, r1, r2, r3};
        for (int r = 0; r < kPieceMatrix; ++r) {
            for (int c = 0; c < kPieceMatrix; ++c) {
                m[r][c] = (rows[r][c] == '#') ? 1 : 0;
            }
        }
        return m;
    };

    static const std::array<PieceShape, 7> shapes = {
        // I —— 满宽，4 旋转态为水平/竖直（4×4 里左右各空一行）
        PieceShape{{
                       mk("....", "####", "....", "...."),
                       mk("..#.", "..#.", "..#.", "..#."),
                       mk("....", "....", "####", "...."),
                       mk(".#..", ".#..", ".#..", ".#.."),
                   },
                   QColor(80, 200, 240)}, // 青
        // O —— 2×2 方块，4 态全相同（旋转无变化，但为统一都填）
        PieceShape{{
                       mk(".##.", ".##.", "....", "...."),
                       mk(".##.", ".##.", "....", "...."),
                       mk(".##.", ".##.", "....", "...."),
                       mk(".##.", ".##.", "....", "...."),
                   },
                   QColor(240, 220, 80)}, // 黄
        // T —— T 形
        PieceShape{{
                       mk(".#..", "###.", "....", "...."),
                       mk(".#..", ".##.", ".#..", "...."),
                       mk("....", "###.", ".#..", "...."),
                       mk(".#..", "##..", ".#..", "...."),
                   },
                   QColor(170, 100, 230)}, // 紫
        // S —— S 形
        PieceShape{{
                       mk(".##.", "##..", "....", "...."),
                       mk(".#..", ".##.", "..#.", "...."),
                       mk("....", ".##.", "##..", "...."),
                       mk("#...", "##..", ".#..", "...."),
                   },
                   QColor(90, 220, 110)}, // 绿
        // Z —— Z 形
        PieceShape{{
                       mk("##..", ".##.", "....", "...."),
                       mk("..#.", ".##.", ".#..", "...."),
                       mk("....", "##..", ".##.", "...."),
                       mk(".#..", "##..", "#...", "...."),
                   },
                   QColor(230, 90, 90)}, // 红
        // J —— J 形
        PieceShape{{
                       mk("#...", "###.", "....", "...."),
                       mk(".##.", ".#..", ".#..", "...."),
                       mk("....", "###.", "..#.", "...."),
                       mk(".#..", ".#..", "##..", "...."),
                   },
                   QColor(90, 110, 230)}, // 蓝
        // L —— L 形
        PieceShape{{
                       mk("..#.", "###.", "....", "...."),
                       mk(".#..", ".#..", ".##.", "...."),
                       mk("....", "###.", "#...", "...."),
                       mk("##..", ".#..", ".#..", "...."),
                   },
                   QColor(240, 170, 60)}, // 橙
    };
    return shapes;
}

// ============================================================================
// 构造
// ============================================================================
TetrisBoard::TetrisBoard(QWidget* parent) : QWidget(parent) {
    // 键盘焦点：子控件默认不抢键，必须 StrongFocus 否则主窗口装配后棋盘收不到方向键。
    setFocusPolicy(Qt::StrongFocus);

    setMinimumSize(kBoardCols * 12 + 2, kBoardRows * 12 + 2);

    timer_ = new QTimer(this);
    timer_->setInterval(700); // 起步 700ms/格，applyLevelSpeed 会按等级再调
    connect(timer_, &QTimer::timeout, this, &TetrisBoard::step);

    // 背景黑底（画布区），让网格与方块更显眼
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(24, 24, 28));
    setPalette(pal);

    restart();
}

QSize TetrisBoard::sizeHint() const {
    return {kBoardCols * cell_size_ + 2, kBoardRows * cell_size_ + 2};
}

// ============================================================================
// 局开始 / 出块
// ============================================================================
void TetrisBoard::restart() {
    board_ = {};
    score_ = 0;
    level_ = 1;
    lines_ = 0;
    paused_ = false;
    game_over_ = false;
    next_type_ = static_cast<int>(QRandomGenerator::global()->bounded(7));
    spawnNext();
    applyLevelSpeed();
    timer_->start();
    update();
    emitStats();
}

void TetrisBoard::spawnNext() {
    current_.type = next_type_;
    current_.rotation = 0;
    next_type_ = static_cast<int>(QRandomGenerator::global()->bounded(7));
    // 出生位：水平居中（4×4 包络第 3、4 列对多数方块是主体），顶部贴 0 行。
    current_.col = kBoardCols / 2 - kPieceMatrix / 2;
    current_.row = 0;
    if (collides(current_.type, current_.rotation, current_.row, current_.col)) {
        // 出生即碰撞 = 堆顶 = 游戏结束；停钟，发 gameOver 让窗口弹结算。
        game_over_ = true;
        timer_->stop();
        emit gameOver();
    }
}

// ============================================================================
// 旋转 / 平移 / 碰撞
// ============================================================================
const std::array<std::array<int, kPieceMatrix>, kPieceMatrix>& TetrisBoard::currentMatrix() const {
    return pieceShapes()[current_.type].rotations[current_.rotation];
}

bool TetrisBoard::collides(int type, int rotation, int row, int col) const {
    const auto& m = pieceShapes()[type].rotations[rotation];
    for (int r = 0; r < kPieceMatrix; ++r) {
        for (int c = 0; c < kPieceMatrix; ++c) {
            if (m[r][c] == 0) {
                continue;
            }
            const int br = row + r; // board row
            const int bc = col + c; // board col
            // 边界：列越界即撞；行允许「负」（出生区），只当下溢出才算撞。
            if (bc < 0 || bc >= kBoardCols || br >= kBoardRows) {
                return true;
            }
            if (br < 0) {
                continue; // 顶部缓冲，不算撞
            }
            if (board_[br][bc] != 0) {
                return true; // 撞已堆格子
            }
        }
    }
    return false;
}

void TetrisBoard::rotateCurrent() {
    const int new_rot = (current_.rotation + 1) % 4;
    // 壁踢序列：原位 → 左 1 → 右 1 → 左 2 → 右 2 → 上 1（踢墙/踢地）。
    // 简化版（非 SRS），实测足够防卡墙；全部失败则回滚不转。
    static const int kick_dc[] = {0, -1, 1, -2, 2, 0};
    static const int kick_dr[] = {0, 0, 0, 0, 0, -1};
    for (int k = 0; k < 6; ++k) {
        const int nr = current_.row + kick_dr[k];
        const int nc = current_.col + kick_dc[k];
        if (!collides(current_.type, new_rot, nr, nc)) {
            current_.rotation = new_rot;
            current_.row = nr;
            current_.col = nc;
            update();
            return;
        }
    }
    // 旋转失败：静默回滚（保持原态原位）
}

bool TetrisBoard::tryMove(int dRow, int dCol) {
    const int nr = current_.row + dRow;
    const int nc = current_.col + dCol;
    if (collides(current_.type, current_.rotation, nr, nc)) {
        return false;
    }
    current_.row = nr;
    current_.col = nc;
    update();
    return true;
}

// ============================================================================
// 锁定 / 消行 / 计分
// ============================================================================
void TetrisBoard::lockPiece() {
    const auto& m = currentMatrix();

    // 顶出判定：方块任一格被锁到棋盘顶部之上(br<0)，说明堆到顶了——判 game over，
    // 而非静默丢弃那些格子（否则方块形态被截断 + 漏判结束）。
    bool lock_out = false;
    for (int r = 0; r < kPieceMatrix && !lock_out; ++r) {
        for (int c = 0; c < kPieceMatrix; ++c) {
            if (m[r][c] != 0 && current_.row + r < 0) {
                lock_out = true;
                break;
            }
        }
    }
    if (lock_out) {
        game_over_ = true;
        timer_->stop();
        if (score_ > high_score_) {
            high_score_ = score_;
        }
        emitStats();
        emit gameOver();
        return;
    }

    for (int r = 0; r < kPieceMatrix; ++r) {
        for (int c = 0; c < kPieceMatrix; ++c) {
            if (m[r][c] == 0) {
                continue;
            }
            const int br = current_.row + r;
            const int bc = current_.col + c;
            if (br < 0 || br >= kBoardRows || bc < 0 || bc >= kBoardCols) {
                continue; // 包络外不落（理论上 collides 已挡，双保险）
            }
            // board 存 type+1：0=空，1..7=7 种方块；+1 让「空(0)」与「I 型(0 型)」可区分。
            // 颜色不入 board（只存类型索引），paintEvent 查 shapeOf(type-1).color 取色。
            board_[br][bc] = current_.type + 1;
        }
    }

    const int cleared = clearFullLines();
    if (cleared > 0) {
        // 经典计分：1/2/3/4 行 = 100/300/500/800，乘当前等级
        static const int line_score[] = {0, 100, 300, 500, 800};
        score_ += line_score[cleared] * level_;
        lines_ += cleared;
        // 每 10 行升 1 级，速度加快
        const int new_level = 1 + lines_ / 10;
        if (new_level != level_) {
            level_ = new_level;
            applyLevelSpeed();
        }
    }
    if (score_ > high_score_) {
        high_score_ = score_;
    }
    emitStats();

    // 锁完出下一块；spawnNext 内部检测堆顶 game over 并停钟
    spawnNext();
    update();
}

int TetrisBoard::clearFullLines() {
    // 从下往上扫满行：碰到满行「移除」——做法是把该行之上所有行下移一格。
    // 关键：必须自底向上处理，否则一次下移会污染尚未检查的行。
    int cleared = 0;
    for (int r = kBoardRows - 1; r >= 0; --r) {
        bool full = true;
        for (int c = 0; c < kBoardCols; ++c) {
            if (board_[r][c] == 0) {
                full = false;
                break;
            }
        }
        if (!full) {
            continue;
        }
        // 该行满：r..1 全部下移一格（row 0 清空）
        for (int rr = r; rr > 0; --rr) {
            board_[rr] = board_[rr - 1];
        }
        board_[0].fill(0);
        ++cleared;
        // 同一行下移后又可能满（连锁），r 不递减（for 里 --r，所以这里 ++r 抵消）
        ++r;
    }
    return cleared;
}

void TetrisBoard::softDrop() {
    if (!tryMove(1, 0)) {
        lockPiece(); // 到底就锁
    } else {
        score_ += 1; // 软降每格 +1
        emitStats();
    }
}

void TetrisBoard::hardDrop() {
    int dropped = 0;
    while (tryMove(1, 0)) {
        ++dropped;
    }
    score_ += dropped * 2; // 硬降每格 +2（鼓励硬降）
    if (score_ > high_score_) {
        high_score_ = score_;
    }
    lockPiece();
}

void TetrisBoard::step() {
    if (paused_ || game_over_) {
        return;
    }
    if (!tryMove(1, 0)) {
        lockPiece(); // 自然下落到底 → 锁定
    }
}

void TetrisBoard::applyLevelSpeed() {
    // 等级 → interval：每级减 60ms，下限 80ms 防过快不可玩。
    const int interval = std::max(80, 700 - (level_ - 1) * 60);
    timer_->setInterval(interval);
}

void TetrisBoard::emitStats() {
    if (score_ > high_score_) {
        high_score_ = score_;
    }
    emit statsChanged();
}

// ============================================================================
// 暂停
// ============================================================================
void TetrisBoard::setPaused(bool paused) {
    if (game_over_) {
        return;
    }
    paused_ = paused;
    if (paused_) {
        timer_->stop();
    } else {
        timer_->start();
    }
    update();
}

// ============================================================================
// 键盘：←/→ 移、↑ 旋转、↓ 软降、空格 硬降、P 暂停
// ============================================================================
void TetrisBoard::keyPressEvent(QKeyEvent* event) {
    if (game_over_) {
        QWidget::keyPressEvent(event);
        return;
    }
    switch (event->key()) {
        case Qt::Key_Left:
            if (!paused_)
                tryMove(0, -1);
            return;
        case Qt::Key_Right:
            if (!paused_)
                tryMove(0, +1);
            return;
        case Qt::Key_Up:
            if (!paused_)
                rotateCurrent();
            return;
        case Qt::Key_Down:
            if (!paused_)
                softDrop();
            return;
        case Qt::Key_Space:
            if (!paused_)
                hardDrop();
            return;
        // Key_P 在此保留（不交 QAction 独占）：真实 UI 经事件派发链时，QAction 的
        // WindowShortcut 会先 accept、棋盘收不到（单触发、不冲突）；但自动化/offscreen 直接
        // 给棋盘投递按键时走这里。Key_R 仍交 QAction（R 不需 offscreen 测）。
        case Qt::Key_P:
            setPaused(!paused_);
            return;
        default:
            break;
    }
    QWidget::keyPressEvent(event);
}

// ============================================================================
// 自绘：棋盘背景 + 网格 + 已堆方块 + 当前方块 + 投影 + 暂停/结束遮罩
// ============================================================================
void TetrisBoard::paintEvent(QPaintEvent* event) {
    (void)event;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    const int cs = cell_size_;
    const int x0 = (width() - kBoardCols * cs) / 2; // 水平居中
    const int y0 = (height() - kBoardRows * cs) / 2;

    // 1. 已堆方块
    for (int r = 0; r < kBoardRows; ++r) {
        for (int c = 0; c < kBoardCols; ++c) {
            const int type_plus1 = board_[r][c];
            if (type_plus1 == 0) {
                continue;
            }
            const QColor col = pieceShapes()[type_plus1 - 1].color;
            drawBlock(p, x0 + c * cs, y0 + r * cs, cs, col, true);
        }
    }

    // 2. 网格线（画在方块之上、当前方块之下，浅色）
    p.setPen(QPen(QColor(60, 60, 70), 1));
    for (int c = 0; c <= kBoardCols; ++c) {
        p.drawLine(x0 + c * cs, y0, x0 + c * cs, y0 + kBoardRows * cs);
    }
    for (int r = 0; r <= kBoardRows; ++r) {
        p.drawLine(x0, y0 + r * cs, x0 + kBoardCols * cs, y0 + r * cs);
    }

    // 边框
    p.setPen(QPen(QColor(140, 140, 160), 2));
    p.setBrush(Qt::NoBrush);
    p.drawRect(x0 - 1, y0 - 1, kBoardCols * cs + 2, kBoardRows * cs + 2);

    // 3. 当前方块 + 投影（暂停/结束不画当前）
    if (!game_over_ && !paused_) {
        const auto& m = currentMatrix();
        const QColor cur_col = pieceShapes()[current_.type].color;
        // 投影：整块一路下落到撞底（按整块行偏移，不是单格——否则缝隙形会画错位）。
        int ghost_dr = 0;
        while (!collides(current_.type, current_.rotation, current_.row + ghost_dr + 1,
                         current_.col)) {
            ++ghost_dr;
        }
        // 投影先画（在当前方块下层，与当前不重合时才画）
        if (ghost_dr > 0) {
            for (int r = 0; r < kPieceMatrix; ++r) {
                for (int c = 0; c < kPieceMatrix; ++c) {
                    if (m[r][c] == 0)
                        continue;
                    const int br = current_.row + ghost_dr + r;
                    const int bc = current_.col + c;
                    if (br < 0)
                        continue;
                    drawGhost(p, x0 + bc * cs, y0 + br * cs, cs, cur_col);
                }
            }
        }
        // 当前方块
        for (int r = 0; r < kPieceMatrix; ++r) {
            for (int c = 0; c < kPieceMatrix; ++c) {
                if (m[r][c] == 0)
                    continue;
                const int br = current_.row + r;
                const int bc = current_.col + c;
                if (br < 0)
                    continue;
                drawBlock(p, x0 + bc * cs, y0 + br * cs, cs, cur_col, true);
            }
        }
    }

    // 4. 暂停 / 结束遮罩
    if (paused_ || game_over_) {
        p.fillRect(rect(), QColor(0, 0, 0, 140));
        p.setPen(Qt::white);
        QFont f = p.font();
        f.setPointSize(20);
        f.setBold(true);
        p.setFont(f);
        const QString text = game_over_ ? "GAME OVER\nPress R" : "PAUSED\nPress P";
        p.drawText(rect(), Qt::AlignCenter, text);
    }
}

// ============================================================================
// 方块格绘制：立体感（高光左上、阴影右下）+ 投影半透明虚线
// ============================================================================
void TetrisBoard::drawBlock(QPainter& p, int x, int y, int size, const QColor& base, bool border) {
    const QRect rect(x + 1, y + 1, size - 2, size - 2);
    // 主色填充
    p.fillRect(rect, base);
    // 高光：左、上各画一条更亮的线（光源左上）
    p.setPen(base.lighter(160));
    p.drawLine(rect.left(), rect.top(), rect.right(), rect.top());
    p.drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());
    // 阴影：右、下各画一条更暗的线
    p.setPen(base.darker(160));
    p.drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());
    p.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    if (border) {
        p.setPen(QPen(QColor(20, 20, 24), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(rect);
    }
}

void TetrisBoard::drawGhost(QPainter& p, int x, int y, int size, const QColor& base) {
    const QRect rect(x + 2, y + 2, size - 4, size - 4);
    // 半透明同色填充
    QColor fill = base;
    fill.setAlpha(60);
    p.fillRect(rect, fill);
    // 虚线描边
    QPen pen(base.lighter(130), 1, Qt::DashLine);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect);
}
