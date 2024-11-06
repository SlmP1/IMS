#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <string>
#include <vector>

#define MAX_BUFFER 1024

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

    // ODBC connection string
    SQLCHAR connStr[] = "Driver={ODBC Driver 17 for SQL Server};Server=DESKTOP-UQ6MEMJ;Database=Inventory;Trusted_Connection=yes;";

    // Allocate an environment handle
    retCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    CheckSQLReturnCode(retCode, hEnv, SQL_HANDLE_ENV, "Error allocating environment handle.");

    // Set ODBC version
    retCode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    CheckSQLReturnCode(retCode, hEnv, SQL_HANDLE_ENV, "Error setting ODBC version.");

    // Allocate a connection handle
    retCode = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    CheckSQLReturnCode(retCode, hDbc, SQL_HANDLE_DBC, "Error allocating connection handle.");

    // Connect to the database
    retCode = SQLDriverConnectA(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    CheckSQLReturnCode(retCode, hDbc, SQL_HANDLE_DBC, "Error connecting to the database.");
}

// Function to insert a new product into the database
void InsertRecord(SQLHDBC& hDbc, const std::string& productName, const double price) {
    SQLHSTMT hStmt;
    SQLRETURN retCode;

    // Allocate a statement handle
    retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error allocating statement handle.");

    // SQL insert query
    SQLCHAR query[] = "INSERT INTO Products (ProductName, Price) VALUES (?, ?)";
    retCode = SQLPrepareA(hStmt, query, SQL_NTS);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error preparing insert statement.");

    // Bind the product name parameter
    retCode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)productName.c_str(), productName.length(), NULL);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error binding ProductName parameter.");

    // Bind the price parameter
    retCode = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, (SQLPOINTER)&price, 0, NULL);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error binding Price parameter.");

    // Execute the insert query
    retCode = SQLExecute(hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing insert query." << std::endl;
        CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error executing insert query.");
    }
    else {
        std::cout << "Product inserted successfully." << std::endl;
    }

    // Free statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
void ReadRecords(SQLHDBC& hDbc) {
    SQLHSTMT hStmt;
    SQLRETURN retCode;

    retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error allocating statement handle.");

    SQLCHAR query[] = "SELECT ProductID, ProductName, Price FROM Products";
    retCode = SQLExecDirectA(hStmt, query, SQL_NTS);
    CheckSQLReturnCode(retCode, hStmt, SQL_HANDLE_STMT, "Error executing SELECT query.");

    SQLINTEGER productID;
    SQLCHAR productName[MAX_BUFFER];
    SQLDOUBLE price;

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        SQLGetData(hStmt, 1, SQL_C_LONG, &productID, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_CHAR, productName, sizeof(productName), NULL);
        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &price, 0, NULL);

        std::cout << "Product ID: " << productID << ", Product Name: " << productName << ", Price: $" << price << std::endl;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

// Function to disconnect from the database and free resources
void DisconnectDatabase(SQLHENV& hEnv, SQLHDBC& hDbc) {
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

int main() {
    SQLHENV hEnv;
    SQLHDBC hDbc;

    // Connect to the database
    ConnectToDatabase(hEnv, hDbc);

    // Insert a new product
    InsertRecord(hDbc, "Washing Machine", -50.99);

    // Read - Display all products
    std::cout << "\nCurrent Products in Database:" << std::endl;
    ReadRecords(hDbc);

    // Disconnect from the database
    DisconnectDatabase(hEnv, hDbc);

    return 0;
}
