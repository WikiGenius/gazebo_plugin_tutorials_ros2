#include <gazebo/common/common.hh>         // For common Gazebo functions like ModelPlugin, event, GetName
#include <gazebo/gazebo.hh>                // For accessing all Gazebo classes
#include <gazebo/physics/physics.hh>       // For Gazebo physics, to access ModelPtr
#include <ignition/math/Color.hh>          // For Color() from Ignition Math
#include <ignition/math/Vector3.hh>        // For Vector3d() from Ignition Math

#include <rclcpp/rclcpp.hpp>               // ROS 2 main header
#include <std_msgs/msg/string.hpp>         // For std_msgs/String message type

namespace gazebo {
class LightControllerROSPlugin : public ModelPlugin {
public:
  LightControllerROSPlugin() : ModelPlugin() {}

  void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf) override {
    // Ensure ROS 2 is initialized
    if (!rclcpp::ok()) {
      RCLCPP_FATAL(rclcpp::get_logger("LightControllerROSPlugin"),
                   "A ROS 2 node for Gazebo has not been initialized. Exiting.");
      return;
    }

    RCLCPP_INFO(rclcpp::get_logger("LightControllerROSPlugin"),
                "ROS 2 Model Plugin Loaded!");

    // Create ROS 2 node
    this->ros_node_ = std::make_shared<rclcpp::Node>("light_controller");

    // Create a subscription to the /light_color topic
    this->sub_light_ = this->ros_node_->create_subscription<std_msgs::msg::String>(
        "/light_color", 10,
        std::bind(&LightControllerROSPlugin::ColourCallback, this, std::placeholders::_1));

    // Initialize Gazebo transport node
    transport::NodePtr node(new transport::Node());
    node->Init();

    // Advertise a topic to modify light properties
    this->light_pub_ = node->Advertise<msgs::Light>("~/light/modify");

    // Store the model pointer
    this->model_ = _model;

    // Connect to the world update event
    this->update_connection_ = event::Events::ConnectWorldUpdateBegin(
        std::bind(&LightControllerROSPlugin::OnUpdate, this));

    // Get model, link, and light names from SDF
    this->model_name_ = model_->GetName();

    auto link = this->model_->GetSDF()->GetElement("link");
    this->link_name_ = link->Get<std::string>("name");
    auto sdf_light = link->GetElement("light");
    this->light_name_ = sdf_light->Get<std::string>("name");

    // Construct the complete light name for Gazebo messages
    this->complete_light_name_ =
        this->model_name_ + "::" + this->link_name_ + "::" + this->light_name_;

    std::cout << "Complete light name: " << this->complete_light_name_ << std::endl;

    // Start spinning in a separate thread
    this->ros_spinner_thread_ = std::thread([this]() {
      rclcpp::spin(this->ros_node_);
    });
  }

  // Control the light's color based on the input string
  void ControlLight(const std::string &colour) {
    msgs::Light light_msg;
    light_msg.set_name(this->complete_light_name_);

    if (colour == "red") {
      msgs::Set(light_msg.mutable_diffuse(),
                ignition::math::Color(1.0, 0.0, 0.0, 1.0));
    } else if (colour == "green") {
      msgs::Set(light_msg.mutable_diffuse(),
                ignition::math::Color(0.0, 1.0, 0.0, 1.0));
    } else if (colour == "blue") {
      msgs::Set(light_msg.mutable_diffuse(),
                ignition::math::Color(0.0, 0.0, 1.0, 1.0));
    }

    // Publish the light modification message
    this->light_pub_->Publish(light_msg);
  }

  // ROS 2 callback to handle messages from /light_color topic
  void ColourCallback(const std_msgs::msg::String::SharedPtr msg) {
    RCLCPP_INFO(this->ros_node_->get_logger(), "Received color: [%s]",
                msg->data.c_str());
    this->light_colour_name_ = msg->data;
  }

  // Called at every simulation update iteration
  void OnUpdate() {
    // Update the light color in the simulation
    this->ControlLight(this->light_colour_name_);
  }

  ~LightControllerROSPlugin() override {
    if (this->ros_spinner_thread_.joinable()) {
      this->ros_spinner_thread_.join();
    }
    rclcpp::shutdown();
  }

private:
  physics::ModelPtr model_;                // Pointer to the model
  std::string model_name_, link_name_, light_name_; // Names of the model, link, and light
  std::string complete_light_name_;        // Full name of the light in Gazebo

  transport::PublisherPtr light_pub_;      // Gazebo transport publisher for light modifications
  event::ConnectionPtr update_connection_; // Pointer to the update event connection

  // ROS 2 members
  rclcpp::Node::SharedPtr ros_node_;       // ROS 2 node
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_light_; // ROS 2 subscriber
  std::string light_colour_name_;          // Current light color from ROS topic
  std::thread ros_spinner_thread_;         // Separate thread for spinning the ROS 2 node
};

// Register this plugin with Gazebo
GZ_REGISTER_MODEL_PLUGIN(LightControllerROSPlugin)
}  // namespace gazebo
