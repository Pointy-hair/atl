#ifndef ATL_GAZEBO_KINEMATICS_GIMBAL_HPP
#define ATL_GAZEBO_KINEMATICS_GIMBAL_HPP

#include <float.h>
#include <iomanip>
#include <iostream>

#include "atl/control/pid.hpp"
#include "atl/data/data.hpp"
#include "atl/utils/utils.hpp"

namespace atl {

/**
 * Attitude Controller for a 2-Axis gimbal, this controller implements 2 PID
 * controllers for both roll and pitch and has a fixed control rate of 100Hz.
 */
class Gimbal2AxisController {
public:
  PID roll_controller;
  PID pitch_controller;
  double dt;
  Vec2 outputs;

  Gimbal2AxisController()
      : roll_controller{0.3, 0.0, 0.2}, pitch_controller{0.3, 0.0, 0.2},
        dt{0.0}, outputs{0.0, 0.0} {}

  /**
   * Update gimbal attitude controller.
   *
   * @param setpoints, actual Vectors of size 2 with roll and pitch setpoints
   *  and actual attitudes in radians.
   * @param dt Update time step in seconds
   *
   * @returns motor inputs for roll and pitch as a vector of size 2 for a
   * 2-axis gimbal.
   *
   * The update rate is capped at 100 Hz. That is, each call to `update`
   * increments an internal counter by `dt`. If the counter reaches 0.001, it
   * is reset and the controller is updated. Otherwise, the arguments other
   * than `dt` are ignored.
   */
  Vec2 update(Vec2 setpoints, Vec2 actual, double dt);
};

/**
 * 2-axis Gimbal model with PID attitude controllers
 *
 * This model has the following assumptions:
 *
 * - gimbal motion is linear
 * - gimbal is downward facing (origin is downwards)
 * - no friction
 * - no gimbal lock protection
 * - running at a rate of 100hz
 */
class Gimbal2AxisModel {
public:
  Vec4 states;

  double Ix;
  double Iy;
  Pose camera_offset;

  Vec2 joint_setpoints;
  Gimbal2AxisController joint_controller;

  Quaternion frame_orientation;
  Quaternion joint_orientation;
  Vec2 target_attitude_W;

  Gimbal2AxisModel()
      : states{0.0, 0.0, 0.0, 0.0}, Ix{0.01}, Iy{0.01},
        camera_offset{BODY_FRAME, 0.0, deg2rad(90.0), 0.0, 0.0, 0.0, 0.0},
        joint_setpoints{0.0, 0.0}, joint_controller{}, frame_orientation{},
        joint_orientation{}, target_attitude_W{0.0, 0.0} {}

  Gimbal2AxisModel(Vec4 pose)
      : states{pose}, Ix{0.01}, Iy{0.01},
        camera_offset{BODY_FRAME, 0.0, deg2rad(90.0), 0.0, 0.0, 0.0, 0.0},
        joint_setpoints{0.0, 0.0}, joint_controller{}, frame_orientation{},
        joint_orientation{}, target_attitude_W{0.0, 0.0} {}

  /**
   * Update gimbal model
   *
   * @param motor_inputs Vector of roll and pitch in radians
   * @param dt Simulation time step or time since the model was last updated,
   *  in seconds
   */
  void update(Vec2 motor_inputs, double dt);

  /**
   * Update gimbal attitude controller
   *
   * @param dt Time step or time since the model was last updated, in seconds
   */
  Vec2 attitudeControllerControl(double dt);

  /**
   * Set gimbal frame orientation
   *
   * @param frame_W Frame orientation in inertial frame (NWU coordinate
   * system)
   */
  void setFrameOrientation(Quaternion frame_W);

  /**
   * Set gimbal joint attitude
   *
   * @param euler_W Gimbal attitude in inertial frame (NWU coordinate system)
   */
  void setAttitude(Vec2 euler_W);

  /**
   * Obtain gimbal target in body frame
   *
   * @param target_C target in camera frame (EDN coordinate system)
   */
  Vec3 getTargetInBF(Vec3 target_C);

  /**
   * Obtain gimbal target in body planar frame
   *
   * @param target_C Target in camera frame (EDN coordinate system)
   * @param body_W Gimbal body in inertial frame (NWU coordinate system)
   * @param joint_B Gimbal body in body frame (NWU coordinate system)
   */
  Vec3 getTargetInBPF(Vec3 target_C, Quaternion body_W, Quaternion joint_B);

  /**
   * Track target
   *
   * @param target_C Target in camera frame (EDN coordinate system)
   */
  void trackTarget(Vec3 target_C);

  /**
   * Obtain gimbal state
   *
   * The state vector is:
   *
   * - roll
   * - roll velocity
   * - pitch
   * - pitch velocity
   *
   * as a vector of size 4.
   */
  Vec4 getState();

  /** Print gimbal state */
  void printState();
};

} // namespace atl

#endif // ATL_GAZEBO_KINEMATICS_GIMBAL_HPP
