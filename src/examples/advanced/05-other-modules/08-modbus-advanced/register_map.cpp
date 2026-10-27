/// @file    register_map.cpp
/// @brief   Implementation of the Modbus register mapping table.
///
/// Contains the predefined industrial device register layout and
/// IEEE 754 float conversion for two-register values.

#include "register_map.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <cstring>

RegisterMap::RegisterMap()
{
    // Predefined holding registers for a simulated industrial sensor/controller device.
    // Layout follows common PLC convention: address 4xxxx maps to holding registers.
    m_entries = {
        {40001, 2, "Temperature",  "float",   QString::fromUtf8("°C"),
         "Process fluid temperature measured by PT100 sensor"},
        {40003, 2, "Pressure",     "float",   "bar",
         "Pipe pressure from 4-20mA transducer"},
        {40005, 1, "Motor Speed",  "quint16", "RPM",
         "Current motor rotational speed"},
        {40006, 1, "Status Flags", "quint16", "bitmask",
         "Bitfield: bit0=Running, bit1=Alarm, bit2=OverTemp, bit3=Fault"},
    };

    // Build index for O(1) address lookup instead of linear scan
    for (int i = 0; i < m_entries.size(); ++i) {
        m_addressIndex[m_entries[i].startAddress] = i;
    }
}

RegisterEntry RegisterMap::getEntry(int address) const
{
    auto it = m_addressIndex.constFind(address);
    if (it != m_addressIndex.constEnd()) {
        return m_entries[it.value()];
    }
    // Sentinel entry: callers should check name.isEmpty() to detect miss
    return {-1, 0, {}, {}, {}, {}};
}

QString RegisterMap::formatValue(int address, const QVector<quint16>& registers) const
{
    const RegisterEntry entry = getEntry(address);

    // Unknown register address: return raw hex dump
    if (entry.name.isEmpty()) {
        return QString("Unknown (raw: 0x%1)")
            .arg(registers.isEmpty() ? 0 : registers[0], 4, 16, QChar('0'));
    }

    if (entry.dataType == "float") {
        // IEEE 754 float occupies two 16-bit registers (Modbus convention: high word first)
        if (registers.size() < 2) {
            return QString("ERR: need 2 registers, got %1").arg(registers.size());
        }

        // Combine two quint16 into a 32-bit IEEE 754 float.
        // High register occupies the upper 16 bits, low register the lower 16 bits.
        const quint32 combined =
            (static_cast<quint32>(registers[0]) << 16) | static_cast<quint32>(registers[1]);

        float value = 0.0F;
        std::memcpy(&value, &combined, sizeof(value));

        return QString("%1 %2").arg(static_cast<double>(value), 0, 'f', 2).arg(entry.unit);

    } else if (entry.dataType == "quint16") {
        if (registers.isEmpty()) {
            return "ERR: no data";
        }

        const quint16 rawVal = registers[0];

        if (entry.unit == "bitmask") {
            // Decode status bitmask into individual flag labels
            QStringList flags;
            if (rawVal & 0x0001) flags << "Running";
            if (rawVal & 0x0002) flags << "Alarm";
            if (rawVal & 0x0004) flags << "OverTemp";
            if (rawVal & 0x0008) flags << "Fault";
            if (flags.isEmpty()) {
                flags << "Idle";
            }
            return QString("0x%1 [%2]")
                .arg(rawVal, 4, 16, QChar('0'))
                .arg(flags.join(" | "));
        }

        return QString("%1 %2").arg(rawVal).arg(entry.unit);
    }

    return QString("Unsupported type: %1").arg(entry.dataType);
}

const QVector<RegisterEntry>& RegisterMap::allEntries() const
{
    return m_entries;
}

int RegisterMap::addressToOffset(int address)
{
    // Holding register convention: 4xxxx addresses map to table offset (xxxx - 1)
    return address - 40001;
}
