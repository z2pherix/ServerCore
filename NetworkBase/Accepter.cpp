#include "Accepter.h"
#include "SessionManager.h"
#include "NetworkCore.h"

Accepter::Accepter()
{
	acceptList_.clear();
	acceptEventCount_ = 0;
}


Accepter::~Accepter()
{
	StopThread();
}

void Accepter::StartThread()
{
	isRunning_ = true;

	accpetThread_ = std::make_shared<std::thread>( [&] () { Process(); } );
}

void Accepter::StopThread()
{
	isRunning_ = false;
}

bool Accepter::_RegistServerSocket( std::shared_ptr<ServerSocket> socket )
{
#ifndef WIN32
	epoll_event eEvent;
	memset( &eEvent, 0x0, sizeof( epoll_event ) );

	eEvent.events = EPOLLIN;
	eEvent.data.ptr = (void*)socket;

	epoll_ctl( epollFD_, EPOLL_CTL_ADD, socket->GetSocket(), &eEvent );
#else
	WSAEVENT event = WSACreateEvent();

	if( event == WSA_INVALID_EVENT )
		return false;

	if( WSAEventSelect( socket->GetSocket(), event, FD_ACCEPT ) != 0 )
		return false;

	acceptEventSocket_[acceptEventCount_] = socket;
	acceptEvent_[acceptEventCount_] = event;
	++acceptEventCount_;
	
#endif

	return true;
}

bool Accepter::_SelectServerSocket()
{
	acceptList_.clear();

#ifndef WIN32
	memset( epollEvents_, 0x0, sizeof( epoll_event ) * EPOLL_MAX_EVENTS );

	int nfds = epoll_wait( epollFD_, epollEvents_, EPOLL_MAX_EVENTS - 1, -1 );

	if( nfds <= 0 ) 
	{
		return false;
	}


	for( int i = 0; i < nfds ; ++i ) 
	{
		int event = epollEvents_[ i ].events;
		int event_fd = epollEvents_[ i ].data.fd;

		if( event & EPOLLIN ) 
		{
			acceptList_.push_back( static_cast<std::shared_ptr<ServerSocket>>(epollEvents_[ i ].data.ptr) );
		} 
	}
#else
	DWORD index = WSAWaitForMultipleEvents( acceptEventCount_, acceptEvent_, FALSE, WSA_INFINITE, FALSE );
	index = index - WSA_WAIT_EVENT_0;

	WSAResetEvent( acceptEvent_[index] );

	WSANETWORKEVENTS networkEvents;
	if( WSAEnumNetworkEvents( acceptEventSocket_[index]->GetSocket(), acceptEvent_[index], &networkEvents ) == SOCKET_ERROR )
	{
		return false;
	}

	if( networkEvents.lNetworkEvents == FD_ACCEPT )
	{
		acceptList_.push_back( acceptEventSocket_[index] );
	}
	else
	{
		return false;
	}
	
#endif

	return true;
}

bool Accepter::InitAccepter()
{
#ifndef WIN32
	epollFD_ = epoll_create( EPOLL_MAX_EVENTS );

	if( epollFD_ < 0 ) 
	{
		return false;
	}
#else
#endif

	return true;
}

bool Accepter::AddAcceptPort( int port )
{
	auto serverSocket = std::make_shared<ServerSocket>();

	if( serverSocket->CreateSocket() == SOCKET_ERROR )
		return false;

	serverSocket->InitServerSocket( port );

	if( serverSocket->Bind()  != 0 )
		return false;

	if( _RegistServerSocket( serverSocket ) == false )
		return false;
	
	if( serverSocket->Listen() != 0 )
		return false;

	isAccepter_ = true;

	return true;
}

void Accepter::Process()
{
	while( IsRunning() == true )
	{
		if( isAccepter_ == false )
			continue;

		if( _SelectServerSocket() == false )
			return;

		for( auto listenSock  : acceptList_ )
		{
			Socket newSocket = listenSock->Accept();

			if( newSocket.GetSocket() == INVALID_SOCKET )
				continue;

			Session* newSession = NetworkCore::GetInstance()->CreateSession( newSocket );

			if( newSession == nullptr )
				continue;

			NetworkCore::GetInstance()->AddSession( newSession, listenSock->GetPort() );
		}
	}
}