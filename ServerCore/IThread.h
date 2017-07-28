#pragma once
#include <thread>

class IThread
{
private:

	std::thread*				thread_ = nullptr;
	volatile bool				isRunning_ = false;

public:
	IThread();
	virtual ~IThread();

	bool IsRunning() { return isRunning_; }

	void StartThread();
	void StopThread();
	void JoinThread();

	virtual void Process() = 0;
};

