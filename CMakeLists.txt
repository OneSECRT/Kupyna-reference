cmake_minimum_required(VERSION 3.10)
project(KupynaReference C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")

add_definitions(-D_POSIX_C_SOURCE=199309L)

set(SOURCES
    kupyna.c
    kupyna.h
    tables.c
    tables.h
    perf.h
    main.c
)

add_executable(kupyna-reference ${SOURCES})
