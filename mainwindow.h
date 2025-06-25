#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QLabel>
#include <QPrinter>
#include <QPainter>
#include <QTextDocument>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include "sqlworker.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

/**
 * @brief Main window class for SQL Table Editor application
 * Handles all user interface interactions and delegates business logic to SQLWorker
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for MainWindow
     * @param parent Parent widget pointer
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor for MainWindow
     */
    ~MainWindow();

private slots:
    /**
     * @brief Handle file chooser button click
     */
    void OnChooseFileClicked();

    /**
     * @brief Handle load file button click
     */
    void OnLoadFileClicked();

    /**
     * @brief Handle table selection change in combo box
     */
    void OnTableSelectionChanged();

    /**
     * @brief Handle add button toggle
     */
    void OnAddButtonClicked();

    /**
     * @brief Handle delete button toggle
     */
    void OnDeleteButtonClicked();

    /**
     * @brief Handle edit button toggle
     */
    void OnEditButtonClicked();

    /**
     * @brief Handle update button click
     */
    void OnUpdateButtonClicked();

    /**
     * @brief Handle cancel button click to discard changes
     */
    void OnCancelButtonClicked();

    /**
     * @brief Handle print button click to export table
     */
    void OnPrintButtonClicked();

    /**
     * @brief Handle row double click for deletion
     */
    void OnRowDoubleClicked(int row, int column);

private:
    /**
     * @brief Initialize the user interface components
     */
    void InitializeUI();

    /**
     * @brief Setup button connections and properties
     */
    void SetupConnections();

    /**
     * @brief Update button states based on current mode
     */
    void UpdateButtonStates();

    /**
     * @brief Reset all toggle buttons to normal state
     */
    void ResetToggleButtons();

    /**
     * @brief Load selected table data into the table widget
     */
    void LoadTableData();

    /**
     * @brief Add new empty row to the table
     */
    void AddNewRow();

    /**
     * @brief Delete specified row from table
     */
    void DeleteRow(int row);

    /**
     * @brief Enable table editing mode
     */
    void EnableTableEditing();

    /**
     * @brief Disable table editing mode
     */
    void DisableTableEditing();

    /**
     * @brief Export current table to PDF file
     * @param filePath Path where PDF file will be saved
     * @return true if export successful, false otherwise
     */
    bool ExportTableToPDF(const QString &filePath);

    /**
     * @brief Export current table to Excel-compatible CSV file
     * @param filePath Path where Excel file will be saved
     * @return true if export successful, false otherwise
     */
    bool ExportTableToExcel(const QString &filePath);

    /**
     * @brief Generate HTML table representation for PDF export
     * @return QString containing HTML table markup
     */
    QString GenerateHTMLTable();

    // UI Components
    QWidget *CentralWidget;              // Main central widget container for the application
    QVBoxLayout *MainLayout;             // Main vertical layout for organizing UI elements
    QHBoxLayout *FileLayout;             // Layout for file operation controls
    QHBoxLayout *TableLayout;            // Layout for table selection controls
    QHBoxLayout *ButtonLayout;           // Layout for action button controls

    QPushButton *ChooseFileButton;       // Button to choose SQL file from filesystem
    QPushButton *LoadFileButton;         // Button to load the selected SQL file
    QLabel *FilePathLabel;               // Label showing current file path (empty if no file selected)

    QComboBox *TableComboBox;            // Dropdown for table selection (empty until file loaded)
    QLabel *TableLabel;                  // Label for table selection section

    QPushButton *AddButton;              // Toggle button for adding rows (green when active)
    QPushButton *DeleteButton;           // Toggle button for deleting rows (green when active)
    QPushButton *EditButton;             // Toggle button for editing cells (green when active)
    QPushButton *UpdateButton;           // Button to save changes to the SQL file
    QPushButton *CancelButton;           // Button to discard all pending changes
    QPushButton *PrintButton;            // Button to export table to PDF and Excel files

    QTableWidget *DataTable;             // Main data display table for SQL content

    // State variables
    SQLWorker *Worker;                   // Worker object for SQL operations
    QString CurrentFilePath;             // Path to currently loaded SQL file (empty if none loaded)
    QString CurrentTableName;            // Name of currently selected table (empty if none selected)
    bool IsAddMode;                      // Flag indicating add mode is active (true) or inactive (false)
    bool IsDeleteMode;                   // Flag indicating delete mode is active (true) or inactive (false)
    bool IsEditMode;                     // Flag indicating edit mode is active (true) or inactive (false)
    bool HasUnsavedChanges;              // Flag indicating pending changes (true) or no changes (false)

    // Constants
    static const QString NORMAL_BUTTON_STYLE;  // Default button style
    static const QString ACTIVE_BUTTON_STYLE;  // Green active button style
    static const QString DISABLED_BUTTON_STYLE;  // Style for disabled buttons
};

#endif // MAINWINDOW_H
