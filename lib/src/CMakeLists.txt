cmake_minimum_required(VERSION 3.7)
project(wmsm_lib)

set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=native -ffast-math")
set(CMAKE_SOURCE_DIR src)

set(SOURCE_FILES_COMMON common.h common.c)
set(SOURCE_FILES_BIN bin.c bin.h mpool.c mpool.h linkedlist.h)
set(SOURCE_FILES_PROBLEM problem.c problem.h  ${SOURCE_FILES_COMMON})
set(SOURCE_FILES_PLATFORM platform.c platform.h ${SOURCE_FILES_BIN} ${SOURCE_FILES_COMMON})
set(SOURCE_FILES_SCHEDULE schedule.c schedule.h ${SOURCE_FILES_COMMON})
set(SOURCE_FILES_EXTRA extra.h extra.c ${SOURCE_FILES_COMMON})

add_library(wsms SHARED ${SOURCE_FILES_PROBLEM} ${SOURCE_FILES_PLATFORM} ${SOURCE_FILES_SCHEDULE})

add_library(wsms_extra SHARED ${SOURCE_FILES_EXTRA} ${SOURCE_FILES_PROBLEM})

#add_executable(wmsm_test main.c ${SOURCE_FILES_PROBLEM} ${SOURCE_FILES_TOOLS})
