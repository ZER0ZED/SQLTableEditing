#ifndef SQLWORKER_H
#define SQLWORKER_H

#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMap>
#include <QDebug>
#include <QVariant>

/**
 * @brief Worker class for SQL database file operations
 * Handles all SQL parsing, table manipulation, and database I/O operations
 */
class SQLWorker
{
public:
    /**
     * @brief Constructor for SQLWorker
     */
    SQLWorker();

    /**
     * @brief Destructor for SQLWorker
     */
    ~SQLWorker();

    /**
     * @brief Load SQL database file and parse its structure
     * @param filePath Path to the SQL database file to load
     * @return true if file loaded successfully, false otherwise
     */
    bool LoadSQLFile(const QString &filePath);

    /**
     * @brief Get list of available table names from loaded database
     * @return QStringList containing all table names
     */
    QStringList GetTableNames() const;

    /**
     * @brief Load specific table data into QTableWidget
     * @param tableName Name of the table to load
     * @param tableWidget Target QTableWidget to populate
     * @return true if table loaded successfully, false otherwise
     */
    bool LoadTableData(const QString &tableName, QTableWidget *tableWidget);

    /**
     * @brief Add new row to specified table
     * @param tableName Name of the table to modify
     * @param rowData QStringList containing cell values for new row
     * @return true if row added successfully, false otherwise
     */
    bool AddRowToTable(const QString &tableName, const QStringList &rowData);

    /**
     * @brief Delete specific row from table
     * @param tableName Name of the table to modify
     * @param rowIndex Index of the row to delete (0-based)
     * @return true if row deleted successfully, false otherwise
     */
    bool DeleteRowFromTable(const QString &tableName, int rowIndex);

    /**
     * @brief Update entire table with new data
     * @param tableName Name of the table to replace
     * @param tableWidget Source QTableWidget containing new data
     * @return true if table updated successfully, false otherwise
     */
    bool UpdateCompleteTable(const QString &tableName, QTableWidget *tableWidget);

    /**
     * @brief Save all changes back to the SQL database file (no-op for SQL as changes are immediate)
     * @return true always (SQL changes are committed immediately)
     */
    bool SaveSQLFile();

    /**
     * @brief Get current loaded file path
     * @return QString containing the file path
     */
    QString GetCurrentFilePath() const;

    /**
     * @brief Check if SQL database file is currently loaded
     * @return true if file is loaded, false otherwise
     */
    bool IsFileLoaded() const;

private:
    /**
     * @brief Parse SQL database and extract table structure
     */
    void ParseSQLStructure();

    /**
     * @brief Get column information for specified table
     * @param tableName Name of the table
     * @return QStringList containing column names
     */
    QStringList GetTableColumns(const QString &tableName);

    /**
     * @brief Check if database connection is valid
     * @return true if connection is valid, false otherwise
     */
    bool ValidateDatabaseConnection();

    /**
     * @brief Generate unique connection name for this worker instance
     * @return QString containing unique connection name
     */
    QString GenerateConnectionName();

    /**
     * @brief Create backup of original table data for rollback functionality
     * @param tableName Name of the table to backup
     */
    void CreateTableBackup(const QString &tableName);

    /**
     * @brief Restore table from backup
     * @param tableName Name of the table to restore
     * @return true if restore successful, false otherwise
     */
    bool RestoreTableFromBackup(const QString &tableName);

    // Global variables for SQL operations
    QString CurrentFilePath;             // Path to currently loaded SQL database file (empty if no file loaded)
    QSqlDatabase SqlDatabase;            // Database connection object (invalid if no file loaded)
    QStringList AvailableTableNames;     // List of table names in current database (empty if no tables found)
    bool FileLoaded;                     // Flag indicating if database file is loaded (true) or not loaded (false)
    QString ConnectionName;              // Unique connection name for this worker instance
    QMap<QString, QStringList> TableBackups;  // Backup storage for table data (table name -> serialized data)

    // SQL query constants
    static const QString GET_TABLES_QUERY;        // Query to get all table names
    static const QString GET_COLUMNS_QUERY;       // Query template to get column information
    static const QString SELECT_ALL_QUERY;        // Query template to select all data from table
    static const QString DELETE_ALL_QUERY;        // Query template to delete all data from table
    static const QString INSERT_QUERY_TEMPLATE;   // Query template for inserting rows
};

#endif // SQLWORKER_H
