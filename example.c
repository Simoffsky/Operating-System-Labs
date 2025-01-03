#include "library.h"
#include <stdio.h>

int main() {
    int exit_code;

    // Пример: запуск без ожидания
    printf("Процесс без ожидания...\n");
    if (start_process("sleep 5", 0, NULL) == 0) {
        printf("Процесс запущен в фоновом режиме.\n");
    } else {
        printf("Ошибка запуска процесса.\n");
    }

    // Пример: ожидание завершения
    printf("Процесс с ожиданием...\n");
    if (start_process("sleep 3", 1, &exit_code) == 0) {
        printf("Процесс с ожиданием завершился с кодом: %d\n", exit_code);
    } else {
        printf("Ошибка запуска процесса.\n");
    }
    return 0;
}
