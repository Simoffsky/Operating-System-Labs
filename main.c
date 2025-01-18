#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "com.h"

#define DAY_IN_SECONDS 86400
#define YEAR_IN_SECONDS 31536000
#define DAY_IN_MINUTES 1440
#define MAX_MEASUREMENTS 2000

typedef struct {
    time_t timestamp;
    float temperature;
} Measurement;


// Функция для записи измерений в лог
void write_to_log(const char *filename, const Measurement *measurement) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return;
    }
    fprintf(file, "%ld %.2f\n", measurement->timestamp, measurement->temperature);
    fclose(file);
}

// Функция для очистки старых данных в логах (если данные старше указанного времени)
void cleanup_log(const char *filename, int retention_seconds) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // Нечего чистить.
        return;
    }

    Measurement measurements[MAX_MEASUREMENTS];
    int count = 0;
    
    while (fscanf(file, "%ld %f\n", &measurements[count].timestamp, &measurements[count].temperature) == 2) {
        if (time(NULL) - measurements[count].timestamp <= retention_seconds) {
            count++;
        }
    }
    fclose(file);

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Ошибка открытия файла для записи");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%ld %.2f\n", measurements[i].timestamp, measurements[i].temperature);
    }
    fclose(file);
}

// Функция для записи измерений и вычисления средней температуры
void record_temperature(float temperature) {
    time_t now;
    struct tm *timeinfo;
    
    time(&now);
    timeinfo = localtime(&now);

    Measurement measurement = {now, temperature};

    write_to_log("measurements.log", &measurement);
    cleanup_log("measurements.log", DAY_IN_SECONDS);

    // Логика для записи средней температуры за час
    static float hourly_temperatures[60] = {0};  
    static int hourly_index = 0;
    static int hourly_count = 0;
    static float hourly_sum = 0;
    
    hourly_sum += temperature;
    hourly_temperatures[hourly_index] = temperature;
    hourly_index = (hourly_index + 1) % 60;  // Цикличный индекс
    hourly_count++;

    if (hourly_count == 60) {
        float hourly_average = hourly_sum / 60;
        Measurement hourly_measurement = {now, hourly_average};
        write_to_log("hourly_average.log", &hourly_measurement);
        cleanup_log("hourly_average.log", 30 * DAY_IN_SECONDS); // 30 дней = месяц
        hourly_sum = 0;
        hourly_count = 0;
    }
    
    // Логика для записи средней температуры за день
    static float daily_temperatures[DAY_IN_MINUTES] = {0}; 
    static int daily_index = 0;
    static int daily_count = 0;
    static float daily_sum = 0;

    daily_sum += temperature;
    daily_temperatures[daily_index] = temperature;
    daily_index = (daily_index + 1) % DAY_IN_MINUTES;  // Цикличный индекс
    daily_count++;

    if (daily_count == DAY_IN_MINUTES) {
        float daily_average = daily_sum / DAY_IN_MINUTES;
        Measurement daily_measurement = {now, daily_average};
        write_to_log("daily_average.log", &daily_measurement);
        cleanup_log("daily_average.log", YEAR_IN_SECONDS);
        daily_sum = 0;
        daily_count = 0;
    }

}

int main() {
#ifdef _WIN32
    HANDLE portRead;
    const char* portReadName = "COM1"; 
#else
    int portRead;
    const char* portReadName = "/dev/pts/6"; 
#endif 
    portRead = setup_com_port(portReadName);
    srand(time(NULL));
    char buffer[20];
    float temperature;
    char *endptr;
    while (1) {
        read_com(portRead, buffer, 20);
        temperature = strtof(buffer, &endptr);
        printf("temp: %f\n", temperature);
        record_temperature(temperature);
        sleep(60); 
    }
    return 0;
}