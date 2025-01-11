#include "com.c"

int main() {
#ifdef _WIN32
    HANDLE portRead;
    HANDLE portWrite;
    const char* portReadName = "COM1"; 
    const char* portWriteName = "COM2"; 
#else
    int portRead;
    int portWrite;
    const char* portReadName = "/dev/pts/7";  
    const char* portWriteName = "/dev/pts/8"; 
#endif 
    // Настройка COM портов
    portRead = setup_com_port(portReadName);
    if (portRead == -1) {
        return 1;
    }

    portWrite = setup_com_port(portWriteName);
    if (portWrite == -1) {
        return 1;
    }

    char buffer[128];
    const char* dataToSend = "Hello, COM!";
    while (1) {

        write_com(portWrite, dataToSend);

        read_com(portRead, buffer, sizeof(buffer));
        printf("readed: %s\n", buffer);
        sleep(1); 
    }

    // Закрытие портов
    #ifdef _WIN32
        CloseHandle(portRead);
        CloseHandle(portWrite);
    #else
        close(portRead);
        close(portWrite);
    #endif

    return 0;
}
