from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    # Path to your world file
    world_file = os.path.join(
        get_package_share_directory('velodyne_gazebo_plugins'),
        'worlds',
        'test_world.sdf'
    )
    # RViz2 configuration
    rviz_config_file = os.path.join(
        # Replace with your RViz package name
        get_package_share_directory('velodyne_gazebo_plugins'),
        'config',
        'velodyne.rviz'  # Replace with your RViz config file
    )
    # Gazebo launch configuration
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory(
                'gazebo_ros'), 'launch', 'gazebo.launch.py')
        ),
        launch_arguments={'world': world_file, 'verbose': 'true'}.items(),
    )

    # Static transform from 'world' to 'lidar_link'
    static_transform_publisher = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher',
        output='screen',
        arguments=['0', '0', '1', '0', '0', '0', 'world', 'lidar_link']
    )
    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_config_file]
    )

    return LaunchDescription([
        gazebo,
        static_transform_publisher,  # Adding static transform
        rviz,
    ])
