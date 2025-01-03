// Кроссплатформенная работа с семафорами

#ifdef _WIN32
#include <windows.h>
#else
#include <semaphore.h>
#endif

typedef struct {
#ifdef _WIN32
    HANDLE handle;
#else
    sem_t* handle;
#endif
} Semaphore;


// Инициализация семафора
int semaphore_init(Semaphore* sem, const char* name, unsigned int value);

// Уничтожение семафора
int semaphore_destroy(Semaphore* sem, const char* name);

// Ожидание семафора
int semaphore_wait(Semaphore* sem);

// Сигнал семафора
int semaphore_signal(Semaphore* sem);


#ifdef _WIN32

int semaphore_init(Semaphore* sem, const char* name, unsigned int value) {
    sem->handle = CreateSemaphoreA(NULL, value, LONG_MAX, name);
    if (sem->handle == NULL) {
        fprintf(stderr, "Failed to create semaphore: %ld\n", GetLastError());
        return -1;
    }
    return 0;
}

int semaphore_destroy(Semaphore* sem, const char* name) {
    if (!CloseHandle(sem->handle)) {
        fprintf(stderr, "Failed to close semaphore: %ld\n", GetLastError());
        return -1;
    }
    return 0;
}

int semaphore_wait(Semaphore* sem) {
    DWORD result = WaitForSingleObject(sem->handle, INFINITE);
    if (result != WAIT_OBJECT_0) {
        fprintf(stderr, "Failed to wait for semaphore: %ld\n", GetLastError());
        return -1;
    }
    return 0;
}

int semaphore_signal(Semaphore* sem) {
    if (!ReleaseSemaphore(sem->handle, 1, NULL)) {
        fprintf(stderr, "Failed to signal semaphore: %ld\n", GetLastError());
        return -1;
    }
    return 0;
}

#else // POSIX

#include <fcntl.h>
#include <errno.h>

int semaphore_init(Semaphore* sem, const char* name, unsigned int value) {
    sem->handle = sem_open(name, O_CREAT, 0644, value);
    if (sem->handle == SEM_FAILED) {
        perror("Failed to create semaphore");
        return -1;
    }
    return 0;
}

int semaphore_destroy(Semaphore* sem, const char* name) {
    if (sem_close(sem->handle) == -1) {
        perror("Failed to close semaphore");
        return -1;
    }
    if (sem_unlink(name) == -1) {
        perror("Failed to unlink semaphore");
        return -1;
    }
    return 0;
}

int semaphore_wait(Semaphore* sem) {
    if (sem_wait(sem->handle) == -1) {
        perror("Failed to wait for semaphore");
        return -1;
    }
    return 0;
}

int semaphore_signal(Semaphore* sem) {
    if (sem_post(sem->handle) == -1) {
        perror("Failed to signal semaphore");
        return -1;
    }
    return 0;
}

#endif