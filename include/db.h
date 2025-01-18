#ifndef DB_H
#define DB_H

#include <mysql.h>
#include "measurement.h"

#define NUM_MEASUREMENTS 10

// Функция для настройки соединения с базой данных
MYSQL *configure_db_conn();

// Функция для вставки температуры с временем
void insert_temperatures(MYSQL *conn,  Measurement temperatures[NUM_MEASUREMENTS]);

// Функция для извлечения температур из БД
Measurement *get_temperatures(MYSQL *conn, time_t start, time_t end, size_t *size, double *average);
#endif // DB_H