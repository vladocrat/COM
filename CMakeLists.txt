cmake_minimum_required(VERSION 3.14)

project(com_port LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTEST")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

include_directories(test)
include_directories(src)

set(TEST_SOURCES
    test/test_serialport.h test/test_serialport.cpp
    test/test_packet.h test/test_packet.cpp)

set(SOURCES src/sercontroller.h src/sercontroller.cpp
    src/serialport.h src/serialport.cpp
    src/pimpl.h
    src/packet.h src/packet.cpp
    src/protocol.h
    src/weightchannel.h src/weightchannel.cpp)

add_executable(com_port
    ${SOURCES}
    ${TEST_SOURCES}
    main.cpp
)

target_link_libraries(com_port Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::SerialPort Qt${QT_VERSION_MAJOR}::Test)

install(TARGETS com_port
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
