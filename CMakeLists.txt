#####################################################################
## canbusutil:
#####################################################################

cmake_minimum_required(VERSION 3.16)
project(canfuzz)

set(CMAKE_PREFIX_PATH "/usr1/qt/qt-install/")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(ROOT_DIR "/usr1/qt/myqtcan/")
#set(ROOT_INCLUDE_DIR "${ROOT_DIR}/include")
#set(ROOT_SOURCE_DIR "${ROOT_DIR}/src")
#set(ROOT_LIB_DIR "${ROOT_DIR}/lib")

# Compiler
set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-O0 -Wall")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
#add_definitions()

# Qt6
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 COMPONENTS Core Network SerialBus SerialPort)

include_directories(
    ${PROJECT_SOURCE_DIR}/thirdparty
	${PROJECT_SOURCE_DIR}/common
	${PROJECT_SOURCE_DIR}/mutator
	${PROJECT_SOURCE_DIR}/fuzzer
	${PROJECT_SOURCE_DIR}/canbus
	${PROJECT_SOURCE_DIR}/monitor
)

link_directories(
#    ${PROJECT_SOURCE_DIR}/lib
)

add_subdirectory(common)
add_subdirectory(mutator)
add_subdirectory(fuzzer)
add_subdirectory(canbus)
add_subdirectory(monitor)

add_executable(
    ${PROJECT_NAME}
	${CANBUS_SOURCES}
	${COMMON_SOURCES}
	${FUZZER_SOURCES}
	${MUTATOR_SOURCES}
	${MONITOR_SOURCES}
	cmdparser.cpp
	cmdparser.h
    main.cpp 
)

set(LIB_LIST 
    Qt6::Core 
    Qt6::Network
    Qt6::SerialBus
    Qt6::SerialPort
)

target_link_libraries(${PROJECT_NAME} ${LIB_LIST})


