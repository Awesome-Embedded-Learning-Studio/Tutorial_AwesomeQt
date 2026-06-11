/// @file    web_profile_manager.h
/// @brief   Manages QWebEngineProfile configuration for the demo application.
///
/// Encapsulates profile setup: user-agent, cookie policy, and custom scheme
/// handler registration. Demonstrates how to isolate a WebEngine profile
/// from the default chromium profile.

#pragma once

#include <QObject>
#include <QWebEngineProfile>

class CustomSchemeHandler;

/// @brief Configures a QWebEngineProfile with custom settings and scheme handlers.
///
/// Separates profile configuration from UI logic so the same setup can be
/// reused across different windows or tests.
class WebProfileManager : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the profile manager.
    /// @param[in] parent  Parent QObject for Qt object-tree ownership.
    explicit WebProfileManager(QObject* parent = nullptr);

    /// @brief Applies all custom settings to the given profile.
    /// @param[in] profile  The off-the-record profile to configure.
    /// @note Must be called before the profile is associated with any page.
    ///       Off-the-record profiles do not persist data to disk.
    void setupProfile(QWebEngineProfile* profile);

    /// @brief Returns the custom scheme handler (for testing or further config).
    /// @return Pointer to the handler; ownership stays with this manager.
    CustomSchemeHandler* schemeHandler() const;

private:
    CustomSchemeHandler* m_schemeHandler;  ///< Handles demo:// URLs
};
