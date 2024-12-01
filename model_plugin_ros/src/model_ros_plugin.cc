#include <gazebo/common/common.hh>         // For Gazebo common functions
#include <gazebo/gazebo.hh>                // For accessing all Gazebo classes
#include <gazebo/physics/physics.hh>       // For Gazebo physics, ModelPtr
#include <ignition/math/Vector3.hh>        // For Vector3d from Ignition Math
#include <rclcpp/rclcpp.hpp>               // For ROS 2
#include <std_msgs/msg/bool.hpp>           // For std_msgs/Bool message type
#include <thread>                          // For multithreading
#include <functional>                      // For std::bind()

namespace gazebo {
class ModelRosPlugin : public ModelPlugin {
public:
  ModelRosPlugin() : ModelPlugin() {
    this->logger_name_ = "simple_model_ros_plugin";
    this->command_topic_ = "/model_move_up";
    this->vel_ = 0.1;
    this->activate_move_ = false;
    this->count_ = 0;
  }

  void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf) override {
    if (!rclcpp::ok()) {
      int argc = 0;
      char **argv = nullptr;
      rclcpp::init(argc, argv);
    }

    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "ROS 2 Model Plugin Loaded!");

    // Initialize ROS 2 node
    this->ros_node_ = std::make_shared<rclcpp::Node>("gazebo_client");

    // Create a subscription to the /model_move_up topic
    this->subscription_ = this->ros_node_->create_subscription<std_msgs::msg::Bool>(
        this->command_topic_, 10,
        std::bind(&ModelRosPlugin::ActivateCallback, this, std::placeholders::_1));

    // Spin in a separate thread to process messages
    this->ros_spinner_thread_ = std::thread([this]() {
      rclcpp::spin(this->ros_node_);
    });

    // Store the model pointer
    this->model_ = _model;

    std::cout << "Model Name = " << this->model_->GetName() << std::endl;

    // Assign velocity from SDF if specified
    if (_sdf->HasElement("model_vel")) {
      this->vel_ = _sdf->Get<double>("model_vel");
    }
    std::cout << "model_vel = " << this->vel_ << std::endl;

    // Listen to the update event
    this->update_connection_ = event::Events::ConnectWorldUpdateBegin(
        std::bind(&ModelRosPlugin::OnUpdate, this));
  }

  // Callback for the ROS 2 subscription
  void ActivateCallback(const std_msgs::msg::Bool::SharedPtr msg) {
    RCLCPP_INFO(rclcpp::get_logger(this->logger_name_), "Received Message = %d", msg->data);
    this->activate_move_ = msg->data;
  }

  // Update function called at every simulation iteration
  void OnUpdate() {
    if (this->activate_move_) {
      if (this->count_ < 10000) {
        this->model_->SetLinearVel(ignition::math::Vector3d(0, 0, this->vel_));
      } else {
        this->activate_move_ = false;
        this->count_ = 0;
      }
      this->count_++;
    }
  }

  ~ModelRosPlugin() override {
    if (this->ros_spinner_thread_.joinable()) {
      this->ros_spinner_thread_.join();
    }
    rclcpp::shutdown();
  }

private:
  // Pointer to the model
  physics::ModelPtr model_;
  std::string logger_name_;

  // ROS 2-related members
  rclcpp::Node::SharedPtr ros_node_;             // ROS 2 Node
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subscription_; // ROS 2 Subscription
  std::thread ros_spinner_thread_;              // Separate thread for spinning

  // Plugin data members
  int count_;
  double vel_;
  bool activate_move_;
  std::string command_topic_;

  // Pointer to the update event connection
  event::ConnectionPtr update_connection_;
};

// Register this plugin with Gazebo
GZ_REGISTER_MODEL_PLUGIN(ModelRosPlugin)
}  // namespace gazebo
