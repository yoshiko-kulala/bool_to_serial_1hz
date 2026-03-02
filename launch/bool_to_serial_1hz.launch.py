from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    port_arg = DeclareLaunchArgument("port", default_value="/dev/ttyACM0")
    baud_arg = DeclareLaunchArgument("baud", default_value="115200")
    topic_arg = DeclareLaunchArgument("topic", default_value="/to_arduino")

    node = Node(
        package="bool_to_serial_1hz",
        executable="bool_to_serial_1hz_node",
        name="bool_to_serial_1hz",
        output="screen",
        parameters=[
            {
                "port": LaunchConfiguration("port"),
                "baud": LaunchConfiguration("baud"),
                "topic": LaunchConfiguration("topic"),
            }
        ],
    )

    return LaunchDescription([port_arg, baud_arg, topic_arg, node])
