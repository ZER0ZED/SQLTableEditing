#include "mainwindow.h"

// Define button style constants
const QString MainWindow::NORMAL_BUTTON_STYLE = "QPushButton { background-color: #f0f0f0; border: 1px solid #c0c0c0; padding: 5px; color: black; }";
const QString MainWindow::ACTIVE_BUTTON_STYLE = "QPushButton { background-color: #90EE90; border: 2px solid #228B22; padding: 5px; font-weight: bold; color: black; }";
const QString MainWindow::DISABLED_BUTTON_STYLE = "QPushButton:disabled { background-color: #e0e0e0; border: 1px solid #d0d0d0; padding: 5px; color: #a0a0a0; }";

/**
 * @brief Constructor initializes the main window and sets up UI components
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , CentralWidget(nullptr)           // Main widget container
    , MainLayout(nullptr)              // Primary layout manager
    , FileLayout(nullptr)              // File operation layout
    , TableLayout(nullptr)             // Table selection layout
    , ButtonLayout(nullptr)            // Action button layout
    , ChooseFileButton(nullptr)        // File selection button
    , LoadFileButton(nullptr)          // File loading button
    , FilePathLabel(nullptr)           // Current file path display
    , TableComboBox(nullptr)           // Table selection dropdown
    , TableLabel(nullptr)              // Table selection label
    , AddButton(nullptr)               // Row addition toggle button
    , DeleteButton(nullptr)            // Row deletion toggle button
    , EditButton(nullptr)              // Cell editing toggle button
    , UpdateButton(nullptr)            // Changes save button
    , CancelButton(nullptr)            // Changes discard button
    , PrintButton(nullptr)             // Table export button
    , DataTable(nullptr)               // Main data display table
    , Worker(nullptr)                  // SQL processing worker
    , CurrentFilePath("")              // Path to active SQL file
    , CurrentTableName("")             // Name of selected table
    , IsAddMode(false)                 // Add mode state flag
    , IsDeleteMode(false)              // Delete mode state flag
    , IsEditMode(false)                // Edit mode state flag
    , HasUnsavedChanges(false)         // Unsaved changes indicator
{
    InitializeUI();
    SetupConnections();

    // Initialize worker for SQL operations
    Worker = new SQLWorker();

    // Set initial window properties
    setWindowTitle("Professional SQL Table Editor");
    setMinimumSize(800, 600);
    resize(1200, 800);
}

/**
 * @brief Destructor cleans up allocated resources
 */
MainWindow::~MainWindow()
{
    delete Worker;  // Clean up SQL worker instance
}

/**
 * @brief Initialize all user interface components and layouts
 */
void MainWindow::InitializeUI()
{
    // Create central widget and main layout
    CentralWidget = new QWidget(this);
    setCentralWidget(CentralWidget);

    MainLayout = new QVBoxLayout(CentralWidget);
    MainLayout->setSpacing(10);
    MainLayout->setContentsMargins(10, 10, 10, 10);

    // Setup file operations section
    FileLayout = new QHBoxLayout();
    ChooseFileButton = new QPushButton("Choose SQL File", this);
    LoadFileButton = new QPushButton("Load File", this);
    FilePathLabel = new QLabel("No file selected", this);

    // Configure file operation buttons
    ChooseFileButton->setMinimumHeight(35);
    LoadFileButton->setMinimumHeight(35);
    LoadFileButton->setEnabled(false);  // Disabled until file chosen

    FilePathLabel->setStyleSheet("QLabel { background-color: #ffffff; border: 1px solid #c0c0c0; padding: 5px; color: black; font-weight: normal; }");
    FilePathLabel->setWordWrap(true);
    FilePathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    FilePathLabel->setMinimumWidth(300);
    FilePathLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    FileLayout->addWidget(ChooseFileButton);
    FileLayout->addWidget(LoadFileButton);
    FileLayout->addWidget(FilePathLabel, 1);  // Stretch factor for path label

    // Setup table selection section
    TableLayout = new QHBoxLayout();
    TableLabel = new QLabel("Select Table:", this);
    TableComboBox = new QComboBox(this);

    TableComboBox->setMinimumHeight(30);
    TableComboBox->setEnabled(false);  // Disabled until file loaded

    TableLayout->addWidget(TableLabel);
    TableLayout->addWidget(TableComboBox, 1);  // Stretch factor for combo box

    // Setup action buttons section
    ButtonLayout = new QHBoxLayout();
    AddButton = new QPushButton("Add Row", this);
    DeleteButton = new QPushButton("Delete Row", this);
    EditButton = new QPushButton("Edit Cells", this);
    UpdateButton = new QPushButton("Update SQL", this);
    CancelButton = new QPushButton("Cancel", this);
    PrintButton = new QPushButton("Print Table", this);

    // Configure action buttons
    AddButton->setMinimumHeight(35);
    DeleteButton->setMinimumHeight(35);
    EditButton->setMinimumHeight(35);
    UpdateButton->setMinimumHeight(35);
    CancelButton->setMinimumHeight(35);
    PrintButton->setMinimumHeight(35);

    // Set initial button states
    AddButton->setCheckable(true);      // Make toggle button
    DeleteButton->setCheckable(true);   // Make toggle button
    EditButton->setCheckable(true);     // Make toggle button

    // Apply button styling with disabled state
    QString combinedStyle = NORMAL_BUTTON_STYLE + DISABLED_BUTTON_STYLE;
    AddButton->setStyleSheet(combinedStyle);
    DeleteButton->setStyleSheet(combinedStyle);
    EditButton->setStyleSheet(combinedStyle);
    UpdateButton->setStyleSheet(combinedStyle);
    CancelButton->setStyleSheet(combinedStyle);
    PrintButton->setStyleSheet(combinedStyle);

    // Disable action buttons until table is selected
    AddButton->setEnabled(false);
    DeleteButton->setEnabled(false);
    EditButton->setEnabled(false);
    UpdateButton->setEnabled(false);
    CancelButton->setEnabled(false);
    PrintButton->setEnabled(false);

    ButtonLayout->addWidget(AddButton);
    ButtonLayout->addWidget(DeleteButton);
    ButtonLayout->addWidget(EditButton);
    ButtonLayout->addWidget(UpdateButton);
    ButtonLayout->addWidget(CancelButton);
    ButtonLayout->addWidget(PrintButton);
    ButtonLayout->addStretch();  // Push buttons to left

    // Setup main data table
    DataTable = new QTableWidget(this);
    DataTable->setAlternatingRowColors(true);
    DataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    DataTable->horizontalHeader()->setStretchLastSection(true);
    DataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Initially read-only

    // Add all layouts to main layout
    MainLayout->addLayout(FileLayout);
    MainLayout->addLayout(TableLayout);
    MainLayout->addLayout(ButtonLayout);
    MainLayout->addWidget(DataTable, 1);  // Table gets most space
}

/**
 * @brief Setup signal-slot connections for UI interactions
 */
void MainWindow::SetupConnections()
{
    // File operation connections
    connect(ChooseFileButton, &QPushButton::clicked, this, &MainWindow::OnChooseFileClicked);
    connect(LoadFileButton, &QPushButton::clicked, this, &MainWindow::OnLoadFileClicked);

    // Table selection connection
    connect(TableComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::OnTableSelectionChanged);

    // Connect action buttons
    connect(AddButton, &QPushButton::clicked, this, &MainWindow::OnAddButtonClicked);
    connect(DeleteButton, &QPushButton::clicked, this, &MainWindow::OnDeleteButtonClicked);
    connect(EditButton, &QPushButton::clicked, this, &MainWindow::OnEditButtonClicked);
    connect(UpdateButton, &QPushButton::clicked, this, &MainWindow::OnUpdateButtonClicked);
    connect(CancelButton, &QPushButton::clicked, this, &MainWindow::OnCancelButtonClicked);
    connect(PrintButton, &QPushButton::clicked, this, &MainWindow::OnPrintButtonClicked);

    // Table interaction connections
    connect(DataTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::OnRowDoubleClicked);
}

/**
 * @brief Handle file chooser button click to select SQL file
 */
void MainWindow::OnChooseFileClicked()
{
    // Open file dialog to select SQL file
    QString _filePath = QFileDialog::getOpenFileName(  // Path to selected SQL file (empty if canceled)
        this,
        "Select SQL Database File",
        QDir::homePath(),
        "SQLite Database Files (*.db *.sqlite *.sqlite3);;All Files (*.*)"
        );

    if (!_filePath.isEmpty()) {
        // Update file path display and enable load button
        CurrentFilePath = _filePath;

        // Ensure file path is visible by using a cleaner display format
        QFileInfo _fileInfo(_filePath);
        QString _displayText = _fileInfo.fileName() + " (" + _filePath + ")";
        FilePathLabel->setText(_displayText);
        FilePathLabel->setToolTip(_filePath); // Show full path on hover

        LoadFileButton->setEnabled(true);

        // Auto-load file after selection (as requested)
        OnLoadFileClicked();
    }
}

/**
 * @brief Handle load file button click to parse and load SQL data
 */
void MainWindow::OnLoadFileClicked()
{
    if (CurrentFilePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No file selected.");
        return;
    }

    // Reset UI state
    TableComboBox->clear();
    DataTable->setRowCount(0);
    DataTable->setColumnCount(0);

    // Load SQL file using worker
    if (Worker->LoadSQLFile(CurrentFilePath)) {
        // Get available tables from the loaded file
        QStringList _tableNames = Worker->GetTableNames();  // List of table names in the SQL file (empty if none found)

        if (_tableNames.isEmpty()) {
            QMessageBox::warning(this, "Warning", "No tables found in the SQL database file.");
            return;
        }

        // Populate table selection dropdown
        TableComboBox->addItems(_tableNames);
        TableComboBox->setEnabled(true);

        QMessageBox::information(this, "Success", "SQL database file loaded successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to load SQL database file. Please check if it is a valid SQLite database file.");
    }
}

/**
 * @brief Handle table selection change in combo box
 */
void MainWindow::OnTableSelectionChanged()
{
    if (TableComboBox->currentIndex() >= 0) {
        CurrentTableName = TableComboBox->currentText();
        LoadTableData();

        // Configure for table usage
        TableComboBox->setEnabled(true);
        AddButton->setEnabled(true);
        DeleteButton->setEnabled(true);
        EditButton->setEnabled(true);
        UpdateButton->setEnabled(true);
        CancelButton->setEnabled(true);
        PrintButton->setEnabled(true);  // Enable print button when table is selected

        // Reset any active modes
        ResetToggleButtons();
    }
}

/**
 * @brief Handle add button toggle for row addition mode
 */
void MainWindow::OnAddButtonClicked()
{
    if (IsAddMode) {
        // Cancel add mode if already active
        IsAddMode = false;
        AddButton->setStyleSheet(NORMAL_BUTTON_STYLE);
        AddButton->setChecked(false);
        DisableTableEditing();
    } else {
        // Activate add mode
        ResetToggleButtons();  // Deactivate other modes
        IsAddMode = true;
        AddButton->setStyleSheet(ACTIVE_BUTTON_STYLE);
        AddButton->setChecked(true);
        AddNewRow();
    }
}

/**
 * @brief Handle delete button toggle for row deletion mode
 */
void MainWindow::OnDeleteButtonClicked()
{
    if (IsDeleteMode) {
        // Cancel delete mode if already active
        IsDeleteMode = false;
        DeleteButton->setStyleSheet(NORMAL_BUTTON_STYLE);
        DeleteButton->setChecked(false);
    } else {
        // Activate delete mode
        ResetToggleButtons();  // Deactivate other modes
        IsDeleteMode = true;
        DeleteButton->setStyleSheet(ACTIVE_BUTTON_STYLE);
        DeleteButton->setChecked(true);

        QMessageBox::information(this, "Delete Mode",
                                 "Delete mode activated. Double-click any row to delete it.");
    }
}

/**
 * @brief Handle edit button toggle for cell editing mode
 */
void MainWindow::OnEditButtonClicked()
{
    if (IsEditMode) {
        // Cancel edit mode if already active
        IsEditMode = false;
        EditButton->setStyleSheet(NORMAL_BUTTON_STYLE);
        EditButton->setChecked(false);
        DisableTableEditing();
    } else {
        // Activate edit mode
        ResetToggleButtons();  // Deactivate other modes
        IsEditMode = true;
        EditButton->setStyleSheet(ACTIVE_BUTTON_STYLE);
        EditButton->setChecked(true);
        EnableTableEditing();
    }
}

/**
 * @brief Handle update button click to save all changes to SQL file
 */
void MainWindow::OnUpdateButtonClicked()
{
    if (!HasUnsavedChanges && !IsAddMode && !IsDeleteMode && !IsEditMode) {
        QMessageBox::information(this, "Info", "No changes to save.");
        return;
    }

    bool success = false;

    if (IsAddMode || IsEditMode) {
        // Save current table state for add or edit operations
        success = Worker->UpdateCompleteTable(CurrentTableName, DataTable);
    } else if (IsDeleteMode) {
        // For delete operations, apply changes now using current table state
        success = Worker->UpdateCompleteTable(CurrentTableName, DataTable);
    } else {
        // No active mode but changes exist
        success = Worker->UpdateCompleteTable(CurrentTableName, DataTable);
    }

    if (success) {
        QMessageBox::information(this, "Success", "Changes saved successfully to SQL database file.");

        // Reset all modes and reload data
        ResetToggleButtons();
        LoadTableData();
        HasUnsavedChanges = false;
    } else {
        QMessageBox::critical(this, "Error", "Failed to save changes to SQL database file.");
    }
}

/**
 * @brief Handle cancel button click to discard all changes
 */
void MainWindow::OnCancelButtonClicked()
{
    if (!HasUnsavedChanges && !IsAddMode && !IsDeleteMode && !IsEditMode) {
        QMessageBox::information(this, "Info", "No changes to discard.");
        return;
    }

    // Ask for confirmation before discarding changes
    int result = QMessageBox::question(this, "Confirm Discard",
                                       "Are you sure you want to discard all changes?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        // Reset all toggle buttons and modes
        ResetToggleButtons();

        // Reload the data from the original SQL file
        LoadTableData();

        // Reset unsaved changes flag
        HasUnsavedChanges = false;

        QMessageBox::information(this, "Info", "All changes have been discarded.");
    }
}

/**
 * @brief Handle print button click to export table to PDF and Excel files
 */
void MainWindow::OnPrintButtonClicked()
{
    if (CurrentTableName.isEmpty() || DataTable->rowCount() == 0) {
        QMessageBox::warning(this, "Warning", "No table data to export.");
        return;
    }

    // Create export directory
    QString _downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (_downloadsPath.isEmpty()) {
        _downloadsPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    QDir _exportDir(_downloadsPath);
    if (!_exportDir.exists()) {
        _exportDir.mkpath(_downloadsPath);
    }

    // Generate file names with timestamp
    QString _timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString _baseName = QString("%1_%2").arg(CurrentTableName, _timestamp);

    QString _pdfPath = _exportDir.filePath(_baseName + ".pdf");
    QString _excelPath = _exportDir.filePath(_baseName + ".csv");

    bool _pdfSuccess = false;
    bool _excelSuccess = false;

    // Export to PDF
    _pdfSuccess = ExportTableToPDF(_pdfPath);

    // Export to Excel-compatible CSV
    _excelSuccess = ExportTableToExcel(_excelPath);

    // Show results
    QString _message;
    if (_pdfSuccess && _excelSuccess) {
        _message = QString("Table exported successfully!\n\nPDF: %1\nExcel: %2").arg(_pdfPath, _excelPath);
        QMessageBox::information(this, "Export Successful", _message);
    } else if (_pdfSuccess) {
        _message = QString("PDF exported successfully: %1\n\nExcel export failed.").arg(_pdfPath);
        QMessageBox::warning(this, "Partial Export", _message);
    } else if (_excelSuccess) {
        _message = QString("Excel exported successfully: %1\n\nPDF export failed.").arg(_excelPath);
        QMessageBox::warning(this, "Partial Export", _message);
    } else {
        QMessageBox::critical(this, "Export Failed", "Both PDF and Excel export failed.");
    }
}

/**
 * @brief Handle row double-click for deletion in delete mode
 */
void MainWindow::OnRowDoubleClicked(int row, int column)
{
    Q_UNUSED(column)  // Column parameter not needed for row deletion

    if (IsDeleteMode && row >= 0) {
        int _result = QMessageBox::question(  // Dialog result: QMessageBox::Yes or QMessageBox::No
            this,
            "Confirm Deletion",
            QString("Are you sure you want to delete row %1?").arg(row + 1),
            QMessageBox::Yes | QMessageBox::No
            );

        if (_result == QMessageBox::Yes) {
            // Only delete from the displayed table, not from the SQL file
            // The actual SQL update happens when the Update button is clicked
            DeleteRow(row);
            HasUnsavedChanges = true;  // Mark that changes need to be saved
        }
    }
}

/**
 * @brief Reset all toggle buttons to unchecked state
 */
void MainWindow::ResetToggleButtons()
{
    // Uncheck all toggle buttons
    AddButton->setChecked(false);
    DeleteButton->setChecked(false);
    EditButton->setChecked(false);

    // Reset button styles - combine normal style with disabled style for proper appearance
    QString _combinedStyle = NORMAL_BUTTON_STYLE + DISABLED_BUTTON_STYLE;  // Style for buttons that includes disabled state appearance

    // Apply appropriate style based on whether buttons are enabled
    if (AddButton->isEnabled()) {
        AddButton->setStyleSheet(NORMAL_BUTTON_STYLE);
    } else {
        AddButton->setStyleSheet(_combinedStyle);
    }

    if (DeleteButton->isEnabled()) {
        DeleteButton->setStyleSheet(NORMAL_BUTTON_STYLE);
    } else {
        DeleteButton->setStyleSheet(_combinedStyle);
    }

    if (EditButton->isEnabled()) {
        EditButton->setStyleSheet(NORMAL_BUTTON_STYLE);
    } else {
        EditButton->setStyleSheet(_combinedStyle);
    }

    if (PrintButton->isEnabled()) {
        PrintButton->setStyleSheet(NORMAL_BUTTON_STYLE);
    } else {
        PrintButton->setStyleSheet(_combinedStyle);
    }

    // Reset mode flags
    IsAddMode = false;
    IsDeleteMode = false;
    IsEditMode = false;

    // Reset table edit mode
    DataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

/**
 * @brief Load selected table data into the table widget using worker
 */
void MainWindow::LoadTableData()
{
    if (CurrentTableName.isEmpty() || !Worker->IsFileLoaded()) {
        return;
    }

    if (Worker->LoadTableData(CurrentTableName, DataTable)) {
        DataTable->resizeColumnsToContents();
        HasUnsavedChanges = false;
    } else {
        QMessageBox::warning(this, "Warning", "Failed to load table data.");
    }
}

/**
 * @brief Add new empty row to the bottom of the table
 */
void MainWindow::AddNewRow()
{
    int _newRow = DataTable->rowCount();  // Index of the new row to be added (0-based)
    DataTable->insertRow(_newRow);

    // Fill new row with empty items to make them editable
    for (int _col = 0; _col < DataTable->columnCount(); ++_col) {  // Loop through columns (0 to columnCount-1)
        QTableWidgetItem *_item = new QTableWidgetItem("");  // New empty cell item
        DataTable->setItem(_newRow, _col, _item);
    }

    // Enable editing for the new row
    DataTable->setEditTriggers(QAbstractItemView::DoubleClicked);
    DataTable->scrollToBottom();  // Scroll to show new row
    HasUnsavedChanges = true;  // Mark that changes need to be saved
}

/**
 * @brief Delete specified row from the table display
 */
void MainWindow::DeleteRow(int row)
{
    if (row >= 0 && row < DataTable->rowCount()) {
        DataTable->removeRow(row);
        HasUnsavedChanges = true;
    }
}

/**
 * @brief Enable table cell editing mode
 */
void MainWindow::EnableTableEditing()
{
    DataTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
}

/**
 * @brief Disable table cell editing mode
 */
void MainWindow::DisableTableEditing()
{
    DataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

/**
 * @brief Export current table to PDF file
 * @param filePath Path where PDF file will be saved
 * @return true if export successful, false otherwise
 */
bool MainWindow::ExportTableToPDF(const QString &filePath)
{
    QPrinter _printer(QPrinter::HighResolution);
    _printer.setOutputFormat(QPrinter::PdfFormat);
    _printer.setOutputFileName(filePath);
    _printer.setPageSize(QPageSize::A4);
    _printer.setPageOrientation(QPageLayout::Landscape);

    QTextDocument _document;
    _document.setHtml(GenerateHTMLTable());

    _document.print(&_printer);

    return QFile::exists(filePath);
}

/**
 * @brief Export current table to Excel-compatible CSV file
 * @param filePath Path where Excel file will be saved
 * @return true if export successful, false otherwise
 */
bool MainWindow::ExportTableToExcel(const QString &filePath)
{
    QFile _file(filePath);
    if (!_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // Write UTF-8 BOM first for Excel compatibility
    _file.write("\xEF\xBB\xBF");

    QTextStream _stream(&_file);
    // Qt 6 uses UTF-8 by default, Qt 5 should handle it properly too

    // Write headers
    QStringList _headers;
    for (int _col = 0; _col < DataTable->columnCount(); ++_col) {
        QTableWidgetItem *_headerItem = DataTable->horizontalHeaderItem(_col);
        QString _headerText = _headerItem ? _headerItem->text() : QString("Column_%1").arg(_col + 1);

        // Escape quotes and wrap in quotes if contains comma, quote, or newline
        if (_headerText.contains(',') || _headerText.contains('"') || _headerText.contains('\n')) {
            _headerText.replace('"', "\"\"");
            _headerText = '"' + _headerText + '"';
        }
        _headers.append(_headerText);
    }
    _stream << _headers.join(',') << '\n';

    // Write data rows
    for (int _row = 0; _row < DataTable->rowCount(); ++_row) {
        QStringList _rowData;
        for (int _col = 0; _col < DataTable->columnCount(); ++_col) {
            QTableWidgetItem *_item = DataTable->item(_row, _col);
            QString _cellText = _item ? _item->text() : "";

            // Escape quotes and wrap in quotes if contains comma, quote, or newline
            if (_cellText.contains(',') || _cellText.contains('"') || _cellText.contains('\n')) {
                _cellText.replace('"', "\"\"");
                _cellText = '"' + _cellText + '"';
            }
            _rowData.append(_cellText);
        }
        _stream << _rowData.join(',') << '\n';
    }

    _file.close();
    return true;
}

/**
 * @brief Generate HTML table representation for PDF export
 * @return QString containing HTML table markup
 */
QString MainWindow::GenerateHTMLTable()
{
    QString _html = "<html><head><style>";
    _html += "body { font-family: Arial, sans-serif; margin: 20px; }";
    _html += "h1 { color: #333; text-align: center; margin-bottom: 20px; }";
    _html += "table { border-collapse: collapse; width: 100%; margin: 0 auto; }";
    _html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
    _html += "th { background-color: #f2f2f2; font-weight: bold; }";
    _html += "tr:nth-child(even) { background-color: #f9f9f9; }";
    _html += ".info { font-size: 12px; color: #666; text-align: center; margin-top: 20px; }";
    _html += "</style></head><body>";

    // Add title
    _html += QString("<h1>Table: %1</h1>").arg(CurrentTableName);

    // Add table
    _html += "<table>";

    // Add headers
    _html += "<tr>";
    for (int _col = 0; _col < DataTable->columnCount(); ++_col) {
        QTableWidgetItem *_headerItem = DataTable->horizontalHeaderItem(_col);
        QString _headerText = _headerItem ? _headerItem->text() : QString("Column_%1").arg(_col + 1);
        _html += QString("<th>%1</th>").arg(_headerText.toHtmlEscaped());
    }
    _html += "</tr>";

    // Add data rows
    for (int _row = 0; _row < DataTable->rowCount(); ++_row) {
        _html += "<tr>";
        for (int _col = 0; _col < DataTable->columnCount(); ++_col) {
            QTableWidgetItem *_item = DataTable->item(_row, _col);
            QString _cellText = _item ? _item->text() : "";
            _html += QString("<td>%1</td>").arg(_cellText.toHtmlEscaped());
        }
        _html += "</tr>";
    }

    _html += "</table>";

    // Add export info
    QString _timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    _html += QString("<div class='info'>Exported on %1 | Total rows: %2</div>")
                 .arg(_timestamp)
                 .arg(DataTable->rowCount());

    _html += "</body></html>";

    return _html;
}
