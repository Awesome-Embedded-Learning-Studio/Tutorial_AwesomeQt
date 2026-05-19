/// @file    custom_combo_popup.cpp
/// @brief   CustomComboPopup 类实现——覆写 showPopup 防止弹窗被屏幕边缘截断。
///
/// 对应教程：进阶层 03-QtWidgets/27-QComboBox 进阶。

#include "custom_combo_popup.h"

#include <QFrame>
#include <QGuiApplication>
#include <QScreen>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

CustomComboPopup::CustomComboPopup(QWidget* parent)
    : QComboBox(parent)
{
    // AdjustToContents 让弹窗宽度自适应最宽的选项，避免文本被截断
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

// ─────────────────────────────────────────────────────────────────────────────
// showPopup 覆写
// ─────────────────────────────────────────────────────────────────────────────

void CustomComboPopup::showPopup()
{
    // 先让默认实现创建并显示弹窗
    QComboBox::showPopup();

    // 查找弹窗容器——QComboBox 内部弹窗是一个 QFrame 派生类
    auto* popup = findChild<QFrame*>();
    if (!popup) {
        return;
    }

    QRect geo = popup->geometry();
    clampToScreen(geo);
    popup->setGeometry(geo);
}

// ─────────────────────────────────────────────────────────────────────────────
// 屏幕边界约束
// ─────────────────────────────────────────────────────────────────────────────

void CustomComboPopup::clampToScreen(QRect& popup) const
{
    // 获取当前控件所在屏幕的可用几何区域（排除任务栏等）
    auto* scr = screen();
    QRect avail =
        scr ? scr->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();

    // 如果弹窗底部超出屏幕底部，尝试移到 QComboBox 上方
    if (popup.bottom() > avail.bottom()) {
        int topAbove = mapToGlobal(QPoint(0, 0)).y() - popup.height();
        if (topAbove >= avail.top()) {
            popup.moveTop(topAbove);
        } else {
            // 上方也放不下，就贴住屏幕底部
            popup.moveBottom(avail.bottom());
        }
    }

    // 如果弹窗右侧超出屏幕右边缘，左移对齐右边界
    if (popup.right() > avail.right()) {
        popup.moveRight(avail.right());
    }

    // 如果弹窗左侧超出屏幕左边缘，右移对齐左边界
    if (popup.left() < avail.left()) {
        popup.moveLeft(avail.left());
    }
}
