#pragma once

#include <memory>
#include <list>
#include <thread>

#include "ServerSocket.h"


class Accepter
{
	bool			isAccepter_ = false;
	volatile bool	isRunning_ = false;
#ifndef WIN32
	int				epollFD_;
	epoll_event		epollEvents_[ EPOLL_MAX_EVENTS ];
#else
	int				acceptEventCount_;
	WSAEVENT		acceptEvent_[WSA_MAXIMUM_WAIT_EVENTS];

	std::shared_ptr<ServerSocket> acceptEventSocket_[WSA_MAXIMUM_WAIT_EVENTS];
#endif

	std::list<std::shared_ptr<ServerSocket>>	acceptList_;
	std::shared_ptr<std::thread>				accpetThread_;

	bool _RegistServerSocket( std::shared_ptr<ServerSocket> socket );
	bool _SelectServerSocket();

public:
	Accepter();
	virtual ~Accepter();

	virtual void Process();

	bool InitAccepter();
	bool AddAcceptPort( int port );

	virtual bool IsRunning() { return isRunning_; }

	virtual void StartThread();
	virtual void StopThread();
};

