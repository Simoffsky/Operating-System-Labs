#include "com.c"


// Функция для получения рандомной температуры.
float get_current_temperature_rnd() {
    return 20.0 + (rand() % 100) / 10.0; // Температура от 20.0 до 30.0
}


int main() {
#ifdef _WIN32
    HANDLE portWrite;
    const char* portWriteName = "COM2"; 
#else
    int portWrite;
    const char* portWriteName = "/dev/pts/8"; 
#endif 
    portWrite = setup_com_port(portWriteName);

    char buffer[20];
    float temperature;
    while (1) {
        temperature = get_current_temperature_rnd();
        sprintf(buffer, "%.2f", temperature);
        write_com(portWrite, buffer);
        printf("write: %s\n", buffer);
        sleep(1); 
    }
    return 0;
}
