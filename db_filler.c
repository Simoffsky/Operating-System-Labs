#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <mysql.h>

#include "measurement.h"
#include "com.h"
#include "db.h"


// Соединение для БД
MYSQL *conn;

// handle_signal обрабатывает сигнал прерывания (ctrl + c)
void handle_signal(int sig) {
    printf("Shutdown...\n");
    if (conn != NULL ) 
        mysql_close(conn);
    
    exit(0);  
}

int main() {
    conn = configure_db_conn();
    signal(SIGINT, handle_signal);

#ifdef _WIN32
    HANDLE portRead;
    const char* portReadName = "COM1"; 
#else
    int portRead;
    const char* portReadName = "/dev/pts/6"; 
#endif 

    portRead = setup_com_port(portReadName);
    Measurement temperatures[NUM_MEASUREMENTS];
    int temp_index;
    char buffer[20];
    float temperature_from_port;
    char *endptr;
    time_t now;

    while(1) { 
        time(&now);
        read_com(portRead, buffer, 20);
        
        temperature_from_port = strtof(buffer, &endptr);
        temperatures[temp_index].value = temperature_from_port;
        temperatures[temp_index].timestamp = now;
        printf("temperature from com: %0.1f\n", temperature_from_port);

        // Настало время записывать данные в БД
        if (++temp_index == NUM_MEASUREMENTS) {
            insert_temperatures(conn, temperatures);
            temp_index = 0;
        }
 
        sleep(1); 
    }

    mysql_close(conn);

    return EXIT_SUCCESS;
}
