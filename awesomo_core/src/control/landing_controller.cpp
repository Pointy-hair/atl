#include "awesomo_core/control/landing_controller.hpp"


namespace awesomo {

// TRAJECTORY
Trajectory::Trajectory(void) {
  this->loaded = false;
  this->pos.clear();
  this->vel.clear();
  this->inputs.clear();
  this->target_bf.clear();
  this->p0 << 0.0, 0.0, 0.0;
}

int Trajectory::load(std::string filepath, Vec3 p0) {
  MatX traj_data;
  Vec2 p, v, u, target_bf;

  // pre-check
  if (file_exists(filepath) == false) {
    log_err("File not found: %s", filepath.c_str());
    return -1;
  }

  // load trajectory file
  // assumes each column is:
  // - x
  // - vx
  // - z
  // - vz
  // - az
  // - theta
  // - target_bf_x
  // - target_bf_z
  this->reset();
  csv2mat(filepath, true, traj_data);
  if (traj_data.rows() == 0) {
    log_err(ETROWS, filepath.c_str());
    return -2;
  } else if (traj_data.cols() != 8) {
    log_err(ETCOLS, filepath.c_str());
    return -2;
  }

  // set trajectory class
  for (int i = 0; i < traj_data.rows(); i++) {
    p << traj_data(i, 0), traj_data(i, 2);
    v << traj_data(i, 1), traj_data(i, 3);
    u << traj_data(i, 4), traj_data(i, 5);
    target_bf << traj_data(i, 6), traj_data(i, 7);

    this->pos.push_back(p);
    this->vel.push_back(v);
    this->inputs.push_back(u);
    this->target_bf.push_back(target_bf);
  }

  this->p0 = p0;
  this->loaded = true;
  return 0;
}

int Trajectory::update(Vec3 pos, Vec2 &wp_pos, Vec2 &wp_vel, Vec2 &q_pos) {
  int retval;
  Vec2 wp_pos_start, wp_pos_end, wp_pos_last;

  // pre-check
  if (this->loaded == false) {
    return -1;
  } else if (this->pos.size() < 2) {
    wp_pos = this->pos.at(0);
    wp_vel = this->vel.at(0);
    return 0;
  }

  // setup
  wp_pos_start = this->pos.at(0);
  wp_pos_end = this->pos.at(1);
  wp_pos_last = this->pos.back();
  q_pos(0) = (this->p0.block(0, 0, 2, 1) - pos.block(0, 0, 2, 1)).norm();
  q_pos(1) = pos(2);

  // find next waypoint position and velocity
  retval = closest_point(wp_pos_start, wp_pos_end, q_pos, wp_pos);
  wp_vel = this->vel.at(1);

  // update trajectory waypoints
  if (retval == 2) {
    this->pos.pop_front();
    this->vel.pop_front();
    this->inputs.pop_front();
    this->target_bf.pop_front();
  }

  // std::cout << "pos: " << pos.transpose() << std::endl;
  std::cout << "p0: " << this->p0.block(0, 0, 2, 1).transpose() << std::endl;
  std::cout << "q_pos: " << q_pos.transpose() << std::endl;
  // std::cout << "wp_start: " << wp_pos_start.transpose() << std::endl;
  // std::cout << "wp_end: " << wp_pos_end.transpose() << std::endl;
  // std::cout << "wp_last: " << wp_pos_last.transpose() << std::endl;
  // std::cout << "wp_vel: " << wp_vel.transpose() << std::endl;
  std::cout << "wp: " << wp_pos.transpose() << std::endl;
  std::cout << "target_bf: " << this->target_bf.at(0).transpose() << std::endl;
  // std::cout << "waypoints: " << this->pos.size() << std::endl;
  std::cout << std::endl;

  return 0;
}

void Trajectory::reset(void) {
  this->loaded = false;
  this->pos.clear();
  this->vel.clear();
  this->inputs.clear();
  this->target_bf.clear();
  this->p0 << 0.0, 0.0, 0.0;
}


// TRAJECTORY INDEX
TrajectoryIndex::TrajectoryIndex(void) {
  this->loaded = false;

  this->traj_dir = "";
  this->index_data = MatX();
  this->pos_thres = 0.0;
  this->vel_thres = 0.0;
}

int TrajectoryIndex::load(std::string index_file,
                          double pos_thres,
                          double vel_thres) {
  // pre-check
  if (file_exists(index_file) == false) {
    log_err("File not found: %s", index_file.c_str());
    return -1;
  }

  // load trajectory index
  // assumes each column is: (index, p0_x, p0_z, pf_x, pf_z, z)
  csv2mat(index_file, true, this->index_data);
  this->traj_dir = std::string(dirname((char *) index_file.c_str()));
  this->pos_thres = pos_thres;
  this->vel_thres = vel_thres;

  if (this->index_data.rows() == 0) {
    log_err(ETIROWS, index_file.c_str());
    return -2;
  } else if (this->index_data.cols() != 3) {
    log_err(ETICOLS, index_file.c_str());
    return -2;
  }

  this->loaded = true;
  return 0;
}

int TrajectoryIndex::find(Vec3 pos, double v, Trajectory &traj) {
  bool p_ok, v_ok;
  std::vector<int> matches;
  std::string traj_file;

  // pre-check
  if (this->loaded == false) {
    return -1;
  }

  // NOTE: the following is not the most efficient way of implementing a lookup
  // table, a better way could involve a search tree and traverse it or even a
  // bucket based approach. The following implements a list traversal type
  // search which is approx O(n), ok for small lookups.

  // find rows in the index that have same approx
  // start height (z) and velocity (v)
  for (int i = 0; i < this->index_data.rows(); i++) {
    p_ok = fabs(pos(2) - this->index_data(i, 1)) < this->pos_thres;
    v_ok = fabs(v - this->index_data(i, 2)) < this->vel_thres;

    if (p_ok && v_ok) {
      matches.push_back(i);
    }
  }

  // check number of matches
  if (matches.size() == 0) {
    return -2;  // found no trajectory
  }

  // load trajectory
  traj_file = this->traj_dir + "/";
  traj_file += std::to_string((int) matches[0]) + ".csv";
  if (traj.load(traj_file, pos) != 0) {
    return -3;
  }

  return 0;
}


// LANDING CONTROLLER
LandingController::LandingController(void) {
  this->configured = false;

  this->pctrl_dt = 0.0;
  this->vctrl_dt = 0.0;
  this->blackbox_dt = 0.0;

  this->x_controller = PID(0.0, 0.0, 0.0);
  this->y_controller = PID(0.0, 0.0, 0.0);
  this->z_controller = PID(0.0, 0.0, 0.0);
  this->hover_throttle = 0.0;

  this->vx_controller = PID(0.0, 0.0, 0.0);
  this->vy_controller = PID(0.0, 0.0, 0.0);
  this->vz_controller = PID(0.0, 0.0, 0.0);

  this->roll_limit[0] = 0.0;
  this->roll_limit[1] = 0.0;
  this->pitch_limit[0] = 0.0;
  this->pitch_limit[1] = 0.0;
  this->throttle_limit[0] = 0.0;
  this->throttle_limit[1] = 0.0;

  this->pctrl_setpoints << 0.0, 0.0, 0.0;
  this->pctrl_outputs << 0.0, 0.0, 0.0, 0.0;
  this->vctrl_setpoints << 0.0, 0.0, 0.0;
  this->vctrl_outputs << 0.0, 0.0, 0.0, 0.0;
  this->att_cmd = AttitudeCommand();

  this->traj_index;
  this->trajectory;

  this->blackbox_enable = false;
  this->blackbox_rate = FLT_MAX;
  this->blackbox;
}

LandingController::~LandingController(void) {
  if (this->blackbox) {
    this->blackbox.close();
  }
}

int LandingController::configure(std::string config_file) {
  ConfigParser parser;
  std::string config_dir;
  std::string traj_index_file;
  std::string blackbox_file;

  // load config
  // clang-format off
  parser.addParam<double>("roll_controller.k_p", &this->y_controller.k_p);
  parser.addParam<double>("roll_controller.k_i", &this->y_controller.k_i);
  parser.addParam<double>("roll_controller.k_d", &this->y_controller.k_d);

  parser.addParam<double>("pitch_controller.k_p", &this->x_controller.k_p);
  parser.addParam<double>("pitch_controller.k_i", &this->x_controller.k_i);
  parser.addParam<double>("pitch_controller.k_d", &this->x_controller.k_d);

  parser.addParam<double>("throttle_controller.k_p", &this->z_controller.k_p);
  parser.addParam<double>("throttle_controller.k_i", &this->z_controller.k_i);
  parser.addParam<double>("throttle_controller.k_d", &this->z_controller.k_d);
  parser.addParam<double>("throttle_controller.hover_throttle", &this->hover_throttle);

  parser.addParam<double>("vx_controller.k_p", &this->vx_controller.k_p);
  parser.addParam<double>("vx_controller.k_i", &this->vx_controller.k_i);
  parser.addParam<double>("vx_controller.k_d", &this->vx_controller.k_d);

  parser.addParam<double>("vy_controller.k_p", &this->vy_controller.k_p);
  parser.addParam<double>("vy_controller.k_i", &this->vy_controller.k_i);
  parser.addParam<double>("vy_controller.k_d", &this->vy_controller.k_d);

  parser.addParam<double>("vz_controller.k_p", &this->vz_controller.k_p);
  parser.addParam<double>("vz_controller.k_i", &this->vz_controller.k_i);
  parser.addParam<double>("vz_controller.k_d", &this->vz_controller.k_d);

  parser.addParam<double>("roll_limit.min", &this->roll_limit[0]);
  parser.addParam<double>("roll_limit.max", &this->roll_limit[1]);

  parser.addParam<double>("pitch_limit.min", &this->pitch_limit[0]);
  parser.addParam<double>("pitch_limit.max", &this->pitch_limit[1]);

  parser.addParam<double>("throttle_limit.min", &this->throttle_limit[0]);
  parser.addParam<double>("throttle_limit.max", &this->throttle_limit[1]);

  parser.addParam<std::string>("trajectory_index", &traj_index_file);
  parser.addParam<std::string>("blackbox_file", &blackbox_file);
  // clang-format on
  if (parser.load(config_file) != 0) {
    return -1;
  }

  // load trajectory index
  config_dir = std::string(dirname((char *) config_file.c_str()));
  paths_combine(config_dir, traj_index_file, traj_index_file);
  if (this->traj_index.load(traj_index_file) != 0) {
    return -2;
  }

  // prepare blackbox file
  if (this->prepBlackbox(blackbox_file) != 0) {
    log_err("Failed to open blackbox file at [%s]", blackbox_file.c_str());
    return -3;
  }

  // convert roll and pitch limits from degrees to radians
  this->roll_limit[0] = deg2rad(this->roll_limit[0]);
  this->roll_limit[1] = deg2rad(this->roll_limit[1]);
  this->pitch_limit[0] = deg2rad(this->pitch_limit[0]);
  this->pitch_limit[1] = deg2rad(this->pitch_limit[1]);

  // initialize setpoints to zero
  this->pctrl_setpoints << 0.0, 0.0, 0.0;
  this->pctrl_outputs << 0.0, 0.0, 0.0, 0.0;
  this->vctrl_setpoints << 0.0, 0.0, 0.0;
  this->vctrl_outputs << 0.0, 0.0, 0.0, 0.0;

  this->configured = true;
  return 0;
}

int LandingController::loadTrajectory(Vec3 pos,
                                      Vec3 target_pos_bf,
                                      double v) {
  int retval;

  // find trajectory
  retval = this->traj_index.find(pos, v, this->trajectory);

  // check retval
  if (retval == -2) {
    log_err(ETIFAIL, pos(2), v);
    return -1;
  } else if (retval == -3) {
    log_err(ETLOAD);
    return -1;
  } else {
    log_info(TLOAD, pos(2), v);
  }

  return 0;
}

int LandingController::prepBlackbox(std::string blackbox_file) {
  // setup
  this->blackbox.open(blackbox_file);
  if (!this->blackbox) {
    return -1;
  }

  // write header
  this->blackbox << "dt" << ",";
  this->blackbox << "x" << ",";
  this->blackbox << "y" << ",";
  this->blackbox << "z" << ",";
  this->blackbox << "vx" << ",";
  this->blackbox << "vy" << ",";
  this->blackbox << "vz" << ",";
  this->blackbox << "wp_pos_x" << ",";
  this->blackbox << "wp_pos_y" << ",";
  this->blackbox << "wp_pos_z" << ",";
  this->blackbox << "wp_vel_x" << ",";
  this->blackbox << "wp_vel_y" << ",";
  this->blackbox << "wp_vel_z" << ",";
  this->blackbox << "target_pos_bf_x" << ",";
  this->blackbox << "target_pos_bf_y" << ",";
  this->blackbox << "target_pos_bf_z" << ",";
  this->blackbox << "target_vel_bf_x" << ",";
  this->blackbox << "target_vel_bf_y" << ",";
  this->blackbox << "target_vel_bf_z" << ",";
  this->blackbox << std::endl;

  return 0;
}

int LandingController::record(Vec3 pos,
                              Vec3 vel,
                              Vec2 wp_pos,
                              Vec2 wp_vel,
                              Vec3 target_pos_bf,
                              Vec3 target_vel_bf,
                              double dt) {
  // pre-check
  this->blackbox_dt += dt;
  if (this->blackbox_enable && this->blackbox_dt > this->blackbox_rate) {
    return 0;
  }

  // record
  this->blackbox << dt << ",";
  this->blackbox << pos(0) << ",";
  this->blackbox << pos(1) << ",";
  this->blackbox << pos(2) << ",";
  this->blackbox << vel(0) << ",";
  this->blackbox << vel(1) << ",";
  this->blackbox << vel(2) << ",";
  this->blackbox << wp_pos(0) << ",";
  this->blackbox << wp_pos(1) << ",";
  this->blackbox << wp_vel(0) << ",";
  this->blackbox << wp_vel(1) << ",";
  this->blackbox << target_pos_bf(0) << ",";
  this->blackbox << target_pos_bf(1) << ",";
  this->blackbox << target_pos_bf(2) << ",";
  this->blackbox << target_vel_bf(0) << ",";
  this->blackbox << target_vel_bf(1) << ",";
  this->blackbox << target_vel_bf(2) << ",";
  this->blackbox << std::endl;
  this->blackbox_dt = 0.0;

  return 0;
}

Vec4 LandingController::calculatePositionErrors(Vec3 errors,
                                                double yaw,
                                                double dt) {
  double r, p, y, t;
  Vec3 euler;
  Mat3 R;

  // check rate
  this->pctrl_dt += dt;
  if (this->pctrl_dt < 0.01) {
    return this->pctrl_outputs;
  }

  // roll, pitch, yaw and throttle (assuming NWU frame)
  // clang-format off
  r = -this->y_controller.calculate(errors(1), 0.0, this->pctrl_dt);
  p = this->x_controller.calculate(errors(0), 0.0, this->pctrl_dt);
  y = yaw;
  t = this->hover_throttle + this->z_controller.calculate(errors(2), 0.0, this->pctrl_dt);
  t /= fabs(cos(r) * cos(p));  // adjust throttle for roll and pitch
  // clang-format o

  // limit roll, pitch
  r = (r < this->roll_limit[0]) ? this->roll_limit[0] : r;
  r = (r > this->roll_limit[1]) ? this->roll_limit[1] : r;
  p = (p < this->pitch_limit[0]) ? this->pitch_limit[0] : p;
  p = (p > this->pitch_limit[1]) ? this->pitch_limit[1] : p;

  // limit throttle
  t = (t < 0) ? 0.0 : t;
  t = (t > 1.0) ? 1.0 : t;

  // keep track of setpoints and outputs
  this->pctrl_setpoints = errors;
  this->pctrl_outputs << r, p, y, t;
  this->pctrl_dt = 0.0;

  return pctrl_outputs;
}

Vec4 LandingController::calculateVelocityErrors(Vec3 errors,
                                                double yaw,
                                                double dt) {
  double r, p, y, t;
  Vec3 euler;
  Mat3 R;

  // check rate
  this->vctrl_dt += dt;
  if (this->vctrl_dt < 0.01) {
    return this->vctrl_outputs;
  }

  // roll, pitch, yaw and throttle (assuming NWU frame)
  // clang-format off
  r = -this->vy_controller.calculate(errors(1), 0.0, this->vctrl_dt);
  p = this->vx_controller.calculate(errors(0), 0.0, this->vctrl_dt);
  y = 0.0;
  t = this->vz_controller.calculate(errors(2), 0.0, this->vctrl_dt);
  t /= fabs(cos(r) * cos(p));  // adjust throttle for roll and pitch
  // clang-format on

  // limit roll, pitch, throttle
  r = (r < this->roll_limit[0]) ? this->roll_limit[0] : r;
  r = (r > this->roll_limit[1]) ? this->roll_limit[1] : r;
  p = (p < this->pitch_limit[0]) ? this->pitch_limit[0] : p;
  p = (p > this->pitch_limit[1]) ? this->pitch_limit[1] : p;
  t = (t < this->throttle_limit[0]) ? this->throttle_limit[0] : t;
  t = (t > this->throttle_limit[1]) ? this->throttle_limit[1] : t;

  // keep track of setpoints and outputs
  this->vctrl_setpoints = errors;
  this->vctrl_outputs << r, p, y, t;
  this->vctrl_dt = 0.0;

  return this->vctrl_outputs;
}

AttitudeCommand LandingController::calculate(Vec3 pos_errors,
                                             Vec3 vel_errors,
                                             double yaw,
                                             double dt) {
  this->calculatePositionErrors(pos_errors, yaw, dt);
  this->calculateVelocityErrors(vel_errors, yaw, dt);
  this->att_cmd = AttitudeCommand(this->pctrl_outputs + this->vctrl_outputs);
  return this->att_cmd;
}

AttitudeCommand LandingController::calculate(Vec3 target_pos_bf,
                                             Vec3 target_vel_bf,
                                             Vec3 pos,
                                             Vec3 pos_prev,
                                             double yaw,
                                             double dt) {
  int retval;
  Vec3 vel, perrors, verrors;
  Vec2 wp_pos, wp_pos_last, wp_vel, q_pos;

  // calculate velocity
  vel(0) = (pos(0) - pos_prev(0)) / dt;
  vel(1) = (pos(1) - pos_prev(1)) / dt;
  vel(2) = (pos(2) - pos_prev(2)) / dt;

  // obtain position and velocity waypoints
  retval = this->trajectory.update(pos, wp_pos, wp_vel, q_pos);

  // calculate position and velocity errors
  // perrors(0) = wp_pos(0) - q_pos(0);
  perrors(0) = this->trajectory.target_bf.at(0)(0) + target_pos_bf(0);
  perrors(1) = target_pos_bf(1);
  perrors(2) = wp_pos(1) - q_pos(1);

  verrors(0) = wp_vel(0) - (vel.block(0, 0, 2, 1).norm());
  verrors(1) = target_vel_bf(1);
  verrors(2) = wp_vel(1) - vel(2);

  // calculate control outputs and record
  this->calculate(perrors, verrors, yaw, dt);
  this->record(pos, vel, wp_pos, wp_vel, target_pos_bf, target_vel_bf, dt);

  return this->att_cmd;
}

void LandingController::reset(void) {
  this->x_controller.reset();
  this->y_controller.reset();
  this->z_controller.reset();

  this->vx_controller.reset();
  this->vy_controller.reset();
  this->vz_controller.reset();
}

void LandingController::printOutputs(void) {
  double r, p, t;

  r = rad2deg(this->pctrl_outputs(0));
  p = rad2deg(this->pctrl_outputs(1));
  t = this->pctrl_outputs(3);

  std::cout << "roll: " << std::setprecision(2) << r << "\t";
  std::cout << "pitch: " << std::setprecision(2) << p << "\t";
  std::cout << "throttle: " << std::setprecision(2) << t << std::endl;
}

void LandingController::printErrors(void) {
  double p, i, d;

  p = this->x_controller.error_p;
  i = this->x_controller.error_i;
  d = this->x_controller.error_d;

  std::cout << "x_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << i << std::endl;

  p = this->y_controller.error_p;
  i = this->y_controller.error_i;
  d = this->y_controller.error_d;

  std::cout << "y_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << i << std::endl;

  p = this->z_controller.error_p;
  i = this->z_controller.error_i;
  d = this->z_controller.error_d;

  std::cout << "z_controller: " << std::endl;
  std::cout << "\terror_p: " << std::setprecision(2) << p << "\t";
  std::cout << "\terror_i: " << std::setprecision(2) << i << "\t";
  std::cout << "\terror_d: " << std::setprecision(2) << i << std::endl;
}

}  // end of awesomo namespace