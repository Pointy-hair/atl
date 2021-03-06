#ifndef atl_GAZEBO_DF_CAMERA_CLIENT_HPP
#define atl_GAZEBO_DF_CAMERA_CLIENT_HPP

#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "atl/gazebo/gazebo_node.hpp"

namespace atl {
namespace gaz {

// PUBLISH TOPICS
#define IMAGE_TOPIC "~/df_camera/image"

class DFCameraGClient : public GazeboNode {
public:
  bool connected;
  cv::Mat image;

  DFCameraGClient();
  ~DFCameraGClient();
  int configure();
  virtual void imageCallback(ConstImagePtr &msg);
};

} // namespace gaz
} // namespace atl
#endif
