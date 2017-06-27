#include "Socket.h"


Socket::Socket()
{
}

Socket::Socket( SOCKET socket )
{
	socket_ = socket;
}

Socket::~Socket()
{
}

void Socket::SetSocket( SOCKET socket )
{
	socket_ = socket;
}

SOCKET Socket::CreateSocket()
{
	socket_ = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( socket_ == INVALID_SOCKET )
	{
		lastError_ = GetLastError();
		return SOCKET_ERROR;
	}

	return socket_;
}

void Socket::CloseSocket()
{
	if( socket_ == INVALID_SOCKET )
		return;

#ifndef WIN32
	close( socket_ );
#else
	closesocket( socket_ );
#endif

	socket_ = INVALID_SOCKET;
}

bool Socket::SetBlocking( bool block )
{
	int ret = 0;
	unsigned long ul = 0;

	if( block == true ) 
	{
		ul = 0;
	}
	else
	{
		ul = 1;
	}

#ifndef _WIN32
	ret = ioctl( socket_, FIONBIO, &ul );
#else
	ret = ioctlsocket( socket_, FIONBIO, (u_long*)&ul );
#endif

	if( ret == SOCKET_ERROR ) 
	{
		return false;
	}

	return true;
}

bool Socket::Connect( const char* serveraddress, int port )
{
	SOCKADDR_IN sockaddr;

	hostent* pHostEntry;
	in_addr inAddr;

	pHostEntry = gethostbyname( serveraddress );

	if( pHostEntry == nullptr )
		return false;

	memcpy(&inAddr, pHostEntry->h_addr, 4);


	//! address Set
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons( port );
	sockaddr.sin_addr.s_addr = inet_addr( inet_ntoa(inAddr) );

	//! Connect
	if( ::connect( socket_, (struct sockaddr *)&sockaddr, sizeof( sockaddr ) ) == SOCKET_ERROR  )
	{
		return false;
	}

	SetBlocking( false );

	return true;
}
