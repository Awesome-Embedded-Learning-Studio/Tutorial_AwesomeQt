/**
 * @file tetris_window.h
 * @brief 俄罗斯方块主窗口——棋盘 + 副面板（Next/分数/等级/行数/最高分）+ 键盘 + 菜单
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QString>

class QLabel;
class TetrisBoard;

/// @brief Next 预览画布——按下一个方块形态画一个小方块组（自绘 QWidget）。
class NextPreview : public QWidget {
    Q_OBJECT
  public:
    explicit NextPreview(QWidget* parent = nullptr);
    void setPiece(int type);
    QSize sizeHint() const override;

  protected:
    void paintEvent(QPaintEvent* event) override;

  private:
    int type_ = 0;
};

/// @brief 俄罗斯方块主窗口（app 栏整机范式）。
///
/// QMainWindow 装配：中央是棋盘，右侧副面板放 Next 预览 + 分数/等级/行数/最高分 +
/// 操作说明。键盘走棋盘自己的 keyPressEvent（StrongFocus），主窗口只兜底 R 重开、
/// P 暂停、菜单触发。最高分用 QSettings 持久化（应用名 + key）。
class TetrisWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit TetrisWindow(QWidget* parent = nullptr);

  protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override; // 退出时落盘最高分（statsChanged 已不再每次写）

  private slots:
    void onRestart();
    void onPauseToggle();

  private:
    void setupCentral();
    void setupMenuBar();
    void refreshSidePanel();
    int loadHighScore();
    void saveHighScore();

    TetrisBoard* board_{nullptr};
    NextPreview* next_preview_{nullptr};
    QLabel* score_label_{nullptr};
    QLabel* level_label_{nullptr};
    QLabel* lines_label_{nullptr};
    QLabel* high_score_label_{nullptr};
    QLabel* status_label_{nullptr};

    QAction* restart_action_{nullptr};
    QAction* pause_action_{nullptr};

    /// 最高分持久化键（QSettings 应用/键）。
    static constexpr const char* kHighScoreKey_ = "tetris/highscore";
};
