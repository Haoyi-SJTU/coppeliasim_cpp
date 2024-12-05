#include "client.h"

namespace coppeliasim_cpp
{

	CoppeliaSimClient::CoppeliaSimClient(const std::string &address, const int port, LogMode mode)
		: clientID(-1), connectionPort(port), logMode(mode),
		  connectionAddress(std::make_unique<simxChar[]>(address.length() + 1))
	{
		// Use std::ranges::copy to copy the string to the allocated array
		std::ranges::copy(address, connectionAddress.get());
		connectionAddress[address.length()] = '\0'; // Null-terminate the string
	}

	// 连接server 设置delta t 开启同步模式
	bool CoppeliaSimClient::initialize(float tstep)
	{
		clientID = simxStart(connectionAddress.get(), connectionPort, true, true, 2000, 5);
		if (clientID == -1)
		{
			std::cout << "Failed to connect to CoppeliaSim.\n";
			return false;
		}
		log_msg("Connected to CoppeliaSim successfully!");
		// 设置delta t  simConst.h说这个参数废弃了 但给的新参数查无此名
		simxSetFloatingParameter(clientID,simxInt floatparam_simulation_time_step,tstep,simx_opmode_oneshot);
		// 同步模式
		simxSynchronous(clientID,true);
		return true;
	}

	bool CoppeliaSimClient::isConnected() const
	{
		const int connectionState = simxGetConnectionId(clientID);
		if (connectionState == -1)
		{
			std::cout << "Connection to CoppeliaSim lost.\n";
			return false;
		}
		return true;
	}

	int CoppeliaSimClient::getClientID() const
	{
		return clientID;
	}

	void CoppeliaSimClient::startSimulation() const
	{
		simxInt errorCode = simxStartSimulation(clientID, simx_opmode_blocking);
		print_error(errorCode);
	}

	void CoppeliaSimClient::stopSimulation() const
	{
		simxStopSimulation(clientID, simx_opmode_blocking);
		log_msg("Simulation stopped.");
	}

	void CoppeliaSimClient::setLogMode(LogMode mode)
	{
		logMode = mode;
	}

	void CoppeliaSimClient::setIntegerSignal(const std::string &signalName, int signalValue) const
	{
		simxSetIntegerSignal(clientID, signalName.c_str(), signalValue, simx_opmode_oneshot);
		log_msg("Signal: " + signalName + " set to: " + std::to_string(signalValue));
	}

	void CoppeliaSimClient::setStringSignal(const std::string &signalName, const std::string &signalValue) const
	{
		const auto signalData = reinterpret_cast<const simxUChar *>(signalValue.c_str());
		const auto signalLength = static_cast<simxInt>(strlen(signalValue.c_str()));
		simxSetStringSignal(clientID, signalName.c_str(), signalData, signalLength, simx_opmode_blocking);
		log_msg("Signal: " + signalName + " set to: " + signalValue);
	}

	void CoppeliaSimClient::setFloatSignal(const std::string &signalName, const float &signalValue) const
	{
		simxSetFloatSignal(clientID, signalName.c_str(), signalValue, simx_opmode_oneshot);
		log_msg("Signal: " + signalName + " set to: " + std::to_string(signalValue));
	}

	int CoppeliaSimClient::getIntegerSignal(const std::string &signalName) const
	{
		simxInt signalValue;
		simxGetIntegerSignal(clientID, signalName.c_str(), &signalValue, simx_opmode_blocking);
		log_msg("Signal: " + signalName + " read as: " + std::to_string(signalValue));
		return signalValue;
	}

	std::string CoppeliaSimClient::getStringSignal(const std::string &signalName) const
	{
		simxInt signalLength;
		simxUChar *signalValue;
		simxGetStringSignal(clientID, signalName.c_str(), &signalValue, &signalLength, simx_opmode_blocking);

		std::string internalSignalValue = std::string(reinterpret_cast<char *>(signalValue), signalLength);
		log_msg("Signal: " + signalName + " read as: " + internalSignalValue);
		return internalSignalValue;
	}

	//读对象句柄
	int CoppeliaSimClient::getObjectHandle(const std::string &objectName) const
	{
		simxInt objectHandle;
		simxInt errorCode = simxGetObjectHandle(clientID, objectName.c_str(), &objectHandle, simx_opmode_blocking);
		print_error(errorCode);
		return objectHandle;
	}

	Pose CoppeliaSimClient::getObjectPose(int objectHandle) const
	{
		const Position position = getObjectPosition(objectHandle);
		const Orientation orientation = getObjectOrientation(objectHandle);
		return {position, orientation};
	}

	Position CoppeliaSimClient::getObjectPosition(int objectHandle) const
	{
		simxFloat position[3];
		simxGetObjectPosition(clientID, objectHandle, -1, position, simx_opmode_blocking);
		return {position[0], position[1], position[2]};
	}

	Orientation CoppeliaSimClient::getObjectOrientation(int objectHandle) const
	{
		simxFloat orientation[3];
		simxGetObjectOrientation(clientID, objectHandle, -1, orientation, simx_opmode_blocking);
		return {orientation[0], orientation[1], orientation[2]};
	}

	float CoppeliaSimClient::getFloatSignal(const std::string &signalName) const
	{
		simxFloat signalValue;
		simxGetFloatSignal(clientID, signalName.c_str(), &signalValue, simx_opmode_blocking);
		log_msg("Signal: " + signalName + " read as: " + std::to_string(signalValue));
		return signalValue;
	}

	void CoppeliaSimClient::log_msg(const std::string &message) const
	{
		switch (logMode)
		{
		case LogMode::LOG_CMD:
			std::cout << message << std::endl;
			break;
		case LogMode::LOG_COPPELIA:
			if (clientID != -1) // Ensure we're connected before trying to log to CoppeliaSim
				simxAddStatusbarMessage(clientID, message.c_str(), simx_opmode_oneshot);
			break;
		case LogMode::LOG_COPPELIA_CMD:
			std::cout << message << std::endl;
			if (clientID != -1) // Ensure we're connected before trying to log to CoppeliaSim
				simxAddStatusbarMessage(clientID, message.c_str(), simx_opmode_oneshot);
			break;
		case LogMode::NO_LOGS:
		default:
			// Do nothing
			break;
		}
	}

	// 根据错误码打印错误原因
	void CoppeliaSimClient::print_error(simxInt errorCode) const
	{
		switch (errorCode)
		{
		case simx_return_ok:
			// 正常
			break;
		case simx_return_novalue_flag:
			// input buffer doesn't contain the specified command.
			//  可能尚未使能data streaming,或数据流没有开始
			log_msg("error(1): no value");
			break;
		case simx_return_timeout_flag:
			// simx_opmode_blocking模式函数超时
			log_msg("error(2): timeout");
			break;
		case simx_return_illegal_opmode_flag:
			// 不支持该操作模式
			log_msg("error(4): unsupported mode");
			break;
		case simx_return_remote_error_flag:
			// command caused an error on the server side
			log_msg("error(8): server side");
			break;
		case simx_return_split_progress_flag:
			// 前一个相似的命令还没有结束 simx_opmode_oneshot_split模式
			log_msg("error(16): previous command unfinished");
			break;
		case simx_return_local_error_flag:
			// 客户端错误
			log_msg("error(32): client side");
			break;
		case simx_return_initialize_error_flag:
			// 尚未初始化
			log_msg("error(64): simxStart was not yet called");
			break;
		default:
			// 未知错误
			log_msg("error(" + std::to_string(errorCode) + "): unknown error");
		}
	}

	// //设置单个关节速度上限
	void CoppeliaSimClient::setVelocityLimit(int joint_handle, float vel_limit)
	{
		simxInt errorCode = simxSetObjectFloatParameter(clientID, joint_handle, sim_jointfloatparam_upper_limit, vel_limit, simx_opmode_oneshot);
		print_error(errorCode);
	}

	void CoppeliaSimClient::control_loop(bool)
	{
		// simxInt errorCode = ;
		// 
	}

	CoppeliaSimClient::~CoppeliaSimClient()
	{
		simxFinish(clientID);
	}

}
