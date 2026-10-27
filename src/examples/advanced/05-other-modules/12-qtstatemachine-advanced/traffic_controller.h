/// @file    traffic_controller.h
/// @brief   Hierarchical state machine for a traffic light controller.
///
/// Demonstrates QStateMachine with nested states, QHistoryState for
/// resuming interrupted operation, and custom event-driven transitions.
/// Corresponds to tutorial: advanced 05-other-modules/12-QtStateMachine.

#pragma once

#include <QAbstractTransition>
#include <QFinalState>
#include <QHistoryState>
#include <QObject>
#include <QState>
#include <QStateMachine>

/// @brief Custom transition that matches a specific QEvent::Type.
///
/// Qt6's QStateMachine::postEvent() delivers events to the active state's
/// transitions. This class overrides eventTest() to accept only the event
/// type specified at construction time.
class EventTypeTransition : public QAbstractTransition
{
    Q_OBJECT

public:
    /// @brief Constructs a transition triggered by the given event type.
    /// @param[in] type    The QEvent::Type that triggers this transition.
    /// @param[in] source  Parent QState that owns this transition.
    explicit EventTypeTransition(QEvent::Type type, QState* source = nullptr);

protected:
    /// @brief Returns true only when the incoming event matches m_eventType.
    /// @param[in] event The event dispatched by the state machine.
    bool eventTest(QEvent* event) override;

    /// @brief No-op; no side effects needed on transition.
    /// @param[in] event The event that triggered this transition.
    void onTransition(QEvent* event) override;

private:
    QEvent::Type m_eventType;  ///< The event type this transition listens for.
};

/// @brief Traffic light controller driven by a hierarchical Qt state machine.
///
/// Top-level states: Operating (normal cycle), Emergency (flashing red),
/// and Shutdown (final). The Operating state contains child states Green,
/// Yellow, and Red that cycle sequentially. A QHistoryState inside
/// Operating remembers which child was active before an emergency
/// interrupt, allowing seamless resume.
class TrafficController : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the state machine and wires all transitions.
    /// @param[in] parent Parent QObject for ownership.
    explicit TrafficController(QObject* parent = nullptr);

    /// @brief Starts the state machine — enters the normal traffic cycle.
    void start();

    /// @brief Posts an emergency event to transition to flashing-red mode.
    void triggerEmergency();

    /// @brief Posts a resolve event to return to the previous light via history.
    void resolveEmergency();

    /// @brief Posts a shutdown event to transition to the final state.
    void shutdown();

private:
    /// @brief Builds the entire state hierarchy and connects transitions.
    void buildStateMachine();

    /// @brief Connects state entry/exit to debug output lambdas.
    void connectStateSignals();

    /// @brief Schedules the next event after entering a traffic-light state.
    /// @param[in] ms         Delay in milliseconds before posting the event.
    /// @param[in] eventType  The custom event type to post.
    void scheduleEvent(int ms, int eventType);

    // State machine and top-level states
    QStateMachine* m_machine;       ///< Owns all states; parent = this
    QState*        m_stateOperating;  ///< Normal cycle container
    QState*        m_stateEmergency;  ///< Flashing-red override
    QFinalState*   m_stateShutdown;   ///< Terminal state

    // Child states inside Operating
    QState* m_stateGreen;   ///< Green light phase
    QState* m_stateYellow;  ///< Yellow light phase
    QState* m_stateRed;     ///< Red light phase

    // History state to remember last active child in Operating
    QHistoryState* m_historyOperating;
};
