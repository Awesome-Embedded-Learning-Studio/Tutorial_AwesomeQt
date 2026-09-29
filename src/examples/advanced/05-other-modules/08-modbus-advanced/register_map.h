/// @file    register_map.h
/// @brief   Modbus register mapping table with metadata and formatting.
///
/// Defines a structured mapping between Modbus holding register addresses
/// and their human-readable descriptions, data types, and units.
/// Corresponds to tutorial: advanced 05-other-modules/08-Modbus advanced.

#pragma once

#include <QMap>
#include <QString>
#include <QVector>

/// @brief Describes a single Modbus register or register group.
struct RegisterEntry
{
    int startAddress;        ///< Starting register address (e.g. 40001)
    int quantity;            ///< Number of consecutive registers occupied
    QString name;            ///< Human-readable parameter name
    QString dataType;        ///< Data type label: "float", "quint16", etc.
    QString unit;            ///< Engineering unit: "C", "bar", "RPM", etc.
    QString description;     ///< Detailed description of the register's purpose
};

/// @brief Predefined Modbus register map for a typical industrial device.
///
/// Provides a fixed mapping table for holding registers 40001-40006,
/// covering temperature, pressure, motor speed, and status flags.
/// Used by ModbusManager to interpret raw register values.
class RegisterMap
{
public:
    /// @brief Constructs the register map with predefined entries.
    /// @note The map is populated at construction time so all lookups are O(1).
    RegisterMap();

    /// @brief Looks up a register entry by its starting address.
    /// @param[in] address  The Modbus holding register start address.
    /// @return The matching RegisterEntry, or a sentinel with empty fields if not found.
    RegisterEntry getEntry(int address) const;

    /// @brief Formats raw register words into a human-readable value string.
    /// @param[in] address    The register start address (used to determine data type).
    /// @param[in] registers  Raw 16-bit register values from the device.
    /// @return Formatted string, e.g. "25.30 C" or "1500 RPM".
    /// @note For float values stored across two registers, IEEE 754 conversion is applied.
    QString formatValue(int address, const QVector<quint16>& registers) const;

    /// @brief Returns all predefined register entries.
    /// @return Const reference to the internal entry vector.
    const QVector<RegisterEntry>& allEntries() const;

    /// @brief Converts a holding register address to a zero-based table offset.
    /// @param[in] address  The Modbus address (e.g. 40001).
    /// @return Zero-based protocol address used by QModbusDataUnit.
    static int addressToOffset(int address);

private:
    QVector<RegisterEntry> m_entries;  ///< Ordered list of all register entries
    QMap<int, int> m_addressIndex;     ///< startAddress -> index into m_entries for O(1) lookup
};
