cmake_minimum_required(VERSION 3.10)

project(lab4)

# Поиск MySQL заголовков и библиотек
find_path(MYSQL_INCLUDE_DIR mysql.h
    PATHS
    "/usr/include/mariadb"
    "C:/Program Files/MariaDB/MariaDB Connector C 64-bit/include"
    "$ENV{MYSQL_HOME}/include"
    NO_DEFAULT_PATH
)

find_library(MYSQL_LIBRARY
    NAMES mariadbclient libmariadb
    PATHS
    "/usr/lib/x86_64-linux-gnu" 
    "C:/Program Files/MariaDB/MariaDB Connector C 64-bit/lib"
    "$ENV{MYSQL_HOME}/lib"
    NO_DEFAULT_PATH
)

if (NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL library or include directory not found. Check your MYSQL_HOME setting.")
endif()


include_directories(include)

add_executable(server server.c db/db.c)
add_executable(emulator emulator.c com/com.c)
add_executable(db_filler db_filler.c com/com.c db/db.c)

target_include_directories(server PRIVATE include ${MYSQL_INCLUDE_DIR})
target_link_libraries(server PRIVATE ${MYSQL_LIBRARY})

target_include_directories(emulator PRIVATE include)

target_include_directories(db_filler PRIVATE include ${MYSQL_INCLUDE_DIR})
target_link_libraries(db_filler PRIVATE ${MYSQL_LIBRARY})

