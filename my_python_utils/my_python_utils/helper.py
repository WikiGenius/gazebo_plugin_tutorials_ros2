import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, ExecuteProcess
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.conditions import IfCondition


def declare_simulation_arguments(world_path, gdb=False, valgrind=False):
    """Declare common simulation-related launch arguments."""
    return [
        DeclareLaunchArgument('world', default_value=world_path,
                              description='world path'),
        DeclareLaunchArgument('pause', default_value='false',
                              description='Start the Gazebo simulation paused.'),
        DeclareLaunchArgument('gui', default_value='true',
                              description='Open Gazebo with the GUI.'),
        DeclareLaunchArgument(name='gdb', default_value='true' if gdb else 'false',
                              description='Set "true" to run gzserver with gdb.'),
        DeclareLaunchArgument(name='valgrind', default_value='true' if valgrind else 'false',
                              description='Set "true" to run gzserver with valgrind.'),
        DeclareLaunchArgument('verbose', default_value='true',
                              description='verbose or not'),
        DeclareLaunchArgument('extra_gazebo_args', default_value='',
                              description='Extra arguments for Gazebo.'),
    ]


def load_gazebo_launch(gzserver_launch_path, gzclient_launch_path, world_path, verbose, pause):
    """Load Gazebo server and client launch files."""
    gzserver_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(gzserver_launch_path),
        launch_arguments={'world': world_path, 'verbose': verbose,
                          'pause': pause,
                          'extra_gazebo_args': LaunchConfiguration('extra_gazebo_args'),
                          'gdb': LaunchConfiguration("gdb"), 'valgrind': LaunchConfiguration("valgrind")}.items()
    )

    gzclient_launch = IncludeLaunchDescription(
        condition=IfCondition(LaunchConfiguration('gui')),
        launch_description_source=PythonLaunchDescriptionSource(
            gzclient_launch_path),
    )

    return gzserver_launch, gzclient_launch
