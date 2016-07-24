#include "awesomo/util.hpp"

Pose::Pose(void)
{
    Eigen::Quaterniond q;

    this->q = q.setIdentity();
    this->position = Eigen::Vector3d::Zero(3,1);
}


Pose::Pose(
    float roll,
    float pitch,
    float yaw,
    float x,
    float y,
    float z
)
{
    euler2Quaternion(roll, pitch, yaw, this->q);
    this->position << x, y, z;
}

Pose::Pose(Eigen::Quaterniond q, Eigen::Vector3d position)
{
    this->q = q;
    this->position = position;
}

Eigen::Matrix3d Pose::rotationMatrix(void)
{
    return this->q.toRotationMatrix();
}


double deg2rad(double d)
{
    return d * (M_PI / 180);
}

double rad2deg(double d)
{
    return d * (180 / M_PI);
}

int fltcmp(double f1, double f2)
{
	if (fabs(f1 - f2) <= 0.0001) {
		return 0;
	} else if (f1 > f2) {
		return 1;
	} else {
		return -1;
	}
}

int euler2Quaternion(
    const double roll,
    const double pitch,
    const double yaw,
    Eigen::Quaterniond &q
)
{
    Eigen::AngleAxisd rollAngle(roll, Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd pitchAngle(pitch, Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd yawAngle(yaw, Eigen::Vector3d::UnitZ());

    q = yawAngle * pitchAngle * rollAngle;
    q.normalize();
    return 0;
}

int euler2RotationMatrix(
    const double roll,
    const double pitch,
    const double yaw,
    Eigen::Matrix3d &rot
)
{
    Eigen::Quaterniond q_normalized;
    euler2Quaternion(roll, pitch, yaw, q_normalized);
    rot = q_normalized.toRotationMatrix();
    return 0;
}

static double tic(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((double) t.tv_sec + ((double) t.tv_usec) / 1000000.0);
}
