/// @file    microwave_controller.h
/// @brief   SCXML state machine controller for a microwave oven simulation.
///
/// Wraps QScxmlStateMachine to load, run, and interact with the microwave
/// SCXML state machine. Demonstrates data model access, event submission,
/// and state change monitoring via the Qt SCXML C++ API.
///
/// Corresponding tutorial: Advanced 05-Other-Modules/13-QtScxml.

#pragma once

#include <QObject>
#include <QScxmlStateMachine>
#include <QString>
#include <QVariant>

/// @brief Controller that manages a microwave SCXML state machine.
///
/// Provides a simplified interface to load an SCXML file, submit events,
/// read/write data model variables, and observe state transitions.
/// All state changes are logged to stdout for demonstration purposes.
class MicrowaveController : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the controller with an optional parent.
    /// @param[in] parent  Parent QObject for Qt object tree ownership.
    explicit MicrowaveController(QObject* parent = nullptr);

    /// @brief Destroys the controller. The state machine is child-owned.
    ~MicrowaveController() override = default;

    /// @brief Loads the SCXML file and initializes the state machine.
    /// @param[in] filePath  Absolute or relative path to the .scxml file.
    /// @return true if loaded and started successfully, false on error.
    /// @note The state machine is started automatically after a successful load.
    bool loadMachine(const QString& filePath);

    /// @brief Submits a named event to the SCXML state machine.
    /// @param[in] eventName  The event name (e.g. "start", "stop", "openDoor").
    /// @note If the machine is not running, the event is silently ignored.
    void submitEvent(const QString& eventName);

    /// @brief Sets a data model variable by name.
    /// @param[in] name   The SCXML data id (e.g. "timer", "powerLevel").
    /// @param[in] value  The value to assign.
    /// @note The SCXML ECMAScript data model uses string-based property access.
    void setData(const QString& name, const QVariant& value);

    /// @brief Reads a data model variable by name.
    /// @param[in] name  The SCXML data id to query.
    /// @return The current value, or an invalid QVariant if not found.
    QVariant getData(const QString& name) const;

    /// @brief Checks whether the state machine is currently in a given state.
    /// @param[in] stateName  The SCXML state id to check.
    /// @return true if the machine is active in that state.
    bool isInState(const QString& stateName) const;

signals:
    /// @brief Emitted when the state machine transitions to a new state.
    /// @param stateName  The name of the entered state.
    void stateChanged(const QString& stateName);

private:
    /// @brief Connects internal signal handlers for state monitoring.
    /// @note Called once during loadMachine() to wire up logging callbacks.
    void connectStateHandlers();

    QScxmlStateMachine* m_machine;  ///< The loaded SCXML state machine (child-owned).
};
