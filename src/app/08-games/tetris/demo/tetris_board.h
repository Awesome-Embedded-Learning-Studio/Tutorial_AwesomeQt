/**
 * @file tetris_board.h
 * @brief 俄罗斯方块棋盘——自绘 QWidget + 7 形态方块 + 旋转/碰撞/锁定/消行
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QColor>
#include <QSize>
#include <QTimer>
#include <QWidget>
#include <array>

class QKeyEvent;
class QPaintEvent;

/// 棋盘列数（标准 Tetris = 10）。
constexpr int kBoardCols = 10;
/// 棋盘行数（标准 Tetris = 20，含顶部出生区不计额外缓冲）。
constexpr int kBoardRows = 20;
/// 方块矩阵边长（4×4 包络，旋转用矩阵转置在包络内进行）。
constexpr int kPieceMatrix = 4;

/// @brief 俄罗斯方块棋盘（自绘 QWidget）。
///
/// 纯 Widgets 自绘：paintEvent 用 QPainter 逐格画「棋盘网格 + 已堆方块 +
/// 当前下落方块 + 投影」。游戏逻辑（出生/旋转/碰撞/锁定/消行/计分/等级）全在此类，
/// 主窗口只负责装配 + 键盘路由 + 副面板（Next/分数）。
///
/// 设计要点：
/// - 方块用 4×4 矩阵表示，旋转 = 矩阵转置 + 行反转（顺时针 90°），在同一包络里转，
///   免去 SRS 壁踢的复杂度（本项目用简化版：旋转后若碰撞则尝试左/右/上踢，全失败回滚）。
/// - 计时下落走 QTimer，interval 随等级递减，撞底锁定。
/// - 出生即碰撞 = 游戏结束（堆到顶）。
class TetrisBoard : public QWidget {
    Q_OBJECT
  public:
    explicit TetrisBoard(QWidget* parent = nullptr);

    /// 格子像素边长（sizeHint 据此算）。
    int cellSize() const { return cell_size_; }

    /// 重新开始：清棋盘、分数归零、随机出块。
    void restart();

    bool isPaused() const { return paused_; }
    bool isGameOver() const { return game_over_; }

    int score() const { return score_; }
    int level() const { return level_; }
    int lines() const { return lines_; }
    int highScore() const { return high_score_; }
    /// 回填持久化的历史最高分（restart 会清零，由窗口在构造后注入）。
    void setHighScore(int v) { high_score_ = v; }

    /// 下一个方块的形态（副面板预览用）。
    int nextPiece() const { return next_type_; }

    /// @brief 7 种方块之一的形态定义：4 个旋转态（每个 4×4 的 0/1 矩阵）+ 颜色。
    /// 公开供 NextPreview 等复用，避免形态定义重复导致漂移。
    struct PieceShape {
        std::array<std::array<std::array<int, kPieceMatrix>, kPieceMatrix>, 4> rotations;
        QColor color;
    };

    /// 7 种方块的形态表（I/O/T/S/Z/J/L）。
    static const std::array<PieceShape, 7>& pieceShapes();
    /// 取某形态（type 0..6）。
    static const PieceShape& shapeOf(int type) { return pieceShapes()[type]; }

    QSize sizeHint() const override;

  signals:
    void statsChanged(); ///< 分数/等级/行数/最高分变化时发，主窗口刷新副面板
    void gameOver();     ///< 出生即碰撞（堆顶）

  public slots:
    void setPaused(bool paused);

  protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

  private:
    /// 一个具体下落中的方块（形态 + 当前行列偏移 + 当前旋转态索引）。
    struct ActivePiece {
        int type = 0;     ///< 方块类型 0..6（I/O/T/S/Z/J/L）
        int rotation = 0; ///< 0..3
        int row = 0;      ///< 左上角行（向下为正，单位 = 行）
        int col = 0;      ///< 左上角列（向右为正，单位 = 列）
    };

    /// 当前方块 4×4 矩阵（按 type/rotation 取）。
    const std::array<std::array<int, kPieceMatrix>, kPieceMatrix>& currentMatrix() const;

    /// 给定 type/rotation 矩阵的「1」格是否会撞：越棋盘边界 或 撞已堆格子。
    bool collides(int type, int rotation, int row, int col) const;

    void spawnNext();                 ///< 把 next_type_ 提升为当前方块并置出生位；出生即碰撞 = over
    void rotateCurrent();             ///< 顺时针转，碰撞则壁踢，全失败回滚
    bool tryMove(int dRow, int dCol); ///< 平移/下移一格，碰撞不执行返回 false
    void lockPiece();                 ///< 把当前方块烙进棋盘，消行，结算，出新块
    int clearFullLines();             ///< 清满行 + 上方下移，返回消行数
    void softDrop();                  ///< 软降一格（+1 分）
    void hardDrop();                  ///< 硬降：一路下到底 + 锁定（每行 +2 分）
    void step();                      ///< QTimer tick：自然下落一格，到底则锁定
    void applyLevelSpeed();           ///< 等级 → interval（有下限防过快）

    void emitStats();

    /// 画一个实心方块格：主色填 + 亮边 + 暗角，带立体感（高光在左上、阴影在右下）。
    static void drawBlock(QPainter& p, int x, int y, int size, const QColor& base, bool border);
    /// 画投影格：同色半透明填充 + 虚线描边，区别于实心方块。
    static void drawGhost(QPainter& p, int x, int y, int size, const QColor& base);

    /// board_[row][col] = 方块类型+1（0 表示空），加 1 区分「空(0)」与「I 型(0 型)」。
    std::array<std::array<int, kBoardCols>, kBoardRows> board_{};

    ActivePiece current_{};
    int next_type_ = 0;

    QTimer* timer_{nullptr};
    bool paused_ = false;
    bool game_over_ = false;

    int score_ = 0;
    int level_ = 1;
    int lines_ = 0;
    int high_score_ = 0;

    int cell_size_ = 28; ///< 格子像素边长（paintEvent 与 sizeHint 共用）
};
