#include "../include/thormang_teleop_joy/thormang_teleop_joy.h"

TeleopThormang::TeleopThormang()
    : mode_change(0),button_mode_change_pre(0),rpy_status(0),toggle_angle(1),button_angle_pre(0),walk_distance_10(0),walk_distance_1(0),button_walk_pre_10(0),button_walk_pre_1(0), walking_cmd_now(0), walking_cmd_pre(0),step_numb(0),step_numb_pre(0),timing(0), state(-1), memory_x(0), memory_y(0)
{
    arm_pub_ = nh_.advertise<thormang_ctrl_msgs::TaskCmdboth>("thormang_task_cmd", 3);
    walk_pub_ = nh_.advertise<thormang_ctrl_msgs::WalkingCmd>("thormang_walking_cmd", 3);
    joy_to_ui_pub_= nh_.advertise<thormang_ctrl_msgs::Joy_to_UI>("thormang_joy_to_ui",3);
    joy_sub_ = nh_.subscribe<sensor_msgs::Joy>("joy", 10, &TeleopThormang::joyCallback, this);
    step_num_sub_ = nh_.subscribe<thormang_ctrl_msgs::Step_number>("thormang_ctrl/thormang_step_number", 3, &TeleopThormang::StepNumCallback, this);
}

void TeleopThormang::joyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{
    // mode selection (mode0->mode1->mode2->mode0...)

    if (joy->buttons[8]==1)
        if (joy->buttons[8]!=button_mode_change_pre)
        {
            if(mode_change==0)
                mode_change=1;
            else if(mode_change==1)
                mode_change=2;
            else
                mode_change=0;

            // if(mode_change==2)
            //	mode_change=0;
            // else
            //	mode_change=mode_change+1;
        }

    button_mode_change_pre=joy->buttons[8];


    // mode 0 (initial mode, joystick off)
    if (mode_change==0)
    {
        //mode publish to ui
        joy_to_ui.mode="Joystick Off";
        joy_to_ui_pub_.publish(joy_to_ui);
    }



    // mode 1 (moving arm and walking with walking distance)

    if (mode_change==1)
    {
        // mode publish to ui
        joy_to_ui.mode="Arm / Walk with distance";
        joy_to_ui_pub_.publish(joy_to_ui);


        //arm
        // whether you control roll pitch yaw of right or left arm.
        if (joy->buttons[9]==1)
            rpy_status=0;
        else if (joy->buttons[10]==1)
            rpy_status=1;


        //toggle roll pitch yaw direction
        if (joy->buttons[0]==1)
            if (joy->buttons[0]!=button_angle_pre)
                toggle_angle= !toggle_angle;

        button_angle_pre=joy->buttons[0];



        //      thormang_ctrl_msgs::TaskCmdboth taskcmd

        taskcmd.x_l = 0.0f;
        taskcmd.y_l = 0.0f;
        taskcmd.z_l = 0.0f;
        taskcmd.roll_l = 0.0f;
        taskcmd.pitch_l = 0.0f;
        taskcmd.yaw_l = 0.0f;

        taskcmd.x_r = 0.0f;
        taskcmd.y_r = 0.0f;
        taskcmd.z_r = 0.0f;
        taskcmd.roll_r = 0.0f;
        taskcmd.pitch_r = 0.0f;
        taskcmd.yaw_r = 0.0f;

        taskcmd.rel = taskcmd.REL;

        taskcmd.x_l = 100*joy->axes[1];
        taskcmd.y_l = 100*joy->axes[0];
        if (joy->buttons[4]==1)
            taskcmd.z_l=100*joy->buttons[4];
        else if (joy->axes[2]==0)          // joy->axes[2] is 0 when started and once it is pushed, it changes from 1 to -1. So if you do not set z as 0 when joy->axes[2]==0, arm will move automatically when started.
            taskcmd.z_l=0;
        else
            taskcmd.z_l=50*(joy->axes[2]-1);

        if (rpy_status==0) // roll,pitch, yaw of left arm
        {
            if(toggle_angle==1) // when rotation direction is clock wise
            {
                taskcmd.roll_l=360*joy->buttons[2];
                taskcmd.pitch_l=360*joy->buttons[3];
                taskcmd.yaw_l=360*joy->buttons[1];
                taskcmd.rotation_dir=taskcmd.clock_wise;
            }
            else // counter clock wise
            {
                taskcmd.roll_l=-360*joy->buttons[2];
                taskcmd.pitch_l=-360*joy->buttons[3];
                taskcmd.yaw_l=-360*joy->buttons[1];
                taskcmd.rotation_dir=taskcmd.counter_clock_wise;
            }

            taskcmd.arm=taskcmd.LEFT_ARM;
        }



        taskcmd.x_r = 100*joy->axes[4];
        taskcmd.y_r = 100*joy->axes[3];
        if (joy->buttons[5]==1)
            taskcmd.z_r=100*joy->buttons[5];
        else if (joy->axes[5]==0)
            taskcmd.z_r=0;
        else
            taskcmd.z_r=50*(joy->axes[5]-1);

        if (rpy_status==1)
        {
            if(toggle_angle==1)
            {
                taskcmd.roll_r=360*joy->buttons[2];
                taskcmd.pitch_r=360*joy->buttons[3];
                taskcmd.yaw_r=360*joy->buttons[1];
                taskcmd.rotation_dir=taskcmd.clock_wise;
            }
            else
            {
                taskcmd.roll_r=-360*joy->buttons[2];
                taskcmd.pitch_r=-360*joy->buttons[3];
                taskcmd.yaw_r=-360*joy->buttons[1];
                taskcmd.rotation_dir=taskcmd.counter_clock_wise;
            }
            taskcmd.arm=taskcmd.RIGHT_ARM;
        }

        taskcmd.subtask = taskcmd.ARM_TASK;

        arm_pub_.publish(taskcmd);



        // walking


        // setting walk distance in 10cm unit
        if (joy->axes[7]==1)
        {
            if (joy->axes[7]!=button_walk_pre_10)
                walk_distance_10++;
        }
        else if (joy->axes[7]==-1)
            if (joy->axes[7]!=button_walk_pre_10)
                walk_distance_10--;

        button_walk_pre_10=joy->axes[7];



        // setting walk distance in 1cm unit

        if (joy->axes[6]==-1)
        {
            if (joy->axes[6]!=button_walk_pre_1)
                walk_distance_1++;
        }
        else if (joy->axes[6]==1)
            if (joy->axes[6]!=button_walk_pre_1)
                walk_distance_1=walk_distance_1-1;

        button_walk_pre_1=joy->axes[6];


        //    thormang_ctrl_msgs::Joy_to_UI joy_to_ui

        joy_to_ui.walk_distance_10=walk_distance_10;
        joy_to_ui.walk_distance_1=walk_distance_1;

        joy_to_ui_pub_.publish(joy_to_ui);



        //walking cmd setting
        //  thormang_ctrl_msgs::WalkingCmd walkingcmd


        if (joy->buttons[6]==1)
        {
            walkingcmd.command = "init";
            walkingcmd.planner = "Diamond";
            walkingcmd.walking_alg = "Open Loop";
            walkingcmd.x = 0;
            walkingcmd.y = 0;
            walkingcmd.height = 0;
            walkingcmd.theta = 0;
            walkingcmd.imp_time = 0;
            walkingcmd.recov_time= 0;

            walk_pub_.publish(walkingcmd);
        }
        else if(joy->buttons[7]==1)
        {
            walkingcmd.command = "start";
            walkingcmd.planner = "Diamond";
            walkingcmd.walking_alg = "Open Loop";
            walkingcmd.x = 0.01*(10*walk_distance_10+walk_distance_1);
            walkingcmd.y = 0;
            walkingcmd.height = 0;
            walkingcmd.theta = 0;
            walkingcmd.imp_time = 0;
            walkingcmd.recov_time= 0;

            if (0.01*(10*walk_distance_10+walk_distance_1) !=0)
            {
                walk_pub_.publish(walkingcmd);
                walk_distance_10=0;
                walk_distance_1=0;
            }
        }
    }





    // mode 2 (walking with joystick)

    if(mode_change==2)
    {
        //mode publish to ui
        joy_to_ui.mode="Walk with joystick";
        joy_to_ui_pub_.publish(joy_to_ui);


        if (joy->buttons[0] == 1)
        {

            //ROS_INFO("%d", step_numb);
            // Set1. move to direction(x) w/o rotation
            // state is -1: When normal(walk init is finished) 0: When walkcmd.x(!=0) by joy is executing 1: When walkcmd.x=0 is executing
            /* if (state == 1 && memory_x == 0)
                           {
                                   walking_cmd_now = 0; // when zero-walking is executing (state==1) do not send walkingcmd.x(!=0). Otherwise zero-walking is disturbed while executing.
                           } */

            if (memory_x == 200 || memory_x == -200){
                memory_x = 0; memory_y = 0;}

            if (joy->axes[1]==0) //  && (joy->buttons[0] == 1)
            {
                walking_cmd_now = 0; // Although you put down joystick<1>, if you don't press the button<A>, Thormang will not stop by walking.
            }
            else if (joy->axes[1]>0)
            {


                if (state == -1) // && walking_cmd_now == 1  // when normal(state==-1) and joystick signal is larger than 0(walking_cmd_now==1) send walkingcmd.x=100 ONCE(walking_cmd_pre!=walking_cmd_now). if(joy->axes[1]!=0) is not used since whenever joystick is moved slightly, signal will be sent thus walkingcmd will be renewed.
                {
                    walkingcmd.command = "start";
                    walkingcmd.planner = "Diamond";
                    walkingcmd.walking_alg = "Open Loop";
                    walkingcmd.x = 100;
                    walkingcmd.y = 0;
                    walkingcmd.height = 0;
                    walkingcmd.theta = 0;
                    walkingcmd.imp_time = 0;
                    walkingcmd.recov_time = 0;
                    walk_pub_.publish(walkingcmd);

                    joy_to_ui.walking_cmd = "forward walking";
                    joy_to_ui_pub_.publish(joy_to_ui);

                    state = 0;

                    walking_cmd_now = 1; //send walkingcmd, joystick signal is positive.
                }
                else if (state == 0 && memory_x == 0 && memory_y == 0 && walking_cmd_now != 1) // && walking_cmd_now == 1
                {
                    memory_x = 100;
                    walking_cmd_now = 0;

                    ROS_INFO("get memory_x 100");
                }
            }
            else
            {


                if (state == -1) //&& walking_cmd_now == -1
                {
                    walkingcmd.command = "start";
                    walkingcmd.planner = "Diamond";
                    walkingcmd.walking_alg = "Open Loop";
                    walkingcmd.x = -100;
                    walkingcmd.y = 0;
                    walkingcmd.height = 0;
                    walkingcmd.theta = 0;
                    walkingcmd.imp_time = 0;
                    walkingcmd.recov_time = 0;
                    walk_pub_.publish(walkingcmd);

                    joy_to_ui.walking_cmd = "backward walking";
                    joy_to_ui_pub_.publish(joy_to_ui);

                    state = 0;

                    walking_cmd_now = -1;
                }
                else if (state == 0 && memory_x == 0 && memory_y == 0 && walking_cmd_now != -1) //&& walking_cmd_now == -1  // input command while walking(we don't have to command stop before commanding another walking command)
                {
                    memory_x = -100;
                    walking_cmd_now = 0;

                    ROS_INFO("get memory_x -100");
                }
            }


            //ros::spinOnce();
            // ROS_INFO("%d", step_numb);


            if (state == 1 && memory_x != 0  && joy->axes[1]!=0)
            {
                if (joy->axes[1]>0){
                    memory_x = 100;
                    ROS_INFO("get new memory_x 100");}
                else if (joy->axes[1]<0){
                    memory_x = -100;
                    ROS_INFO("get new memory_x -100");}
            }

            walking_cmd_pre = walking_cmd_now;

            /*
                           if (joy->axes[1]>0)
                                   walking_cmd_pre = 1;
                           else if (joy->axes[1] == 0)
                                   walking_cmd_pre = 0;
                           else
                                   walking_cmd_pre = -1;
                                   */
        }
        else if (joy->buttons[1] == 1)
        {
            // Set2. move to direction(x,y) w/ rotation
            //ROS_INFO("%d", step_numb);
            // Set1. move to direction(x) w/o rotation
            // state is -1: When normal(walk init is finished) 0: When walkcmd.x(!=0) by joy is executing 1: When walkcmd.x=0 is executing
            /* if (state == 1 && memory_x == 0)
                           {
                                   walking_cmd_now = 0; // when zero-walking is executing (state==1) do not send walkingcmd.x(!=0). Otherwise zero-walking is disturbed while executing.
                           } */

            if (memory_x == 200 || memory_x == -200){
                memory_x = 0; memory_y = 0;}

            if (joy->axes[1]==0) //  && (joy->buttons[0] == 1)
            {
                walking_cmd_now = 0; // Although you put down joystick<1>, if you don't press the button<A>, Thormang will not stop by walking.
            }
            else if (joy->axes[1]>0)
            {

                if (state == -1) // && walking_cmd_now == 1  // when normal(state==-1) and joystick signal is larger than 0(walking_cmd_now==1) send walkingcmd.x=100 ONCE(walking_cmd_pre!=walking_cmd_now). if(joy->axes[1]!=0) is not used since whenever joystick is moved slightly, signal will be sent thus walkingcmd will be renewed.
                {
                    walkingcmd.command = "start";
                    walkingcmd.planner = "Diamond";
                    walkingcmd.walking_alg = "Open Loop";
                    walkingcmd.x = joy->axes[1] *100;
                    walkingcmd.y = joy->axes[0] *100;
                    //walkingcmd.x = (joy->axes[1] / (fabs(joy->axes[1]) + fabs(joy->axes[2])))*100;
                    //walkingcmd.y = (joy->axes[2] / (fabs(joy->axes[1]) + fabs(joy->axes[2])))*100;
                    walkingcmd.height = 0;
                    walkingcmd.theta = 0;
                    walkingcmd.imp_time = 0;
                    walkingcmd.recov_time = 0;
                    walk_pub_.publish(walkingcmd);

                    joy_to_ui.walking_cmd = "forward walking";
                    joy_to_ui_pub_.publish(joy_to_ui);

                    state = 0;

                    walking_cmd_now = 1; //send walkingcmd, joystick signal is positive.
                }
                else if (state == 0 && memory_x == 0 && memory_y == 0 && walking_cmd_now != 1) // && walking_cmd_now == 1
                {
                    memory_x = joy->axes[1] *100;
                    memory_y = joy->axes[0] *100;
                    walking_cmd_now = 0;

                    ROS_INFO("get memory_xy 100");
                }
            }
            else
            {
                if (state == -1) //&& walking_cmd_now == -1
                {
                    walkingcmd.command = "start";
                    walkingcmd.planner = "Diamond";
                    walkingcmd.walking_alg = "Open Loop";
                    walkingcmd.x = joy->axes[1] *100;
                    walkingcmd.y = joy->axes[0] *100;
                    //walkingcmd.x = (joy->axes[1] / (fabs(joy->axes[1]) + fabs(joy->axes[2])))*100;
                    //walkingcmd.y = (joy->axes[2] / (fabs(joy->axes[1]) + fabs(joy->axes[2])))*100;
                    walkingcmd.height = 0;
                    walkingcmd.theta = 0;
                    walkingcmd.imp_time = 0;
                    walkingcmd.recov_time = 0;
                    walk_pub_.publish(walkingcmd);

                    joy_to_ui.walking_cmd = "backward walking";
                    joy_to_ui_pub_.publish(joy_to_ui);

                    state = 0;

                    walking_cmd_now = -1;
                }
                else if (state == 0 && memory_x == 0 && memory_y == 0 && walking_cmd_now != -1) //&& walking_cmd_now == -1  // input command while walking(we don't have to command stop before commanding another walking command)
                {
                    memory_x = joy->axes[1] *100;
                    memory_y = joy->axes[0] *100;
                    walking_cmd_now = 0;

                    ROS_INFO("get memory_xy -100");
                }
            }


            //ros::spinOnce();
            // ROS_INFO("%d", step_numb);


            if (state == 1 && memory_x != 0 && joy->axes[1]!=0)
            {
                if (joy->axes[1]>0){
                    memory_x = joy->axes[1] *100;
                    memory_y = joy->axes[0] *100;
                    // memory_x = (joy->axes[1] / fabs(joy->axes[1]) + fabs(joy->axes[2]))*100;
                    // memory_y = (joy->axes[2] / fabs(joy->axes[1]) + fabs(joy->axes[2]))*100;
                    ROS_INFO("get new memory_xy 100");}
                else if (joy->axes[1]<0){
                    memory_x = joy->axes[1] *100;
                    memory_y = joy->axes[0] *100;
                    // memory_x = (joy->axes[1] / fabs(joy->axes[1]) + fabs(joy->axes[2]))*100;
                    //memory_y = (joy->axes[2] / fabs(joy->axes[1]) + fabs(joy->axes[2]))*100;
                    ROS_INFO("get new memory_xy -100");}
            }

            walking_cmd_pre = walking_cmd_now;

            /*
                           if (joy->axes[1]>0)
                                   walking_cmd_pre = 1;
                           else if (joy->axes[1] == 0)
                                   walking_cmd_pre = 0;
                           else
                                   walking_cmd_pre = -1;
                                   */

        }

    }


}



void TeleopThormang::StepNumCallback(const thormang_ctrl_msgs::Step_number::ConstPtr& step_num)
{
    step_numb=step_num->step_number;

    if(step_numb!=step_numb_pre)   // when step number is changed
        timing=1;
    else
        timing=0;

    step_numb_pre=step_numb;

    //ROS_INFO("StepNumCallback");

    if(mode_change==2)
    {

        // since timing infomation should be revised when zero-walking, init and sending walkingcmd.x=100 again, they should be in stepnumcallback function.
        if (walking_cmd_now==0)    //when joystick signal is 0
        {
            //joy_to_ui.walking_cmd="stopping";
            //joy_to_ui_pub_.publish(joy_to_ui);

            if(state==0 && timing==1 && memory_x != 200 && memory_x != -200) // While walking(state==0), send walkingcmd.x=0(zero-walking) when step number changes.
            {
                ROS_INFO("stopping");
                joy_to_ui.walking_cmd="stopping";
                joy_to_ui_pub_.publish(joy_to_ui);

                walkingcmd.command = "start";
                walkingcmd.planner = "Diamond";
                walkingcmd.walking_alg = "Open Loop";
                walkingcmd.x = 0;
                walkingcmd.y = 0;
                walkingcmd.height = 0;
                walkingcmd.theta = 0;
                walkingcmd.imp_time = 0;
                walkingcmd.recov_time= 0;
                walk_pub_.publish(walkingcmd);
                state=1;
            }

            if(state==1 && step_numb==1 && memory_x == 0) // when zero-walking is executing(state==1) and the first step of zero-walking is executed (step_numb==1).
            {
                ROS_INFO("stop complete");
                joy_to_ui.walking_cmd="stop complete";
                joy_to_ui_pub_.publish(joy_to_ui);

                /*
             ROS_INFO("INIT");
              walkingcmd.command = "init";
              walkingcmd.planner = "Diamond";
              walkingcmd.walking_alg = "Open Loop";
              walkingcmd.x = 0;
              walkingcmd.y = 0;
              walkingcmd.height = 0;
              walkingcmd.theta = 0;
              walkingcmd.imp_time = 0;
              walkingcmd.recov_time= 0;
              walk_pub_.publish(walkingcmd);
              */
                state=-1;
            }
            if (state==1 && step_numb==1 && memory_x > 0 && memory_y == 0) // if there is memory
            {
                walkingcmd.command = "start";
                walkingcmd.planner = "Diamond";
                walkingcmd.walking_alg = "Open Loop";
                walkingcmd.x = 100;
                walkingcmd.y = 0;
                walkingcmd.height = 0;
                walkingcmd.theta = 0;
                walkingcmd.imp_time = 0;
                walkingcmd.recov_time = 0;
                walk_pub_.publish(walkingcmd);

                joy_to_ui.walking_cmd = "forward walking";
                joy_to_ui_pub_.publish(joy_to_ui);

                state = 0;
                memory_x = 200;
                walking_cmd_now = 1;

                ROS_INFO("reset memory_x");
            }
            if (state==1 && step_numb==1 && memory_x < 0 && memory_y == 0) // if there is memory
            {
                walkingcmd.command = "start";
                walkingcmd.planner = "Diamond";
                walkingcmd.walking_alg = "Open Loop";
                walkingcmd.x = -100;
                walkingcmd.y = 0;
                walkingcmd.height = 0;
                walkingcmd.theta = 0;
                walkingcmd.imp_time = 0;
                walkingcmd.recov_time = 0;
                walk_pub_.publish(walkingcmd);

                joy_to_ui.walking_cmd = "backward walking";
                joy_to_ui_pub_.publish(joy_to_ui);

                state = 0;
                memory_x = -200;
                walking_cmd_now = -1;

                ROS_INFO("reset memory_x");
            }
            if (state==1 && step_numb==1 && memory_x > 0 && memory_y != 0) // if there is memory
            {
                walkingcmd.command = "start";
                walkingcmd.planner = "Diamond";
                walkingcmd.walking_alg = "Open Loop";
                walkingcmd.x = memory_x;
                walkingcmd.y = memory_y;
                walkingcmd.height = 0;
                walkingcmd.theta = 0;
                walkingcmd.imp_time = 0;
                walkingcmd.recov_time = 0;
                walk_pub_.publish(walkingcmd);

                joy_to_ui.walking_cmd = "forward walking";
                joy_to_ui_pub_.publish(joy_to_ui);

                state = 0;
                memory_x = 200;
                memory_y = 200;
                walking_cmd_now = 1;

                ROS_INFO("reset memory_x");
            }
            if (state==1 && step_numb==1 && memory_x < 0 && memory_y != 0) // if there is memory
            {
                walkingcmd.command = "start";
                walkingcmd.planner = "Diamond";
                walkingcmd.walking_alg = "Open Loop";
                walkingcmd.x = memory_x;
                walkingcmd.y = memory_y;
                walkingcmd.height = 0;
                walkingcmd.theta = 0;
                walkingcmd.imp_time = 0;
                walkingcmd.recov_time = 0;
                walk_pub_.publish(walkingcmd);

                joy_to_ui.walking_cmd = "backward walking";
                joy_to_ui_pub_.publish(joy_to_ui);

                state = 0;
                memory_x = -200;
                memory_y = -200;
                walking_cmd_now = -1;

                ROS_INFO("reset memory_x");
            }
        }



        if(walking_cmd_now==1 && step_numb==1000)  // when 100m walking is done and joystick signal is still larger than 0, walk 100m forward again.
        {
            walkingcmd.command = "start";
            walkingcmd.planner = "Diamond";
            walkingcmd.walking_alg = "Open Loop";
            walkingcmd.x = 100;
            walkingcmd.y = 0;
            walkingcmd.height = 0;
            walkingcmd.theta = 0;
            walkingcmd.imp_time = 0;
            walkingcmd.recov_time= 0;
            walk_pub_.publish(walkingcmd);

            joy_to_ui.walking_cmd="forward walking";
            joy_to_ui_pub_.publish(joy_to_ui);

            state=0;
        }
        if(walking_cmd_now==-1 && step_numb==1000) //when -100m walking is done and joystick signal is still smaller than 0, walk backward 100m again.
        {
            walkingcmd.command = "start";
            walkingcmd.planner = "Diamond";
            walkingcmd.walking_alg = "Open Loop";
            walkingcmd.x = -100;
            walkingcmd.y = 0;
            walkingcmd.height = 0;
            walkingcmd.theta = 0;
            walkingcmd.imp_time = 0;
            walkingcmd.recov_time= 0;
            walk_pub_.publish(walkingcmd);

            joy_to_ui.walking_cmd="backward walking";
            joy_to_ui_pub_.publish(joy_to_ui);

            state=0;
        }

    }

}


int main(int argc, char** argv)
{
    ros::init(argc, argv, "teleop_thormang");
    TeleopThormang teleop_thormang;

    while(ros::ok())
    {
        //ROS_INFO("state : %d",teleop_thormang.state);
        //ROS_INFO("walking_cmd %d", teleop_thormang.walking_cmd_now);
        //ROS_INFO("memory_x : %d",teleop_thormang.memory_x);
        ros::spinOnce();
    }
    //ros::spin();
}

