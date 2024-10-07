import os
from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration
from my_python_utils import helper


def generate_launch_description():
    # Package directories
    gazebo_ros_share = get_package_share_directory('gazebo_ros')
    model_plugin_share = get_package_share_directory(
        'model_plugin')

    # Paths to world and launch files
    world_path = os.path.join(
        model_plugin_share, 'worlds', 'custom_model.world')
    gzserver_launch_path = os.path.join(
        gazebo_ros_share, 'launch', 'gzserver.launch.py')
    gzclient_launch_path = os.path.join(
        gazebo_ros_share, 'launch', 'gzclient.launch.py')

    # Declare simulation-related launch arguments
    launch_args = helper.declare_simulation_arguments(world_path)

    # Gazebo server and client launch files
    world = LaunchConfiguration('world')
    pause = LaunchConfiguration('pause')
    verbose = LaunchConfiguration('verbose')
    gzserver_launch, gzclient_launch = helper.load_gazebo_launch(
        gzserver_launch_path, gzclient_launch_path, world, verbose, pause)

    return LaunchDescription(
        [
            *launch_args,
            gzserver_launch,
            gzclient_launch
        ]
    )
