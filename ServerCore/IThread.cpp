#include "IThread.h"



IThread::IThread()
{
}


IThread::~IThread()
{
	StopThread();
}

void IThread::StartThread()
{
	isRunning_ = true;

	thread_ = new std::thread( [&] () { Process(); } );
}

void IThread::JoinThread()
{
	if( thread_ != nullptr &&
		thread_->joinable() == true )
	{
		thread_->join();
	}
}

void IThread::StopThread()
{
	isRunning_ = false;

	if( thread_ != nullptr )
	{
		if( thread_->joinable() == true )
		{
			thread_->detach();
		}

		delete thread_;
	}

	thread_ = nullptr;
}
