#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <string>

static speed_t baud_to_speed(int baud)
{
  switch (baud) {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    default: return B115200;
  }
}

static int open_serial(const std::string & port, int baud)
{
  int fd = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    return -1;
  }

  termios tio {};
  if (tcgetattr(fd, &tio) != 0) {
    ::close(fd);
    return -1;
  }

  const speed_t spd = baud_to_speed(baud);
  cfsetispeed(&tio, spd);
  cfsetospeed(&tio, spd);

  tio.c_cflag |= (CLOCAL | CREAD);
  tio.c_cflag &= ~PARENB;
  tio.c_cflag &= ~CSTOPB;
  tio.c_cflag &= ~CSIZE;
  tio.c_cflag |= CS8;

  tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  tio.c_iflag &= ~(IXON | IXOFF | IXANY);
  tio.c_oflag &= ~OPOST;

  tcflush(fd, TCIFLUSH);
  if (tcsetattr(fd, TCSANOW, &tio) != 0) {
    ::close(fd);
    return -1;
  }

  return fd;
}

class BoolToSerial1Hz : public rclcpp::Node
{
public:
  BoolToSerial1Hz()
  : Node("bool_to_serial_1hz")
  {
    port_ = declare_parameter<std::string>("port", "/dev/ttyACM0");
    baud_ = declare_parameter<int>("baud", 115200);
    topic_ = declare_parameter<std::string>("topic", "/to_arduino");
    reconnect_sec_ = declare_parameter<double>("reconnect_sec", 1.0);

    last_value_ = false;

    sub_ = create_subscription<std_msgs::msg::Bool>(
      topic_, rclcpp::QoS(10),
      std::bind(&BoolToSerial1Hz::on_msg, this, std::placeholders::_1));

    open_or_log();
    send_current_value(true);

    using namespace std::chrono_literals;
    send_timer_ = create_wall_timer(1s, std::bind(&BoolToSerial1Hz::on_send_timer, this));
    reconnect_timer_ = create_wall_timer(200ms, std::bind(&BoolToSerial1Hz::on_reconnect_timer, this));
  }

  ~BoolToSerial1Hz() override
  {
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }

private:
  void open_or_log()
  {
    if (fd_ >= 0) {
      return;
    }

    fd_ = open_serial(port_, baud_);
    if (fd_ >= 0) {
      RCLCPP_INFO(get_logger(), "Opened serial %s @ %d", port_.c_str(), baud_);
    } else {
      RCLCPP_WARN(get_logger(), "Failed to open %s (will retry)", port_.c_str());
    }
  }

  void close_fd(const char * reason)
  {
    if (fd_ >= 0) {
      RCLCPP_WARN(get_logger(), "Closing serial: %s", reason);
      ::close(fd_);
      fd_ = -1;
    }
  }

  void on_reconnect_timer()
  {
    if (fd_ >= 0) {
      return;
    }

    auto now = this->now();
    if (now < next_reconnect_) {
      return;
    }

    next_reconnect_ = now + rclcpp::Duration::from_seconds(reconnect_sec_);
    open_or_log();

    if (fd_ >= 0) {
      send_current_value(true);
    }
  }

  void on_msg(const std_msgs::msg::Bool::SharedPtr msg)
  {
    last_value_ = msg->data;
  }

  void on_send_timer()
  {
    send_current_value(false);
  }

  void send_current_value(bool force)
  {
    if (fd_ < 0) {
      RCLCPP_WARN_THROTTLE(
        get_logger(), *get_clock(), 2000,
        "Serial not open, cannot send (will retry)");
      return;
    }

    const char out = last_value_ ? '1' : '0';
    char buf[2] = {out, '\n'};

    const int n = ::write(fd_, buf, 2);
    if (n == 2) {
      if (force) {
        RCLCPP_INFO(get_logger(), "Sent (force) %c", out);
      }
      return;
    }

    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000, "Serial write would block");
      return;
    }

    RCLCPP_ERROR(get_logger(), "Serial write error: %s", std::strerror(errno));
    close_fd("write error");
  }

  std::string port_;
  int baud_ {115200};
  std::string topic_;
  double reconnect_sec_ {1.0};

  int fd_ {-1};
  rclcpp::Time next_reconnect_ {0, 0, RCL_ROS_TIME};
  bool last_value_ {false};

  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_;
  rclcpp::TimerBase::SharedPtr send_timer_;
  rclcpp::TimerBase::SharedPtr reconnect_timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<BoolToSerial1Hz>());
  rclcpp::shutdown();
  return 0;
}
