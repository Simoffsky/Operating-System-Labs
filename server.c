#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <mysql.h>
#include "db.h"

#define PORT 8080

// Соединение для БД
MYSQL *conn;

int server_fd;

// Функция для формирования ответа с массивом структур
char* calculate_range(Measurement* measurements, size_t size, float* average_out) {
    printf("calculate_range start\n");
    if (size <= 0) {
        char* empty_response = malloc(32);
        strcpy(empty_response, "{\"error\": \"Empty data\"}");
        return empty_response;
    }

    float sum = 0.0;
    size_t response_size = 64; 
    char* response = malloc(response_size);
    if (!response) return NULL;

    strcpy(response, "{\"measurements\": [");

    for (int i = 0; i < size; i++) {
        char entry[128];
        sprintf(entry, "{\"value\": %.2f, \"timestamp\": %ld}", 
                measurements[i].value, measurements[i].timestamp);

        size_t new_size = strlen(response) + strlen(entry) + 3;
        if (new_size > response_size) {
            response_size = new_size + 64;
            response = realloc(response, response_size);
            if (!response) return NULL;
        }

        strcat(response, entry);
        if (i < size - 1) strcat(response, ",");
        sum += measurements[i].value;
    }

    strcat(response, "]");
    *average_out = sum / size;

    char average_part[64];
    sprintf(average_part, ", \"average\": %.2f}", *average_out);

    response = realloc(response, strlen(response) + strlen(average_part) + 1);
    if (!response) return NULL;

    strcat(response, average_part);
    printf("calculate_range end\n");
    return response;
}

// Обработчик HTTP-запросов
void handle_request(int client_socket) {
    char buffer[1024];
    recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (strncmp(buffer, "GET /data?", 10) != 0) {
        const char *error_response = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "\r\nInvalid Endpoint";
        send(client_socket, error_response, strlen(error_response), 0);
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }

    time_t start = 0, end = 0;
    char *start_pos = strstr(buffer, "start=");
    char *end_pos = strstr(buffer, "end=");
    if (start_pos && end_pos) {
        start = (time_t)strtoull(start_pos + 6, NULL, 10);
        end = (time_t)strtoull(end_pos + 4, NULL, 10);
    } else {
        const char *error_response = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "\r\nMissing parameters";
        send(client_socket, error_response, strlen(error_response), 0);
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }
    

    Measurement* measurements;
    size_t size = 0;
    double average = 0;
    measurements = get_temperatures(conn, start, end, &size, &average);


    char* response_body = calculate_range(measurements, size, &average);
    free(measurements);

    if (!response_body) {
        const char *error_response = 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "\r\nMemory allocation failed";
        send(client_socket, error_response, strlen(error_response), 0);
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif  
        return;
    }

    char http_header[256];
    sprintf(http_header, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: %zu\r\n\r\n", 
            strlen(response_body));

    send(client_socket, http_header, strlen(http_header), 0);
    send(client_socket, response_body, strlen(response_body), 0);

    free(response_body);

#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

// handle_signal обрабатывает сигнал прерывания (ctrl + c)
void handle_signal(int sig) {
    printf("Shutdown...\n");
    if (conn != NULL ) 
        mysql_close(conn);
    
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif

    exit(0);  
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
#endif

    conn = configure_db_conn();
    signal(SIGINT, handle_signal);

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
#ifdef _WIN32
        closesocket(server_fd);
        WSACleanup();
#else
        close(server_fd);
#endif
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
#ifdef _WIN32
        closesocket(server_fd);
        WSACleanup();
#else
        close(server_fd);
#endif
        exit(EXIT_FAILURE);
    }
    printf("HTTP server is running on port %d...\n", PORT);

    while (1) {
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Client accept failed");
            continue;
        }
        handle_request(client_socket);
    }


    return 0;
}
