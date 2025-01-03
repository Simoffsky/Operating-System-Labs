#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "log.c"
#include "sem.c"
#include "shared_mem.c"
#include "threads.c"

#ifdef _WIN32
#include <windows.h>
#endif

// Первый процесс который захватит этот семафор будет основным процессом и сможет порождать копии и писать в лог-файл, 
// остальные процессы будут просто увеличивать значение счётчика.
#define SEM_NAME_MASTER "/semaphore_masterr"
Semaphore *sem_master;

// Нужен для синхронизации записи в разделяемую память (обновление счётчика должно быть атомарным).
#define SEM_NAME_COUNTER "/semaphore_counter"


// Замаппленный с разделяемой памяти счётчик, общий на все процессы.
SharedMemory *shared_memory;


// Поток, увеличивающий счётчик каждые 300 мс
void* increment_thread(void* arg) { 

    while (1) {
        increment_counter(shared_memory);
#ifdef _WIN32
        Sleep(300); 
#else
        usleep(300 * 1000); 
#endif
    }
    return NULL;
}

// Функция для записи времени и значения счётчика в лог
void log_counter() {
    int counter_val = get_counter(shared_memory);
    char buf[64];

    snprintf(buf, sizeof(buf), "Counter value: %d", counter_val);
    do_log(buf);
}

// Поток, записывающий значение счётчика в лог каждую секунду
void* log_thread(void* arg) {
    while (1) {
        log_counter();
        #ifdef _WIN32
        Sleep(1000); 
#else
        sleep(1);
#endif
    }
    return NULL;
}



// handle_signal обрабатывает сигнал прерывания (ctrl + c)
void handle_signal(int sig) {
    printf("Shutdown...\n");
    if (sem_master != NULL) {
        semaphore_signal(sem_master);
        free(sem_master);
    }
        
    
    if (shared_memory != NULL) {
        destroy_shared_memory(shared_memory);
        shmem_destroy_semaphore();
    }
    exit(0);  
}

int main() {
    thread_t inc_thread, logger_thread;

    shmem_init_semaphore();

    // Создаем или получаем разделяемую память
    shared_memory = get_shared_memory(SHM_NAME);
    if (shared_memory == NULL) {
        perror("get_shared_memory");
        return 1;
    }


    // Создаем поток для инкремента
    if (thread_create(&inc_thread, increment_thread, NULL) != 0) {
        perror("increment_thread");
        return 1;
    }

    // Создаем или открываем семафор для того чтобы попытаться стать основным процессом который будет
    // писать в лог и порождать копии
    sem_master = malloc(sizeof(Semaphore));
    semaphore_init(sem_master, SEM_NAME_MASTER, 1);

    do_log("Started");
    // Если семафор свободный - становимся главным процессом.
    semaphore_wait(sem_master);
    // Обработка сигнала прерывания SIGINT (Ctrl + C) для освобождения семафора.
    signal(SIGINT, handle_signal);
    set_zero_shared_memory(shared_memory);

    // Создаем поток для логирования
    if (thread_create(&logger_thread, log_thread, NULL) != 0) {
        perror("thread_create_log_thread");
        return 1;
    }


    // Ожидание завершения потоков
    thread_join(inc_thread);
    thread_join(logger_thread);
    return 0;
}