#pragma once
#include "NetworkModel.h"

#ifndef _WIN32
#include <sys/epoll.h>
#include <deque>
#endif

class EpollModel :	public NetworkModel
{

#ifndef _WIN32
	int						epollFd_; 
	epoll_event				epollEvents_[ EPOLL_MAX_EVENTS ];
#endif

public:
	EpollModel();
	virtual ~EpollModel();

	virtual bool InitNetworkModel();

	virtual bool AddSession( Session* newSession );
	virtual bool RemoveSession( Session* newSession );

	virtual void SelectSession();
};

