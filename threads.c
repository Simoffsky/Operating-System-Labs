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

int thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg) {
    return pthread_create(thread, NULL, start_routine, arg);
}

int thread_join(thread_t thread) {
    return pthread_join(thread, NULL);
}

void thread_exit() {
    pthread_exit(NULL);
}
#endif

