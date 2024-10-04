cmake_minimum_required(VERSION 3.24)
project(aero_logger_driver LANGUAGES CXX)

find_package(Protobuf REQUIRED)
find_package(Boost REQUIRED)

add_library(aero_logger src/aero_logger.cpp)
add_library(aero_sensor src/aero_sensor.pb.cc)

include_directories(${Boost_INCLUDE_DIRS})
target_include_directories(aero_logger PUBLIC include)
target_include_directories(aero_sensor PUBLIC include)

target_link_libraries(aero_sensor PUBLIC protobuf::libprotobuf)
target_link_libraries(aero_sensor PUBLIC ${Boost_LIBRARIES})
target_link_libraries(aero_logger PUBLIC aero_sesnor)

add_executable(aero_logger_driver app/main.cpp)
target_link_libraries(aero_logger_driver PUBLIC aero_logger)

install(TARGETS aero_logger_driver DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
