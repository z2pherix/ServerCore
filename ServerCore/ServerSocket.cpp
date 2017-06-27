#include "ServerSocket.h"



ServerSocket::ServerSocket()
{
	listenPort_ = 0;
}


ServerSocket::~ServerSocket()
{
}

SOCKET ServerSocket::InitServerSocket( int listenPort )
{
	if( CreateSocket() == INVALID_SOCKET )
		return INVALID_SOCKET;

	listenPort_ = listenPort;
	return GetSocket();
}

int ServerSocket::Bind()
{
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = htonl( INADDR_ANY );
	service.sin_port = htons( listenPort_ );

	if( ::bind( socket_, (SOCKADDR *)&service, sizeof(service) ) == SOCKET_ERROR )
	{
		CloseSocket();
		return SOCKET_ERROR;
	}

	return 0;
}

int ServerSocket::Listen()
{
	struct linger ling;
	ling.l_onoff = 1;
	ling.l_linger = 0;
	setsockopt( socket_, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof( struct linger ));

	SetBlocking( false );

	if( ::listen( socket_, SOMAXCONN ) == SOCKET_ERROR )
	{
		lastError_ = WSAGetLastError();
		return SOCKET_ERROR;
	}

	return 0;
}

SOCKET ServerSocket::Accept()
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons( listenPort_ );
	addr.sin_addr.s_addr = INADDR_ANY;

	socklen_t len = sizeof(struct sockaddr_in);

	return ::accept( socket_, (struct sockaddr *) &addr, &len );
}