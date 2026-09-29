/// @file    microwave_controller.cpp
/// @brief   Implementation of MicrowaveController — SCXML state machine wrapper.
///
/// Loads an SCXML file at runtime using QScxmlStateMachine::fromFile(), sets
/// up state monitoring connections, and provides convenience methods for
/// event submission and data model access.

#include "microwave_controller.h"

#include <QScxmlDataModel>
#include <QScxmlError>

#include <cstdio>

MicrowaveController::MicrowaveController(QObject* parent)
    : QObject(parent)
    , m_machine(nullptr)
{
}

bool MicrowaveController::loadMachine(const QString& filePath)
{
    std::fprintf(stderr, "[INFO] Loading SCXML from: %s\n",
                 filePath.toUtf8().constData());

    // fromFile creates a new QScxmlStateMachine on the heap; parent it here
    m_machine = QScxmlStateMachine::fromFile(filePath);

    if (m_machine == nullptr) {
        std::fprintf(stderr, "[ERROR] Failed to load SCXML file: %s\n",
                     filePath.toUtf8().constData());
        return false;
    }

    // Check for parse errors before starting
    const auto errors = m_machine->parseErrors();
    if (!errors.isEmpty()) {
        for (const auto& err : errors) {
            std::fprintf(stderr, "[ERROR] SCXML parse error: %s\n",
                         err.toString().toUtf8().constData());
        }
        return false;
    }

    // Take ownership so the machine is destroyed with this controller
    m_machine->setParent(this);

    // Wire up state change logging
    connectStateHandlers();

    // Initialize and start the state machine
    if (!m_machine->init()) {
        std::fprintf(stderr, "[ERROR] SCXML state machine init() failed\n");
        return false;
    }

    m_machine->start();
    std::fprintf(stderr, "[INFO] SCXML state machine loaded and started\n");
    return true;
}

void MicrowaveController::submitEvent(const QString& eventName)
{
    if (m_machine == nullptr || !m_machine->isRunning()) {
        std::fprintf(stderr, "[WARN] Cannot submit event: machine not running\n");
        return;
    }

    std::fprintf(stderr, ">> Submitting event: %s\n",
                 eventName.toUtf8().constData());
    m_machine->submitEvent(eventName);
}

void MicrowaveController::setData(const QString& name, const QVariant& value)
{
    if (m_machine == nullptr) {
        std::fprintf(stderr, "[WARN] Cannot set data: machine not loaded\n");
        return;
    }

    QScxmlDataModel* dm = m_machine->dataModel();
    if (dm == nullptr) {
        std::fprintf(stderr, "[WARN] No data model available\n");
        return;
    }

    // setScxmlProperty uses the SCXML data id as the property name
    if (!dm->setScxmlProperty(name, value, QString())) {
        std::fprintf(stderr, "[WARN] Failed to set data: %s = %s\n",
                     name.toUtf8().constData(),
                     value.toString().toUtf8().constData());
    } else {
        std::fprintf(stderr, "[DATA] Set: %s = %s\n",
                     name.toUtf8().constData(),
                     value.toString().toUtf8().constData());
    }
}

QVariant MicrowaveController::getData(const QString& name) const
{
    if (m_machine == nullptr) {
        return {};
    }

    QScxmlDataModel* dm = m_machine->dataModel();
    if (dm == nullptr) {
        return {};
    }

    return dm->scxmlProperty(name);
}

bool MicrowaveController::isInState(const QString& stateName) const
{
    if (m_machine == nullptr) {
        return false;
    }
    return m_machine->isActive(stateName);
}

void MicrowaveController::connectStateHandlers()
{
    if (m_machine == nullptr) {
        return;
    }

    // Connect to the log signal from <log> elements in the SCXML file
    connect(m_machine, &QScxmlStateMachine::log,
            this, [](const QString& label, const QString& msg) {
                std::fprintf(stderr, "[SCXML LOG] %s: %s\n",
                             label.toUtf8().constData(),
                             msg.toUtf8().constData());
            });

    // Monitor the 'idle' state
    m_machine->connectToState("idle", this, [this](bool active) {
        if (active) {
            std::fprintf(stderr, "[STATE] Entered: idle\n");
            emit stateChanged("idle");
        }
    });

    // Monitor the 'doorOpen' state
    m_machine->connectToState("doorOpen", this, [this](bool active) {
        if (active) {
            std::fprintf(stderr, "[STATE] Entered: doorOpen\n");
            emit stateChanged("doorOpen");
        }
    });

    // Monitor the 'cooking' state
    m_machine->connectToState("cooking", this, [this](bool active) {
        if (active) {
            std::fprintf(stderr, "[STATE] Entered: cooking\n");
            emit stateChanged("cooking");
        }
    });

    // Monitor the 'paused' state
    m_machine->connectToState("paused", this, [this](bool active) {
        if (active) {
            std::fprintf(stderr, "[STATE] Entered: paused\n");
            emit stateChanged("paused");
        }
    });

    // Monitor the 'done' state
    m_machine->connectToState("done", this, [this](bool active) {
        if (active) {
            std::fprintf(stderr, "[STATE] Entered: done\n");
            emit stateChanged("done");
        }
    });

    // Log when the machine reaches a stable state
    connect(m_machine, &QScxmlStateMachine::reachedStableState, this, [this]() {
        const QStringList active = m_machine->activeStateNames();
        std::fprintf(stderr, "[STABLE] Active states: %s\n",
                     active.join(", ").toUtf8().constData());
    });
}
