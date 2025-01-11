#include "com.c"

int main() {
    void* portRead;
    void* portWrite;
#ifdef _WIN32
    const char* portReadName = "COM1"; 
    const char* portWriteName = "COM2"; 
#else
    const char* portReadName = "/dev/ttyS0";  
    const char* portWriteName = "/dev/ttyS1"; 
#endif 
    // Настройка COM портов
    portRead = setup_com_port(portReadName);
    if (portRead == NULL) {
        return 1;
    }

    portWrite = setup_com_port(portWriteName);
    if (portWrite == NULL) {
        return 1;
    }

    // Бесконечный цикл чтения и записи
    char buffer[128];
    const char* dataToSend = "Hello, COM!";
    while (1) {
        // Чтение данных
        read_com(portRead, buffer, sizeof(buffer));

        // Запись данных
        write_com(portWrite, dataToSend);

        sleep(1);  // Пауза между операциями
    }

    // Закрытие портов
    #ifdef _WIN32
        CloseHandle(portRead);
        CloseHandle(portWrite);
    #else
        close((int)(intptr_t)portRead);
        close((int)(intptr_t)portWrite);
    #endif

    return 0;
}
