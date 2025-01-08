#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>  
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Кроссплатформенная функция получения текущего времени в строковом формате с миллисекундами
void time_to_str(char* buffer, size_t size) {
    struct timeval tv;
    struct tm t;
    
#ifdef _WIN32
    SYSTEMTIME st;
    GetSystemTime(&st);

    int milliseconds = st.wMilliseconds;
    t.tm_year = st.wYear - 1900;
    t.tm_mon = st.wMonth - 1;
    t.tm_mday = st.wDay;
    t.tm_hour = st.wHour;
    t.tm_min = st.wMinute;
    t.tm_sec = st.wSecond;
    t.tm_isdst = -1;
#else
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &t);
    int milliseconds = tv.tv_usec / 1000;
#endif

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &t);
    snprintf(buffer + strlen(buffer), size - strlen(buffer), ".%03d", milliseconds);
}

// Функция логирования
void do_log(const char* data) {
    FILE* file = fopen("log.txt", "a");
    if (!file) {
        perror("Failed to open log file");
        return;
    }

    #ifdef _WIN32
    int pid = GetCurrentProcessId();
    #else
    int pid = getpid();
    #endif
    
    char time_buf[64];
    time_to_str(time_buf, sizeof(time_buf));

    // Форматируем и записываем в лог
    fprintf(file, "[%d] %s: %s\n", pid, time_buf, data);
    fflush(file);
    fclose(file);
}
