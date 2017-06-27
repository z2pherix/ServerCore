#include "CommandQueue.h"



CommandQueue::CommandQueue()
{
}


CommandQueue::~CommandQueue()
{
	std::unique_lock<std::mutex> lock { commandMutex_ };

	for( auto itr : commandQueue_ )
	{
		if( itr.cmdMessage_ != nullptr )
		{
			delete itr.cmdMessage_;
		}
	}

	commandQueue_.clear();
}

bool CommandQueue::_Empty()
{
	std::unique_lock<std::mutex> lock { commandMutex_ };
	return commandQueue_.empty();
}

bool CommandQueue::Pop( Command& cmd )
{
	if( _Empty() == true )
	{
		std::unique_lock<std::mutex> lock( queueLock_ );
		queueCond_.wait( lock );
	}
	
	std::unique_lock<std::mutex> lock { commandMutex_ };

	if( commandQueue_.empty() == true )
	{
		return false;
	}

	cmd = commandQueue_.front();
	commandQueue_.pop_front();

	return true;
}

void CommandQueue::Push( Command& cmd )
{
	if( cmd.cmdMessage_ == nullptr )
		return;
	{
		std::unique_lock<std::mutex> lock { commandMutex_ };
		commandQueue_.push_back( cmd );
	}

	std::unique_lock<std::mutex> lock( queueLock_ );
	queueCond_.notify_one();
}
