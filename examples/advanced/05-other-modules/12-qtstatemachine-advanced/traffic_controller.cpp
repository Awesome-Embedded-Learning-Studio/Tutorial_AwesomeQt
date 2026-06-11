/// @file    traffic_controller.cpp
/// @brief   Implementation of the hierarchical traffic-light state machine.
///
/// Builds a three-level state hierarchy and demonstrates QHistoryState
/// by resuming the exact traffic phase that was active before an emergency.

#include "traffic_controller.h"

#include <cstdio>

#include <QCoreApplication>
#include <QTimer>

// ---------------------------------------------------------------------------
// Custom event types used to drive transitions inside the state machine.
// QStateMachine::postEvent dispatches these; transitions match on type.
// ---------------------------------------------------------------------------
enum class TrafficEvent : int
{
    kToYellow  = QEvent::User + 1,  ///< Green -> Yellow
    kToRed     = QEvent::User + 2,  ///< Yellow -> Red
    kToGreen   = QEvent::User + 3,  ///< Red -> Green (cycle restart)
    kEmergency = QEvent::User + 4,  ///< Any -> Emergency
    kResolve   = QEvent::User + 5,  ///< Emergency -> Operating (history)
    kShutdown  = QEvent::User + 6,  ///< Any -> Shutdown
};

// ---------------------------------------------------------------------------
// Timing constants (milliseconds) for each traffic-light phase.
// ---------------------------------------------------------------------------
static constexpr int kGreenDurationMs  = 3000;
static constexpr int kYellowDurationMs = 1500;
static constexpr int kRedDurationMs    = 3000;

// ---------------------------------------------------------------------------
// EventTypeTransition implementation
// ---------------------------------------------------------------------------

/// @brief Stores the event type this transition should match.
/// @param[in] type   The QEvent::Type that triggers this transition.
/// @param[in] source Parent QState (ownership by Qt state hierarchy).
EventTypeTransition::EventTypeTransition(QEvent::Type type, QState* source)
    : QAbstractTransition(source)
    , m_eventType(type)
{
}

/// @brief Returns true when the event's type matches the stored type.
/// @param[in] event The event delivered by QStateMachine::postEvent.
/// @return True if the event type matches; false otherwise.
bool EventTypeTransition::eventTest(QEvent* event)
{
    return event != nullptr && event->type() == m_eventType;
}

/// @brief No side effects needed when the transition fires.
/// @param[in] event The triggering event (unused).
void EventTypeTransition::onTransition(QEvent* event)
{
    Q_UNUSED(event);
}

// ---------------------------------------------------------------------------
// TrafficController implementation
// ---------------------------------------------------------------------------

/// @brief Constructs all state objects but does NOT start the machine.
/// @param[in] parent Owning QObject.
/// @note The machine is built eagerly so that callers can inspect states
///       before calling start() if desired.
TrafficController::TrafficController(QObject* parent)
    : QObject(parent)
    , m_machine(new QStateMachine(this))
    , m_stateOperating(new QState())
    , m_stateEmergency(new QState())
    , m_stateShutdown(new QFinalState())
    , m_stateGreen(new QState(m_stateOperating))
    , m_stateYellow(new QState(m_stateOperating))
    , m_stateRed(new QState(m_stateOperating))
    , m_historyOperating(new QHistoryState(m_stateOperating))
{
    buildStateMachine();
    connectStateSignals();
}

/// @brief Starts the state machine; enters the Operating/Green state.
void TrafficController::start()
{
    m_machine->start();
}

/// @brief Posts a custom event that triggers the emergency transition.
/// @note The event is processed asynchronously by the state machine's
///       event loop, so the caller returns immediately.
void TrafficController::triggerEmergency()
{
    m_machine->postEvent(new QEvent(
        static_cast<QEvent::Type>(TrafficEvent::kEmergency)));
}

/// @brief Posts a custom event that returns to the previous operating state.
/// @note QHistoryState remembers which child of m_stateOperating was last
///       active, so the light resumes exactly where it left off.
void TrafficController::resolveEmergency()
{
    m_machine->postEvent(new QEvent(
        static_cast<QEvent::Type>(TrafficEvent::kResolve)));
}

/// @brief Posts a custom event that moves the machine to the final state.
/// @note Once the final state is entered, QStateMachine emits finished().
void TrafficController::shutdown()
{
    m_machine->postEvent(new QEvent(
        static_cast<QEvent::Type>(TrafficEvent::kShutdown)));
}

/// @brief Constructs the full state hierarchy and wires all transitions.
///
/// Layout:
///
///   Machine
///   +-- Operating (compound state)
///   |   +-- Green   --[kToYellow]--> Yellow
///   |   +-- Yellow  --[kToRed]   --> Red
///   |   +-- Red     --[kToGreen] --> Green
///   |   +-- History (shallow) --> remembers Green|Yellow|Red
///   +-- Emergency --[kResolve] --> Operating (via History)
///   +-- Shutdown (QFinalState)
///
///   Any top-level state --[kEmergency]--> Emergency
///   Any top-level state --[kShutdown] --> Shutdown
void TrafficController::buildStateMachine()
{
    // -- Set initial child states ------------------------------------------
    m_stateOperating->setInitialState(m_stateGreen);

    // -- Wire the normal cycle inside Operating ----------------------------
    // Each transition uses EventTypeTransition which matches a custom event
    // type posted by scheduleEvent() after a timed delay.
    auto* greenToYellow = new EventTypeTransition(
        static_cast<QEvent::Type>(TrafficEvent::kToYellow), m_stateGreen);
    greenToYellow->setTargetState(m_stateYellow);
    m_stateGreen->addTransition(greenToYellow);

    auto* yellowToRed = new EventTypeTransition(
        static_cast<QEvent::Type>(TrafficEvent::kToRed), m_stateYellow);
    yellowToRed->setTargetState(m_stateRed);
    m_stateYellow->addTransition(yellowToRed);

    auto* redToGreen = new EventTypeTransition(
        static_cast<QEvent::Type>(TrafficEvent::kToGreen), m_stateRed);
    redToGreen->setTargetState(m_stateGreen);
    m_stateRed->addTransition(redToGreen);

    // -- History state: shallow history remembers which child was active ---
    m_historyOperating->setDefaultState(m_stateGreen);

    // -- Emergency state returns to Operating via History ------------------
    auto* resolveTransition = new EventTypeTransition(
        static_cast<QEvent::Type>(TrafficEvent::kResolve), m_stateEmergency);
    resolveTransition->setTargetState(m_historyOperating);
    m_stateEmergency->addTransition(resolveTransition);

    // -- Global transitions: Emergency and Shutdown from any top state -----
    auto* operatingToEmergency = new EventTypeTransition(
        static_cast<QEvent::Type>(TrafficEvent::kEmergency),
        m_stateOperating);
    operatingToEmergency->setTargetState(m_stateEmergency);
    m_stateOperating->addTransition(operatingToEmergency);

    auto* emergencySelfLoop = new EventTypeTransition(
        static_cast<QEvent::Type>(TrafficEvent::kEmergency),
        m_stateEmergency);
    emergencySelfLoop->setTargetState(m_stateEmergency);
    m_stateEmergency->addTransition(emergencySelfLoop);

    auto* operatingToShutdown = new EventTypeTransition(
        static_cast<QEvent::Type>(TrafficEvent::kShutdown),
        m_stateOperating);
    operatingToShutdown->setTargetState(m_stateShutdown);
    m_stateOperating->addTransition(operatingToShutdown);

    auto* emergencyToShutdown = new EventTypeTransition(
        static_cast<QEvent::Type>(TrafficEvent::kShutdown),
        m_stateEmergency);
    emergencyToShutdown->setTargetState(m_stateShutdown);
    m_stateEmergency->addTransition(emergencyToShutdown);

    // -- Add top-level states to the machine -------------------------------
    m_machine->addState(m_stateOperating);
    m_machine->addState(m_stateEmergency);
    m_machine->addState(m_stateShutdown);
    m_machine->setInitialState(m_stateOperating);
}

/// @brief Connects entry/exit signals for every state to print diagnostics.
/// @note Using fprintf to stderr so output is visible in all console environments.
void TrafficController::connectStateSignals()
{
    // -- Entry / exit for Operating (container) ----------------------------
    QObject::connect(m_stateOperating, &QState::entered, this, []() {
        std::fprintf(stderr, "[State] ENTER Operating\n");
    });
    QObject::connect(m_stateOperating, &QState::exited, this, []() {
        std::fprintf(stderr, "[State] EXIT  Operating\n");
    });

    // -- Child states inside Operating -------------------------------------
    QObject::connect(m_stateGreen, &QState::entered, this, [this]() {
        std::fprintf(stderr, "[State]   ENTER Green  (go!)\n");
        scheduleEvent(kGreenDurationMs, static_cast<int>(TrafficEvent::kToYellow));
    });
    QObject::connect(m_stateGreen, &QState::exited, this, []() {
        std::fprintf(stderr, "[State]   EXIT  Green\n");
    });

    QObject::connect(m_stateYellow, &QState::entered, this, [this]() {
        std::fprintf(stderr, "[State]   ENTER Yellow (caution)\n");
        scheduleEvent(kYellowDurationMs, static_cast<int>(TrafficEvent::kToRed));
    });
    QObject::connect(m_stateYellow, &QState::exited, this, []() {
        std::fprintf(stderr, "[State]   EXIT  Yellow\n");
    });

    QObject::connect(m_stateRed, &QState::entered, this, [this]() {
        std::fprintf(stderr, "[State]   ENTER Red    (stop)\n");
        scheduleEvent(kRedDurationMs, static_cast<int>(TrafficEvent::kToGreen));
    });
    QObject::connect(m_stateRed, &QState::exited, this, []() {
        std::fprintf(stderr, "[State]   EXIT  Red\n");
    });

    // -- Emergency state ---------------------------------------------------
    QObject::connect(m_stateEmergency, &QState::entered, this, []() {
        std::fprintf(stderr, "[State] ENTER Emergency (flashing red)\n");
    });
    QObject::connect(m_stateEmergency, &QState::exited, this, []() {
        std::fprintf(stderr, "[State] EXIT  Emergency\n");
    });

    // -- Shutdown (final) state --------------------------------------------
    QObject::connect(m_stateShutdown, &QState::entered, this, []() {
        std::fprintf(stderr, "[State] ENTER Shutdown (final)\n");
    });

    // -- When the machine finishes, quit the application ------------------
    QObject::connect(m_machine, &QStateMachine::finished,
                     QCoreApplication::instance(), &QCoreApplication::quit);
}

/// @brief Posts a custom event after a delay using a single-shot QTimer.
/// @param[in] ms         Delay in milliseconds.
/// @param[in] eventType  The custom QEvent::Type to post.
/// @note The QTimer's parent is set to this controller so it is cleaned up
///       even if the machine transitions away before the timer fires.
void TrafficController::scheduleEvent(int ms, int eventType)
{
    auto* timer = new QTimer(this);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, this, [this, timer, eventType]() {
        m_machine->postEvent(
            new QEvent(static_cast<QEvent::Type>(eventType)));
        timer->deleteLater();
    });
    timer->start(ms);
}
