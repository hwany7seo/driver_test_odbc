#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cas_cci.h>

//#define DB_HOST "192.168.2.33"
#define DB_HOST "localhost"
#define DB_PORT 33000
#define DB_NAME "demodb"
#define USER "dba"
#define PASS ""
#define TEST_COUNT 50000
#define BATCH_SIZE 50000

void checkError(int req, const char *msg) {
    if (req < 0) {
        printf("[ERROR] %s Error Code: %d\n", msg, req);
        exit(EXIT_FAILURE);
    }
    // else {
    //     printf("[Success] %s Code: %d\n", msg, req);
    // }
}

int commit(int conn, T_CCI_ERROR *error) {
    int ret = cci_end_tran(conn, CCI_TRAN_COMMIT, error);
    return ret;
}

int rollback(int conn, T_CCI_ERROR *error) {
    int ret = cci_end_tran(conn, CCI_TRAN_ROLLBACK, error);
    return ret;
}

int main() {
    int conn, req, error, data, ind;
    char query[1024];
    int id, rowCount = 0;
    char name[256];
    T_CCI_ERROR err_buf;
    char version[256];

    clock_t start, end;
    double elapsed_time;

    conn = cci_connect(DB_HOST, DB_PORT, DB_NAME, USER, PASS);
    checkError(conn, "Connecting to Database");
    printf("Connected successfully\n");

    req = cci_prepare_and_execute(conn, "DROP TABLE IF EXISTS test_table", 0, &error, &err_buf);
    checkError(req, "Preparing Drop Statement");

    req = cci_prepare(conn, "CREATE TABLE test_table (id INT, name VARCHAR(255))", 0, &err_buf);
    checkError(req, "Preparing Create Statement");
    error = cci_execute(req, 0, 0, &err_buf);
    checkError(error, "Executing Create Statement");

    // 데이터 삽입 시작 시간 측정
    start = clock();

    for (id = 1; id <= TEST_COUNT; id++) {
        snprintf(name, sizeof(name), "oripdo%d", id);
        snprintf(query, sizeof(query), "INSERT INTO test_table (id, name) VALUES (%d, '%s')", id, name);
        req = cci_prepare_and_execute(conn, query, 0, &error, &err_buf);
        if (req < 0) {
            printf("Insert execute error: %d, %s\n", err_buf.err_code, err_buf.err_msg);
            printf("Error inserting data at ID: %d\n", id);
            rollback(conn, &err_buf);
            exit(EXIT_FAILURE);
        }

        if (id % BATCH_SIZE == 0) {
            if (commit(conn, &err_buf) < 0) {
                goto handle_error;
            }
        }
    }

    if (commit(conn, &err_buf) < 0) {
        goto handle_error;
    }

    end = clock();
    elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("data inserted (elapsed time: %.9fs)\n", elapsed_time);

    // 데이터 조회 시작 시간 측정
    start = clock();

    req = cci_prepare(conn, "SELECT * FROM test_table", 0, &err_buf);
    checkError(req, "Preparing Select Statement");
    error = cci_execute(req, 0, 0, &err_buf);
    checkError(error, "Executing Select Statement");

    while (1) {
        error = cci_cursor(req, 1, CCI_CURSOR_CURRENT, &err_buf);
        if (error == CCI_ER_NO_MORE_DATA) {
            break;
        }
        if (error < 0) {
            printf("cursor error: %d, %s\n", err_buf.err_code, err_buf.err_msg);
            goto handle_error;
        }

        error = cci_fetch(req, &err_buf);
        if (error < 0) {
            printf("fetch error: %d, %s\n", err_buf.err_code, err_buf.err_msg);
            goto handle_error;
        }

        error = cci_get_data(req, 1, CCI_A_TYPE_INT, &data, &ind);
        if (error < 0) {
            printf("get data error: %d\n", error);
            goto handle_error;
        } else {
            rowCount++;
        }
    }

    end = clock();
    elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("data selected. rowCount: %d elapsed_time: %.9fs\n", rowCount, elapsed_time);

    // Closing the request handle
    error = cci_close_req_handle(req);
    if (error < 0) {
        printf("close_req_handle error: %d, %s\n", err_buf.err_code, err_buf.err_msg);
        goto handle_error;
    }

    // Disconnecting with the server
    error = cci_disconnect(conn, &err_buf);
    if (error < 0) {
        printf("error: %d, %s\n", err_buf.err_code, err_buf.err_msg);
        goto handle_error;
    }

    return 0;

handle_error:
    if (req > 0)
        cci_close_req_handle(req);
    if (conn > 0)
        cci_disconnect(conn, &err_buf);
    return 1;
}
