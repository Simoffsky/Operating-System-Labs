#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep(x) Sleep(x * 1000)  // Для Windows используем Sleep() с миллисекундами
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <termios.h>
    #include <errno.h>
#endif

// Функция для настройки порта, зависит от операционной системы
void* setup_com_port(const char* port_name) {
    #ifdef _WIN32
        HANDLE hSerial;
        DCB dcbSerialParams = {0};
        COMMTIMEOUTS timeouts = {0};

        // Открытие порта
        hSerial = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
        if (hSerial == INVALID_HANDLE_VALUE) {
            printf("Error opening port %s\n", port_name);
            return NULL;
        }

        // Настройка параметров порта
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            printf("Error getting COM state for %s\n", port_name);
            CloseHandle(hSerial);
            return NULL;
        }
        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        if (!SetCommState(hSerial, &dcbSerialParams)) {
            printf("Error setting COM state for %s\n", port_name);
            CloseHandle(hSerial);
            return NULL;
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
            return NULL;
        }

        return hSerial;  // Возвращаем дескриптор порта

    #else
        int fd;
        struct termios options;

        // Открытие порта
        fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd == -1) {
            perror("Error opening port");
            return NULL;
        }

        // Настройка параметров порта
        if (tcgetattr(fd, &options) < 0) {
            perror("Error getting port settings");
            close(fd);
            return NULL;
        }
        cfsetispeed(&options, B9600);  // Устанавливаем скорость порта
        cfsetospeed(&options, B9600);  // Устанавливаем скорость порта

        options.c_cflag &= ~PARENB;    // Без четности
        options.c_cflag &= ~CSTOPB;    // Один стоповый бит
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;        // 8 бит данных
        options.c_cflag &= ~CRTSCTS;   // Без контроля потока
        options.c_cflag |= CREAD | CLOCAL;  // Разрешаем чтение и игнорируем модемный контроль

        // Настройка тайм-аутов
        options.c_cc[VMIN] = 1;       // Ожидание хотя бы одного байта
        options.c_cc[VTIME] = 5;      // Тайм-аут в 0.5 секунд

        if (tcsetattr(fd, TCSANOW, &options) < 0) {
            perror("Error setting port settings");
            close(fd);
            return NULL;
        }

        return (void*)fd;  // Возвращаем файловый дескриптор порта
    #endif
}

// Чтение из порта
int read_com(void* port, char* buffer, size_t bufferSize) {
    #ifdef _WIN32
        HANDLE hSerial = (HANDLE)port;
        DWORD bytesRead;
        if (!ReadFile(hSerial, buffer, bufferSize - 1, &bytesRead, NULL)) {
            printf("Error reading from COM port\n");
            return -1;
        }
        buffer[bytesRead] = '\0';  // Завершающий ноль для строки
        printf("Bytes Read: %d\n", bytesRead);
        return bytesRead;
    #else
        int fd = (int)(intptr_t)port;
        int bytesRead = read(fd, buffer, bufferSize - 1);
        if (bytesRead < 0) {
            perror("Error reading from serial port");
            return -1;
        }
        buffer[bytesRead] = '\0';  // Завершающий ноль для строки
        return bytesRead;
    #endif
}

// Запись в порт
int write_com(void* port, const char* data) {
    #ifdef _WIN32
        HANDLE hSerial = (HANDLE)port;
        DWORD bytesWritten;
        if (!WriteFile(hSerial, data, strlen(data), &bytesWritten, NULL)) {
            printf("Error writing to COM port\n");
            return -1;
        }
        printf("Data sent: %s\n", data);
        return bytesWritten;
    #else
        int fd = (int)(intptr_t)port;
        int bytesWritten = write(fd, data, strlen(data));
        if (bytesWritten < 0) {
            perror("Error writing to serial port");
            return -1;
        }
        printf("Data sent: %s\n", data);
        return bytesWritten;
    #endif
}

