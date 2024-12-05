#include "client.h"

static constexpr int jointnum = 12;//关节个数
bool ctrl_loop_flag = 1;// 1闭环控制 0开环

struct robot_vrep
{
	// 句柄
	int handle_motor_joint;
	int handle_arm_joint[jointnum];
	int handle_small_joint[jointnum];
	// 位置
	float pos_motor_joint;
	float pos_arm_joint[jointnum];
	float pos_small_joint[jointnum];
	// 速度
	float vel_motor_joint;
	float vel_arm_joint[jointnum];
	float vel_small_joint[jointnum];
};