/// @file    main.cpp
/// @brief   Console demo for simulated Modbus register read/write operations.
///
/// Demonstrates:
///   1. Initializing a simulated industrial device
///   2. Reading all registers and displaying a formatted report
///   3. Writing to a register (set motor speed to 1500 RPM)
///   4. Reading back the changed value for verification
///   5. Printing the full register map with descriptions
/// Corresponds to tutorial: advanced 05-other-modules/08-Modbus advanced.

#include "modbus_manager.h"
#include "register_map.h"

#include <QCoreApplication>
#include <QTextStream>

/// @brief Prints a section header to stdout for visual separation.
/// @param[in] title  The demo section title.
static void printHeader(const QString& title)
{
    QTextStream out(stdout);
    out << "\n" << QString(50, '=') << "\n";
    out << "  " << title << "\n";
    out << QString(50, '=') << "\n\n";
}

/// @brief Entry point: runs five sequential Modbus demos, then exits.
/// @param[in] argc  Argument count (unused).
/// @param[in] argv  Argument values (unused).
/// @return EXIT_SUCCESS on completion.
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTextStream out(stdout);
    out << "Modbus Advanced Example - Simulated Register Operations\n";
    out << "=========================================================\n";

    // ---- Demo 1: Initialize simulated device ----
    printHeader("Demo 1: Initialize Simulated Device");

    ModbusManager manager;
    // Populates the register store with realistic industrial process values
    manager.simulateDevice();

    // ---- Demo 2: Read all registers and display report ----
    printHeader("Demo 2: Read All Registers");

    // Read the entire register range (6 registers from 40001 to 40006)
    QVector<quint16> allRaw = manager.readRegisters(1, 40001, 6);
    out << "\n";

    // Print the formatted report with decoded values
    manager.printRegisterReport();

    // ---- Demo 3: Write to a register (motor speed) ----
    printHeader("Demo 3: Write Motor Speed to 1500 RPM");

    // Motor speed is at address 40005 (single quint16 register)
    const bool writeOk = manager.writeRegister(1, 40005, 1500);
    out << "Write result: " << (writeOk ? "SUCCESS" : "FAILED") << "\n\n";

    // ---- Demo 4: Read back and verify ----
    printHeader("Demo 4: Read Back Motor Speed");

    QVector<quint16> speedRaw = manager.readRegisters(1, 40005, 1);

    const RegisterMap& map = manager.registerMap();
    const RegisterEntry speedEntry = map.getEntry(40005);
    const QString formatted = map.formatValue(40005, speedRaw);
    out << "\nVerified: " << speedEntry.name << " = " << formatted << "\n";

    if (!speedRaw.isEmpty() && speedRaw[0] == 1500) {
        out << "Verification PASSED: value matches 1500 RPM.\n";
    } else {
        out << "Verification FAILED: unexpected value.\n";
    }

    // Print updated report showing the changed motor speed
    out << "\n";
    manager.printRegisterReport();

    // ---- Demo 5: Show register map with descriptions ----
    printHeader("Demo 5: Register Map Layout");

    manager.printRegisterMap();

    out << "All demos completed successfully.\n";

    return EXIT_SUCCESS;
}
