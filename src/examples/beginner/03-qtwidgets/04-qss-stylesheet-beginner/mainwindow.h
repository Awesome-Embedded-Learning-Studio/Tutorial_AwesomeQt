#pragma once

#include <QWidget>

class QPushButton;
class QLabel;

// ============================================================================
// 主窗口
// ============================================================================
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void toggleTheme();
    void applyTheme(bool dark);

    bool m_isDark = false;
    QPushButton *m_themeButton = nullptr;
    QLabel *m_statusLabel = nullptr;
};
