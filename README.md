# Лабораторная работа #2

В рамках данной лабораторной работы необходимо:
- Написать кроссплатформенную (unix/windows) библиотеку, которая содержит функцию запуска программ в фоновом режиме, с возможностью подождать
завершение дочерней программы и получить код ответа.
- Написать тестовую утилиту для проверки библиотеки.

## Описание
В библиотеке присутствует функция `start_process` которая позволяет запускать процесс в фоновом режиме.

- Для Windows используется API CreateProcessA.
- Для Unix систем создается дочерний процесс через fork и выполняется команда с помощью execl.

Если флаг ожидания (`wait != 0`) то родительский процесс будет ждать дочерний, а код ответа запишется по указателю в `exit_code`.

Сама библиотека находится в директории [library](library)

Для проверки библиотеки написана утилита [example.c](example.c)
## Запуск

#### Windows
Актуализировать исходный код и собрать проект при помощи скрипта `build.cmd`

Запустить `example` утилиту в `build/example.exe`

#### Unix
Актуализировать исходный код и собрать проект при помощи скрипта `build.sh`

Запустить `example` утилиту в `build/example`
