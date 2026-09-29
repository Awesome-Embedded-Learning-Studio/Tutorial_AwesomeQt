/**
 * Qt NFC 基础示例
 *
 * 本示例演示 QtNfc 模块的核心功能：
 * 1. QNearFieldManager 检测 NFC 标签的靠近和移开
 * 2. QNearFieldTarget 读取标签上的 NDEF 消息
 * 3. QNdefMessage / QNdefRecord 解析 NDEF 数据
 * 4. 构造并写入 NDEF 消息到标签
 *
 * 核心要点：
 * - NFC 操作全部异步，通过信号/槽获取结果
 * - NDEF 是 NFC 标签上的通用数据格式
 * - 平台支持以 Android 为主，桌面环境通常无 NFC 硬件
 * - 写入前需确认标签容量和写入权限
 *
 * 注意：本示例可在桌面编译运行，但 NFC 功能需要实际硬件支持。
 * 在没有 NFC 硬件的平台上，isSupported() 返回 false。
 */

#include <QMainWindow>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QUrl>

class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QNearFieldManager;
class QNearFieldTarget;
class QNdefMessage;

#include <QNearFieldTarget>

// ========================================
// NFC 标签读写演示窗口
// ========================================

class NfcWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NfcWindow(QWidget *parent = nullptr);
    ~NfcWindow() override = default;

private:

    /// 检查设备 NFC 是否可用
    void checkNfcAvailability();

    /// 启动 NFC 标签检测
    void startDetection();

    /// 检测到 NFC 标签
    void onTargetDetected(QNearFieldTarget *target);

    /// 标签移开
    void onTargetLost(QNearFieldTarget *target);

    /// NDEF 消息读取完成
    void onNdefRead(const QNdefMessage &message);

    /// 写入 NDEF 消息到当前标签
    void writeToTarget();

    /// NFC 操作错误
    void onTargetError(QNearFieldTarget::Error error,
                       const QNearFieldTarget::RequestId &id);

    /// 追加日志
    void appendLog(const QString &text);

private:
    QLabel *status_label_;
    QPushButton *start_button_;
    QPushButton *write_button_;
    QLineEdit *uri_layout_;
    QLineEdit *text_layout_;
    QTextEdit *log_text_;

    QNearFieldManager *nfc_manager_ = nullptr;
    QNearFieldTarget *current_target_ = nullptr;
};
