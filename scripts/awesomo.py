#!/usr/bin/env python2
from math import pi

import rospy
from std_msgs.msg import Float64
from std_msgs.msg import String
from std_msgs.msg import Bool
from geometry_msgs.msg import Vector3
from geometry_msgs.msg import PoseStamped
from awesomo_msgs.msg import PCtrlSettings
from awesomo_msgs.msg import TCtrlSettings
from awesomo_msgs.msg import LCtrlSettings


class ROSNode(object):
    def __init__(self):
        self.pubs = {}
        self.subs = {}

    def register_publisher(self, topic, msg_type, queue_size=1):
        self.pubs[topic] = rospy.Publisher(topic,
                                           msg_type,
                                           queue_size=queue_size)

    def register_subscriber(self, topic, msg_type, callback):
        self.subs[topic] = rospy.Subscriber(topic, msg_type, callback)


class Camera(ROSNode):
    def __init__(self):
        super(Camera, self).__init__()
        self.image_topic = "/awesomo/camera/image"
        self.mode_topic = "/prototype/camera/mode"

        self.register_publisher(self.mode_topic, String)

    def set_mode(self, mode):
        msg = String()
        msg.data = mode
        self.pubs[self.mode_topic].publish(msg)


class LandingZone(ROSNode):
    def __init__(self):
        super(LandingZone, self).__init__()
        self.position_topic = "/awesomo/lz/position/set"
        self.velocity_topic = "/awesomo/lz/velocity/set"
        self.angular_velocity_topic = "/awesomo/lz/angular_velocity/set"

        self.register_publisher(self.position_topic, Vector3)
        self.register_publisher(self.velocity_topic, Float64)
        self.register_publisher(self.angular_velocity_topic, Float64)

    def set_position(self, pos):
        msg = Vector3()
        msg.x = pos[0]
        msg.y = pos[1]
        msg.z = pos[2]
        self.pubs[self.position_topic].publish(msg)

    def set_velocity(self, vel):
        msg = Float64()
        msg.data = vel
        self.pubs[self.velocity_topic].publish(msg)

    def set_angular_velocity(self, ang_vel):
        msg = Float64()
        msg.data = ang_vel
        self.pubs[self.angular_velocity_topic].publish(msg)


class Gimbal(ROSNode):
    def __init__(self):
        super(Gimbal, self).__init__()
        self.attitude_topic = "/awesomo/gimbal/setpoint/attitude"
        self.track_topic = "/awesomo/gimbal/track"

        self.register_publisher(self.attitude_topic, Vector3)
        self.register_publisher(self.track_topic, Vector3)

    def set_attitude(self, attitude):
        msg = Vector3()
        msg.x = attitude[0]
        msg.y = attitude[1]
        msg.z = 0.0
        self.pubs[self.attitude_topic].publish(msg)

    def track(self, track):
        msg = Vector3()
        msg.x = track[0]
        msg.y = track[1]
        msg.z = 0.0
        self.pubs[self.track_topic].publish(msg)


class Quadrotor(ROSNode):
    def __init__(self):
        super(Quadrotor, self).__init__()
        self.arm_topic = "/awesomo/control/arm"
        self.mode_topic = "/awesomo/control/mode"
        self.heading_topic = "/awesomo/control/heading/set"
        self.hover_point_topic = "/awesomo/control/hover/set"
        self.hover_height_topic = "/awesomo/control/hover/height/set"
        self.pctrl_set_topic = "/awesomo/control/position_controller/set"
        self.tctrl_set_topic = "/awesomo/control/tracking_controller/set"
        self.lctrl_set_topic = "/awesomo/control/landing_controller/set"

        self.register_publisher(self.arm_topic, Bool)
        self.register_publisher(self.mode_topic, String)
        self.register_publisher(self.heading_topic, Float64)
        self.register_publisher(self.hover_point_topic, Vector3)
        self.register_publisher(self.hover_height_topic, Float64)
        self.register_publisher(self.pctrl_set_topic, PCtrlSettings)
        self.register_publisher(self.tctrl_set_topic, TCtrlSettings)
        self.register_publisher(self.lctrl_set_topic, LCtrlSettings)

    def set_arm(self, arm):
        msg = Bool()
        msg.data = arm
        self.pubs[self.arm_topic].publish(msg)

    def set_mode(self, mode):
        msg = String()
        msg.data = mode
        self.pubs[self.mode_topic].publish(msg)

    def set_yaw(self, yaw):
        msg = Float64()
        msg.data = yaw * (pi / 180.0)
        self.pubs[self.heading_topic].publish(msg)

    def set_hover_point(self, hover_point):
        msg = Vector3()
        msg.x = hover_point[0]
        msg.y = hover_point[1]
        msg.z = hover_point[2]
        self.pubs[self.hover_point_topic].publish(msg)

    def set_hover_height(self, hover_height):
        msg = Float64()
        msg.data = hover_height
        self.pubs[self.hover_height_topic].publish(msg)

    def set_pctrl_settings(self, params):
        msg = PCtrlSettings()

        msg.roll_controller.min = params["roll"]["min"]
        msg.roll_controller.max = params["roll"]["max"]
        msg.roll_controller.k_p = params["roll"]["k_p"]
        msg.roll_controller.k_i = params["roll"]["k_i"]
        msg.roll_controller.k_d = params["roll"]["k_d"]

        msg.pitch_controller.min = params["pitch"]["min"]
        msg.pitch_controller.max = params["pitch"]["max"]
        msg.pitch_controller.k_p = params["pitch"]["k_p"]
        msg.pitch_controller.k_i = params["pitch"]["k_i"]
        msg.pitch_controller.k_d = params["pitch"]["k_d"]

        msg.throttle_controller.k_p = params["throttle"]["k_p"]
        msg.throttle_controller.k_i = params["throttle"]["k_i"]
        msg.throttle_controller.k_d = params["throttle"]["k_d"]

        msg.hover_throttle = params["throttle"]["hover"]

        self.pubs[self.pctrl_set_topic].publish(msg)

    def set_tctrl_settings(self, params):
        msg = TCtrlSettings()

        msg.roll_controller.min = params["roll"]["min"]
        msg.roll_controller.max = params["roll"]["max"]
        msg.roll_controller.k_p = params["roll"]["k_p"]
        msg.roll_controller.k_i = params["roll"]["k_i"]
        msg.roll_controller.k_d = params["roll"]["k_d"]

        msg.pitch_controller.min = params["pitch"]["min"]
        msg.pitch_controller.max = params["pitch"]["max"]
        msg.pitch_controller.k_p = params["pitch"]["k_p"]
        msg.pitch_controller.k_i = params["pitch"]["k_i"]
        msg.pitch_controller.k_d = params["pitch"]["k_d"]

        msg.throttle_controller.k_p = params["throttle"]["k_p"]
        msg.throttle_controller.k_i = params["throttle"]["k_i"]
        msg.throttle_controller.k_d = params["throttle"]["k_d"]
        msg.hover_throttle = params["throttle"]["hover"]

        msg.vx_controller.k_p = params["vx"]["k_p"]
        msg.vx_controller.k_i = params["vx"]["k_i"]
        msg.vx_controller.k_d = params["vx"]["k_d"]

        msg.vy_controller.k_p = params["vy"]["k_p"]
        msg.vy_controller.k_i = params["vy"]["k_i"]
        msg.vy_controller.k_d = params["vy"]["k_d"]

        msg.vz_controller.k_p = params["vz"]["k_p"]
        msg.vz_controller.k_i = params["vz"]["k_i"]
        msg.vz_controller.k_d = params["vz"]["k_d"]

        self.pubs[self.tctrl_set_topic].publish(msg)

    def set_lctrl_settings(self, params):
        msg = LCtrlSettings()

        msg.roll_controller.min = params["roll"]["min"]
        msg.roll_controller.max = params["roll"]["max"]
        msg.roll_controller.k_p = params["roll"]["k_p"]
        msg.roll_controller.k_i = params["roll"]["k_i"]
        msg.roll_controller.k_d = params["roll"]["k_d"]

        msg.pitch_controller.min = params["pitch"]["min"]
        msg.pitch_controller.max = params["pitch"]["max"]
        msg.pitch_controller.k_p = params["pitch"]["k_p"]
        msg.pitch_controller.k_i = params["pitch"]["k_i"]
        msg.pitch_controller.k_d = params["pitch"]["k_d"]

        msg.throttle_controller.k_p = params["throttle"]["k_p"]
        msg.throttle_controller.k_i = params["throttle"]["k_i"]
        msg.throttle_controller.k_d = params["throttle"]["k_d"]
        msg.hover_throttle = params["throttle"]["hover"]

        msg.vx_controller.k_p = params["vx"]["k_p"]
        msg.vx_controller.k_i = params["vx"]["k_i"]
        msg.vx_controller.k_d = params["vx"]["k_d"]

        msg.vy_controller.k_p = params["vy"]["k_p"]
        msg.vy_controller.k_i = params["vy"]["k_i"]
        msg.vy_controller.k_d = params["vy"]["k_d"]

        msg.vz_controller.k_p = params["vz"]["k_p"]
        msg.vz_controller.k_i = params["vz"]["k_i"]
        msg.vz_controller.k_d = params["vz"]["k_d"]

        self.pubs[self.lctrl_set_topic].publish(msg)


class MAVROS(ROSNode):
    def __init__(self):
        super(MAVROS, self).__init__()
        self.local_pose_topic = "/mavros/local_position/pose"

        self.register_publisher(self.local_pose_topic, PoseStamped)

    def set_local_pose(self, x, y, z):
        msg = PoseStamped()
        msg.pose.position.x = x
        msg.pose.position.y = y
        msg.pose.position.z = z

        msg.pose.orientation.x = 0
        msg.pose.orientation.y = 0
        msg.pose.orientation.z = 0
        msg.pose.orientation.w = 1
        self.pubs[self.local_pose_topic].publish(msg)