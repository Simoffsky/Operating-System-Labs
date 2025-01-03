#include "library.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#endif



/**
 * @brief Запускает указанную программу в фоновом режиме.
 * @param command Команда для выполнения (например, путь к исполняемому файлу или shell-команда).
 * @param wait Флаг ожидания завершения: 
 *             - 0: не ждать завершения процесса;
 *             - 1: ждать завершения процесса и вернуть код завершения.
 * @param exit_code Указатель для сохранения кода завершения дочернего процесса (используется только если wait = 1).
 *
 * @return Код ошибки: 
 *         -  0: успех;
 *         - -1: ошибка (например, неверная команда или ошибка выполнения).
 *
 */int start_process(const char *command, int wait, int *exit_code) {
    if (!command) {
        return -1; 
    }


#ifdef _WIN32
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    if (!CreateProcessA(NULL, (LPSTR)command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return -1; 
    }

    CloseHandle(pi.hThread);

    if (wait) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD status;
        if (!GetExitCodeProcess(pi.hProcess, &status)) {
            CloseHandle(pi.hProcess);
            return -1; 
        }
        *exit_code = (int)status;
        CloseHandle(pi.hProcess);
    }
    return 0;

#else
    pid_t pid = fork();
    if (pid < 0) {
        return -1; 
    } else if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        exit(errno); 
    } else {
        if (wait) {
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                return -1; 
            }
            if (WIFEXITED(status)) {
                *exit_code = WEXITSTATUS(status);
            } else {
                *exit_code = -1; 
            }
        }
    }
    return 0;
#endif
}
