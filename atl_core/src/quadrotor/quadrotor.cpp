#include "atl/quadrotor/quadrotor.hpp"

namespace atl {

int Quadrotor::configure(const std::string &config_path) {
  std::string config_file;
  std::string mission_file;
  ConfigParser parser;

  // position controller
  config_file = config_path + "/controllers/" + "position_controller.yaml";
  CONFIGURE_CONTROLLER(this->position_controller, config_file, FCONFPCTRL);

  // tracking controller
  config_file = config_path + "/controllers/" + "tracking_controller.yaml";
  CONFIGURE_CONTROLLER(this->tracking_controller, config_file, FCONFTCTRL);

  // landing controller
  config_file = config_path + "/controllers/" + "landing_controller.yaml";
  CONFIGURE_CONTROLLER(this->landing_controller, config_file, FCONFLCTRL);

  // waypoint controller
  config_file = config_path + "/controllers/" + "waypoint_controller.yaml";
  CONFIGURE_CONTROLLER(this->waypoint_controller, config_file, FCONFWCTRL);

  // load config
  parser.addParam("hover_position", &this->hover_position);
  parser.addParam("recover_height", &this->recover_height);
  parser.addParam("auto_track", &this->auto_track);
  parser.addParam("auto_land", &this->auto_land);
  parser.addParam("auto_disarm", &this->auto_disarm);
  parser.addParam("target_lost_threshold", &this->target_lost_threshold);
  parser.addParam("min_discover_time", &this->min_discover_time);
  parser.addParam("min_tracking_time", &this->min_tracking_time);
  parser.addParam("mission", &mission_file);
  if (parser.load(config_path + "/config.yaml") != 0) {
    return -1;
  }

  // mission
  paths_combine(config_path, mission_file, mission_file);
  if (this->mission.configure(mission_file) != 0) {
    return -2;
  }

  // misc
  this->landing_target.lost_threshold = this->target_lost_threshold;
  this->current_mode = DISCOVER_MODE;
  this->configured = true;

  return 0;
error:
  return -1;
}

int Quadrotor::setHomePoint(const double latitude, const double longitude) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  this->home_set = true;
  this->home_lat = latitude;
  this->home_lon = longitude;

  return 0;
}

int Quadrotor::setMode(const enum Mode &mode) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set mode
  this->current_mode = mode;
  switch (mode) {
    case DISARM_MODE: LOG_INFO(INFO_KMODE); break;
    case HOVER_MODE: LOG_INFO(INFO_HMODE); break;
    case DISCOVER_MODE: LOG_INFO(INFO_DMODE); break;
    case TRACKING_MODE: LOG_INFO(INFO_TMODE); break;
    case LANDING_MODE: LOG_INFO(INFO_LMODE); break;
    case WAYPOINT_MODE: LOG_INFO(INFO_WMODE); break;
    default: LOG_ERROR(EINVMODE); return -2;
  }

  return 0;
}

int Quadrotor::setPose(const Pose &pose) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set pose
  this->pose = pose;
  return 0;
}

int Quadrotor::setVelocity(const Vec3 &velocity) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set pose
  this->velocity = velocity;

  return 0;
}

int Quadrotor::setYaw(const double yaw) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set yaw
  this->yaw_setpoint = yaw;

  return 0;
}

int Quadrotor::setTargetPosition(const Vec3 &position) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set target position
  this->landing_target_prev.position_B = this->landing_target.position_B;
  this->landing_target.setTargetPosition(position);

  return 0;
}

int Quadrotor::setTargetVelocity(const Vec3 &velocity) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set target velocity
  this->landing_target_prev.velocity_B = this->landing_target.velocity_B;
  this->landing_target.setTargetVelocity(velocity);

  return 0;
}

int Quadrotor::setTargetDetected(bool detected) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set target detected
  this->landing_target.update(detected);

  return 0;
}

int Quadrotor::setHoverXYPosition(const Vec3 &position) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set hover x-y position
  this->hover_position(0) = position(0);
  this->hover_position(1) = position(1);

  return 0;
}

int Quadrotor::setHoverPosition(const Vec3 &position) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // set hover position
  this->hover_position(0) = position(0);
  this->hover_position(1) = position(1);
  this->hover_position(2) = position(2);

  return 0;
}

bool Quadrotor::conditionsMet(const bool *conditions, int nb_conditions) {
  for (int i = 0; i < nb_conditions; i++) {
    if (conditions[i] == false) {
      return false;
    }
  }

  return true;
}

int Quadrotor::stepHoverMode(const double dt) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // hover
  this->position_controller.update(this->hover_position,
                                   this->pose,
                                   this->yaw_setpoint,
                                   dt);
  this->att_cmd = AttitudeCommand(this->position_controller.outputs);

  return 0;
}

int Quadrotor::stepDiscoverMode(const double dt) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // hover in place
  this->stepHoverMode(dt);
  if (this->discover_tic.tv_sec == 0) {
    tic(&this->discover_tic);
  }

  // transition
  bool conditions[3] = {false, false, false};
  conditions[0] = this->landing_target.detected;
  conditions[1] = this->landing_target.losted == false;
  conditions[2] = mtoc(&this->discover_tic) > this->min_discover_time;

  // transition to tracking mode
  if (this->conditionsMet(conditions, 3) && this->auto_track) {
    LOG_INFO("Transitioning to [TRACKING MODE]!");
    this->setMode(TRACKING_MODE);
    this->discover_tic = (struct timespec){0, 0};
  }

  return 0;
}

int Quadrotor::stepTrackingMode(const double dt) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // track target
  this->tracking_controller.update(this->landing_target.position_B,
                                   this->pose.position,
                                   this->hover_position,
                                   this->yaw_setpoint,
                                   dt);
  this->att_cmd = AttitudeCommand(this->tracking_controller.outputs);

  // update hover position and tracking timer
  this->setHoverXYPosition(this->pose.position);
  if (this->tracking_tic.tv_sec == 0) {
    tic(&this->tracking_tic);
  }

  // check conditions
  bool conditions[3] = {false, false, false};
  conditions[0] = this->landing_target.detected;
  conditions[1] = this->landing_target.losted == false;
  conditions[2] = mtoc(&this->tracking_tic) > this->min_tracking_time;

  // transition to landing mode
  if (this->conditionsMet(conditions, 3) && this->auto_land) {
    LOG_INFO("Transitioning to [LANDING MODE]!");
    this->setMode(LANDING_MODE);
    this->tracking_tic = (struct timespec){0, 0};

    // transition back to discover mode
  } else if (this->landing_target.isTargetLosted()) {
    LOG_INFO("Landing Target is lost!");
    LOG_INFO("Transitioning back to [DISCOVER_MODE]!");
    this->setMode(DISCOVER_MODE);
    this->tracking_tic = (struct timespec){0, 0};
    this->hover_position(2) = this->recover_height;
  }

  return 0;
}

int Quadrotor::stepLandingMode(const double dt) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // land on target
  this->landing_controller.update(this->landing_target.position_B,
                                  this->velocity,
                                  this->yaw_setpoint,
                                  dt);
  this->att_cmd = AttitudeCommand(this->landing_controller.outputs);

  // update hover position and tracking timer
  this->setHoverPosition(this->pose.position);
  if (this->landing_tic.tv_sec == 0) {
    tic(&this->landing_tic);
  }

  // check conditions
  bool conditions[3] = {false, false, false};
  conditions[0] = this->landing_target.detected;
  conditions[1] = this->landing_target.losted == false;
  conditions[2] = this->landing_target.position_B.norm() < 0.1;

  // transition to disarm mode
  if (this->conditionsMet(conditions, 3) && this->auto_disarm) {
    this->setMode(DISARM_MODE);
    this->landing_tic = (struct timespec){0, 0};

  } else if (this->landing_target.isTargetLosted()) {
    // transition back to discovery mode
    LOG_INFO("Landing Target is losted!");
    LOG_INFO("Transitioning back to [DISCOVER_MODE]!");
    this->setMode(DISCOVER_MODE);
    this->landing_tic = (struct timespec){0, 0};
    this->hover_position(2) = this->recover_height;
  }

  return 0;
}

int Quadrotor::stepWaypointMode(const double dt) {
  int retval;

  // pre-check
  if (this->configured == false) {
    return -1;
  } else if (this->home_set == false) {
    return -2;
  }

  // convert mission waypoints to local frame
  if (this->mission.local_waypoints.size() == 0) {
    retval = this->mission.setHomePoint(this->home_lat, this->home_lon);
    if (retval != 0) {
      return -3;
    }
  }

  // hover at first waypoint
  if (this->wp_mission_ready == false) {
    const Vec3 wp_start = this->mission.local_waypoints[0];
    this->setHoverPosition(wp_start);
    this->stepHoverMode(dt);
    this->yaw_setpoint = this->mission.waypointHeading();

    // check if quadrotor is at first waypoint
    const double wp_threshold = this->mission.threshold_waypoint_reached;
    if ((this->pose.position - wp_start).norm() < wp_threshold) {
      LOG_INFO("Quadrotor arrived at first waypoint!");
      LOG_INFO("Waypoing mission in 5s!");
      this->wp_mission_ready = true;
      tic(&this->waypoint_tic);
    }

  } else if (this->wp_mission_ready && toc(&this->waypoint_tic) <= 5.0) {
    // hover at first waypoint and do a 5 second count down
    const Vec3 wp_start = this->mission.local_waypoints[0];
    this->setHoverPosition(wp_start);
    this->stepHoverMode(dt);
    this->yaw_setpoint = this->mission.waypointHeading();

    // count down
    float wp_clock = toc(&this->waypoint_tic);
    if (wp_clock > 1.0 && this->waypoint_countdown[0] == false) {
      LOG_INFO("... 4s");
      this->waypoint_countdown[0] = true;
    } else if (wp_clock > 2.0 && this->waypoint_countdown[1] == false) {
      LOG_INFO("... 3s");
      this->waypoint_countdown[1] = true;
    } else if (wp_clock > 3.0 && this->waypoint_countdown[2] == false) {
      LOG_INFO("... 2s");
      this->waypoint_countdown[2] = true;
    } else if (wp_clock > 4.0 && this->waypoint_countdown[3] == false) {
      LOG_INFO("... 1s");
      LOG_INFO("Executing waypoint mission!");
      this->waypoint_countdown[3] = true;
    }

  } else if (this->wp_mission_ready && toc(&this->waypoint_tic) > 5.0) {
    // travel through waypoints
    retval = this->waypoint_controller.update(this->mission,
                                              this->pose,
                                              this->velocity,
                                              dt);
    this->att_cmd = AttitudeCommand(this->waypoint_controller.outputs);
    this->yaw_setpoint = this->att_cmd.rpy(2);
  }

  // update hover position
  this->setHoverPosition(this->pose.position);

  // check conditions
  if (retval == -1) {
    // transition to hover mode
    LOG_ERROR("Failed to load mission!");
    LOG_INFO("Transitioning to [HOVER MODE]!");
    this->setMode(HOVER_MODE);

  } else if (retval == -2) {
    // transition to hover mode
    LOG_INFO("Mission complete!");
    LOG_INFO("Transitioning to [HOVER MODE]!");
    this->setMode(HOVER_MODE);
  }

  return 0;
}

int Quadrotor::reset() {
  this->position_controller.reset();
  this->tracking_controller.reset();
  this->landing_controller.reset();
  this->waypoint_controller.reset();

  return 0;
}

int Quadrotor::step(const double dt) {
  // pre-check
  if (this->configured == false) {
    return -1;
  }

  // step
  int retval;
  switch (this->current_mode) {
    case DISARM_MODE: retval = 0; break;
    case HOVER_MODE: retval = this->stepHoverMode(dt); break;
    case DISCOVER_MODE: retval = this->stepDiscoverMode(dt); break;
    case TRACKING_MODE: retval = this->stepTrackingMode(dt); break;
    case LANDING_MODE: retval = this->stepLandingMode(dt); break;
    case WAYPOINT_MODE: retval = this->stepWaypointMode(dt); break;
    default:
      LOG_ERROR(EINVMODE);
      retval = this->stepHoverMode(dt);
      break;
  }

  return retval;
}

} // namespace atl
