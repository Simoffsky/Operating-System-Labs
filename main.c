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
#define SEM_NAME_MASTER "/semaphore_master"
Semaphore *sem_master;

// Нужен для синхронизации записи в разделяемую память (потому что обновление счётчика должно быть атомарным).
#define SEM_NAME_COUNTER "/semaphore_counter"


// Замаппленный с разделяемой памятью счётчик, общий на все процессы.
SharedMemory *shared_memory;


char* program_path;

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

// Поток, обрабатывающий пользовательский ввод
void* input_thread(void* arg) {
    int new_value;
    while (1) {
        printf("Enter a new counter value: ");
        if (scanf("%d", &new_value) == 1) {
            set_counter(shared_memory, new_value);
            printf("Counter updated to %d\n", new_value);
        } else {
            printf("Invalid input. Please enter an integer.\n");
            while (getchar() != '\n');
        }
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

int create_copy(char* param) {
#ifdef _WIN32
    char program_path[MAX_PATH];
    if (GetModuleFileName(NULL, program_path, MAX_PATH) == 0) {
        printf("Failed to get the program path\n");
        exit(1);
    }

   
    char command_line[MAX_PATH + 256];
    snprintf(command_line, sizeof(command_line), "\"%s\" %s", program_path, param);


    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(
            NULL,             
            command_line,      
            NULL,              
            NULL,              
            FALSE,             
            0,                
            NULL,             
            NULL,              
            &si,              
            &pi  
    )) {
        printf("Failed to create process\n");
        exit(1);
    }

    return pi.dwProcessId;

#else
    int pid = fork();
    if (pid == 0) {
        char *new_argv[] = {program_path, param, NULL};
        execv(program_path, new_argv);
    } else if (pid < 1) {
        printf("Failed to create child process\n");
        exit(0);
    }
    return pid;
#endif // _WIN32
}


void* copies_thread(void* arg) {
    int copy_1_pid = 0;
    int copy_2_pid = 0;

    while(1) {
        if (copy_1_pid == 0) 
            copy_1_pid = create_copy("--copy1");
        else 
            do_log("Copy 1 is still active. Skipping launch.");
        
        if (copy_2_pid == 0) 
            copy_2_pid = create_copy("--copy2");
        else
            do_log("Copy 2 is still active. Skipping launch.");
    

#ifdef _WIN32
    Sleep(3000); 
#else
    usleep(3000 * 1000); 
#endif
    if (check_process_finished(copy_1_pid)) 
        copy_1_pid = 0;
    if (check_process_finished(copy_2_pid)) 
        copy_2_pid = 0;
    }
}

void handle_copy_1() {
    do_log("(Copy1) Started");
    shmem_wait_semaphore();
    shared_memory->counter += 10;
    shmem_signal_semaphore();

    do_log("(Copy1) Exit");
    exit(0);
}

void handle_copy_2() {
    do_log("(Copy2) Started");
    shmem_wait_semaphore();
    shared_memory->counter *= 2;
    shmem_signal_semaphore();


#ifdef _WIN32
    Sleep(2000); 
#else
    usleep(2000 * 1000); 
#endif
    
    shmem_wait_semaphore();
    shared_memory->counter /= 2;
    shmem_signal_semaphore();

    do_log("(Copy2) Exit");
    exit(0);
}


int main(int argc, char *argv[]) {
    shmem_init_semaphore();

    shared_memory = get_shared_memory(SHM_NAME);
    if (shared_memory == NULL) {
        perror("get_shared_memory");
        return 1;
    }

    // Дочерние копии
    if (argc > 1) {
        if (strcmp(argv[1], "--copy1") == 0) 
            handle_copy_1();
        if (strcmp(argv[1], "--copy2") == 0) 
            handle_copy_2();
    }

    program_path = argv[0];
    thread_t inc_thread, logger_thread, inp_thread, cop_thread;
    if (thread_create(&inc_thread, increment_thread, NULL) != 0) {
        perror("increment_thread");
        return 1;
    }

    if (thread_create(&inp_thread, input_thread, NULL) != 0) {
        perror("input_thread");
        return 1;
    }

    // Открываем семафор для того чтобы попытаться стать основным процессом который будет
    // писать в лог и порождать копии
    sem_master = malloc(sizeof(Semaphore));
    semaphore_init(sem_master, SEM_NAME_MASTER, 1);

    do_log("Started");
    // Если семафор свободный - становимся главным процессом.
    semaphore_wait(sem_master);
    do_log("I'm master thread");
    // Обработка сигнала прерывания SIGINT (Ctrl + C) для освобождения семафора.
    signal(SIGINT, handle_signal);
    set_zero_shared_memory(shared_memory);

    // Создаем поток для логирования
    if (thread_create(&logger_thread, log_thread, NULL) != 0) {
        perror("thread_create_log_thread");
        return 1;
    }

    //Создание копий fork()
    if (thread_create(&cop_thread, copies_thread, NULL) != 0) {
        perror("copies_thread");
        return 1;
    }


    // Ожидание завершения потоков
    thread_join(inc_thread);
    thread_join(inp_thread);
    thread_join(logger_thread);
    return 0;
}