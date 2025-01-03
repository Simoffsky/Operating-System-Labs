#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include "log.c"

#define SHM_NAME "/my_shared_memory"
#define SHM_SIZE sizeof(int)

// Первый процесс который захватит этот семафор будет основным процессом и сможет порождать копии и писать в лог-файл, 
// остальные процессы будут просто увеличивать значение счётчика.
#define SEM_NAME_MASTER "/semaphore_marrrr"
sem_t *sem_master;

// Нужен для синхронизации записи в разделяемую память (обновление счётчика должно быть атомарным).
#define SEM_NAME_COUNTER "/semaphore_counter"


// Замаппленный с разделяемой памяти счётчик, общий на все процессы.
int *shared_counter;


// Поток, увеличивающий счётчик каждые 300 мс
void* increment_thread(void* arg) { 
    // Создаем или открываем семафор для атомарного обновления счётчика
    sem_t *sem = sem_open(SEM_NAME_COUNTER, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    while (1) {
        sem_wait(sem);
        (*shared_counter) += 1;
        sem_post(sem);
        usleep(300 * 1000); // 300 мс
    }
    return NULL;
}

// Функция для записи времени и значения счётчика в лог
void log_counter(sem_t* sem) {
    sem_wait(sem);
    int counter_val = *shared_counter;
    sem_post(sem);

    char buf[64];

    snprintf(buf, sizeof(buf), "Counter value: %d", counter_val);
    do_log(buf);
}

// Поток, записывающий значение счётчика в лог каждую секунду
void* log_thread(void* arg) {
    // Создаем или открываем семафор для атомарного обновления счётчика
    sem_t *sem = sem_open(SEM_NAME_COUNTER, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    while (1) {
        log_counter(sem);
        sleep(1); // 1 секунда
    }
    return NULL;
}



// handle_signal освобождает семафор если получен сигнал прерывания
void handle_signal(int sig) {
    if (sem_master != NULL) {
        sem_post(sem_master);
        sem_close(sem_master);  
        sem_unlink(SEM_NAME_MASTER);
        printf("master semaphore closed\n");
    }
    exit(0);  
}

int main() {
    pthread_t inc_thread, logger_thread;


    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }


    shared_counter = (int *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_counter == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    if (pthread_create(&inc_thread, NULL, increment_thread, NULL) != 0) {
        perror("increment_thread");
        return 1;
    }

    // Создаем или открываем семафор для того чтобы попытаться стать основным процессом который будет
    // писать в лог и порождать копии
    sem_master = sem_open(SEM_NAME_MASTER, O_CREAT, 0666, 1);
    if (sem_master == SEM_FAILED) {
        perror("sem_open_master");
        exit(1);
    }

    do_log("Started");
    // Если семафор свободный - становимся главным процессом.
    sem_wait(sem_master);

    // Обработка сигнала прерывания SIGINT (Ctrl + C) для освобождения семафора.
    signal(SIGINT, handle_signal);

    memset(shared_counter, 0, SHM_SIZE);

    // Создание потоков
    if (pthread_create(&logger_thread, NULL, log_thread, NULL) != 0) {
        perror("Error creating logger thread");
        return 1;
    }


    // // Ожидание завершения потоков (здесь бесконечный цикл)
    pthread_join(inc_thread, NULL);
    pthread_join(logger_thread, NULL);
    munmap(shared_counter, SHM_SIZE);
    return 0;
}