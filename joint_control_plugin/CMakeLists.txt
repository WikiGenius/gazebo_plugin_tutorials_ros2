cmake_minimum_required(VERSION 3.8)
project(joint_control_plugin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(gazebo_dev REQUIRED)

include_directories(
  include/
  ${GAZEBO_INCLUDE_DIRS}
  ${rclcpp_INCLUDE_DIRS}
  ${std_msgs_INCLUDE_DIRS}
  ${rcl_interfaces_INCLUDE_DIRS}
)
set(MODEL_PLUGIN_ROS_LIBS gazebo_dev rclcpp std_msgs)

function(install_plugin plugin_library src)
  add_library(${plugin_library} SHARED ${src})
  ament_target_dependencies(${plugin_library} ${MODEL_PLUGIN_ROS_LIBS})
  install(TARGETS ${plugin_library} LIBRARY DESTINATION lib)
endfunction(install_plugin)

install_plugin(joint_controller_plugin ./src/joint_controller_plugin.cc)


install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY worlds DESTINATION share/${PROJECT_NAME})
install(DIRECTORY models DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
