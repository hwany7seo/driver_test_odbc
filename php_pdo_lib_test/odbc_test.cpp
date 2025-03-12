#define SQL_ODBC3
#include <windows.h>
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <chrono>

#define DSN "CUBRID Driver Unicode"  
#define USER "dba"
#define PASS ""
#define TEST_COUNT 100
#define BATCH_SIZE 50000

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
    std::cout << "Test Start" << std::endl;

    // ODBC 환경 설정
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    checkError(ret, hEnv, SQL_HANDLE_ENV, "Allocating Environment Handle");
    ret = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    checkError(ret, hEnv, SQL_HANDLE_ENV, "Setting ODBC Version");

    // ODBC 연결 설정
    ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    checkError(ret, hDbc, SQL_HANDLE_DBC, "Allocating Connection Handle");
    ret = SQLConnect(hDbc, (SQLCHAR*)DSN, SQL_NTS, (SQLCHAR*)USER, SQL_NTS, (SQLCHAR*)PASS, SQL_NTS);
    checkError(ret, hDbc, SQL_HANDLE_DBC, "Connecting to Database");
    std::cout << "Connected successfully" << std::endl;

    // 테이블 삭제 및 생성
    const char* dropTableSQL = "DROP TABLE IF EXISTS test_table";
    const char* createTableSQL = "CREATE TABLE test_table (id INT, name VARCHAR(255))";
    SQLExecDirect(hDbc, (SQLCHAR*)dropTableSQL, SQL_NTS);
    SQLExecDirect(hDbc, (SQLCHAR*)createTableSQL, SQL_NTS);
    std::cout << "Table created successfully" << std::endl;

    // 데이터 삽입
    auto start = std::chrono::high_resolution_clock::now();
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    checkError(ret, hStmt, SQL_HANDLE_STMT, "Allocating Statement Handle");

    const char* insertSQL = "INSERT INTO test_table (id, name) VALUES (%d, '%s')";
    SQLSetConnectAttr(hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);

    SQLINTEGER id;
    SQLCHAR name[256];
    for (id = 1; id <= TEST_COUNT; id++) {
        snprintf((char*)name, sizeof(name), "oripdo%d", id);
        char insertQuery[512];
        snprintf(insertQuery, sizeof(insertQuery), insertSQL, id, name);
        ret = SQLExecDirect(hStmt, (SQLCHAR*)insertQuery, SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cerr << "Error inserting data at ID: " << id << std::endl;
            SQLEndTran(SQL_HANDLE_DBC, hDbc, SQL_ROLLBACK);
            exit(EXIT_FAILURE);
        }
        // if (id % BATCH_SIZE == 0) {
        //     SQLTransact(hEnv, hDbc, SQL_COMMIT);
        // }
    }
    SQLTransact(hEnv, hDbc, SQL_COMMIT);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Data inserted in " << std::chrono::duration<double>(end - start).count() << "s" << std::endl;

    // 데이터 개수 확인
    SQLCHAR countSQL[] = "SELECT COUNT(*) FROM test_table";
    SQLExecDirect(hStmt, countSQL, SQL_NTS);
    SQLINTEGER count;
    SQLBindCol(hStmt, 1, SQL_C_LONG, &count, 0, NULL);
    SQLFetch(hStmt);
    std::cout << "Data count after insert: " << count << std::endl;

    // 데이터 조회
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

    // 정리 및 연결 해제
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    return 0;
}
