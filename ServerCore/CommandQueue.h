#pragma once

#include <list>
#include <mutex>
#include <condition_variable>

#include "Command.h"

class CommandQueue
{
	std::mutex					queueLock_;
	std::condition_variable		queueCond_;

	std::list<Command>			commandQueue_;
	std::mutex					commandMutex_;

	bool _Empty();

public:
	CommandQueue();
	virtual ~CommandQueue();

	bool Pop( Command& obj );
	void Push( Command& obj );
};


