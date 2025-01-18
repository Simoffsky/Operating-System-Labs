#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "measurement.h"


MYSQL *configure_db_conn() {

    MYSQL *conn;
    conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "mysql_init() failed\n");
        return NULL;
    }

    if (!mysql_real_connect(conn, "127.0.0.1", "simoff", "123", "temperature_db", 3306, NULL, 0)) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        return NULL;
    }
}

// Функция для вставки температур с временем
void insert_temperatures(MYSQL *conn, Measurement temperatures[NUM_MEASUREMENTS]) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];

    const char *query = "INSERT INTO temperature (timestamp, value) VALUES (?, ?)";

    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "Error initializing statement: %s\n", mysql_error(conn));
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Error preparing statement: %s\n", mysql_error(conn));
        mysql_stmt_close(stmt);
        return;
    }

    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&temperatures[0].timestamp;
    bind[0].is_null = 0;
    bind[0].length = 0;


    bind[1].buffer_type = MYSQL_TYPE_FLOAT;
    bind[1].buffer = (char *)&temperatures[0].value;
    bind[1].is_null = 0;
    bind[1].length = 0;

    for (int i = 0; i < NUM_MEASUREMENTS; i++) {
        bind[0].buffer = (char *)&temperatures[i].timestamp;
        bind[1].buffer = (char *)&temperatures[i].value;

        if (mysql_stmt_bind_param(stmt, bind)) {
            fprintf(stderr, "Error binding parameters: %s\n", mysql_error(conn));
            mysql_stmt_close(stmt);
            return;
        }

        if (mysql_stmt_execute(stmt)) {
            fprintf(stderr, "Error executing statement: %s\n", mysql_error(conn));
            mysql_stmt_close(stmt);
            return;
        }
        printf("Inserted temperature: %.1f at Unix timestamp %ld\n", temperatures[i].value, temperatures[i].timestamp);
    }

    mysql_stmt_close(stmt);
}


// Функция для извлечения температур из БД
Measurement *get_temperatures(MYSQL *conn, time_t start, time_t end, size_t *size, double *average) {
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[3];
    MYSQL_RES *prepare_meta_result;
    *size = 0;
    * average = 0;
    const char *query = "SELECT timestamp, value FROM temperature WHERE timestamp BETWEEN ? AND ?";
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "Error initializing statement: %s\n", mysql_error(conn));
        return NULL;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Error preparing statement: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }


    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&start;
    bind[0].is_null = 0;
    bind[0].length = 0;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (char *)&end;
    bind[1].is_null = 0;
    bind[1].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "Error binding parameters: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error executing statement: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }

    prepare_meta_result = mysql_stmt_result_metadata(stmt);
    if (!prepare_meta_result) {
        fprintf(stderr, "Error retrieving metadata: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }


    memset(bind, 0, sizeof(bind));

    time_t timestamp;
    float value;

    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].buffer = (char *)&timestamp;
    bind[0].is_null = 0;
    bind[0].length = 0;

    bind[1].buffer_type = MYSQL_TYPE_FLOAT;
    bind[1].buffer = (char *)&value;
    bind[1].is_null = 0;
    bind[1].length = 0;

    if (mysql_stmt_bind_result(stmt, bind)) {
        fprintf(stderr, "Error binding result: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return NULL;
    }

    Measurement *results = NULL;
    size_t count = 0;
    double sum = 0;


    while (!mysql_stmt_fetch(stmt)) {
        results = realloc(results, (count + 1) * sizeof(Measurement));
        if (!results) {
            fprintf(stderr, "Memory allocation error\n");
            mysql_stmt_close(stmt);
            return NULL;
        }

        results[count].timestamp = timestamp;
        results[count].value = value;
        sum += value;
        count++;
    }

    *size = count;
    *average = (count > 0) ? (sum / count) : 0.0;

    // Очистка
    mysql_free_result(prepare_meta_result);
    mysql_stmt_close(stmt);

    return results;
}



