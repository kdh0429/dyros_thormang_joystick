#ifndef _THORMANG_TELEOP_JOY_H
#define _THORMANG_TELEOP_JOY_H
// ROS LIBRARY
#include <ros/ros.h>
// ROS MESSAGE
#include <thormang_ctrl_msgs/TaskCmdboth.h>
#include <thormang_ctrl_msgs/WalkingCmd.h>
#include <thormang_ctrl_msgs/Joy_to_UI.h>
#include <thormang_ctrl_msgs/Step_number.h>
#include <sensor_msgs/Joy.h>
    


class TeleopThormang
    {
   public:
     TeleopThormang();
     void joyCallback(const sensor_msgs::Joy::ConstPtr& joy);
     void StepNumCallback(const thormang_ctrl_msgs::Step_number::ConstPtr& step_num);
int state; // -1: When normal / 0: When walkcmd by joy is executing / 1: When walkcmd.x=0(zero-walking) is executing
int walking_cmd_now; // whether walking command is forward walking(1) or init/zero walking (0) or backward walking(-1)
int memory_x; // memorize x value by stopping to move after stopping
   
   private:
     ros::NodeHandle nh_;
     ros::Publisher arm_pub_;
     ros::Publisher walk_pub_;
     ros::Publisher joy_to_ui_pub_;
     ros::Subscriber joy_sub_;
     ros::Subscriber step_num_sub_;
   
    thormang_ctrl_msgs::TaskCmdboth taskcmd;
    thormang_ctrl_msgs::Joy_to_UI joy_to_ui;
    thormang_ctrl_msgs::WalkingCmd walkingcmd;


// Mode selection. 'joystick off(0)' or 'moving arm and walking with walking distance mode(1)' or 'walking with joystick mode(2)'.
int mode_change;
bool button_mode_change_pre; // previous signal(to toggle)


// In 'moving arm and walking with walking distance mode'    
bool rpy_status;      // whether you control right or left roll pitch yaw
bool toggle_angle;	// whether roll, pitch, yaw angle is clock-wise or counter-clock-wise
bool button_angle_pre;

int walk_distance_10; //walking distance in 10cm unit
int walk_distance_1; //walking distance in 1cm unit
int button_walk_pre_10; //previous signal
int button_walk_pre_1; //previous signal


// In walking with joystick mode
int walking_cmd_pre; // To compare joystick signla with its previous value signal

// since walking should stop when walking cycle is completed (to stop safely), step number is set to stop walking when step number is changed (walking cycle is completed).
int step_numb; 
int step_numb_pre;
bool timing; // timing is 1 when step number is changed.



float memory_y; // memorize y value by stopping to move after stopping
   };
 

 int main(int argc, char** argv);

#endif
