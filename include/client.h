
#pragma once

#include <iostream>

#include <string>
#include <algorithm> 
#include <memory>
#include <cstring>

#define LOG_ON_COPPELIA false
#define LOG_ON_CMD false

// 在编译时 按照C而不是C++取寻找头文件
extern "C"
{
#include "extApi.h"
#include "extApiPlatform.h"
#include <memory>
#include <cstring>
}

namespace coppeliasim_cpp
{
	//日志是否输出 输出到终端还是vrep界面
	enum class LogMode
	{
		NO_LOGS = 0,
		LOG_CMD,
		LOG_COPPELIA,
		LOG_COPPELIA_CMD
	};

	struct Position
	{
		float x, y, z;
		// 构造函数 按照传入的xyz值赋值 若无则赋值为0
		Position(float x, float y, float z) : x(x), y(y), z(z) {}
		Position() : x(0), y(0), z(0) {}
	};

	struct Orientation
	{
		float alpha, beta, gamma;
		Orientation(float alpha, float beta, float gamma) : alpha(alpha), beta(beta), gamma(gamma) {}
		Orientation() : alpha(0), beta(0), gamma(0) {}
	};

	struct Pose
	{
		Position position;
		Orientation orientation;
		Pose(Position position, Orientation orientation) : position(position), orientation(orientation) {}
		Pose() : position(), orientation() {}
	};

	class CoppeliaSimClient
	{
	private:
		int clientID;//在initialize()中返回ID
		simxInt connectionPort;
		LogMode logMode;
		std::unique_ptr<simxChar[]> connectionAddress;
	public:
		CoppeliaSimClient(const std::string& connectionAddress = "127.0.0.1", 
			int connectionPort = 19999, LogMode logMode = LogMode::LOG_COPPELIA_CMD);

		bool initialize(float);// 连接server 设置delta t
		bool isConnected() const;
		int getClientID() const;//返回ID

		void startSimulation() const;
		void stopSimulation() const;

		void CoppeliaSimClient::control_loop(bool);//使能闭环控制

		void setIntegerSignal(const std::string& signalName, int signalValue) const;
		void setFloatSignal(const std::string& signalName, const float& signalValue) const;
		void setStringSignal(const std::string& signalName, const std::string& signalValue) const;
		int getIntegerSignal(const std::string& signalName) const;
		float getFloatSignal(const std::string& signalName) const;
		std::string getStringSignal(const std::string& signalName) const;

		int getObjectHandle(const std::string& objectName) const;//读对象句柄
		Pose getObjectPose(int objectHandle) const;
		Position getObjectPosition(int objectHandle) const;
		Orientation getObjectOrientation(int objectHandle) const;

		void setLogMode(LogMode mode);
		void log_msg(const std::string& message) const;
		void print_error(simxInt) const;// 根据错误码打印错误原因

		void setVelocityLimit(int, float);//设置单个关节速度上限

		~CoppeliaSimClient();
	};
}

