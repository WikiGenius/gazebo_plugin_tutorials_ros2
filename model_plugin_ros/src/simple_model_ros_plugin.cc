// #include "model_plugin_ros/simple_model_ros_plugin.hpp"

#include <gazebo/common/common.hh> // for common fn in gazebo like ModelPlugin, event
#include <gazebo/gazebo.hh>        // for accessing all gazebo classes
#include <gazebo/physics/physics.hh> // for gazebo physics, to access -- ModelPtr
#include <ignition/math/Vector3.hh> // to access Vector3d() from ignition math class
#include <gazebo_ros/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

namespace gazebo {
class SimpleModelRosPlugin : public ModelPlugin {

public:
  SimpleModelRosPlugin() : ModelPlugin() {
    this->logger_name_ = "simple_model_Ros_plugin";
    this->command_topic_ = "/model_move_up";
    this->vel_ = 0.1;
  }

  void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf) {

    if (!rclcpp::ok()) {
      RCLCPP_FATAL_STREAM(rclcpp::get_logger(this->logger_name_),
                          "A ROS node for Gazebo has not been initialized");
      return;
    }
    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_),
                "ROS Model Plugin Loaded!");

    this->ros_node_ = gazebo_ros::Node::Get(_sdf);

    this->sub_ = this->ros_node_->create_subscription<std_msgs::msg::Bool>(
        this->command_topic_, qos(),
        std::bind(&SimpleModelRosPlugin::Activate_Callback, this,
                  std::placeholders::_1));

    // Store the pointer to the model
    this->model_ = _model;

    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "Model Name=%s",
                this->model_->GetName().c_str());

    if (_sdf->HasElement("model_vel")) // check if element existence
    {
      // use _sdf pointer & Get to find value in <model_vel>
      this->vel_ = _sdf->Get<double>("model_vel");
    }
    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "model_vel=%.2f ",
                this->vel_);
    // Listen to the update event. This event is broadcast every  simulation
    // iteration.
    this->updateConnection_ = event::Events::ConnectWorldUpdateBegin(
        std::bind(&SimpleModelRosPlugin::OnUpdate, this));

    // bind() is use to bind this & OnUpdate i.e
    // this->OnUpdate
  } // bind- we don't have define callback fn
  // input parametes its replace by placeholder
public:
  const rclcpp::QoS &qos(size_t history_depth = 10) {
    // Create a QoS profile with a history depth of 10
    static rclcpp::QoS qos_profile(
        history_depth); // Not marked const because we'll modify it

    // Set reliability, durability, and liveliness
    qos_profile.reliability(rclcpp::ReliabilityPolicy::Reliable);
    qos_profile.durability(rclcpp::DurabilityPolicy::TransientLocal);
    qos_profile.liveliness(rclcpp::LivelinessPolicy::Automatic);

    return qos_profile;
  }

public:
  void Activate_Callback(const std_msgs::msg::Bool &msg) {
    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "Received Message = %d",
                msg.data);
    this->activate_move_ = msg.data;
  }

  //   // keep on updating as simulation iterates
public:
  void OnUpdate() {

    // once rostopic true than it move up for 1000 count

    if (this->activate_move_) { // if ros topic model_move_up recieves true

      if (this->count_ < 10000) // move up till 10000 counts
      {
        // Apply a small linear velocity to the model.
        this->model_->SetLinearVel(ignition::math::Vector3d(0, 0, this->vel_));

      } else {

        // reset values
        this->activate_move_ = false;
        this->count_ = 0;
      }

      this->count_++; // increment count
    }
  }
  // data members
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

// Register this plugin with the simulator
GZ_REGISTER_MODEL_PLUGIN(SimpleModelRosPlugin)
} // namespace gazebo