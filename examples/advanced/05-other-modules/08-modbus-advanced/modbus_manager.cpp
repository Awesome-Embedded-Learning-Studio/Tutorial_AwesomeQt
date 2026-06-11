/// @file    modbus_manager.cpp
/// @brief   Implementation of the simulated Modbus client manager.
///
/// Provides read/write operations against an in-memory register store,
/// with formatting via RegisterMap. Also shows how QModbusDataUnit and
/// QModbusClient would be used with real hardware.

#include "modbus_manager.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QTextStream>

// QModbusDataUnit header is included to show the real API structure.
// In a production build with a real device, these types would be used directly.
#include <QModbusDataUnit>

ModbusManager::ModbusManager(QObject* parent)
    : QObject(parent)
    , m_simulatedServerId(1)
{
}

void ModbusManager::simulateDevice()
{
    // Simulated process values representing a healthy running system:
    //   Temperature: 25.30 C  (room temperature fluid)
    //   Pressure:    4.50 bar (moderate pipeline pressure)
    //   Motor Speed: 1200 RPM (nominal operating speed)
    //   Status:      0x0001   (Running flag set, no alarms)

    quint16 tempHigh = 0;
    quint16 tempLow = 0;
    floatToRegisters(25.30F, tempHigh, tempLow);

    quint16 pressHigh = 0;
    quint16 pressLow = 0;
    floatToRegisters(4.50F, pressHigh, pressLow);

    // Store float registers: address 40001-40002 = Temperature
    m_registerStore[RegisterMap::addressToOffset(40001)] = tempHigh;
    m_registerStore[RegisterMap::addressToOffset(40001) + 1] = tempLow;

    // Store float registers: address 40003-40004 = Pressure
    m_registerStore[RegisterMap::addressToOffset(40003)] = pressHigh;
    m_registerStore[RegisterMap::addressToOffset(40003) + 1] = pressLow;

    // Store quint16 register: address 40005 = Motor Speed
    m_registerStore[RegisterMap::addressToOffset(40005)] = 1200;

    // Store bitmask register: address 40006 = Status Flags (Running = bit0)
    m_registerStore[RegisterMap::addressToOffset(40006)] = 0x0001;

    QTextStream out(stdout);
    out << "[ModbusManager] Simulated device initialized with default values.\n";
    out << "  Server ID: " << m_simulatedServerId << "\n";
    out << "  Registers: " << m_registerStore.size() << " words written\n\n";

    // Show how QModbusDataUnit would be constructed for a real read request.
    // This code demonstrates the API without actually sending network traffic.
    const QModbusDataUnit exampleUnit(
        QModbusDataUnit::HoldingRegisters,
        static_cast<quint16>(RegisterMap::addressToOffset(40001)),
        6
    );
    out << "  [API Demo] QModbusDataUnit for full register range:\n";
    out << "    Type:     HoldingRegisters\n";
    out << "    Start:    " << exampleUnit.startAddress() << "\n";
    out << "    Count:    " << exampleUnit.valueCount() << "\n\n";
}

QVector<quint16> ModbusManager::readRegisters(int serverId, int startAddress, int count)
{
    Q_UNUSED(serverId)

    const int offset = RegisterMap::addressToOffset(startAddress);

    // In a real application:
    //   QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, offset, count);
    //   QModbusReply* reply = m_client->sendReadRequest(request, serverId);
    //   connect(reply, &QModbusReply::finished, ...);
    // Here we read directly from the simulated store.

    QVector<quint16> result;
    result.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto it = m_registerStore.constFind(offset + i);
        if (it != m_registerStore.constEnd()) {
            result.append(it.value());
        } else {
            // Uninitialized register returns 0 per Modbus convention
            result.append(0);
        }
    }

    QTextStream out(stdout);
    out << "[READ] Server " << serverId
        << " | Address " << startAddress
        << " | Count " << count
        << " -> [";
    for (int i = 0; i < result.size(); ++i) {
        if (i > 0) out << ", ";
        out << "0x" << QString("%1").arg(result[i], 4, 16, QChar('0')).toUpper();
    }
    out << "]\n";

    return result;
}

bool ModbusManager::writeRegister(int serverId, int address, quint16 value)
{
    Q_UNUSED(serverId)

    const int offset = RegisterMap::addressToOffset(address);

    // In a real application:
    //   QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, offset, 1);
    //   request.setValue(0, value);
    //   QModbusReply* reply = m_client->sendWriteRequest(request, serverId);

    // Validate that the register exists in our predefined map
    const RegisterEntry entry = m_registerMap.getEntry(address);
    if (entry.name.isEmpty()) {
        QTextStream out(stdout);
        out << "[WRITE FAILED] Address " << address << " not in register map.\n";
        return false;
    }

    m_registerStore[offset] = value;

    QTextStream out(stdout);
    out << "[WRITE] Server " << serverId
        << " | Address " << address
        << " | Value 0x" << QString("%1").arg(value, 4, 16, QChar('0')).toUpper()
        << " (" << value << ")"
        << " -> OK\n";

    return true;
}

void ModbusManager::printRegisterReport() const
{
    QTextStream out(stdout);

    out << "=== Modbus Register Report (Live Values) ===\n";
    out << QString(72, '-') << "\n";
    // QTextStream supports setFieldWidth/setPadChar for columnar output
    out.setFieldAlignment(QTextStream::AlignLeft);
    out << qSetFieldWidth(8)  << "Addr"
        << qSetFieldWidth(16) << "Name"
        << qSetFieldWidth(10) << "Type"
        << qSetFieldWidth(20) << "Value"
        << qSetFieldWidth(0)  << "Description" << "\n";
    out << QString(72, '-') << "\n";

    for (const RegisterEntry& entry : m_registerMap.allEntries()) {
        const int offset = RegisterMap::addressToOffset(entry.startAddress);
        QVector<quint16> raw;
        for (int i = 0; i < entry.quantity; ++i) {
            auto it = m_registerStore.constFind(offset + i);
            raw.append(it != m_registerStore.constEnd() ? it.value() : 0);
        }

        const QString formatted = m_registerMap.formatValue(entry.startAddress, raw);

        out << qSetFieldWidth(8)  << QString::number(entry.startAddress)
            << qSetFieldWidth(16) << entry.name
            << qSetFieldWidth(10) << entry.dataType
            << qSetFieldWidth(20) << formatted
            << qSetFieldWidth(0)  << entry.description << "\n";
    }

    out << QString(72, '-') << "\n\n";
}

void ModbusManager::printRegisterMap() const
{
    QTextStream out(stdout);

    out << "=== Register Map Layout ===\n";
    out << QString(78, '-') << "\n";
    out.setFieldAlignment(QTextStream::AlignLeft);
    out << qSetFieldWidth(8)  << "Addr"
        << qSetFieldWidth(6)  << "Qty"
        << qSetFieldWidth(16) << "Name"
        << qSetFieldWidth(10) << "Type"
        << qSetFieldWidth(8)  << "Unit"
        << qSetFieldWidth(0)  << "Description" << "\n";
    out << QString(78, '-') << "\n";

    for (const RegisterEntry& entry : m_registerMap.allEntries()) {
        out << qSetFieldWidth(8)  << QString::number(entry.startAddress)
            << qSetFieldWidth(6)  << QString::number(entry.quantity)
            << qSetFieldWidth(16) << entry.name
            << qSetFieldWidth(10) << entry.dataType
            << qSetFieldWidth(8)  << entry.unit
            << qSetFieldWidth(0)  << entry.description << "\n";
    }

    out << QString(78, '-') << "\n\n";
}

const RegisterMap& ModbusManager::registerMap() const
{
    return m_registerMap;
}

void ModbusManager::floatToRegisters(float value, quint16& highWord, quint16& lowWord)
{
    QByteArray raw;
    QDataStream stream(&raw, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << value;

    // Big-endian float: first two bytes -> highWord, last two bytes -> lowWord
    QDataStream reader(raw);
    reader.setByteOrder(QDataStream::BigEndian);
    reader >> highWord >> lowWord;
}
