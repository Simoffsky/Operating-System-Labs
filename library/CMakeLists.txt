set(LIBRARY_NAME library)

set(SRC_FILES
    library.c
)

add_library(${LIBRARY_NAME} STATIC ${SRC_FILES})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ${LIBRARY_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

install(FILES process.h DESTINATION include)
