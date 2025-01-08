#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <signal.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


#ifdef _WIN32
typedef HANDLE thread_t;

int thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg) {
    *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start_routine, arg, 0, NULL);
    return *thread == NULL ? -1 : 0;
}

int thread_join(thread_t thread) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

void thread_exit() {
    ExitThread(0);
}

#else // POSIX
typedef pthread_t thread_t;
#include <sys/wait.h>
int thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg) {
    return pthread_create(thread, NULL, start_routine, arg);
}

int thread_join(thread_t thread) {
    return pthread_join(thread, NULL);
}

void thread_exit() {
    pthread_exit(NULL);
}

// check_process_finished возвращает 1 если процесс завершён, иначе 0
int check_process_finished(pid_t pid) {
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        return 1;  
    }

    DWORD exitCode;
    if (GetExitCodeProcess(hProcess, &exitCode)) {
        CloseHandle(hProcess);
        return (exitCode == STILL_ACTIVE) ? 0 : 1;  
    }
    CloseHandle(hProcess);
    return 1;  // В случае ошибки возвращаем 1
#else
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);

    if (result == 0) 
        return 0;
    if (result == pid) 
        return 1;

    perror("waitpid");
    return -1;

#endif // _WIN32
}
#endif // _WIN32

