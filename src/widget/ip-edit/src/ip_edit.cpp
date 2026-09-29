/**
 * @file ip_edit.cpp
 * @brief IpEdit 控件实现——4 段 QLineEdit 跳焦 + 0-255 校验
 * @copyright Copyright (c) 2026
 */

#include "ip_edit.h"

#include <algorithm> // std::clamp

#include <QHBoxLayout>
#include <QIntValidator>
#include <QKeyEvent>
#include <QStringList>

namespace AwesomeQt {

// ============================================================================
// 构造：4 个 QLineEdit + 3 个点，QHBoxLayout 交错排列
// ============================================================================
IpEdit::IpEdit(QWidget* parent) : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* validator = new QIntValidator(0, 255, this); // 0-255 双保险（text() 也会校验）

    for (int i = 0; i < kOctetCount; ++i) {
        auto* edit = new QLineEdit(this);
        edit->setMaxLength(3);
        edit->setAlignment(Qt::AlignCenter);
        edit->setValidator(validator);
        edit->setFixedWidth(40); // 容下 3 位数 + 余量
        octets_[i] = edit;

        // 段文本变化：满 3 位合法自动跳下段（textEdited = 用户输入触发）
        connect(edit, &QLineEdit::textEdited, this, [this, i](const QString& t) {
            if (t.length() >= 3) {
                bool ok = false;
                const int val = t.toInt(&ok);
                if (ok && val >= 0 && val <= 255) {
                    focusNextOctet(i); // 满 3 位合法 → 跳下一段
                }
            }
        });
        // 用 textChanged 兜底（程序 setText 也要发 textChanged）
        connect(edit, &QLineEdit::textChanged, this,
                [this](const QString&) { emit textChanged(text()); });
        // 段回车 / 失焦：末段发 editingFinished
        connect(edit, &QLineEdit::editingFinished, this, [this, i]() {
            if (i == kOctetCount - 1) {
                emit editingFinished();
            }
        });

        layout->addWidget(edit);

        // 段间插点（最后一段后不插）
        if (i < kOctetCount - 1) {
            auto* dot = new QLabel(".", this);
            dot->setAlignment(Qt::AlignCenter);
            dot->setFixedWidth(8);
            dots_[i] = dot;
            layout->addWidget(dot);
        }
    }

    layout->addStretch();      // 余下空间右伸，控件左对齐
    setFocusProxy(octets_[0]); // Tab 进入控件时聚焦第一段
    setFocusPolicy(Qt::StrongFocus);
}

// ============================================================================
// 取地址：拼 "a.b.c.d"，空段补 0
// ============================================================================
QString IpEdit::text() const {
    QStringList parts;
    for (int i = 0; i < kOctetCount; ++i) {
        const QString raw = octet(i) ? octet(i)->text() : QString();
        bool ok = false;
        const int val = raw.toInt(&ok);
        parts.append(ok ? QString::number(val) : QStringLiteral("0"));
    }
    return parts.join('.');
}

// ============================================================================
// 设置地址：按点拆分，越界段夹 0-255、缺段补 0、空串全清
// ============================================================================
void IpEdit::setText(const QString& ip) {
    const QStringList parts = ip.split('.', Qt::SkipEmptyParts);
    // 空串 → 全清
    if (ip.isEmpty()) {
        clear();
        return;
    }

    for (int i = 0; i < kOctetCount; ++i) {
        QLineEdit* edit = octet(i);
        if (!edit) {
            continue;
        }
        // blockSignals 避免每段 setText 各发一次 textChanged；最后统一发一次
        QSignalBlocker blocker(edit);
        if (i < parts.size()) {
            bool ok = false;
            int val = parts[i].toInt(&ok);
            if (!ok) {
                val = 0; // 非数字段补 0
            }
            val = std::clamp(val, 0, 255); // 越界（如 999）夹到 0-255
            edit->setText(QString::number(val));
        } else {
            edit->setText(QStringLiteral("0")); // 缺段补 0
        }
    }
    emit textChanged(text()); // 统一发一次
}

// ============================================================================
// 校验：4 段都 0-255 且非全空
// ============================================================================
bool IpEdit::isValid() const {
    bool any = false;
    for (int i = 0; i < kOctetCount; ++i) {
        const QLineEdit* edit = octet(i);
        if (!edit) {
            return false;
        }
        const QString raw = edit->text();
        if (raw.isEmpty()) {
            return false; // 任一段空即非法
        }
        bool ok = false;
        const int val = raw.toInt(&ok);
        if (!ok || val < 0 || val > 255) {
            return false;
        }
        if (val != 0) {
            any = true;
        }
    }
    return any; // 全 0（如 "0.0.0.0"）视为未填写，不合法
}

// ============================================================================
// 清空所有 4 段
// ============================================================================
void IpEdit::clear() {
    for (int i = 0; i < kOctetCount; ++i) {
        QLineEdit* edit = octet(i);
        if (!edit) {
            continue;
        }
        QSignalBlocker blocker(edit);
        edit->clear();
    }
    emit textChanged(text());
}

// ============================================================================
// 占位提示
// ============================================================================
QString IpEdit::placeholderHint() const {
    return placeholder_hint_;
}

void IpEdit::setPlaceholderHint(const QString& hint) {
    if (placeholder_hint_ == hint) {
        return;
    }
    placeholder_hint_ = hint;
    for (int i = 0; i < kOctetCount; ++i) {
        if (QLineEdit* edit = octet(i)) {
            edit->setPlaceholderText(hint);
        }
    }
    emit placeholderHintChanged(hint);
}

// ============================================================================
// 尺寸
// ============================================================================
QSize IpEdit::sizeHint() const {
    return QSize(240, 32);
}

// ============================================================================
// 段失焦：末段发 editingFinished（editingFinished signal 已连，这里留扩展位）
// ============================================================================
void IpEdit::onOctetEditingFinished() {
    // 由各段 editingFinished signal 直接处理，此处保留给派生扩展
}

// ============================================================================
// 按键拦截：'.' 跳下段（消费掉点）；段首 BackSpace 跳上段
// ============================================================================
void IpEdit::keyPressEvent(QKeyEvent* event) {
    // 找到当前聚焦段索引
    int current = -1;
    for (int i = 0; i < kOctetCount; ++i) {
        if (octet(i) && octet(i)->hasFocus()) {
            current = i;
            break;
        }
    }
    if (current < 0) {
        QWidget::keyPressEvent(event);
        return;
    }

    QLineEdit* edit = octet(current);

    switch (event->key()) {
        case Qt::Key_Period:
        case Qt::Key_Comma: // 兼容小键盘/中文输入习惯
            // 按 '.' → 跳下一段，消费掉这个点（不输入）
            if (current < kOctetCount - 1) {
                focusNextOctet(current);
                event->accept();
                return;
            }
            // 末段按点 → 当作回车发 editingFinished
            if (current == kOctetCount - 1) {
                emit editingFinished();
                event->accept();
                return;
            }
            break;

        case Qt::Key_Backspace:
            // 段首退格且段空 → 跳上段（让它继续删上段末位）
            if (edit && edit->text().isEmpty() && edit->cursorPosition() == 0 && current > 0) {
                QLineEdit* prev = octet(current - 1);
                if (prev) {
                    prev->setFocus();
                    prev->setCursorPosition(prev->text().length()); // 光标置段末
                    prev->backspace();                              // 删上段末位
                }
                event->accept();
                return;
            }
            break;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            // 任意段回车：非末段跳下段，末段发 editingFinished
            if (current < kOctetCount - 1) {
                focusNextOctet(current);
                event->accept();
                return;
            }
            emit editingFinished();
            event->accept();
            return;

        default:
            break;
    }

    QWidget::keyPressEvent(event); // 其余键走默认（含 Tab 由焦点链处理）
}

// ============================================================================
// 焦点流转辅助
// ============================================================================
bool IpEdit::focusNextOctet(int from_index) {
    QLineEdit* next = octet(from_index + 1);
    if (!next) {
        return false;
    }
    next->setFocus();
    next->selectAll(); // 进入即全选，便于覆盖输入
    return true;
}

bool IpEdit::focusPrevOctet(int from_index) {
    QLineEdit* prev = octet(from_index - 1);
    if (!prev) {
        return false;
    }
    prev->setFocus();
    prev->setCursorPosition(prev->text().length());
    return true;
}

QLineEdit* IpEdit::octet(int index) const {
    if (index < 0 || index >= kOctetCount) {
        return nullptr; // 边界保护：越界返回空指针
    }
    return octets_[index];
}

} // namespace AwesomeQt
