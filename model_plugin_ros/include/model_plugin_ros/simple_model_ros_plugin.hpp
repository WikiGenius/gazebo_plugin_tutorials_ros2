#ifndef SIMPLE_MODEL_ROS_HPP_
#define SIMPLE_MODEL_ROS_HPP_

#include <gazebo/common/common.hh> // for common fn in gazebo like ModelPlugin, event
#include <gazebo/gazebo.hh>        // for accessing all gazebo classes
#include <gazebo/physics/physics.hh> // for gazebo physics, to access -- ModelPtr
#include <gazebo_ros/node.hpp>
#include <ignition/math/Vector3.hh> // to access Vector3d() from ignition math class
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

namespace gazebo {
class SimpleModelRosPlugin : public ModelPlugin {

public:
  /// Constructor
  SimpleModelRosPlugin();

protected:
  // Documentation inherited
  void Load(gazebo::physics::ModelPtr _model, sdf::ElementPtr _sdf) override;
  const rclcpp::QoS &qos(size_t history_depth = 10);
  void Activate_Callback(const std_msgs::msg::Bool &msg);
  void OnUpdate();

private:
  std::string command_topic_;
  std::string logger_name_;
  physics::ModelPtr model_; // Pointer to the model
  int count_;               // to keep a count
  double vel_;              // assign vel to model
  event::ConnectionPtr
      updateConnection_; // Pointer to the update event connection
  gazebo_ros::Node::SharedPtr ros_node_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_;
  bool activate_move_; // to store boolen ros callback data
};
} // namespace gazebo

#endif // SIMPLE_MODEL_ROS_HPP_