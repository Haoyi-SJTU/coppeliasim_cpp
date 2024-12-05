#include <iostream>
#include "coppeliasim-cpp-client.h"

using namespace coppeliasim_cpp;
using namespace std;

int main()
{
	CoppeliaSimClient client; // vrep接口
	robot_vrep hrr;//机器人参数

	if (client.initialize(tstep))
	{
		client.startSimulation();
		client.log_msg("Simulation started.");
		// 取句柄
		hrr.handle_motor_joint = client.getObjectHandle("motor_joint");

		while (client.isConnected())
		{
			const Pose obj_pose = client.getObjectPose(hrr.handle_motor_joint);
			std::cout << "Object position: " << obj_pose.position.x << ", " << obj_pose.position.y << ", " << obj_pose.position.z << std::endl;
			// 对机器人做其它操作
		}
	}
	else
		return 1;

	return 0;
}
