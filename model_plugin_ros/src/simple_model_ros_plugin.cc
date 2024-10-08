#include "model_plugin_ros/simple_model_ros_plugin.hpp"

namespace gazebo
{
  // Register this plugin with the simulator
  GZ_REGISTER_MODEL_PLUGIN(SimpleModelRosPlugin)

  SimpleModelRosPlugin::SimpleModelRosPlugin() : ModelPlugin()
  {
    this->logger_name_ = "simple_model_Ros_plugin";
    this->command_topic_ = "/model_move_up";
    this->vel_ = 0.1;
  }

  void SimpleModelRosPlugin::Load(physics::ModelPtr _model, sdf::ElementPtr _sdf)
  {
    if (!rclcpp::ok())
    {
      RCLCPP_FATAL_STREAM(rclcpp::get_logger(this->logger_name_), "A ROS node for Gazebo has not been initialized");
      return;
    }
    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "ROS Model Plugin Loaded!");

    this->ros_node_ = gazebo_ros::Node::Get(_sdf);

    this->sub_ = this->ros_node_->create_subscription<std_msgs::msg::Bool>(
        this->command_topic_, this->qos(),
        std::bind(&SimpleModelRosPlugin::Activate_Callback, this, std::placeholders::_1));

    // Store the pointer to the model
    this->model_ = _model;

    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "Model Name=%s", this->model_->GetName().c_str());

    if (_sdf->HasElement("model_vel")) // check if element existence
    {
      // use _sdf pointer & Get to find value in <model_vel>
      this->vel_ = _sdf->Get<double>("model_vel");
    }
    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "model_vel=%.2f ", this->vel_);
    // Listen to the update event. This event is broadcast every  simulation
    // iteration.
    this->updateConnection_ = event::Events::ConnectWorldUpdateBegin(std::bind(&SimpleModelRosPlugin::OnUpdate, this));

    // bind() is use to bind this & OnUpdate i.e
    // this->OnUpdate
  } // bind- we don't have define callback fn
  // input parametes its replace by placeholder

  const rclcpp::QoS &SimpleModelRosPlugin::qos(size_t history_depth)
  {
    // Create a QoS profile with a history depth of 10
    static rclcpp::QoS qos_profile(history_depth); // Not marked const because we'll modify it

    // Set reliability, durability, and liveliness
    qos_profile.reliability(rclcpp::ReliabilityPolicy::Reliable);
    qos_profile.durability(rclcpp::DurabilityPolicy::TransientLocal);
    qos_profile.liveliness(rclcpp::LivelinessPolicy::Automatic);

    return qos_profile;
  }

  void SimpleModelRosPlugin::Activate_Callback(const std_msgs::msg::Bool &msg)
  {
    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "Received Message = %d", msg.data);
    this->activate_move_ = msg.data;
  }

  //   // keep on updating as simulation iterates
  void SimpleModelRosPlugin::OnUpdate()
  {
    // once rostopic true than it move up for 1000 count

    if (this->activate_move_)
    { // if ros topic model_move_up recieves true

      if (this->count_ < 10000) // move up till 10000 counts
      {
        // Apply a small linear velocity to the model.
        this->model_->SetLinearVel(ignition::math::Vector3d(0, 0, this->vel_));
      }
      else
      {
        // reset values
        this->activate_move_ = false;
        this->count_ = 0;
      }
      this->count_++; // increment count
    }
  }

} // namespace gazebo