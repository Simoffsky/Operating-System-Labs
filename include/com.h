#ifndef COM_H
#define COM_H

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)  // Для Windows используем Sleep() с миллисекундами

// setup_com_port настраивает com-порт и возвращает хендлер порта.
HANDLE setup_com_port(const char* port_name);

// read_com позволяет сосчитывать данные с com-порта.
int read_com(HANDLE hSerial, char* buffer, DWORD bufferSize);

// write_com позволяет записывать данные в com-порт.
int write_com(HANDLE hSerial, const char* data);

#else // UNIX
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

// setup_com_port настраивает com-порт и возвращает дескриптор порта.
int setup_com_port(const char* port_name);

// read_com позволяет сосчитывать данные с com-порта.
int read_com(int fd, char* buffer, size_t bufferSize);

// write_com позволяет записывать данные в com-порт.
int write_com(int fd, const char* data);
#endif // _WIN32

#endif // COM_H