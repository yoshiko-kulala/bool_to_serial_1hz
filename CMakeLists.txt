cmake_minimum_required(VERSION 3.8)
project(bool_to_serial_1hz)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(bool_to_serial_1hz_node src/bool_to_serial_1hz.cpp)
ament_target_dependencies(bool_to_serial_1hz_node rclcpp std_msgs)

install(TARGETS bool_to_serial_1hz_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})

ament_package()
