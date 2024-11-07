#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <string>
#include <vector>

// Define buffer size
#define MAX_BUFFER 1024

// Structure to hold column data
struct ColumnData {
    std::string columnName;
    std::string value;
};

// Function to check SQL return codes and print errors
void CheckSQLReturnCode(SQLRETURN retCode, SQLHANDLE handle, SQLSMALLINT handleType, const char* msg) {
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        SQLCHAR sqlState[6], message[MAX_BUFFER];
        SQLINTEGER nativeError;
        SQLSMALLINT msgLen;
        SQLGetDiagRecA(handleType, handle, 1, sqlState, &nativeError, message, sizeof(message), &msgLen);
        std::cerr << msg << "\nSQLSTATE: " << sqlState << "\nError: " << message << std::endl;
        exit(1);
    }
}

// Function to connect to the database
void ConnectToDatabase(SQLHENV& hEnv, SQLHDBC& hDbc) {
    SQLRETURN retCode;
    SQLCHAR connStr[] = "Driver={ODBC Driver 17 for SQL Server};Server=DESKTOP-UQ6MEMJ;Database=Inventory;Trusted_Connection=yes;";

    retCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    CheckSQLReturnCode(retCode, hEnv, SQL_HANDLE_ENV, "Error allocating environment handle.");

    retCode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    CheckSQLReturnCode(retCode, hEnv, SQL_HANDLE_ENV, "Error setting ODBC version.");

    retCode = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    CheckSQLReturnCode(retCode, hDbc, SQL_HANDLE_DBC, "Error allocating connection handle.");

    retCode = SQLDriverConnectA(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    CheckSQLReturnCode(retCode, hDbc, SQL_HANDLE_DBC, "Error connecting to the database.");
}

// Generic Insert Function
void InsertRecord(SQLHDBC& hDbc, const std::string& tableName, const std::vector<ColumnData>& columns) {
    SQLHSTMT hStmt;
    SQLRETURN retCode;

    retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error allocating statement handle.");

    std::string query = "INSERT INTO " + tableName + " (";
    for (size_t i = 0; i < columns.size(); ++i) {
        query += columns[i].columnName;
        if (i < columns.size() - 1) query += ", ";
    }
    query += ") VALUES (";
    for (size_t i = 0; i < columns.size(); ++i) {
        query += "?";
        if (i < columns.size() - 1) query += ", ";
    }
    query += ")";

    retCode = SQLPrepareA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error preparing insert statement.");

    for (size_t i = 0; i < columns.size(); ++i) {
        retCode = SQLBindParameter(hStmt, i + 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)columns[i].value.c_str(), columns[i].value.length(), NULL);
        CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error binding parameter.");
    }

    retCode = SQLExecute(hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing insert query for table " << tableName << "." << std::endl;
        CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error executing insert query.");
    }
    else {
        std::cout << "Record inserted successfully into " << tableName << " table." << std::endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// Generic Update Function
void UpdateRecord(SQLHDBC& hDbc, const std::string& tableName, const std::vector<ColumnData>& columns, const std::string& condition) {
    SQLHSTMT hStmt;
    SQLRETURN retCode;

    retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error allocating statement handle.");

    std::string query = "UPDATE " + tableName + " SET ";
    for (size_t i = 0; i < columns.size(); ++i) {
        query += columns[i].columnName + " = ?";
        if (i < columns.size() - 1) query += ", ";
    }
    query += " WHERE " + condition;

    retCode = SQLPrepareA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error preparing update statement.");

    for (size_t i = 0; i < columns.size(); ++i) {
        retCode = SQLBindParameter(hStmt, i + 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)columns[i].value.c_str(), columns[i].value.length(), NULL);
        CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error binding parameter.");
    }

    retCode = SQLExecute(hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing update query for table " << tableName << "." << std::endl;
        CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error executing update query.");
    }
    else {
        std::cout << "Record updated successfully in " << tableName << " table." << std::endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// Generic Delete Function
void DeleteRecord(SQLHDBC& hDbc, const std::string& tableName, const std::string& condition) {
    SQLHSTMT hStmt;
    SQLRETURN retCode;

    retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error allocating statement handle.");

    std::string query = "DELETE FROM " + tableName + " WHERE " + condition;

    retCode = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing delete query for table " << tableName << "." << std::endl;
        CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error executing delete query.");
    }
    else {
        std::cout << "Record deleted successfully from " << tableName << " table." << std::endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// Generic Select Function
void SelectRecords(SQLHDBC& hDbc, const std::string& tableName, const std::vector<std::string>& columns, const std::string& condition) {
    SQLHSTMT hStmt;
    SQLRETURN retCode;

    retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error allocating statement handle.");

    std::string query = "SELECT ";
    for (size_t i = 0; i < columns.size(); ++i) {
        query += columns[i];
        if (i < columns.size() - 1) query += ", ";
    }
    query += " FROM " + tableName;
    if (!condition.empty()) query += " WHERE " + condition;

    retCode = SQLExecDirectA(hStmt, (SQLCHAR*)query.c_str(), SQL_NTS);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error executing select query.");

    SQLCHAR buffer[MAX_BUFFER];
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        for (size_t i = 0; i < columns.size(); ++i) {
            SQLGetData(hStmt, i + 1, SQL_C_CHAR, buffer, MAX_BUFFER, NULL);
            std::cout << columns[i] << ": " << buffer << "\t";
        }
        std::cout << std::endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// Function to disconnect from the database
void DisconnectDatabase(SQLHENV& hEnv, SQLHDBC& hDbc) {
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

int main() {
    SQLHENV hEnv;
    SQLHDBC hDbc;

    // Connect to the database
    ConnectToDatabase(hEnv, hDbc);

    // Example usage for Inventory table
    // Insert a new record into Inventory table
    std::vector<ColumnData> columns = { {"SupplierName", "Vazha"}, {"ContactInfo", "Vazhako123"}, {"Address", "Landiastr"} };
    InsertRecord(hDbc, "Suppliers", columns);  // Ensure column names are correct for your DB schema

    //Select and display the updated records from Inventory table
    SelectRecords(hDbc, "Suppliers", {"SupplierID", "SupplierName", "ContactInfo", "Address" }, "1=1");

    // Delete the record for 'Table' from Inventory table
    DeleteRecord(hDbc, "Suppliers", "SupplierID= '4'");
    
    // Update the quantity of the 'Table' item in Inventory table
    columns = { {"SupplierName", "Mamuka"} };
    UpdateRecord(hDbc, "Suppliers", columns, "SupplierName = 'Vazha'");

    // Disconnect from the database
    DisconnectDatabase(hEnv, hDbc);

    return 0;
}




