cmake_minimum_required(VERSION 3.24)
project(lab3_app LANGUAGES CXX)

find_package(Protobuf REQUIRED)

add_library(aero_logger src/aero_logger.cpp)
add_library(aero_sensor src/aero_sensor.pb.cc)

target_include_directories(aero_logegr PUBLIC include)

target_include_directories(aero_sensor PUBLIC include)

target_link_libraries(aero_sensor PUBLIC protobuf::libprotobuf)
target_link_libraries(aero_logger PUBLIC info)

add_executable(aero_sensor_logger app/main.cpp)
target_link_libraries(lab3_client PUBLIC mainFile)

install(TARGETS aero_logger_sensor DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
