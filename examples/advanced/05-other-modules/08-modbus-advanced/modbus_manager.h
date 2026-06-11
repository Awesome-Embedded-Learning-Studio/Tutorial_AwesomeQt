/// @file    modbus_manager.h
/// @brief   Simulated Modbus TCP/RTU client manager with register store.
///
/// Demonstrates how QModbusClient and QModbusDataUnit would be used for
/// real Modbus communication, backed by an in-memory register store
/// so the example runs without hardware.
/// Corresponds to tutorial: advanced 05-other-modules/08-Modbus advanced.

#pragma once

#include "register_map.h"

#include <QMap>
#include <QObject>
#include <QVector>

/// @brief Manages a simulated Modbus device with read/write register operations.
///
/// Instead of connecting to a real PLC, this class maintains an internal
/// QMap<int, quint16> as a virtual register store. The API mirrors how
/// QModbusClient::sendReadRequest / sendWriteRequest work with QModbusDataUnit,
/// so the code structure transfers directly to real hardware deployments.
class ModbusManager : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the manager and initializes the register map.
    /// @param[in] parent  Parent QObject for ownership via Qt object tree.
    explicit ModbusManager(QObject* parent = nullptr);

    /// @brief Populates the simulated register store with realistic default values.
    /// @note Values are chosen to represent a typical running industrial process.
    void simulateDevice();

    /// @brief Reads a range of holding registers from the simulated device.
    /// @param[in] serverId      Modbus slave/server unit ID (1-247).
    /// @param[in] startAddress  The holding register start address (e.g. 40001).
    /// @param[in] count         Number of consecutive 16-bit registers to read.
    /// @return Vector of raw register values. Empty if address is out of range.
    /// @note In a real application this would construct a QModbusDataUnit and
    ///       send it via QModbusClient::sendReadRequest(); here we read from
    ///       the internal store to demonstrate the same data flow.
    QVector<quint16> readRegisters(int serverId, int startAddress, int count);

    /// @brief Writes a single holding register to the simulated device.
    /// @param[in] serverId  Modbus slave/server unit ID (1-247).
    /// @param[in] address   The holding register address (e.g. 40005).
    /// @param[in] value     The 16-bit value to write.
    /// @return true if the address was valid and write succeeded.
    /// @note For float registers, the caller must write both high and low words.
    ///       This method writes one register at a time, matching Modbus function
    ///       code 06 (Write Single Register).
    bool writeRegister(int serverId, int address, quint16 value);

    /// @brief Prints a formatted report of all mapped registers and their values.
    /// @note Output goes to stdout via QTextStream for console demo purposes.
    void printRegisterReport() const;

    /// @brief Prints the register map layout with descriptions (no values).
    void printRegisterMap() const;

    /// @brief Returns a const reference to the internal register map.
    const RegisterMap& registerMap() const;

private:
    /// @brief Converts an IEEE 754 float to two quint16 values (big-endian).
    /// @param[in] value  The float to convert.
    /// @param[out] highWord  Upper 16 bits of the IEEE 754 representation.
    /// @param[out] lowWord   Lower 16 bits of the IEEE 754 representation.
    /// @note Modbus convention transmits the high-order register first.
    static void floatToRegisters(float value, quint16& highWord, quint16& lowWord);

    RegisterMap m_registerMap;                ///< Register metadata and formatting
    QMap<int, quint16> m_registerStore;       ///< Virtual register store: offset -> value
    int m_simulatedServerId;                  ///< Current simulated server ID
};
