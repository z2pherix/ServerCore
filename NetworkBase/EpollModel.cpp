#include "../ServerCore/Packet.h"

#include "EpollModel.h"
#include "Session.h"

EpollModel::EpollModel()
{
}


EpollModel::~EpollModel()
{
}

bool EpollModel::InitNetworkModel()
{
#ifndef _WIN32
	epollFd_ = epoll_create( EPOLL_MAX_EVENTS );

	if( epollFd_ < 0 ) 
	{
		return false;
	}
#endif

	return true;
}

bool EpollModel::AddSession( Session* newSession )
{
	if( newSession == nullptr )
		return false;

#ifndef _WIN32
	epoll_event eEvent;
	memset( &eEvent, 0x0, sizeof( epoll_event ) );

	eEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;// | EPOLLET;
	eEvent.data.ptr = (void*)newSession;

	epoll_ctl( epollFd_, EPOLL_CTL_ADD, fd, &eEvent );
#endif

	return true;
}

bool EpollModel::RemoveSession( Session* newSession )
{
	if( newSession == nullptr )
		return false;

#ifndef _WIN32
	epoll_event eEvent;
	memset( &eEvent, 0x0, sizeof( epoll_event ) );

	epoll_ctl( epollFd_, EPOLL_CTL_DEL, fd, &eEvent );
#endif

	return true;
}

void EpollModel::SelectSession( std::vector<SessionEvent>& sessionList )
{
#ifndef _WIN32
	memset( epollEvents_, 0x0, sizeof( epoll_event ) * EPOLL_MAX_EVENTS );

	nfds = epoll_wait( epollFd_, epollEvents_, EPOLL_MAX_EVENTS - 1, -1 );

	if( nfds <= 0 ) 
	{
		return;
	}

	for( int i = 0; i < nfds ; ++i ) 
	{
		int event = epollEvents_[i].events;
		Session* session = (Session*)epollEvents_[i].data.ptr;

		if( session == NULL )
		{
			continue;
		}

		if( event & EPOLLIN ) 
		{
			int recvSize = itr->RecvPacket();

			if( recvSize < 0 )
			{
				sessionList.push_back( { SESSION_CLOSE, session, 0 } );
				continue;
			}

			sessionList.push_back( { SESSION_RECV, session, recvSize } );
			
		} 

		if( event & EPOLLERR || event & EPOLLHUP || session->IsClose() )
		{
			sessionList.push_back( { SESSION_CLOSE, session, 0 } );
		}

	}
#endif

}

void EpollModel::StopNetworkModel()
{
}