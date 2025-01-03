#include <stdio.h>
#include <stdlib.h>



#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#endif

// Структура для абстракции разделяемой памяти
typedef struct {
    int counter;
} SharedMemory;
#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE sizeof(SharedMemory)

void destroy_shared_memory(SharedMemory* memory);
SharedMemory* get_shared_memory(const char* name);
int get_counter(SharedMemory* memory);
void increment_counter(SharedMemory* memory);

void shmem_init_semaphore();
void shmem_destroy_semaphore();
void shmem_wait_semaphore();
void shmem_signal_semaphore();

#ifdef _WIN32

HANDLE hSemaphore; 



void destroy_shared_memory(SharedMemory* memory) {
    UnmapViewOfFile(memory);
}

SharedMemory* get_shared_memory(const char* name) {
    SharedMemory* memory = (SharedMemory*)MapViewOfFile(
        OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name),
        FILE_MAP_ALL_ACCESS,
        0, 0, sizeof(SharedMemory)
    );
    if (memory == NULL) {
        perror("Failed to access shared memory");
        return NULL;
    }
    return memory;
}



void shmem_init_semaphore() {
    hSemaphore = CreateSemaphoreA(NULL, 1, 1, NULL);
    if (hSemaphore == NULL) {
        perror("Failed to create semaphore");
        exit(1);
    }
}

void shmem_destroy_semaphore() {
    CloseHandle(hSemaphore);
}

void shmem_wait_semaphore() {
    WaitForSingleObject(hSemaphore, INFINITE);
}

void shmem_signal_semaphore() {
    ReleaseSemaphore(hSemaphore, 1, NULL);
}

#else //--- POSIX ---
#include <semaphore.h>
#include <sys/mman.h>
sem_t* sem;  // Семафор для синхронизации


void destroy_shared_memory(SharedMemory* memory) {
    if (munmap(memory, SHM_SIZE) == -1) {
        perror("Failed to unmap shared memory");
    }
}

SharedMemory* get_shared_memory(const char* name) {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }
    return mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
}


void shmem_init_semaphore() {
    sem = sem_open("/shared_memory_semaphore", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("Failed to create semaphore");
        exit(1);
    }
}

void shmem_destroy_semaphore() {
    sem_close(sem);
    sem_unlink("/shared_memory_semaphore");
}

void shmem_wait_semaphore() {
    sem_wait(sem);
}

void shmem_signal_semaphore() {
    sem_post(sem);
}

#endif //--- POSIX END ---


int get_counter(SharedMemory* memory) {
    shmem_wait_semaphore();
    int counter_value = memory->counter;
    shmem_signal_semaphore();

    return counter_value;
}

void increment_counter(SharedMemory* memory) {
    shmem_wait_semaphore();
    memory->counter++;
    shmem_signal_semaphore();
}

void set_zero_shared_memory(SharedMemory* memory) {
    shmem_wait_semaphore();

    memset(memory, 0, sizeof(SharedMemory));

    shmem_signal_semaphore();
}