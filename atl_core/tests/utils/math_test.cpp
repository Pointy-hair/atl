#include "atl/utils/math.hpp"
#include "atl/atl_test.hpp"

namespace atl {

TEST(Math, median) {
  std::vector<double> v;

  v.push_back(6);
  v.push_back(3);
  v.push_back(4);
  v.push_back(1);
  v.push_back(5);
  v.push_back(8);

  EXPECT_FLOAT_EQ(4.5, median(v));

  v.push_back(9);
  EXPECT_FLOAT_EQ(5.0, median(v));
}

TEST(Math, deg2radAndrad2deg) {
  double d_deg;
  double d_rad;

  d_deg = 10;
  d_rad = deg2rad(d_deg);
  EXPECT_FLOAT_EQ(d_deg, rad2deg(d_rad));
}

TEST(Math, euler321ToQuat) {
  float roll;
  float pitch;
  float yaw;
  Vec3 euler;
  Quaternion q;

  // check identity quat is returned
  roll = 0;
  pitch = 0;
  yaw = 0;

  euler << roll, pitch, yaw;
  q = euler321ToQuat(euler);
  EXPECT_FLOAT_EQ(0.0, q.x());
  EXPECT_FLOAT_EQ(0.0, q.y());
  EXPECT_FLOAT_EQ(0.0, q.z());
  EXPECT_FLOAT_EQ(1.0, q.w());

  // check valid quat is returned
  roll = M_PI / 2;
  pitch = M_PI;
  yaw = -M_PI / 2;

  euler << roll, pitch, yaw;
  q = euler321ToQuat(euler);
  EXPECT_FLOAT_EQ(0.5, q.x());
  EXPECT_FLOAT_EQ(0.5, q.y());
  EXPECT_FLOAT_EQ(-0.5, q.z());
  EXPECT_FLOAT_EQ(-0.5, q.w());
}

TEST(Math, sandbox) {
  double roll = deg2rad(10.0);
  double pitch = deg2rad(20.0);
  double yaw = deg2rad(30.0);
  Vec3 euler{roll, pitch, yaw};

  // NWU
  Quaternion q_nwu;
  q_nwu = euler321ToQuat(euler);

  // NWU to NED
  Quaternion q_ned = nwu2ned(q_nwu);
  euler = quatToEuler321(q_ned);
  EXPECT_FLOAT_EQ(10, rad2deg(euler(0)));
  EXPECT_FLOAT_EQ(-20, rad2deg(euler(1)));
  EXPECT_FLOAT_EQ(-30, rad2deg(euler(2)));

  // NED to NWU
  q_nwu = ned2nwu(q_ned);
  euler = quatToEuler321(q_nwu);
  EXPECT_FLOAT_EQ(10, rad2deg(euler(0)));
  EXPECT_FLOAT_EQ(20, rad2deg(euler(1)));
  EXPECT_FLOAT_EQ(30, rad2deg(euler(2)));
}

TEST(Math, euler321ToRot) {
  // test roll, pitch, yaw set to 0
  const double roll = 0.0;
  const double pitch = 0.0;
  const double yaw = 0.0;
  const Vec3 euler{roll, pitch, yaw};
  const Mat3 rot = euler321ToRot(euler);

  const double r01 = 1.0;
  const double r02 = 0.0;
  const double r03 = 0.0;

  const double r11 = 0.0;
  const double r12 = 1.0;
  const double r13 = 0.0;

  const double r21 = 0.0;
  const double r22 = 0.0;
  const double r23 = 1.0;

  EXPECT_FLOAT_EQ(r01, rot(0, 0));
  EXPECT_FLOAT_EQ(r02, rot(0, 1));
  EXPECT_FLOAT_EQ(r03, rot(0, 2));

  EXPECT_FLOAT_EQ(r11, rot(1, 0));
  EXPECT_FLOAT_EQ(r12, rot(1, 1));
  EXPECT_FLOAT_EQ(r13, rot(1, 2));

  EXPECT_FLOAT_EQ(r21, rot(2, 0));
  EXPECT_FLOAT_EQ(r22, rot(2, 1));
  EXPECT_FLOAT_EQ(r23, rot(2, 2));
}

TEST(Math, enu2nwu) {
  Vec3 enu{1.0, 2.0, 3.0};
  Vec3 nwu = enu2nwu(enu);

  EXPECT_FLOAT_EQ(2.0, nwu(0));
  EXPECT_FLOAT_EQ(-1.0, nwu(1));
  EXPECT_FLOAT_EQ(3.0, nwu(2));
}

TEST(Math, nwu2enu) {
  Vec3 nwu{1.0, 2.0, 3.0};
  Vec3 enu = nwu2enu(nwu);

  EXPECT_FLOAT_EQ(-2.0, enu(0));
  EXPECT_FLOAT_EQ(1.0, enu(1));
  EXPECT_FLOAT_EQ(3.0, enu(2));
}

TEST(Math, edn2nwu) {
  Vec3 edn{1.0, 2.0, 3.0};
  Vec3 enu = edn2nwu(edn);

  EXPECT_FLOAT_EQ(3.0, enu(0));
  EXPECT_FLOAT_EQ(-1.0, enu(1));
  EXPECT_FLOAT_EQ(-2.0, enu(2));
}

TEST(Math, target2body) {
  Vec3 target_pos_if;
  Vec3 body_pos_if;
  Vec3 euler;
  Quaternion quat;
  Vec3 target_pos_bf;

  // setup
  target_pos_if << 2.0, 1.0, 0.0;
  body_pos_if << 1.0, 2.0, 0.0;

  // TEST EULER VERSION OF target2body()
  // test 0 degree
  euler << 0.0, 0.0, deg2rad(0.0);
  target2body(target_pos_if, body_pos_if, euler, target_pos_bf);
  std::cout << target_pos_bf.transpose() << std::endl;

  EXPECT_FLOAT_EQ(1.0, target_pos_bf(0));
  EXPECT_FLOAT_EQ(-1.0, target_pos_bf(1));
  EXPECT_FLOAT_EQ(0.0, target_pos_bf(2));

  // test 90 degree
  euler << 0.0, 0.0, deg2rad(90.0);
  target2body(target_pos_if, body_pos_if, euler, target_pos_bf);
  std::cout << target_pos_bf.transpose() << std::endl;

  EXPECT_FLOAT_EQ(-1.0, target_pos_bf(0));
  EXPECT_FLOAT_EQ(-1.0, target_pos_bf(1));
  EXPECT_FLOAT_EQ(0.0, target_pos_bf(2));

  // test 180 degree
  euler << 0.0, 0.0, deg2rad(180.0);
  target2body(target_pos_if, body_pos_if, euler, target_pos_bf);
  std::cout << target_pos_bf.transpose() << std::endl;

  EXPECT_FLOAT_EQ(-1.0, target_pos_bf(0));
  EXPECT_FLOAT_EQ(1.0, target_pos_bf(1));
  EXPECT_FLOAT_EQ(0.0, target_pos_bf(2));

  // test 270 degree
  euler << 0.0, 0.0, deg2rad(270.0);
  target2body(target_pos_if, body_pos_if, euler, target_pos_bf);
  std::cout << target_pos_bf.transpose() << std::endl;

  EXPECT_FLOAT_EQ(1.0, target_pos_bf(0));
  EXPECT_FLOAT_EQ(1.0, target_pos_bf(1));
  EXPECT_FLOAT_EQ(0.0, target_pos_bf(2));

  // TEST QUATERNION VERSION OF target2body()
  // test 0 degree
  euler << 0.0, 0.0, deg2rad(0.0);
  quat = euler123ToQuat(euler);
  target2body(target_pos_if, body_pos_if, quat, target_pos_bf);

  EXPECT_FLOAT_EQ(1.0, target_pos_bf(0));
  EXPECT_FLOAT_EQ(-1.0, target_pos_bf(1));
  EXPECT_FLOAT_EQ(0.0, target_pos_bf(2));

  // test 90 degree
  euler << 0.0, 0.0, deg2rad(90.0);
  quat = euler123ToQuat(euler);
  target2body(target_pos_if, body_pos_if, quat, target_pos_bf);

  EXPECT_FLOAT_EQ(-1.0, target_pos_bf(0));
  EXPECT_FLOAT_EQ(-1.0, target_pos_bf(1));
  EXPECT_FLOAT_EQ(0.0, target_pos_bf(2));

  // test 180 degree
  euler << 0.0, 0.0, deg2rad(180.0);
  quat = euler123ToQuat(euler);
  target2body(target_pos_if, body_pos_if, quat, target_pos_bf);

  EXPECT_FLOAT_EQ(-1.0, target_pos_bf(0));
  EXPECT_FLOAT_EQ(1.0, target_pos_bf(1));
  EXPECT_FLOAT_EQ(0.0, target_pos_bf(2));

  // test 270 degree
  euler << 0.0, 0.0, deg2rad(270.0);
  quat = euler123ToQuat(euler);
  target2body(target_pos_if, body_pos_if, quat, target_pos_bf);

  EXPECT_FLOAT_EQ(1.0, target_pos_bf(0));
  EXPECT_FLOAT_EQ(1.0, target_pos_bf(1));
  EXPECT_FLOAT_EQ(0.0, target_pos_bf(2));
}

TEST(Math, target2bodyplanar) {
  Quaternion quat;
  Vec3 target_pos_bpf;

  // setup
  Vec3 target_if{2.0, 1.0, 0.0};
  Vec3 pos_if{1.0, 2.0, 0.0};
  Vec3 rpy{0.0, 0.0, 0.0};

  // // TEST EULER VERSION OF target2bodyplanar()
  // // test 0 degree
  // Vec3 rpy{0.0, 0.0, deg2rad(0.0)};
  // target2bodyplanar(target_if, pos_if, rpy, target_pos_bpf);
  // std::cout << target_pos_bpf.transpose() << std::endl;
  //
  // EXPECT_FLOAT_EQ(-1.0, target_pos_bpf(0));
  // EXPECT_FLOAT_EQ(-1.0, target_pos_bpf(1));
  // EXPECT_FLOAT_EQ(0.0, target_pos_bpf(2));
  //
  // // test 90 degree
  // rpy << 0.0, 0.0, deg2rad(90.0);
  // target2bodyplanar(target_if, pos_if, rpy, target_pos_bpf);
  // std::cout << target_pos_bpf.transpose() << std::endl;
  //
  // EXPECT_FLOAT_EQ(-1.0, target_pos_bpf(0));
  // EXPECT_FLOAT_EQ(1.0, target_pos_bpf(1));
  // EXPECT_FLOAT_EQ(0.0, target_pos_bpf(2));
  //
  // // test 180 degree
  // rpy << 0.0, 0.0, deg2rad(180.0);
  // target2bodyplanar(target_if, pos_if, rpy, target_pos_bpf);
  // std::cout << target_pos_bpf.transpose() << std::endl;
  //
  // EXPECT_FLOAT_EQ(1.0, target_pos_bpf(0));
  // EXPECT_FLOAT_EQ(1.0, target_pos_bpf(1));
  // EXPECT_FLOAT_EQ(0.0, target_pos_bpf(2));
  //
  // // test 270 degree
  // rpy << 0.0, 0.0, deg2rad(270.0);
  // target2bodyplanar(target_if, pos_if, rpy, target_pos_bpf);
  // std::cout << target_pos_bpf.transpose() << std::endl;
  //
  // EXPECT_FLOAT_EQ(1.0, target_pos_bpf(0));
  // EXPECT_FLOAT_EQ(-1.0, target_pos_bpf(1));
  // EXPECT_FLOAT_EQ(0.0, target_pos_bpf(2));

  // // TEST QUATERNION VERSION OF target2bodyplanar()
  // // test 0 degree
  // rpy << 0.0, 0.0, deg2rad(0.0);
  // quat = euler123ToQuat(rpy);
  // target2bodyplanar(target_if, pos_if, quat, target_pos_bpf);
  // std::cout << target_pos_bpf.transpose() << std::endl;
  //
  // EXPECT_FLOAT_EQ(-1.0, target_pos_bpf(0));
  // EXPECT_FLOAT_EQ(-1.0, target_pos_bpf(1));
  // EXPECT_FLOAT_EQ(0.0, target_pos_bpf(2));
  //
  // // test 90 degree
  // rpy << 0.0, 0.0, deg2rad(90.0);
  // quat = euler123ToQuat(rpy);
  // target2bodyplanar(target_if, pos_if, quat, target_pos_bpf);
  //
  // EXPECT_FLOAT_EQ(-1.0, target_pos_bpf(0));
  // EXPECT_FLOAT_EQ(1.0, target_pos_bpf(1));
  // EXPECT_FLOAT_EQ(0.0, target_pos_bpf(2));

  // test -90 degree
  rpy << 0.0, 0.0, deg2rad(-180.0);
  quat = euler123ToQuat(rpy);
  // quatToEuler(quat, 321, rpy);
  std::cout << rad2deg(rpy(2)) << std::endl;
  std::cout << quat.w() << std::endl;
  std::cout << quat.x() << std::endl;
  std::cout << quat.y() << std::endl;
  std::cout << quat.z() << std::endl;
  // target2bodyplanar(target_if, pos_if, quat, target_pos_bpf);
  // std::cout << target_pos_bpf.transpose() << std::endl;

  // EXPECT_FLOAT_EQ(1.0, target_pos_bpf(0));
  // EXPECT_FLOAT_EQ(1.0, target_pos_bpf(1));
  // EXPECT_FLOAT_EQ(0.0, target_pos_bpf(2));

  // // test 270 degree
  // rpy << 0.0, 0.0, deg2rad(270.0);
  // quat = euler123ToQuat(rpy);
  // target2bodyplanar(target_if, pos_if, quat, target_pos_bpf);
  //
  // EXPECT_FLOAT_EQ(1.0, target_pos_bpf(0));
  // EXPECT_FLOAT_EQ(-1.0, target_pos_bpf(1));
  // EXPECT_FLOAT_EQ(0.0, target_pos_bpf(2));
}

TEST(Math, target2inertial) {
  Vec3 target_pos_bf, target_pos_if, body_pos_if, euler;
  Quaternion quat;

  // setup
  target_pos_bf << 2.0, 1.0, 0.0;
  body_pos_if << 1.0, 2.0, 0.0;

  // TEST EULER VERSION OF target2inertial()
  // test 0 degree
  euler << 0.0, 0.0, deg2rad(0.0);
  target2inertial(target_pos_bf, body_pos_if, euler, target_pos_if);
  std::cout << target_pos_if.transpose() << std::endl;

  EXPECT_NEAR(3.0, target_pos_if(0), 0.001);
  EXPECT_NEAR(3.0, target_pos_if(1), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(2), 0.001);

  // test 90 degree
  euler << 0.0, 0.0, deg2rad(90.0);
  target2inertial(target_pos_bf, body_pos_if, euler, target_pos_if);
  std::cout << target_pos_if.transpose() << std::endl;

  EXPECT_NEAR(0.0, target_pos_if(0), 0.001);
  EXPECT_NEAR(4.0, target_pos_if(1), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(2), 0.001);

  // test 180 degree
  euler << 0.0, 0.0, deg2rad(180.0);
  target2inertial(target_pos_bf, body_pos_if, euler, target_pos_if);
  std::cout << target_pos_if.transpose() << std::endl;

  EXPECT_NEAR(-1.0, target_pos_if(0), 0.001);
  EXPECT_NEAR(1.0, target_pos_if(1), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(2), 0.001);

  // test -90 degree
  euler << 0.0, 0.0, deg2rad(-90.0);
  target2inertial(target_pos_bf, body_pos_if, euler, target_pos_if);
  std::cout << target_pos_if.transpose() << std::endl;

  EXPECT_NEAR(2.0, target_pos_if(0), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(1), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(2), 0.001);

  // TEST QUATERNION VERSION OF target2inertial()
  // test 0 degree
  euler << 0.0, 0.0, deg2rad(0.0);
  quat = euler321ToQuat(euler);
  target2inertial(target_pos_bf, body_pos_if, quat, target_pos_if);
  std::cout << target_pos_if.transpose() << std::endl;

  EXPECT_NEAR(3.0, target_pos_if(0), 0.001);
  EXPECT_NEAR(3.0, target_pos_if(1), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(2), 0.001);

  // test 90 degree
  euler << 0.0, 0.0, deg2rad(90.0);
  quat = euler321ToQuat(euler);
  target2inertial(target_pos_bf, body_pos_if, quat, target_pos_if);
  std::cout << target_pos_if.transpose() << std::endl;

  EXPECT_NEAR(0.0, target_pos_if(0), 0.001);
  EXPECT_NEAR(4.0, target_pos_if(1), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(2), 0.001);

  // test 180 degree
  euler << 0.0, 0.0, deg2rad(180.0);
  quat = euler321ToQuat(euler);
  target2inertial(target_pos_bf, body_pos_if, quat, target_pos_if);
  std::cout << target_pos_if.transpose() << std::endl;

  EXPECT_NEAR(-1.0, target_pos_if(0), 0.001);
  EXPECT_NEAR(1.0, target_pos_if(1), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(2), 0.001);

  // test 270 degree
  euler << 0.0, 0.0, deg2rad(270.0);
  quat = euler321ToQuat(euler);
  target2inertial(target_pos_bf, body_pos_if, quat, target_pos_if);
  std::cout << target_pos_if.transpose() << std::endl;

  EXPECT_NEAR(2.0, target_pos_if(0), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(1), 0.001);
  EXPECT_NEAR(0.0, target_pos_if(2), 0.001);
}

TEST(Math, wrapTo180) {
  double retval;

  // normal cases
  retval = wrapTo180(90.0);
  EXPECT_FLOAT_EQ(90.0, retval);

  retval = wrapTo180(180.0);
  EXPECT_FLOAT_EQ(-180.0, retval);

  retval = wrapTo180(270.0);
  EXPECT_FLOAT_EQ(-90.0, retval);

  retval = wrapTo180(360.0);
  EXPECT_FLOAT_EQ(0.0, retval);

  // edge cases
  retval = wrapTo180(-180.0);
  EXPECT_FLOAT_EQ(-180.0, retval);

  retval = wrapTo180(-90.0);
  EXPECT_FLOAT_EQ(-90.0, retval);

  retval = wrapTo180(450.0);
  EXPECT_FLOAT_EQ(90.0, retval);
}

TEST(Math, wrapTo360) {
  double retval;

  // normal cases
  retval = wrapTo360(90.0);
  EXPECT_FLOAT_EQ(90.0, retval);

  retval = wrapTo360(180.0);
  EXPECT_FLOAT_EQ(180.0, retval);

  retval = wrapTo360(270.0);
  EXPECT_FLOAT_EQ(270.0, retval);

  retval = wrapTo360(360.0);
  EXPECT_FLOAT_EQ(0.0, retval);

  retval = wrapTo360(450.0);
  EXPECT_FLOAT_EQ(90.0, retval);

  // edge cases
  retval = wrapTo360(-180.0);
  EXPECT_FLOAT_EQ(180.0, retval);

  retval = wrapTo360(-90.0);
  EXPECT_FLOAT_EQ(270.0, retval);
}

TEST(Math, cross_track_error) {
  Vec2 pos, p1, p2;

  pos << 2, 2;
  p1 << 1, 1;
  p2 << 5, 5;
  EXPECT_FLOAT_EQ(0.0, cross_track_error(p1, p2, pos));

  pos << 2, 3;
  p1 << 1, 1;
  p2 << 5, 5;
  EXPECT_TRUE(0.0 < cross_track_error(p1, p2, pos));
}

TEST(Math, point_left_right) {
  Vec2 pos, p1, p2;

  pos << 2, 3;
  p1 << 1, 1;
  p2 << 5, 5;
  EXPECT_EQ(1, point_left_right(p1, p2, pos));

  pos << 2, 1;
  p1 << 1, 1;
  p2 << 5, 5;
  EXPECT_EQ(2, point_left_right(p1, p2, pos));

  pos << 2, 2;
  p1 << 1, 1;
  p2 << 5, 5;
  EXPECT_EQ(0, point_left_right(p1, p2, pos));

  pos << 2, 1;
  p1 << 5, 5;
  p2 << 1, 1;
  EXPECT_EQ(1, point_left_right(p1, p2, pos));

  pos << 2, 3;
  p1 << 5, 5;
  p2 << 1, 1;
  EXPECT_EQ(2, point_left_right(p1, p2, pos));

  pos << 2, 2;
  p1 << 5, 5;
  p2 << 1, 1;
  EXPECT_EQ(0, point_left_right(p1, p2, pos));
}

// TEST(Math, closest_point) {
//   int retval;
//   Vec2 p1, p2, p3, closest;
//
//   // setup
//   p1 << 0, 0;
//   p2 << 5, 0;
//
//   // point middle of point a, b
//   p3 << 2, 2;
//   retval = closest_point(p1, p2, p3, closest);
//   EXPECT_EQ(0, retval);
//   EXPECT_FLOAT_EQ(2.0, closest(0));
//   EXPECT_FLOAT_EQ(0.0, closest(1));
//
//   // point before of point a
//   p3 << -1, 2;
//   retval = closest_point(p1, p2, p3, closest);
//   EXPECT_EQ(1, retval);
//   EXPECT_FLOAT_EQ(-1.0, closest(0));
//   EXPECT_FLOAT_EQ(0.0, closest(1));
//
//   // point after point b
//   p3 << 6, 2;
//   retval = closest_point(p1, p2, p3, closest);
//   EXPECT_EQ(2, retval);
//   EXPECT_FLOAT_EQ(6.0, closest(0));
//   EXPECT_FLOAT_EQ(0.0, closest(1));
//
//   // if point 1 and 2 are same
//   p1 << 0, 0;
//   p2 << 0, 0;
//   p3 << 0, 2;
//   retval = closest_point(p1, p2, p3, closest);
//   EXPECT_EQ(-1, retval);
//   EXPECT_FLOAT_EQ(0.0, closest(0));
//   EXPECT_FLOAT_EQ(0.0, closest(1));
// }

TEST(Math, linear_interpolation) {
  Vec2 a, b, result;

  a << 0, 5;
  b << 5, 0;
  result = linear_interpolation(a, b, 0.8);

  std::cout << result << std::endl;
}

} // namespace atl
