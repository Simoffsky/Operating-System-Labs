#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Кроссплатформенная функция получения текущего времени в строковом формате
void time_to_str(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm t;
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    localtime_r(&now, &t);
#endif
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &t);
}

// Функция логирования
void do_log(const char* data) {
    FILE* file = fopen("log.txt", "a");
    if (!file) {
        perror("Failed to open log file");
        return;
    }

    #ifdef _WIN32
    pid_t pid = GetCurrentProcessId();
    #else
    pid_t pid = getpid();
    #endif
    
    char time_buf[64];
    time_to_str(time_buf, sizeof(time_buf));

    // Форматируем и записываем в лог
    fprintf(file, "[%d] %s: %s\n", pid, time_buf, data);

    fclose(file);
}
