#ifndef atl_GAZEBO_DF_CAMERA_PLUGIN_HPP
#define atl_GAZEBO_DF_CAMERA_PLUGIN_HPP

#include <boost/bind.hpp>

#include <gazebo/common/Plugin.hh>
#include <gazebo/gazebo.hh>
#include <gazebo/rendering/Camera.hh>
#include <gazebo/sensors/CameraSensor.hh>
#include <gazebo/sensors/DepthCameraSensor.hh>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "atl/gazebo/gazebo_node.hpp"
#include "atl/utils/utils.hpp"

namespace atl {
namespace gaz {

#define IMAGE_PUB_TOPIC "~/df_camera/image"

class DFCameraGPlugin : public gazebo::SensorPlugin, public GazeboNode {
public:
  gazebo::rendering::ScenePtr scene;
  gazebo::sensors::CameraSensorPtr sensor;
  gazebo::rendering::CameraPtr camera;
  gazebo::event::ConnectionPtr update_conn;

  int seq;
  int image_width;
  int image_height;
  int image_depth;
  std::string image_format;
  cv::Mat image;

  DFCameraGPlugin();
  void Load(gazebo::sensors::SensorPtr sptr, sdf::ElementPtr sdf);
  int configure();
  void onNewFrame(const unsigned char *image,
                  const int image_width,
                  const int image_height,
                  const int image_depth,
                  const std::string &format);
};

} // namespace gaz
} // namespace atl
#endif
