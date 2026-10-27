/// @file    main.cpp
/// @brief   Console demo for the microwave SCXML state machine.
///
/// Demonstrates:
///   1. Loading an SCXML file at runtime.
///   2. Setting data model variables (timer, powerLevel).
///   3. Conditional transitions (start blocked when door is open or timer == 0).
///   4. Delayed events (timerDone fires after the cooking duration).
///   5. State transition logging via QScxmlStateMachine::connectToState().
///
/// The demo manually submits timerDone to show the concept without relying on
/// the SCXML delayed event timing, keeping the demo fast and deterministic.
///
/// Corresponding tutorial: Advanced 05-Other-Modules/13-QtScxml.

#include "microwave_controller.h"
#include "scxml_source_path.h"                                       // CMake-configured SCXML source dir

#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include <cstdio>
#include <functional>

/// @brief Prints a separator line for readability in console output.
static void printSeparator()
{
    std::fprintf(stderr, "===================================================="
                          "============================\n");
}

/// @brief Prints a step header to the console.
/// @param[in] step  The step number.
/// @param[in] title The step description.
static void printStep(int step, const char* title)
{
    std::fprintf(stderr, "\n");
    printSeparator();
    std::fprintf(stderr, "  Step %d: %s\n", step, title);
    printSeparator();
}

/// @brief Helper to run a lambda after a short delay.
/// @param[in] delayMs  Delay in milliseconds.
/// @param[in] func     Callable to invoke after the delay.
/// @note SCXML events are processed asynchronously by the Qt event loop.
///       We use delays to allow the state machine to stabilize between steps.
static void after(int delayMs, std::function<void()> func)
{
    QTimer::singleShot(delayMs, std::move(func));
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    MicrowaveController controller;

    // Resolve the SCXML file path: check alongside the executable first,
    // then fall back to the source directory (typical for build trees).
    QString scxmlPath = QCoreApplication::applicationDirPath()
                        + "/microwave.scxml";
    if (!QFile::exists(scxmlPath)) {
        scxmlPath = QStringLiteral(SCXML_SOURCE_DIR) + "/microwave.scxml";
    }

    if (!controller.loadMachine(scxmlPath)) {
        std::fprintf(stderr, "[FATAL] Failed to load SCXML state machine.\n");
        return 1;
    }

    // Delay constants for the demo steps (ms)
    constexpr int kInitDelay   = 200;   // Wait for initial state to settle
    constexpr int kStepDelay   = 300;   // Wait between each demo step
    constexpr int kQuitDelay   = 300;   // Pause before auto-quit

    after(kInitDelay, [&controller]() {
        // ---- Step 1: Set data model values ----
        printStep(1, "Set data model: timer=30, powerLevel=800");
        controller.setData("timer", 30);
        controller.setData("powerLevel", 800);
        std::fprintf(stderr, "  timer     = %s\n",
                     controller.getData("timer").toString().toUtf8().constData());
        std::fprintf(stderr, "  powerLevel= %s\n",
                     controller.getData("powerLevel").toString().toUtf8().constData());

        // ---- Step 2: Try to start with door open (should fail) ----
        printStep(2, "Open door, then try to start (should be blocked)");
        controller.submitEvent("openDoor");
        after(kStepDelay, [&controller]() {
            // Now in doorOpen — try to start (SCXML will reject it)
            controller.submitEvent("start");
            after(kStepDelay, [&controller]() {
                std::fprintf(stderr, "  In doorOpen state? %s\n",
                             controller.isInState("doorOpen") ? "YES" : "NO");
                std::fprintf(stderr, "  In cooking state?  %s\n",
                             controller.isInState("cooking") ? "YES" : "NO");

                // ---- Step 3: Close door, then start cooking ----
                printStep(3, "Close door, then start cooking");
                controller.submitEvent("closeDoor");
                after(kStepDelay, [&controller]() {
                    controller.submitEvent("start");
                    after(kStepDelay, [&controller]() {
                        std::fprintf(stderr, "  In cooking state?  %s\n",
                                     controller.isInState("cooking") ? "YES" : "NO");

                        // ---- Step 4: Submit timerDone to simulate complete ----
                        printStep(4,
                            "Submit timerDone event (simulates cooking complete)");
                        controller.submitEvent("timerDone");
                        after(kStepDelay, [&controller]() {
                            std::fprintf(stderr, "  In done state?     %s\n",
                                         controller.isInState("done")
                                             ? "YES" : "NO");

                            // ---- Step 5: Summary ----
                            printStep(5,
                                "Demo complete -- all transitions demonstrated");
                            std::fprintf(stderr,
                                "  Final state machine summary:\n");
                            std::fprintf(stderr, "    timer     = %s\n",
                                controller.getData("timer").toString()
                                    .toUtf8().constData());
                            std::fprintf(stderr, "    powerLevel= %s\n",
                                controller.getData("powerLevel").toString()
                                    .toUtf8().constData());
                            std::fprintf(stderr, "    idle?      %s\n",
                                controller.isInState("idle")
                                    ? "YES" : "NO");
                            std::fprintf(stderr, "    doorOpen?  %s\n",
                                controller.isInState("doorOpen")
                                    ? "YES" : "NO");
                            std::fprintf(stderr, "    cooking?   %s\n",
                                controller.isInState("cooking")
                                    ? "YES" : "NO");
                            std::fprintf(stderr, "    paused?    %s\n",
                                controller.isInState("paused")
                                    ? "YES" : "NO");
                            std::fprintf(stderr, "    done?      %s\n",
                                controller.isInState("done")
                                    ? "YES" : "NO");

                            // Auto-quit after the demo
                            after(kQuitDelay, []() {
                                QCoreApplication::quit();
                            });
                        });
                    });
                });
            });
        });
    });

    return app.exec();
}
