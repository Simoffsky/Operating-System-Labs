#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)  // Для Windows используем Sleep() с миллисекундами

HANDLE setup_com_port(const char* port_name);
int read_com(HANDLE hSerial, char* buffer, DWORD bufferSize);
int write_com(HANDLE hSerial, const char* data);

#else //UNIX
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

int setup_com_port(const char* port_name);
int read_com(int fd, char* buffer, size_t bufferSize);
int write_com(int fd, const char* data);
#endif // _WIN32

#ifdef _WIN32
#include <windows.h>

HANDLE setup_com_port(const char* port_name) {
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    // Открытие порта
    hSerial = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening port %s\n", port_name);
        return INVALID_HANDLE_VALUE;
    }

    // Настройка параметров порта
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("Error getting COM state for %s\n", port_name);
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("Error setting COM state for %s\n", port_name);
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    // Настройка тайм-аутов
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("Error setting timeouts for %s\n", port_name);
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    return hSerial;  // Возвращаем дескриптор порта
}

int read_com(HANDLE hSerial, char* buffer, DWORD bufferSize) {
    DWORD bytesRead;
    if (!ReadFile(hSerial, buffer, bufferSize - 1, &bytesRead, NULL)) {
        printf("Error reading from COM port\n");
        return -1;
    }
    buffer[bytesRead] = '\0';  // Завершающий ноль для строки
    printf("Bytes Read: %d\n", bytesRead);
    printf("Data received: %s\n", buffer);
    return bytesRead;
}

int write_com(HANDLE hSerial, const char* data) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data, strlen(data), &bytesWritten, NULL)) {
        printf("Error writing to COM port\n");
        return -1;
    }
    printf("Data sent: %s\n", data);
    return bytesWritten;
}
#else //Unix

int setup_com_port(const char* port_name) {
    int fd;
    struct termios options;

    fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Error opening port");
        return -1;
    }

    if (tcgetattr(fd, &options) < 0) {
        perror("Error getting port settings");
        close(fd);
        return -1;
    }
    cfsetispeed(&options, B9600);  

    options.c_cflag &= ~PARENB;    
    options.c_cflag &= ~CSTOPB;   
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;       
    options.c_cflag |= CREAD | CLOCAL;  
    options.c_cc[VMIN] = 1;      
    options.c_cc[VTIME] = 5;     

    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("Error setting port settings");
        close(fd);
        return -1;
    }

    return fd;  
}

// Чтение из порта
int read_com(int fd, char* buffer, size_t bufferSize) {
    int bytesRead = read(fd, buffer, bufferSize - 1);
    if (bytesRead < 0) {
        perror("Error reading from serial port");
        return -1;
    }
    buffer[bytesRead] = '\0'; 
    return bytesRead;
}

// Запись в порт
int write_com(int fd, const char* data) {
    int bytesWritten = write(fd, data, strlen(data));
    if (bytesWritten < 0) {
        perror("Error writing to serial port");
        return -1;
    }
    printf("Data sent: %s\n", data);
    return bytesWritten;
}

#endif //_WIN32