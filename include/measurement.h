#ifndef MEASURE_H
#define MEASURE_H

// Кол-во одновременно вставляемых строчек в БД.
#define NUM_MEASUREMENTS 10

// Структура для хранения температуры и времени 
typedef struct {
    float value;     
    time_t timestamp; // Unix timestamp
} Measurement;

#endif // MEASURE_H