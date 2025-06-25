#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "mainwindow.h"

/**
 * @brief Main application entry point
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return Application exit code
 */
int main(int argc, char *argv[])
{
    // Create Qt application instance
    QApplication _app(argc, argv);  // Main application object that manages the application's control flow and settings

    // Set application properties for professional appearance
    _app.setApplicationName("SQL Table Editor");
    _app.setApplicationVersion("1.0.0");
    _app.setOrganizationName("Professional Software Solutions");
    _app.setApplicationDisplayName("Professional SQL Table Editor");

    // Set application style for modern appearance
    _app.setStyle(QStyleFactory::create("Fusion"));  // Fusion style provides a modern cross-platform look

    // Apply professional color scheme
    QPalette _darkPalette;  // Color palette for application theming (dark theme)
    _darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    _darkPalette.setColor(QPalette::WindowText, Qt::white);
    _darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    _darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    _darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    _darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    _darkPalette.setColor(QPalette::Text, Qt::white);
    _darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    _darkPalette.setColor(QPalette::ButtonText, Qt::white);
    _darkPalette.setColor(QPalette::BrightText, Qt::red);
    _darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    _darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    _darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    // Comment out dark theme application for now (can be enabled by user preference)
    // _app.setPalette(_darkPalette);

    // Create and show main window
    MainWindow _window;  // Main application window instance
    _window.show();      // Display the window to the user

    // Start application event loop
    return _app.exec();  // Returns application exit code when the main loop exits
}
