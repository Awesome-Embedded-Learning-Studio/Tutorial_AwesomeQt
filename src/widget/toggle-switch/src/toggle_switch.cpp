#include "toggle_switch.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QMouseEvent>

namespace AwesomeQt {

namespace {
constexpr int kHandlePad = 3;        // 滑块到轨道边的留白
constexpr int kDragThreshold = 4;    // 超过这个像素才算拖动，否则当点击

// 线性插值两色（t 自动夹到 [0,1]）
QColor blend(const QColor& a, const QColor& b, qreal t) {
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return QColor(a.red()   + int((b.red()   - a.red())   * t),
                  a.green() + int((b.green() - a.green()) * t),
                  a.blue()  + int((b.blue()  - a.blue())  * t));
}
} // namespace

ToggleSwitch::ToggleSwitch(QWidget* parent) : QWidget(parent) {
    init_animation();
    setCursor(Qt::PointingHandCursor);
}

ToggleSwitch::ToggleSwitch(bool checked, QWidget* parent)
    : QWidget(parent), checked_(checked), handle_pos_(checked ? 1.0 : 0.0) {
    init_animation();
    setCursor(Qt::PointingHandCursor);
}

void ToggleSwitch::init_animation() {
    handle_anim_ = new QPropertyAnimation(this, "handlePos", this);
    handle_anim_->setDuration(150);
    handle_anim_->setEasingCurve(QEasingCurve::OutCubic);
}

void ToggleSwitch::animate_to(bool checked) {
    handle_anim_->stop();
    handle_anim_->setStartValue(handle_pos_);
    handle_anim_->setEndValue(checked ? 1.0 : 0.0);
    handle_anim_->start();
}

bool ToggleSwitch::checked() const { return checked_; }

void ToggleSwitch::setChecked(bool checked) {
    if (checked_ == checked) return;
    checked_ = checked;
    animate_to(checked);
    emit toggled(checked);
}

qreal ToggleSwitch::handlePos() const { return handle_pos_; }

void ToggleSwitch::setHandlePos(qreal pos) {
    if (pos < 0.0) pos = 0.0;
    if (pos > 1.0) pos = 1.0;
    if (handle_pos_ == pos) return;
    handle_pos_ = pos;
    update();
    emit handlePosChanged(pos);
}

QColor ToggleSwitch::trackColorOn() const { return track_on_; }
void ToggleSwitch::setTrackColorOn(const QColor& c) {
    if (track_on_ == c) return;
    track_on_ = c;
    update();
    emit trackColorOnChanged(c);
}

QColor ToggleSwitch::trackColorOff() const { return track_off_; }
void ToggleSwitch::setTrackColorOff(const QColor& c) {
    if (track_off_ == c) return;
    track_off_ = c;
    update();
    emit trackColorOffChanged(c);
}

QSize ToggleSwitch::sizeHint() const { return {52, 28}; }
QSize ToggleSwitch::minimumSizeHint() const { return {36, 20}; }

QRectF ToggleSwitch::track_rect() const {
    // 留 0.5px 让圆角边缘不被控件边界裁切
    return QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
}

qreal ToggleSwitch::handle_range() const {
    const QRectF tr = track_rect();
    const qreal handle_d = tr.height() - 2 * kHandlePad;
    return tr.width() - 2 * kHandlePad - handle_d; // 滑块水平行程
}

qreal ToggleSwitch::x_to_pos(qreal x) const {
    const QRectF tr = track_rect();
    const qreal handle_d = tr.height() - 2 * kHandlePad;
    const qreal left = tr.left() + kHandlePad + handle_d / 2.0;
    const qreal range = handle_range();
    if (range <= 0.0) return handle_pos_; // 控件太窄，滑块动不了，保持原位
    return (x - left) / range;
}

void ToggleSwitch::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF tr = track_rect();
    const qreal radius = tr.height() / 2.0;

    // 轨道：颜色随 handlePos 在 off↔on 间插值
    const QColor track = blend(track_off_, track_on_, handle_pos_);
    p.setPen(Qt::NoPen);
    p.setBrush(track);
    p.drawRoundedRect(tr, radius, radius);

    // 滑块：直径 = 轨道高 - 2*留白；圆心 x 随 handlePos 从左到右
    const qreal handle_d = tr.height() - 2 * kHandlePad;
    const qreal r = handle_d / 2.0;
    const qreal range = handle_range() > 0.0 ? handle_range() : 0.0;
    const qreal cx = tr.left() + kHandlePad + r + handle_pos_ * range;
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(cx, tr.center().y()), r, r);
}

void ToggleSwitch::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(e);
        return;
    }
    press_x_ = e->position().x();
    drag_start_pos_ = handle_pos_;
    dragging_ = false;
    e->accept();
}

void ToggleSwitch::mouseMoveEvent(QMouseEvent* e) {
    if (!(e->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(e);
        return;
    }
    const qreal dx = e->position().x() - press_x_;
    // 移动超过阈值才进入拖动，并停掉动画——否则动画和拖动会打架
    if (!dragging_ && (dx > kDragThreshold || dx < -kDragThreshold)) {
        dragging_ = true;
        handle_anim_->stop();
    }
    if (dragging_) {
        setHandlePos(drag_start_pos_ + x_to_pos(e->position().x()) - x_to_pos(press_x_));
    }
    e->accept();
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(e);
        return;
    }
    if (dragging_) {
        // 拖动结束：按滑块当前位置吸附到最近端
        const bool target = handle_pos_ > 0.5;
        if (target != checked_) {
            checked_ = target;
            emit toggled(target);
        }
        animate_to(target);
    } else {
        // 没拖动就是纯点击：切换
        setChecked(!checked_);
    }
    dragging_ = false;
    e->accept();
}

} // namespace AwesomeQt
