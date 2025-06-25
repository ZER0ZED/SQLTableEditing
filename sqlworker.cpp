#include "sqlworker.h"
#include <QUuid>
#include <QSqlDriver>

// Define SQL query constants
const QString SQLWorker::GET_TABLES_QUERY = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'";
const QString SQLWorker::GET_COLUMNS_QUERY = "PRAGMA table_info(%1)";
const QString SQLWorker::SELECT_ALL_QUERY = "SELECT * FROM %1";
const QString SQLWorker::DELETE_ALL_QUERY = "DELETE FROM %1";
const QString SQLWorker::INSERT_QUERY_TEMPLATE = "INSERT INTO %1 (%2) VALUES (%3)";

/**
 * @brief Constructor initializes SQLWorker with default values
 */
SQLWorker::SQLWorker()
    : CurrentFilePath("")              // Path to active SQL database file
    , AvailableTableNames()            // List of discovered table names
    , FileLoaded(false)                // File loading status flag
    , ConnectionName("")               // Unique connection name
    , TableBackups()                   // Backup storage for rollback functionality
{
    // Generate unique connection name for this worker instance
    ConnectionName = GenerateConnectionName();
}

/**
 * @brief Destructor performs cleanup operations
 */
SQLWorker::~SQLWorker()
{
    // Close database connection if open
    if (SqlDatabase.isOpen()) {
        SqlDatabase.close();
    }

    // Remove the database connection
    if (QSqlDatabase::contains(ConnectionName)) {
        QSqlDatabase::removeDatabase(ConnectionName);
    }
}

/**
 * @brief Load and connect to SQL database file from specified path
 * @param filePath Path to the SQL database file to load (absolute or relative path)
 * @return true if file loaded and connected successfully, false otherwise
 */
bool SQLWorker::LoadSQLFile(const QString &filePath)
{
    // Validate input parameters
    if (filePath.isEmpty()) {
        qDebug() << "Error: Empty file path provided";
        return false;
    }

    // Close existing connection if open
    if (SqlDatabase.isOpen()) {
        SqlDatabase.close();
    }

    // Remove existing connection if it exists
    if (QSqlDatabase::contains(ConnectionName)) {
        QSqlDatabase::removeDatabase(ConnectionName);
    }

    // Create new SQLite database connection
    SqlDatabase = QSqlDatabase::addDatabase("QSQLITE", ConnectionName);
    SqlDatabase.setDatabaseName(filePath);

    // Attempt to open the database
    if (!SqlDatabase.open()) {
        qDebug() << "Error: Cannot open database file" << filePath;
        qDebug() << "Database error:" << SqlDatabase.lastError().text();
        return false;
    }

    // Validate database connection
    if (!ValidateDatabaseConnection()) {
        qDebug() << "Error: Invalid database structure";
        SqlDatabase.close();
        return false;
    }

    // Store file path and extract table information
    CurrentFilePath = filePath;
    ParseSQLStructure();
    FileLoaded = true;

    qDebug() << "Successfully loaded SQL database file:" << filePath;
    qDebug() << "Found" << AvailableTableNames.size() << "tables";

    return true;
}

/**
 * @brief Get list of all available table names from loaded database
 */
QStringList SQLWorker::GetTableNames() const
{
    return AvailableTableNames;
}

/**
 * @brief Load specific table data into provided QTableWidget
 * @param tableName Name of the table to load (must exist in database)
 * @param tableWidget Pointer to QTableWidget that will display the data
 * @return true if data loaded successfully, false on error
 */
bool SQLWorker::LoadTableData(const QString &tableName, QTableWidget *tableWidget)
{
    // Validate input parameters
    if (!FileLoaded || tableName.isEmpty() || !tableWidget) {
        qDebug() << "Error: Invalid parameters for loading table data";
        return false;
    }

    // Check if database connection is still valid
    if (!SqlDatabase.isOpen()) {
        qDebug() << "Error: Database connection is not open";
        return false;
    }

    // Get column information for the table
    QStringList _columnNames = GetTableColumns(tableName);  // List of column names from table schema
    if (_columnNames.isEmpty()) {
        qDebug() << "Error: Could not retrieve column information for table" << tableName;
        return false;
    }

    // Prepare and execute query to get all table data
    QSqlQuery _query(SqlDatabase);  // Query object for executing SQL commands
    QString _queryString = SELECT_ALL_QUERY.arg(tableName);  // Complete SELECT query string

    if (!_query.exec(_queryString)) {
        qDebug() << "Error: Failed to execute query:" << _queryString;
        qDebug() << "SQL error:" << _query.lastError().text();
        return false;
    }

    // Configure table widget dimensions
    tableWidget->clear();
    tableWidget->setColumnCount(_columnNames.size());
    tableWidget->setHorizontalHeaderLabels(_columnNames);

    // Load data into table widget
    int _rowIndex = 0;  // Current row index being processed (0-based)

    while (_query.next()) {  // Iterate through all rows returned by query
        tableWidget->insertRow(_rowIndex);  // Add new row to table widget

        // Process each column in current row
        for (int _col = 0; _col < _columnNames.size(); ++_col) {  // Current column index (0-based)
            QVariant _cellValue = _query.value(_col);  // Raw cell value from database
            QString _cellText = _cellValue.toString();  // Convert to string for display

            QTableWidgetItem *_item = new QTableWidgetItem(_cellText);  // Table cell item containing database cell data
            tableWidget->setItem(_rowIndex, _col, _item);
        }

        _rowIndex++;  // Move to next row
    }

    tableWidget->setRowCount(_rowIndex);

    qDebug() << "Loaded table" << tableName << "with" << _rowIndex << "rows";
    return true;
}

/**
 * @brief Add new row to specified table in database
 */
bool SQLWorker::AddRowToTable(const QString &tableName, const QStringList &rowData)
{
    if (!FileLoaded || tableName.isEmpty()) {
        qDebug() << "Error: Invalid parameters for adding row";
        return false;
    }

    // Get column information for the table
    QStringList _columnNames = GetTableColumns(tableName);  // List of column names for INSERT statement
    if (_columnNames.isEmpty()) {
        qDebug() << "Error: Could not retrieve column information for table" << tableName;
        return false;
    }

    // Prepare column names and values for INSERT statement
    QString _columns = _columnNames.join(", ");  // Comma-separated column names
    QStringList _placeholders;  // List of placeholder values for prepared statement

    for (int _i = 0; _i < _columnNames.size(); ++_i) {  // Generate placeholder for each column
        _placeholders.append("?");  // Use parameter placeholder for safe SQL execution
    }

    QString _values = _placeholders.join(", ");  // Comma-separated placeholder values

    // Create and execute INSERT query
    QSqlQuery _query(SqlDatabase);  // Query object for INSERT operation
    QString _queryString = INSERT_QUERY_TEMPLATE.arg(tableName, _columns, _values);  // Complete INSERT query string

    if (!_query.prepare(_queryString)) {
        qDebug() << "Error: Failed to prepare INSERT query:" << _queryString;
        qDebug() << "SQL error:" << _query.lastError().text();
        return false;
    }

    // Bind values to query parameters
    for (int _i = 0; _i < _columnNames.size(); ++_i) {  // Bind each column value
        QString _value = (_i < rowData.size()) ? rowData[_i] : "";  // Use provided value or empty string
        _query.addBindValue(_value);
    }

    // Execute the INSERT query
    if (!_query.exec()) {
        qDebug() << "Error: Failed to execute INSERT query";
        qDebug() << "SQL error:" << _query.lastError().text();
        return false;
    }

    qDebug() << "Added new row to table" << tableName;
    return true;
}

/**
 * @brief Delete specific row from table by index
 */
bool SQLWorker::DeleteRowFromTable(const QString &tableName, int rowIndex)
{
    if (!FileLoaded || tableName.isEmpty() || rowIndex < 0) {
        qDebug() << "Error: Invalid parameters for deleting row";
        return false;
    }

    // For SQL databases, we need to identify the row to delete
    // Since SQLite doesn't have built-in row numbers, we'll use ROWID
    QSqlQuery _query(SqlDatabase);  // Query object for DELETE operation
    QString _queryString = QString("DELETE FROM %1 WHERE ROWID = (SELECT ROWID FROM %1 LIMIT 1 OFFSET %2)")
                               .arg(tableName).arg(rowIndex);  // DELETE query using ROWID and OFFSET

    if (!_query.exec(_queryString)) {
        qDebug() << "Error: Failed to execute DELETE query:" << _queryString;
        qDebug() << "SQL error:" << _query.lastError().text();
        return false;
    }

    qDebug() << "Deleted row" << rowIndex << "from table" << tableName;
    return true;
}

/**
 * @brief Replace entire table with data from QTableWidget
 * @param tableName Name of the table to update (must exist in database)
 * @param tableWidget Pointer to QTableWidget containing the new data
 * @return true if table updated successfully, false on error
 */
bool SQLWorker::UpdateCompleteTable(const QString &tableName, QTableWidget *tableWidget)
{
    // Validate input parameters
    if (!FileLoaded || tableName.isEmpty() || !tableWidget) {
        qDebug() << "Error: Invalid parameters for updating table data";
        return false;
    }

    // Check if database connection is still valid
    if (!SqlDatabase.isOpen()) {
        qDebug() << "Error: Database connection is not open";
        return false;
    }

    // Start a transaction for atomic updates
    if (!SqlDatabase.transaction()) {
        qDebug() << "Error: Failed to start transaction";
        return false;
    }

    // Delete all existing rows from the table
    QSqlQuery _deleteQuery(SqlDatabase);  // Query object for DELETE operation
    QString _deleteQueryString = DELETE_ALL_QUERY.arg(tableName);  // Complete DELETE query string

    if (!_deleteQuery.exec(_deleteQueryString)) {
        qDebug() << "Error: Failed to delete existing data:" << _deleteQueryString;
        qDebug() << "SQL error:" << _deleteQuery.lastError().text();
        SqlDatabase.rollback();  // Rollback transaction on error
        return false;
    }

    // Get column names from the table widget headers
    QStringList _columnNames;  // List to store column header texts
    for (int _col = 0; _col < tableWidget->columnCount(); ++_col) {  // Current column index (0-based)
        QTableWidgetItem *_headerItem = tableWidget->horizontalHeaderItem(_col);  // Header item for current column
        if (_headerItem) {
            _columnNames.append(_headerItem->text());
        } else {
            // Get column names from database schema if no header exists
            QStringList _dbColumns = GetTableColumns(tableName);  // Database column names
            if (_col < _dbColumns.size()) {
                _columnNames.append(_dbColumns[_col]);
            } else {
                _columnNames.append(QString("Column_%1").arg(_col + 1));  // Generate default name
            }
        }
    }

    // Prepare INSERT query for new data
    QStringList _placeholders;  // List of placeholder values for prepared statement
    for (int _i = 0; _i < _columnNames.size(); ++_i) {  // Generate placeholder for each column
        _placeholders.append("?");
    }

    QString _columns = _columnNames.join(", ");  // Comma-separated column names
    QString _values = _placeholders.join(", ");  // Comma-separated placeholder values
    QString _insertQueryString = INSERT_QUERY_TEMPLATE.arg(tableName, _columns, _values);  // Complete INSERT query template

    QSqlQuery _insertQuery(SqlDatabase);  // Query object for INSERT operations
    if (!_insertQuery.prepare(_insertQueryString)) {
        qDebug() << "Error: Failed to prepare INSERT query:" << _insertQueryString;
        qDebug() << "SQL error:" << _insertQuery.lastError().text();
        SqlDatabase.rollback();  // Rollback transaction on error
        return false;
    }

    // Insert all rows from the table widget
    for (int _row = 0; _row < tableWidget->rowCount(); ++_row) {  // Current row index (0-based)
        // Clear previous bindings
        _insertQuery.finish();
        if (!_insertQuery.prepare(_insertQueryString)) {
            qDebug() << "Error: Failed to re-prepare INSERT query for row" << _row;
            SqlDatabase.rollback();
            return false;
        }

        // Bind data from each cell in the row
        for (int _col = 0; _col < tableWidget->columnCount(); ++_col) {  // Current column index (0-based)
            QTableWidgetItem *_cellItem = tableWidget->item(_row, _col);  // Cell item at current position
            QString _cellValue = _cellItem ? _cellItem->text() : "";  // Use empty string if cell is null
            _insertQuery.addBindValue(_cellValue);
        }

        // Execute the INSERT for this row
        if (!_insertQuery.exec()) {
            qDebug() << "Error: Failed to insert row" << _row;
            qDebug() << "SQL error:" << _insertQuery.lastError().text();
            SqlDatabase.rollback();  // Rollback transaction on error
            return false;
        }
    }

    // Commit the transaction
    if (!SqlDatabase.commit()) {
        qDebug() << "Error: Failed to commit transaction";
        SqlDatabase.rollback();
        return false;
    }

    qDebug() << "Updated table" << tableName << "with" << tableWidget->rowCount() << "rows";
    return true;
}

/**
 * @brief Save current database state (no-op for SQL as changes are immediate)
 */
bool SQLWorker::SaveSQLFile()
{
    // SQL databases automatically persist changes, so this is a no-op
    // Return true to maintain compatibility with XMLWorker interface
    if (!FileLoaded) {
        qDebug() << "Error: No database loaded for saving";
        return false;
    }

    qDebug() << "SQL database changes are automatically saved:" << CurrentFilePath;
    return true;
}

/**
 * @brief Get path of currently loaded database file
 */
QString SQLWorker::GetCurrentFilePath() const
{
    return CurrentFilePath;
}

/**
 * @brief Check if database file is currently loaded and connected
 */
bool SQLWorker::IsFileLoaded() const
{
    return FileLoaded && SqlDatabase.isOpen();
}

/**
 * @brief Parse loaded SQL database and extract table structure information
 */
void SQLWorker::ParseSQLStructure()
{
    AvailableTableNames.clear();

    // Query for all user tables (excluding system tables)
    QSqlQuery _query(GET_TABLES_QUERY, SqlDatabase);  // Query object for getting table names

    if (!_query.exec()) {
        qDebug() << "Error: Failed to query table names";
        qDebug() << "SQL error:" << _query.lastError().text();
        return;
    }

    // Extract table names from query results
    while (_query.next()) {  // Iterate through all returned table names
        QString _tableName = _query.value(0).toString();  // Table name from first column
        if (!_tableName.isEmpty()) {
            AvailableTableNames.append(_tableName);
        }
    }

    qDebug() << "Parsed SQL database structure, found tables:" << AvailableTableNames;
}

/**
 * @brief Get column information for specified table
 */
QStringList SQLWorker::GetTableColumns(const QString &tableName)
{
    QStringList _columns;  // List to store column names

    // Use PRAGMA table_info to get column information
    QSqlQuery _query(SqlDatabase);  // Query object for schema information
    QString _queryString = GET_COLUMNS_QUERY.arg(tableName);  // Complete PRAGMA query string

    if (!_query.exec(_queryString)) {
        qDebug() << "Error: Failed to get column information for table" << tableName;
        qDebug() << "SQL error:" << _query.lastError().text();
        return _columns;
    }

    // Extract column names from PRAGMA results
    while (_query.next()) {  // Iterate through all column information rows
        QString _columnName = _query.value(1).toString();  // Column name is in second field (index 1)
        _columns.append(_columnName);
    }

    return _columns;
}

/**
 * @brief Check if database connection is valid and accessible
 */
bool SQLWorker::ValidateDatabaseConnection()
{
    if (!SqlDatabase.isOpen()) {
        qDebug() << "Error: Database is not open";
        return false;
    }

    // Test the connection by executing a simple query
    QSqlQuery _testQuery("SELECT 1", SqlDatabase);  // Simple test query
    if (!_testQuery.exec()) {
        qDebug() << "Error: Database connection test failed";
        qDebug() << "SQL error:" << _testQuery.lastError().text();
        return false;
    }

    qDebug() << "Database connection validation passed";
    return true;
}

/**
 * @brief Generate unique connection name for this worker instance
 */
QString SQLWorker::GenerateConnectionName()
{
    // Use UUID to ensure unique connection names
    return QString("SQLWorker_Connection_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

/**
 * @brief Create backup of original table data for rollback functionality
 */
void SQLWorker::CreateTableBackup(const QString &tableName)
{
    // This method could be used to implement rollback functionality
    // For now, we rely on SQL transactions for atomicity
    Q_UNUSED(tableName)
    qDebug() << "Table backup functionality not implemented (using SQL transactions instead)";
}

/**
 * @brief Restore table from backup
 */
bool SQLWorker::RestoreTableFromBackup(const QString &tableName)
{
    // This method could be used to implement rollback functionality
    // For now, we rely on reloading data from the database
    Q_UNUSED(tableName)
    qDebug() << "Table restore functionality not implemented (reload data from database instead)";
    return false;
}
