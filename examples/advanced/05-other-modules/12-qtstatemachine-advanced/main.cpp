/// @file    main.cpp
/// @brief   Console demo for the hierarchical traffic-light state machine.
///
/// Drives the TrafficController through a scripted sequence:
/// normal cycle -> emergency interrupt -> resume via history -> shutdown.

#include "traffic_controller.h"

#include <cstdio>

#include <QCoreApplication>
#include <QTimer>

/// @brief Entry point — sets up the controller and a scripted event sequence.
/// @param[in] argc Argument count.
/// @param[in] argv Argument values.
/// @return Exit code from QCoreApplication.
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    std::fprintf(stderr, "=== Traffic Controller State Machine Demo ===\n\n");

    TrafficController controller;
    controller.start();

    // -- Scripted demo sequence using single-shot timers ------------------
    // The timing is chosen so the emergency fires during the Green phase
    // (at t=2s, Green has 3s duration). After resolving, the history state
    // should take us back to Green (the phase that was interrupted).

    // t=2s: Trigger emergency while Green is active
    QTimer::singleShot(2000, [&controller]() {
        std::fprintf(stderr, "\n=== DEMO: Triggering emergency (during Green phase) ===\n");
        controller.triggerEmergency();
    });

    // t=4s: Resolve emergency — should resume Green via history
    QTimer::singleShot(4000, [&controller]() {
        std::fprintf(stderr, "\n=== DEMO: Resolving emergency (history -> Green) ===\n");
        controller.resolveEmergency();
    });

    // t=10s: Trigger emergency again, this time during a different phase
    QTimer::singleShot(10000, [&controller]() {
        std::fprintf(stderr, "\n=== DEMO: Triggering emergency (second time) ===\n");
        controller.triggerEmergency();
    });

    // t=12s: Resolve — history should remember which phase was active
    QTimer::singleShot(12000, [&controller]() {
        std::fprintf(stderr, "\n=== DEMO: Resolving emergency (history restore) ===\n");
        controller.resolveEmergency();
    });

    // t=16s: Clean shutdown
    QTimer::singleShot(16000, [&controller]() {
        std::fprintf(stderr, "\n=== DEMO: Shutting down ===\n");
        controller.shutdown();
    });

    return app.exec();
}
