# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required(VERSION 3.8)
#add_definitions(-DTARGET_64)
IF (WIN32)
    MESSAGE(STATUS "This is Windows.")
    IF (CMAKE_CL_64)
        MESSAGE(STATUS "X64")
        set(SERIALPORT_DIR "${CMAKE_SOURCE_DIR}/serialport/win/x64")
    ELSE ()
        MESSAGE(STATUS "X86")
        set(SERIALPORT_DIR "${CMAKE_SOURCE_DIR}/serialport/win/win32")
    ENDIF ()
    set(SERIALPORT_LIB "serialport.lib")
ELSEIF (UNIX)
    MESSAGE(STATUS "This is Linux.")
ELSEIF (APPLE)
    MESSAGE(STATUS "This is APPLE.")
ENDIF (WIN32)

MESSAGE(STATUS  "###################################")
MESSAGE(STATUS "SERIALPORT_DIR:${SERIALPORT_DIR}")
MESSAGE(STATUS "SERIALPORT_LIB:${SERIALPORT_LIB}")


include_directories("serialport")

project("sensor")

# 包含子项目。
#add_subdirectory ("sensor")
add_library(sensor STATIC sensor/sensor.h sensor/sensor.c)
#add_library(serialport STATIC IMPORTED)
link_directories(sensor ${SERIALPORT_DIR})
#target_link_libraries(sensor ${SERIALPORT_LIB})
add_subdirectory(test/win/win32)