#pragma once

#include <list>
#include <mutex>
#include <condition_variable>

template< typename t >
class ObjectQueue
{
	std::mutex					queueLock_;
	std::condition_variable		queueCond_;

	std::list<t*>				queue_;
	std::mutex					mutex_;

	bool _Empty()
	{
		std::unique_lock<std::mutex> lock { mutex_ };
		return queue_.empty();
	}

public:
	ObjectQueue() {}
	virtual ~ObjectQueue()
	{
		std::unique_lock<std::mutex> lock { mutex_ };
		
		for( auto itr : queue_ )
		{
			if( itr != nullptr )
			{
				delete itr;
			}
		}
		queue_.clear();
	}

	bool Pop( t*& obj )
	{
		if( _Empty() == true )
		{
			std::unique_lock<std::mutex> lock( queueLock_ );
			queueCond_.wait( lock );
		}

		std::unique_lock<std::mutex> lock { mutex_ };

		if( queue_.empty() == true )
		{
			return false;
		}

		obj = queue_.front();
		queue_.pop_front();

		return true;
	}

	void Push( t* obj )
	{
		if( obj == nullptr )
			return;
		{
			std::unique_lock<std::mutex> lock { mutex_ };
			queue_.push_back( obj );
		}

		std::unique_lock<std::mutex> lock( queueLock_ );
		queueCond_.notify_one();
	}
};