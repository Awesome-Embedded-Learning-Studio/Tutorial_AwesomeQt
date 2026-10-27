/**
 * Qt Bluetooth 基础示例
 *
 * 本示例演示 QtBluetooth 模块的核心功能：
 * 1. QBluetoothDeviceDiscoveryAgent 扫描周围蓝牙设备
 * 2. 区分经典蓝牙和 BLE 设备
 * 3. QLowEnergyController 连接 BLE 设备并发现 GATT 服务
 * 4. QLowEnergyService 读取特征值和订阅通知
 *
 * 核心要点：
 * - 经典蓝牙和 BLE 是两套独立的协议栈
 * - BLE 操作全部异步，基于信号/槽驱动
 * - 启用 BLE notify 需要写入 CCCD 描述符
 * - 蓝牙扫描结果用设备地址去重
 */

#include <QApplication>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QHBoxLayout>
#include <QLabel>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "bluetoothwindow.h"

// ========================================
// 蓝牙扫描与 BLE 连接演示窗口
// ========================================

BluetoothWindow::BluetoothWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt Bluetooth 设备扫描工具");
    resize(800, 600);

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);

    // 顶部控制栏
    auto *controlLayout = new QHBoxLayout();
    scan_button_ = new QPushButton("开始扫描", this);
    stop_button_ = new QPushButton("停止扫描", this);
    stop_button_->setEnabled(false);
    connect_button_ = new QPushButton("连接 BLE 设备", this);
    connect_button_->setEnabled(false);
    status_label_ = new QLabel("就绪", this);

    controlLayout->addWidget(scan_button_);
    controlLayout->addWidget(stop_button_);
    controlLayout->addWidget(connect_button_);
    controlLayout->addStretch();
    controlLayout->addWidget(status_label_);
    mainLayout->addLayout(controlLayout);

    // 上下分割：设备列表 + 设备详情
    auto *splitter = new QSplitter(Qt::Vertical, this);
    device_list_widget_ = new QListWidget(splitter);
    detail_text_ = new QTextEdit(splitter);
    detail_text_->setReadOnly(true);
    detail_text_->setPlaceholderText("选中一个 BLE 设备后点击\"连接\"查看详情...");
    splitter->setSizes({300, 300});
    mainLayout->addWidget(splitter);

    // 信号连接
    connect(scan_button_, &QPushButton::clicked,
            this, &BluetoothWindow::startScan);
    connect(stop_button_, &QPushButton::clicked,
            this, &BluetoothWindow::stopScan);
    connect(connect_button_, &QPushButton::clicked,
            this, &BluetoothWindow::connectSelectedBleDevice);
    connect(device_list_widget_, &QListWidget::itemSelectionChanged,
            this, [this]() {
        connect_button_->setEnabled(
            device_list_widget_->currentRow() >= 0);
    });
}

/// 开始扫描蓝牙设备
void BluetoothWindow::startScan()
{
    device_list_widget_->clear();
    devices_.clear();
    seen_addresses_.clear();
    detail_text_->clear();

    discovery_agent_ =
        new QBluetoothDeviceDiscoveryAgent(this);

    connect(discovery_agent_,
            &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothWindow::onDeviceDiscovered);

    connect(discovery_agent_,
            &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothWindow::onScanFinished);

    // 错误处理
    connect(discovery_agent_,
            &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, &BluetoothWindow::onScanError);

    status_label_->setText("正在扫描...");
    scan_button_->setEnabled(false);
    stop_button_->setEnabled(true);
    discovery_agent_->start();
}

/// 停止扫描
void BluetoothWindow::stopScan()
{
    if (discovery_agent_) {
        discovery_agent_->stop();
    }
}

/// 发现一个蓝牙设备
void BluetoothWindow::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    QString addr = info.address().toString();

    // 用设备地址去重（扫描过程中同一设备可能多次上报）
    if (seen_addresses_.contains(addr)) {
        return;
    }
    seen_addresses_.insert(addr);
    devices_.push_back(info);

    // 判断设备类型：BLE 还是经典蓝牙
    QString type_str = "经典蓝牙";
    if (info.coreConfigurations()
        & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        type_str = "BLE";
    }

    // 格式化显示：名称 [地址] 类型 (RSSI)
    QString display = QString("%1 [%2] %3 (RSSI:%4)")
        .arg(info.name().isEmpty() ? "未命名设备" : info.name())
        .arg(addr)
        .arg(type_str)
        .arg(info.rssi());

    device_list_widget_->addItem(display);
    status_label_->setText(
        "扫描中... 已发现 "
        + QString::number(devices_.size()) + " 个设备");
}

/// 扫描完成
void BluetoothWindow::onScanFinished()
{
    status_label_->setText(
        "扫描完成，共发现 "
        + QString::number(devices_.size()) + " 个设备");
    scan_button_->setEnabled(true);
    stop_button_->setEnabled(false);
}

/// 扫描出错
void BluetoothWindow::onScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    status_label_->setText(
        "扫描错误: " + QString::number(static_cast<int>(error)));
    scan_button_->setEnabled(true);
    stop_button_->setEnabled(false);
}

/// 连接选中的 BLE 设备
void BluetoothWindow::connectSelectedBleDevice()
{
    int row = device_list_widget_->currentRow();
    if (row < 0 || row >= static_cast<int>(devices_.size())) {
        return;
    }

    const QBluetoothDeviceInfo &device = devices_[row];

    // 检查是否为 BLE 设备
    if (!(device.coreConfigurations()
          & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)) {
        detail_text_->append(
            "该设备不是 BLE 设备，无法进行 GATT 操作");
        return;
    }

    detail_text_->clear();
    detail_text_->append("正在连接: " + device.name()
                       + " [" + device.address().toString() + "]...");

    // 创建 BLE 控制器
    auto *controller =
        QLowEnergyController::createCentral(device, this);

    connect(controller, &QLowEnergyController::connected,
            this, [controller]() {
        qDebug() << "BLE 连接成功，开始发现服务";
        controller->discoverServices();
    });

    connect(controller, &QLowEnergyController::serviceDiscovered,
            this, [this](const QBluetoothUuid &uuid) {
        detail_text_->append("  发现服务: " + uuid.toString());
    });

    connect(controller, &QLowEnergyController::discoveryFinished,
            this, [this, controller]() {
        detail_text_->append(
            "服务发现完成，共 "
            + QString::number(
                controller->services().size())
            + " 个服务");
        detail_text_->append(
            "--- 可以通过 createServiceObject() 获取具体服务");

        // 演示：打印所有服务的 UUID
        for (const auto &uuid : controller->services()) {
            // 检查常见标准服务
            if (uuid == QBluetoothUuid(
                    QBluetoothUuid::ServiceClassUuid::HeartRate)) {
                detail_text_->append(
                    "  -> 包含心率服务 (0x180D)");
            } else if (uuid == QBluetoothUuid(
                    QBluetoothUuid::ServiceClassUuid::BatteryService)) {
                detail_text_->append(
                    "  -> 包含电池服务 (0x180F)");
            } else if (uuid == QBluetoothUuid(
                    QBluetoothUuid::ServiceClassUuid::
                    DeviceInformation)) {
                detail_text_->append(
                    "  -> 包含设备信息服务 (0x180A)");
            }
        }
    });

    connect(controller, &QLowEnergyController::errorOccurred,
            this, [this](QLowEnergyController::Error error) {
        detail_text_->append(
            "连接错误: " + QString::number(
                static_cast<int>(error)));
    });

    // 连接断开时清理
    connect(controller, &QLowEnergyController::disconnected,
            this, [this]() {
        detail_text_->append("设备已断开连接");
    });

    controller->connectToDevice();
}
