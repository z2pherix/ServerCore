#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>

class Socket
{
protected:

	SOCKET	socket_ = INVALID_SOCKET;
	int		lastError_ = 0;
	bool	isConnected_ = false;

public:

	Socket();
	Socket( SOCKET socket );
	virtual ~Socket();

	void	SetSocket( SOCKET socket );
	SOCKET	CreateSocket();
	void	CloseSocket();
	
	SOCKET GetSocket() { return socket_; }

	bool SetBlocking( bool block );
	bool IsConnected() const { return isConnected_; }

	virtual bool Connect( const char* serveraddress, int port );
};

