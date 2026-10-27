#ifndef BLUETOOTHWINDOW_H
#define BLUETOOTHWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QVector>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>

class QPushButton;
class QLabel;
class QListWidget;
class QTextEdit;

class BluetoothWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BluetoothWindow(QWidget *parent = nullptr);
    ~BluetoothWindow() override = default;

private slots:
    void startScan();
    void stopScan();
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onScanFinished();
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void connectSelectedBleDevice();

private:
    QPushButton *scan_button_;
    QPushButton *stop_button_;
    QPushButton *connect_button_;
    QLabel *status_label_;
    QListWidget *device_list_widget_;
    QTextEdit *detail_text_;

    QBluetoothDeviceDiscoveryAgent *discovery_agent_ = nullptr;
    std::vector<QBluetoothDeviceInfo> devices_;
    QSet<QString> seen_addresses_;
};

#endif // BLUETOOTHWINDOW_H
