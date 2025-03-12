#define SQL_ODBC3
#include <windows.h>
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <chrono>

#define DSN "CUBRID Driver Unicode"  
#define USER "dba"
#define PASS ""
#define TEST_COUNT 50000
#define BATCH_SIZE 100

void checkError(SQLRETURN ret, SQLHANDLE handle, SQLSMALLINT type, const std::string& msg) {
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLCHAR sqlState[6], msgText[256];
        SQLINTEGER nativeError;
        SQLSMALLINT msgLen;
        SQLGetDiagRec(type, handle, 1, sqlState, &nativeError, msgText, sizeof(msgText), &msgLen);
        std::cout << "[ERROR] " << msg << " SQLSTATE: " << sqlState << " MESSAGE: " << msgText << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main() {
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLHSTMT hStmt;
    SQLRETURN ret;
    SQLPOINTER processed;
    std::cout << "Test Start" << std::endl;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    checkError(ret, hEnv, SQL_HANDLE_ENV, "Allocating Environment Handle");
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    checkError(ret, hEnv, SQL_HANDLE_ENV, "Setting ODBC Version");

    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    checkError(ret, hDbc, SQL_HANDLE_DBC, "Allocating Connection Handle");
    ret = SQLConnect(hDbc, (SQLCHAR*)DSN, SQL_NTS, (SQLCHAR*)USER, SQL_NTS, (SQLCHAR*)PASS, SQL_NTS);
    checkError(ret, hDbc, SQL_HANDLE_DBC, "Connecting to Database");
    std::cout << "Connected successfully" << std::endl;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    const char* dropTableSQL = "DROP TABLE IF EXISTS test_table";
    const char* createTableSQL = "CREATE TABLE test_table (id INT, name VARCHAR(255))";
    ret = SQLExecDirect(hStmt, (SQLCHAR*)dropTableSQL, SQL_NTS);
    checkError(ret, hDbc, SQL_HANDLE_DBC, "Dropping Table");
    std::cout << "Table dropped successfully" << std::endl;
    ret = SQLExecDirect(hStmt, (SQLCHAR*)createTableSQL, SQL_NTS);
    checkError(ret, hDbc, SQL_HANDLE_DBC, "Creating Table");
    std::cout << "Table created successfully" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    checkError(ret, hStmt, SQL_HANDLE_STMT, "Allocating Statement Handle");

    SQLSetStmtAttr(hStmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)100, 0);
    SQLSetStmtAttr(hStmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &processed, 0);

    const char* insertSQL = "INSERT INTO test_table (id, name) VALUES (?, ?)";
    ret = SQLExecDirect(hStmt, (SQLCHAR*)insertSQL, SQL_NTS);
    std::cout << "SQLPrepare ret = " << ret << std::endl;
    checkError(ret, hStmt, SQL_HANDLE_STMT, "Preparing Insert Statement");

    SQLINTEGER id;
    SQLCHAR name[256];
    SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
    for (id = 1; id <= TEST_COUNT; id++) {
        snprintf((char*)name, sizeof(name), "oripdo%d", id);
        ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);
        //std::cout << "SQLBindParameter 1 ret = " << ret << std::endl;
        ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, name, 0, NULL);
        //std::cout << "SQLBindParameter 2 ret = " << ret << std::endl;
        std::cout << "SQLBindParameter 2 id = " << id << std::endl;
        if (id % BATCH_SIZE == 0) {
            std::cout << "SQLExecute SQLExecute = " << ret << std::endl;
            ret = SQLExecute(hStmt);
            std::cout << "SQLExecute ret = " << ret << std::endl;
        }
    }
    SQLTransact(hEnv, hDbc, SQL_COMMIT);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Data inserted in " << std::chrono::duration<double>(end - start).count() << "s" << std::endl;

    // SQLCHAR countSQL[] = "SELECT COUNT(*) FROM test_table";
    // SQLExecDirect(hStmt, countSQL, SQL_NTS);
    // SQLINTEGER count;
    // SQLBindCol(hStmt, 1, SQL_C_LONG, &count, 0, NULL);
    // SQLFetch(hStmt);
    // std::cout << "Data count after insert: " << count << std::endl;

    start = std::chrono::high_resolution_clock::now();
    SQLCHAR selectSQL[] = "SELECT * FROM test_table";
    SQLExecDirect(hStmt, selectSQL, SQL_NTS);
    SQLINTEGER rowCount = 0;
    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        rowCount++;
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Data selected. Row count: " << rowCount << " (Elapsed time: "
              << std::chrono::duration<double>(end - start).count() << "s)" << std::endl;

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    return 0;
}
