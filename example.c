#include "library.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    
    // Чиним кодировку консольного вывода для винды и указываем подпрограмму для запуска (sleep\timeout)
    #ifdef _WIN32
    const char* cmd_name = "cmd /c timeout /t 5 >nul 2>&1";
    SetConsoleOutputCP(CP_UTF8);
    #else
    const char* cmd_name = "sleep 5";
    #endif


    int exit_code;
    // Пример: запуск без ожидания, запускается sleep на 5 секунд
    printf("Процесс без ожидания...\n");
    if (start_process(cmd_name, 0, NULL) == 0) {
        printf("Процесс запущен в фоновом режиме.\n");
    } else {
        printf("Ошибка запуска процесса.\n");
    }

    // Пример: ожидание завершения, запускается sleep на 5 секунд
    printf("Процесс с ожиданием...\n");
    if (start_process(cmd_name, 1, &exit_code) == 0) {
        printf("Процесс с ожиданием завершился с кодом: %d\n", exit_code);
    } else {
        printf("Ошибка запуска процесса.\n");
    }
    return 0;
}
